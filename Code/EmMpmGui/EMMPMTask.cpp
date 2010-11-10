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

#include <iostream>
#include <fstream>

#include <QtCore/QDir>
#include <QtCore/QLocale>

#include "emmpm/public/EMMPM.h"
#include "emmpm/common/utilities/random.h"
#include "emmpm/common/utilities/allocate.h"
#include "emmpm/common/io/EMTiffIO.h"

/**
 * @brief
 * @param inputs
 * @param vars
 */
void InitNClassInitialization(EMMPM_Inputs* inputs, EMMPM_WorkingVars* vars)
{
  char startMsg[] = "InitNClassInitialization Starting";
  EMMPM_ShowProgress(startMsg, 1.0f);

  unsigned int i, j;
  int c, l;
  double mu, sigma;
  unsigned int rows = inputs->rows;
  unsigned int cols = inputs->columns;

  unsigned char** y = vars->y;
  unsigned char** xt = vars->xt;
  rows = inputs->rows;
  cols = inputs->columns;

  sigma = 0;
  mu = 0;

  char msgbuff[256];
  memset(msgbuff, 0, 256);

  for(c = 0; c < inputs->classes; c++)
  {
 //   printf("####################################################################\n");
    int x1 = inputs->initCoords[c][0];
    int y1 = inputs->initCoords[c][1];
    int x2 = inputs->initCoords[c][2];
    int y2 = inputs->initCoords[c][3];
    mu = 0;
    snprintf(msgbuff, 256, "m[%d] Coords: %d %d %d %d", c, x1, y1, x2, y2);
    EMMPM_ShowProgress(msgbuff, 1.0);
    for (i=inputs->initCoords[c][1]; i<inputs->initCoords[c][3]; i++) {
      for (j=inputs->initCoords[c][0]; j<inputs->initCoords[c][2]; j++) {
        mu += y[i][j];
   //     printf ("%03d ", y[i][j]);
      }
   //   printf("\n");
    }

    mu /= (y2 - y1)*(x2 - x1);
    vars->m[c] = mu;
    snprintf(msgbuff, 256, "m[%d]=%f", c, mu);
    EMMPM_ShowProgress(msgbuff, 1.0);
  }

  for (l = 0; l < MAX_CLASSES; l++) {
    if (l < inputs->classes) {
      vars->v[l] = 20;
      vars->probs[l] = (double **)get_img(inputs->columns, inputs->rows, sizeof(double));
    }
    else
    {
      vars->v[l] = -1;
      vars->probs[l] = NULL;
    }
  }

  /* Initialize classification of each pixel randomly with a uniform disribution */
  for (i = 0; i < inputs->rows; i++) {
    for (j = 0; j < inputs->columns; j++) {
      vars->x = random2();
      l = 0;
      while ((double)(l + 1) / inputs->classes <= vars->x)  // may incur l = classes when x = 1
        l++;
      xt[i][j] = l;
    }
  }
  char endMsg[] = "InitNClassInitialization Complete";
  EMMPM_ShowProgress(endMsg , 4.0f);
}

/**
 * @brief This is a global static variable to hold a pointer to the current task.
 * The pointer is used to get progress messages from the encoder back to the GUI.
 */
