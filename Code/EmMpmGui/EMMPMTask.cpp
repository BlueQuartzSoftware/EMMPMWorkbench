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
#include "emmpm/common/random.h"

#include "emmpm/public/InitializationFunctions.h"
#include "emmpm/public/ProgressFunctions.h"
#include "emmpm/tiff/EMTiffIO.h"
#include "AIM/Common/AIMArray.hpp"

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
    QImage image = QImage(data->columns, data->rows, QImage::Format_Indexed8);
    if (image.isNull() == true)
    {
      return;
    }
    QVector<QRgb > colorTable(256);
    for (quint32 i = 0; i < 256; ++i)
    {
      colorTable[i] = qRgb(i, i, i);
    }
    image.setColorTable(colorTable);
    size_t index = 0;
    for (int y = 0; y < data->rows; ++y)
    {
      for (int x = 0; x < data->columns; ++x)
      {
        image.setPixel(x, y, data->outputImage[index]);
        ++index;
      }
    }
    emit mySelf->updateImageAvailable(image);

    size_t histIdx = 0;
    emit mySelf->histogramsAboutToBeUpdated();

    for (int d = 0; d < data->dims; ++d)
    {
      for (int l = 0; l < data->classes; ++l)
      {
        histIdx = (256*data->classes*d) + (256*l);
        QVector<double> values(256);
        ::memcpy(values.data(), &(data->histograms[histIdx]), 256*sizeof(double));
        emit mySelf->updateHistogramAvailable(values);
      }
    }




  }

#if 0
  // Check to make sure we are at the end of an em loop
  if (  data->inside_mpm_loop == 0 && NULL != data->outputImage)
  {
    // std::cout << "EM Loop: " << data->currentEMLoop << std::endl;
    char buff[256];
    memset(buff, 0, 256);
    snprintf(buff, 256, "/tmp/GUI_emmpm_out_%d.tif", data->currentEMLoop);

    int err = EMMPM_WriteGrayScaleImage(buff, data->rows, data->columns, "Intermediate Image", data->outputImage);
    if (err < 0)
    {
      std::cout << "Error writing intermediate tiff image." << std::endl;
    }

    std::cout << "Class\tMu\tSigma" << std::endl;
    for (int l = 0; l < data->classes; l++)
    {
       std::cout << l << "\t" << data->m[l] << "\t" << data->v[l] << "\t" << std::endl;
    }

    float hist[EMMPM_MAX_CLASSES][256];
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
#endif
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
EMMPMTask::EMMPMTask(QObject* parent) :
  ProcessQueueTask(parent)
{
  m_data = EMMPM_CreateDataStructure();
  m_data->userData = this;
  m_callbacks = EMMPM_AllocateCallbackFunctionStructure();
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
EMMPM_Data* EMMPMTask::getEMMPM_Data()
{
  return m_data;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EMMPMTask::cancel()
{
  if (NULL != m_data)
  {
    m_data->cancel = 1;
  }
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
  QImage image = QImage(m_data->input_file_name);
  qint32 height = image.height();
  qint32 width = image.width();
  AIMArray<unsigned char>::Pointer inImageBufferPtr = AIMArray<unsigned char>::New();
  quint8* inImage = inImageBufferPtr->allocateDataArray(width*height, true);
  if (NULL == inImage)
  {
    return;
  }

  m_data->rows = height;
  m_data->columns = width;
  m_data->dims = 1;
  // Copy the QImage into the AIMImage object, converting to gray scale as we go.
  QRgb rgbPixel;
  int gray;
  qint32 index;
  for (qint32 y = 0; y<height; y++) {
    for (qint32 x = 0; x<width; x++) {
      index = (y *  width) + x;
      rgbPixel = image.pixel(x, y);
      gray = qGray(rgbPixel);
      inImage[index] = static_cast<unsigned char>(gray);
    }
  }
  m_data->inputImage = inImage;

  // Forcing everything to grayscale
  m_data->dims = 1;
  m_data->inputImageChannels = 1;

  // EMMPM_WriteGrayScaleImage("/tmp/TEST_INPUT_IMAGE.tif", m_data->rows, m_data->columns, "Input image as read by QImage", m_data->inputImage);


  // Allocate our own output image buffer;
  AIMArray<unsigned char>::Pointer outImageBufferPtr = AIMArray<unsigned char>::New();
  quint8* outImagePtr = outImageBufferPtr->allocateDataArray(width*height, true);
  if (NULL == outImagePtr)
  {
    return;
  }
  m_data->outputImage = outImagePtr;
  ::memset(m_data->outputImage, 128, width*height);


#if 0
  int err = EMMPM_ReadInputImage(m_data, m_callbacks);
  if (err < 0)
  {
    UPDATE_PROGRESS(QString("EM/MPM Error Reading Input Image"), 100); emit
    taskFinished(this);
    return;
  }
#endif

  // Set the initialization function based on the command line arguments
  switch(m_data->initType)
  {
    case EMMPM_UserInitArea:
      m_callbacks->EMMPM_InitializationFunc = EMMPM_UserDefinedAreasInitialization;
      break;
    case EMMPM_Basic:
      m_callbacks->EMMPM_InitializationFunc = EMMPM_BasicInitialization;
      break;
    case EMMPM_CurvaturePenalty:
      m_callbacks->EMMPM_InitializationFunc = EMMPM_CurvatureInitialization;
      break;
    case EMMPM_Manual:
      m_callbacks->EMMPM_InitializationFunc = EMMPM_ManualInitialization;
      break;
    default:
      break;
  }

  // Set the Update Stats Callback function
  m_callbacks->EMMPM_ProgressStatsFunc = EMMPMTask::EMMPMUpdate_CallBackWrapper;

  // Allocate all the memory here
  int err = EMMPM_AllocateDataStructureMemory(m_data);
  if (err)
  {
    UPDATE_PROGRESS(QString("Error allocating memory for the EMMPM Data Structure"), 0);
    emit taskFinished(this);
    return;
  }

  // Run the EM/MPM algorithm on the input image
  EMMPM_Run(m_data, m_callbacks);

  // Set the inputimage pointer to NULL so it does not get freed twice
  m_data->inputImage = NULL;

  QImage outQImage(m_data->outputImage, width, height, width, QImage::Format_Indexed8);
  m_data->outputImage = NULL; // set this to NULL so it does not get freed twice.
  QVector<QRgb> colorTable(256);
  for (quint32 i = 0; i < 256; ++i)
  {
    colorTable[i] = qRgb(i, i, i);
  }
  outQImage.setColorTable(colorTable);
  bool success = outQImage.save(m_data->output_file_name);
  if (false == success)
  {
    UPDATE_PROGRESS(QString("EM/MPM Error Writing Output Image"), 100); emit
    emit taskFinished(this);
    return;
  }

#if 0
  err = EMMPM_WriteOutputImage(m_data, m_callbacks);
  if (err < 0)
  {
    UPDATE_PROGRESS(QString("EM/MPM Error Writing Output Image"), 100); emit
    emit taskFinished(this);
    return;
  }
#endif

  UPDATE_PROGRESS(QString("Ending Segmentation"), 0);
  emit taskFinished(this);
}

