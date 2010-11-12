/* ============================================================================
 * Copyright (c) 2010, Michael A. Jackson (BlueQuartz Software)
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice, this
 * list of conditions and the following disclaimer in the documentation and/or
 * other materials provided with the distribution.
 *
 * Neither the name of Michael A. Jackson nor the names of its contributors may
 * be used to endorse or promote products derived from this software without
 * specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 * USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

#include "EMMPMTask.h"

//-- C Inclues
#include <math.h>

//-- C++ includes
#include <iostream>
#include <fstream>

//-- Qt Includes
#include <QtCore/QDir>
#include <QtCore/QLocale>
#include <QtCore/QMutex>
#include <QtCore/QMutexLocker>

//-- EMMPMLib Includes
#include "emmpm/public/EMMPM.h"
#include "emmpm/common/utilities/random.h"
#include "emmpm/common/utilities/allocate.h"
#include "emmpm/common/utilities/InitializationFunctions.h"
#include "emmpm/common/utilities/ProgressFunctions.h"
#include "emmpm/common/io/EMTiffIO.h"

QMutex EMMPMTask_CallBackWrapperMutex;

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EMMPMTask::EMMPMUpdate_CallBackWrapper(EMMPM_Data* data)
//void UpdateStats(EMMPM_Update* update)
{
  /* this is here because this static function could be called from mutliple
   * threads. We use a mutex to ensure that only a single function is able
   * to execute this code at a time.
   */
  QMutexLocker locker(&EMMPMTask_CallBackWrapperMutex);

  // explicitly cast global variable <globalSHPTask> to a pointer to TClassB
  // warning: <globalSHPTask> MUST point to an appropriate object!
  EMMPMTask* mySelf = static_cast<EMMPMTask* > (data->userData);

  emit
  mySelf->progressValueChanged(data->progress);
  emit
  mySelf->progressTextChanged(QString::number(data->progress));

  // Check to make sure we are at the end of an em loop
  if (  data->inside_mpm_loop == 0 && NULL != data->outputImage)
  {
    // std::cout << "EM Loop: " << data->currentEMLoop << std::endl;
    char buff[256];
    memset(buff, 0, 256);
    snprintf(buff, 256, "/tmp/GUI_emmpm_out_%d.tif", data->currentEMLoop);

    int err = writeGrayScaleImage(buff, data->rows, data->columns, "Intermediate Image", data->outputImage);
    if (err < 0)
    {
      std::cout << "Error writing intermediate tiff image." << std::endl;
    }

    std::cout << "Class\tMu\tSigma" << std::endl;
    for (int l = 0; l < data->classes; l++)
    {
       std::cout << l << "\t" << data->m[l] << "\t" << data->v[l] << "\t" << std::endl;
    }

    float hist[MAX_CLASSES][256];
    // Generate a gaussian curve for each class based off the mu and sigma for that class
    for (int c = 0; c < data->classes; ++c)
    {
      float mu = data->m[c];
      float sig = data->v[c];
      float twoSigSqrd = sig * sig * 2.0f;
      float constant = 1.0 / (sig * sqrtf(2.0f * PI));
      for (size_t x = 0; x < 256; ++x)
      {
        hist[c][x] = constant * exp(-1.0f * ((x - mu) * (x - mu)) / (twoSigSqrd));
      }
    }

    memset(buff, 0, 256);
    snprintf(buff, 256, "/tmp/GUI_emmpm_hist_%d.csv", data->currentEMLoop);
    std::ofstream file(buff, std::ios::out | std::ios::binary);
    if (file.is_open())
    {
      for (size_t x = 0; x < 256; ++x)
      {
        file << x;
        for (int c = 0; c < data->classes; ++c)
        {
          file << ", " << hist[c][x];
        }
        file << std::endl;
      }
    }
  }

}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
EMMPMTask::EMMPMTask(QObject* parent) :
  ProcessQueueTask(parent)
{
  m_data = EMMPM_AllocateDataStructure();
  m_data->userData = this;
  m_callbacks = EMMPM_AllocateCallbackFunctionStructure();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
EMMPM_Data* EMMPMTask::getEMMPM_Data()
{
  return m_data;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
EMMPMTask::~EMMPMTask()
{
  //  std::cout << "EMMPMTask::~EMMPMTask()" << std::endl;

  EMMPM_FreeDataStructure(m_data);
  EMMPM_FreeCallbackFunctionStructure(m_callbacks);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EMMPMTask::run()
{
  UPDATE_PROGRESS(QString("Starting Segmentation"), 0);

  this->setInputFilePath(QString(m_data->input_file_name));
  this->setOutputFilePath(QString(m_data->output_file_name));
  // Get our input image from the Image IO functions
  int err = EMMPM_ReadInputImage(m_data, m_callbacks);
  if (err < 0)
  {
    UPDATE_PROGRESS(QString("EM/MPM Error Reading Input Image"), 100); emit
    taskFinished(this);
    return;
  }

  // Set the initialization function based on the command line arguments
  switch(m_data->initType)
  {
    case EMMPM_USER_DEFINED_AREA_INITIALIZATION:
      m_callbacks->EMMPM_InitializationFunc = EMMPM_UserDefinedAreasInitialization;
      break;
    case EMMPM_BASIC_INITIALIZATION:
      m_callbacks->EMMPM_InitializationFunc = EMMPM_BasicInitialization;
      break;
    default:
      break;
  }

  // Set the Update Stats Callback function
  m_callbacks->EMMPM_ProgressStatsFunc = EMMPMTask::EMMPMUpdate_CallBackWrapper;
 // m_callbacks->EMMPM_ProgressFunc = &EMMPM_PrintfProgress;

  // Run the EM/MPM algorithm on the input image
  EMMPM_Execute(m_data, m_callbacks);

  err = EMMPM_WriteOutputImage(m_data, m_callbacks);
  if (err < 0)
  {
    UPDATE_PROGRESS(QString("EM/MPM Error Writing Output Image"), 100); emit
    taskFinished(this);
    return;
  }

  UPDATE_PROGRESS(QString("Ending Segmentation"), 0);
  emit taskFinished(this);
}