void* globalEMMPMTask;
// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EMMPMTask::EMMPMUpdate_CallBackWrapper(EMMPM_Update* update)
//void UpdateStats(EMMPM_Update* update)
{

  // explicitly cast global variable <globalSHPTask> to a pointer to TClassB
  // warning: <globalSHPTask> MUST point to an appropriate object!
  EMMPMTask* mySelf = static_cast<EMMPMTask*>( globalEMMPMTask );
  EMMPM_Files* m_files = mySelf->getEMMPM_Files();
  EMMPM_Inputs* m_inputs = mySelf->getEMMPM_Inputs();
  float m_TotalIterations = m_inputs->emIterations * m_inputs->mpmIterations;
  // call member
  int progress = (float)(update->currentEMLoop*update->currentMPMLoop)/(float)m_TotalIterations * 100.0f;
  emit mySelf->progressValueChanged( progress );
  emit mySelf->progressTextChanged(QString::number(progress));


  std::cout << "EM Loop: " << update->currentEMLoop << std::endl;
  char buff[256];
  memset(buff, 0, 256);
  snprintf(buff, 256, "/tmp/emmpm_out_%d.tif", update->currentEMLoop);
  int err = writeGrayScaleImage(buff, update->height, update->width, "Intermediate Image", update->outputImage);
  if (err < 0)
  {
    std::cout << "Error writing intermediate tiff image." << std::endl;
  }

  std::cout << "Class\tMu\tSigma" << std::endl;
  for (int l = 0; l < update->classes; l++)
  {
//    snprintf(msgbuff, 256, "%d\t%.3f\t%.3f", l, update->m[l], update->v[l]);
//    EMMPM_ShowProgress(msgbuff, vars->progress);
    std::cout << l << "\t" << update->m[l] << "\t" << update->v[l] << "\t" << std::endl;
  }

  float hist[MAX_CLASSES][256];
  // Generate a gaussian curve for each class based off the mu and sigma for that class
  for (int c = 0; c < update->classes; ++c)
  {
    float mu = update->m[c];
    float sig = update->v[c];
    float twoSigSqrd = sig * sig * 2.0f;
    float constant = 1.0 / (sig * sqrtf(2.0f * M_PI));
    for (size_t x = 0; x < 256; ++x)
    {
      hist[c][x] = constant * exp(-1.0f * ((x - mu) * (x - mu)) / (twoSigSqrd));
    }
  }

  memset(buff, 0, 256);
  snprintf(buff, 256, "/tmp/emmpm_hist_%d.csv", update->currentEMLoop);
  std::ofstream file(buff, std::ios::out | std::ios::binary);
  if (file.is_open())
  {
    for (size_t x = 0; x < 256; ++x)
    {
      file << x;
      for (int c = 0; c < update->classes; ++c)
      {
        file << ", " << hist[c][x];
      }
      file << std::endl;
    }
  }


}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
EMMPMTask::EMMPMTask(QObject* parent) :
  ProcessQueueTask(parent)
{
  m_inputs = EMMPM_AllocateInputsStructure();
  m_files = EMMPM_AllocateFilesStructure();
}


EMMPM_Files* EMMPMTask::getEMMPM_Files()
{
  return m_files;
}
EMMPM_Inputs* EMMPMTask::getEMMPM_Inputs()
{
  return m_inputs;
}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
EMMPMTask::~EMMPMTask()
{
  std::cout << "EMMPMTask::~EMMPMTask()" << std::endl;
  EMMPM_FreeTiffImageBuffer(m_files->inputImage); // Release the memory used to read the image
  EMMPM_FreeTiffImageBuffer(m_files->outputImage);
  EMMPM_FreeInputsStructure(m_inputs);
  EMMPM_FreeFilesStructure(m_files);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EMMPMTask::run()
{
  UPDATE_PROGRESS(QString("Starting Segmentation"), 0);

  // Get our input image from the Image IO functions
  int err = EMMPM_ReadInputImage(m_files, m_inputs);
  if (err < 0)
  {
    UPDATE_PROGRESS(QString("EM/MPM Error Reading Input Tif Image"), 100);
    emit taskFinished(this);
    return;
  }

  // Set the initialization function to the Basic
  if (m_inputs->initType == EMMPM_PIXEL_AREA_INITIALIZATION)
  {
    EMMPM_SetInitializationFunction(&InitNClassInitialization);
  }

  // Set these to get our call back working
  globalEMMPMTask = this;
  EMMPM_SetProgressStatsFunction(&EMMPMTask::EMMPMUpdate_CallBackWrapper);

  // Run the EM/MPM algorithm on the input image
  EMMPM_Execute(m_files, m_inputs);

  err = EMMPM_WriteOutputImage(m_files, m_inputs);
  if (err < 0)
  {
    UPDATE_PROGRESS(QString("EM/MPM Error Writing Output Tif Image"), 100);
    emit taskFinished(this);
    return;
  }

  UPDATE_PROGRESS(QString("Ending Segmentation"), 0);
  emit taskFinished(this);
}


