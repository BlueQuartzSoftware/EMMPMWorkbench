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
#include <QtGui/QMessageBox>

//-- MXA Includes
#include "MXA/Utilities/MXAFileInfo.h"

//-- EMMPMLib Includes
#include "EMMPMLib/Core/EMMPM.h"
#include "EMMPMLib/Core/InitializationFunctions.h"
#include "EMMPMLib/tiff/TiffUtilities.h"

#include "AIM/Common/AIMArray.hpp"

QMutex EMMPMTask_CallBackWrapperMutex;

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EMMPMTask::reportProgress(EMMPM_Data::Pointer data)
{

  /* this is here because this static function could be called from mutliple
   * threads. We use a mutex to ensure that only a single function is able
   * to execute this code at a time.
   */
  QMutexLocker locker(&EMMPMTask_CallBackWrapperMutex);

  // explicitly cast global variable <EMMPMTask> to a pointer to EMMPMTask
  // warning: <EMMPMTask> MUST point to an appropriate object!
  EMMPMTask* mySelf = static_cast<EMMPMTask* > (data->userData);

//  float total = data->emIterations * data->mpmIterations;
//  float current = data->currentEMLoop*data->mpmIterations + data->currentMPMLoop;
//  data->progress = 100.0 * (current/total);

 // std::cout << "EMMPMUpdate_CallBackWrapper: Progress=" << data->progress << std::endl;
  emit mySelf->updateProgress((int)data->progress);

  QString msg("EM Loop: ");
  msg.append(QString::number(data->currentEMLoop)).append(" - MPM Loop: ").append(QString::number(data->currentMPMLoop));

  emit mySelf->progressMessage(msg);

  // Check to make sure we are at the end of an em loop
  if ( /* data->inside_mpm_loop == 0 && */ NULL != data->outputImage)
  {
    QImage image = QImage(data->columns, data->rows, QImage::Format_Indexed8);
    if (image.isNull() == true)
    {
      return;
    }

    image.fill(0);
    uchar* front = NULL;
    for (unsigned int y = 0; y < data->rows; ++y)
    {
      front = image.scanLine(y);
     // int bytesPerLine = image.bytesPerLine();
      for (unsigned int x = 0; x < data->columns; ++x)
      {
        front[x] = data->xt[data->columns*y + x];
      }
    }
    image.setColorCount(data->classes);

    // Now overwrite the beginning of the color table with the gray scale values for each class
    for (int g = 0; g < data->classes; ++g)
    {
      image.setColor(g, qRgb(data->grayTable[g], data->grayTable[g], data->grayTable[g]) );
    }
    emit mySelf->updateImageAvailable(image);

    size_t histIdx = 0;
    emit mySelf->histogramsAboutToBeUpdated();

    for (unsigned int d = 0; d < data->dims; ++d)
    {
      for (int l = 0; l < data->classes; ++l)
      {
        histIdx = (256*data->classes*d) + (256*l);
        QVector<real_t> values(256);
        ::memcpy(values.data(), &(data->histograms[histIdx]), 256*sizeof(real_t));
        emit mySelf->updateHistogramAvailable(values);
      }
    }
  }

  // Send the latest value of the MSE to the GUI for plotting or saving.
  if (data->inside_mpm_loop == 0 && data->currentEMLoop > 0)
  {
      emit mySelf->mseValueUpdated(data->currentMSE);
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
  m_data = EMMPM_Data::New();
  m_data->userData = this;

}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
EMMPMTask::~EMMPMTask()
{
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
EMMPM_Data::Pointer EMMPMTask::getEMMPM_Data()
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


  // Clear out the stats file
  if (m_OutputStatsFile.empty() == false)
  {
    FILE* f = fopen(m_OutputStatsFile.c_str(), "wb");
    fclose(f);
  }


  // Run the first image using all the settings from the user interface
  segmentImage(0);
  if (m_data->cancel == 1)
  {
    UPDATE_PROGRESS(QString("Ending Segmentation"), 100);
    emit finished(this);
    return;
  }

  // Now run the next set of images possibly using the output from the previous
  // image's final mu and sigma values as the initial conditions into the next
  // image in the sequence
  InputOutputFilePairList fileList = getInputOutputFilePairList();
  int count = fileList.count();

  for(int i = 1; i < count; ++i)
  {
    if (true == getFeedBackInitialization() )
    {
      m_data->initType = EMMPM_ManualInit;
    }

    segmentImage(i);
    if (m_data->cancel == 1)
    {
      break;
    }
  }


  // Forcibly release our reference to the EMMPM_Data object
  m_data = EMMPM_Data::NullPointer();

  UPDATE_PROGRESS(QString("Ending Segmentation"), 100);

  emit finished(this);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EMMPMTask::segmentImage(int i)
{
  InputOutputFilePairList fileList = getInputOutputFilePairList();

  QString inputFile = fileList.at(i).first;
  QString outputFile = fileList.at(i).second;
  m_data->input_file_name = copyStringToNewBuffer(inputFile);
  m_data->output_file_name = copyStringToNewBuffer(outputFile);

 // std::cout << "Segmenting Image: " << inputFile.toStdString() << std::endl;
  QFileInfo fileInfo(inputFile);
  emit imageStarted(inputFile);

  // Get our input image from the Image IO functions
  QImage image = QImage(m_data->input_file_name);
  if (image.isNull() == true)
  {
    UPDATE_PROGRESS(QString("There was an issue loading the image. Either the input image does not exist or low memory prevented the loading of the image"), 100);

    emit finished(this);
    return;
  }
  qint32 height = image.height();
  qint32 width = image.width();
  AIMArray<unsigned char>::Pointer inImageBufferPtr = AIMArray<unsigned char>::New();
  quint8* inImage = inImageBufferPtr->allocateDataArray(width*height, true);
  if (NULL == inImage)
  {
    emit finished(this);
    return;
  }

  m_data->rows = height;
  m_data->columns = width;
  m_data->dims = 1;
  // Copy the QImage into the AIMImage object, converting to gray scale as we go.
  QRgb rgbPixel;
  int gray;
  qint32 index;
  for (qint32 y = 0; y < height; y++)
  {
    for (qint32 x = 0; x < width; x++)
    {
      index = (y * width) + x;
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

    emit finished(this);
    return;
  }
  m_data->outputImage = outImagePtr;
  ::memset(m_data->outputImage, 128, width*height);


#if 0
  int err = EMMPM_ReadInputImage(m_data, m_callbacks);
  if (err < 0)
  {
    UPDATE_PROGRESS(QString("EM/MPM Error Reading Input Image"), 100); emit

    emit finished(this);
    return;
  }
#endif

  // This takes care of the manual initialization case
  InitializationFunction::Pointer initFunction = InitializationFunction::New();


  // Set the initialization function based on the command line arguments
  switch(m_data->initType)
  {
    case EMMPM_Basic:
      initFunction = BasicInitialization::New();
      break;
    case EMMPM_UserInitArea:
      initFunction = UserDefinedAreasInitialization::New();
      break;
    default:
      break;
  }

  // Allocate all the memory here
  if (NULL != m_data->y) {free(m_data->y); m_data->y = NULL; }
  if (NULL != m_data->xt) {free(m_data->xt); m_data->xt = NULL; }
  if (NULL != m_data->probs) {free(m_data->probs); m_data->probs = NULL; }
  if (m_data->useCurvaturePenalty != 0) {
    if (NULL != m_data->ccost) {free(m_data->ccost); m_data->ccost = NULL; }
  }
  if (NULL != m_data->histograms) {free(m_data->histograms); m_data->histograms = NULL; }

  int err = m_data->allocateDataStructureMemory();
  if (err)
  {
    UPDATE_PROGRESS(QString("Error allocating memory for the EMMPM Data Structure"), 0);

    emit finished(this);
    return;
  }

  // Run the EM/MPM algorithm on the input image
  EMMPM::Pointer emmpm = EMMPM::New();
  emmpm->addObserver(this);
  emmpm->setData(m_data);
  emmpm->setStatsDelegate(static_cast<StatsDelegate*>(this) );
  emmpm->setInitializationFunction(initFunction);
  emmpm->execute();


  // Set the input image pointer to NULL so it does not get freed twice
  m_data->inputImage = NULL;

  QImage outQImage(width, height, QImage::Format_Indexed8);
  outQImage.fill(0);
  uchar* front = NULL;
  for (int y = 0; y < height; ++y)
  {
    front = outQImage.scanLine(y);
    int bytesPerLine = outQImage.bytesPerLine();
    for (int x = 0; x < width; ++x)
    {
      front[x] = m_data->xt[width*y + x];
    }
  }
   outQImage.setColorCount(m_data->classes);

  // Now overwrite the beginning of the color table with the gray scale values for each class
  for (int g = 0; g < m_data->classes; ++g)
  {
    outQImage.setColor(g, qRgb(m_data->grayTable[g], m_data->grayTable[g], m_data->grayTable[g]) );
  }

  // Save the output image to a file
  QFileInfo fi (QString(m_data->output_file_name));
  QString ext = fi.suffix();
  if (ext.compare(QString("tif"), Qt::CaseInsensitive) == 0
      || ext.compare(QString("tiff"), Qt::CaseInsensitive) == 0)
  {
    TiffUtilities tifUtil;
    int err = tifUtil.writeOutputImage(m_data);
    if (err < 0)
    {
      UPDATE_PROGRESS(QString("EM/MPM Error Writing Output Image"), 100); emit

      emit finished(this);
      return;
    }
  }
  else
  {
    outQImage.convertToFormat(QImage::Format_RGB32);
    bool success = outQImage.save(m_data->output_file_name);
     if (false == success)
     {
       UPDATE_PROGRESS(QString("EM/MPM Error Writing Output Image"), 95);
     }
  }


  if (m_OutputStatsFile.empty() == false)
  {
    FILE* f = fopen(m_OutputStatsFile.c_str(), "ab+");
    fprintf(f, "InputFile:%s\n", m_data->input_file_name);
    fprintf(f, "SegmentedFile:%s\n" , m_data->output_file_name);
    fprintf(f, "NumClasses:%d\n", m_data->classes);
    fprintf(f, "Proposed EM Loops:%d\n", m_data->emIterations);
    fprintf(f, "EM Loops Completed:%d\n", m_data->currentEMLoop);
    fprintf(f, "Final MSE:%f\n", m_data->currentMSE);
    fprintf(f, "Stopping Threshold:%f\n", m_data->stoppingThreshold);
    fprintf(f, "Use Stopping Criteria:%d\n", (int)(m_data->useStoppingThreshold));
    fprintf(f, "Class,Mu,Sigma\n");
    // Remember the Sigma is the Square Root of the variance
    for(int i = 0; i < m_data->classes; ++i)
    {
      // note we are taking the Squart Root of the variance to get the standard deviation (sigma)
      fprintf(f, "%d,%f,%f\n", i,  m_data->mean[i] , sqrtf(m_data->variance[i]) );
    }

    fclose(f);
  }

  // Save the histogram/Gaussian curve data to a file
  if (getSaveHistogram() == true)
  {
    std::string histogramFileName =  MXAFileInfo::parentPath(m_data->output_file_name) + MXAFileInfo::Separator;
    histogramFileName.append(MXAFileInfo::fileNameWithOutExtension(m_data->output_file_name));
    histogramFileName.append(".csv");
    //std::cout << "Writing Histogram File: " << histogramFileName << std::endl;
    size_t histIdx = 0;
    int count = m_data->classes;
    int columns = count + 2;
    FILE* f = fopen(histogramFileName.c_str(), "wb");
    double value;
    char comma[2] = {',', 0};

    // Write the header to the file
    fprintf(f, "Bins,Histogram");
    for (int c = 2; c < columns; ++c)
    {
      fprintf(f, ",Class %d", (c-2));
    }
    fprintf(f, "\n");

    for(size_t bin = 0; bin < 256; ++bin)
    {
      fprintf(f, "%ld,%f,", bin, m_ImageHistogram.at(bin));
      for (unsigned int d = 0; d < m_data->dims; ++d)
      {
        for (int c = 0; c < m_data->classes; ++c)
        {
          histIdx = (256 * m_data->dims *c) + (256*d) + bin;
          if (c < columns - 1) { comma[0] = ',';}
          else { comma[0] = '\n';}
          value = m_data->histograms[histIdx];
          fprintf(f, "%f%s", value, comma);
        }
      }
      fprintf(f, "\n");
    }
    fflush(f);
    fclose(f);
  }


  //Clean up the Memory as this class will NOT get deleted right away. This will
  // deallocate the bulk of the memory
  // The AIMArray is managing this memory so set it to NULL so it does not get freed twice.
  m_data->outputImage = NULL;
  if(NULL != m_data->input_file_name)
  {
    free(m_data->input_file_name);
    m_data->input_file_name = NULL;
  }
  if(NULL != m_data->output_file_name)
  {
    free(m_data->output_file_name);
    m_data->output_file_name = NULL;
  }

}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
char* EMMPMTask::copyStringToNewBuffer(const QString &fname)
{
  std::string::size_type size = fname.size() + 1;
  char* buf = NULL;
  if (size > 1)
  {
    buf = (char*)malloc(size);
    ::memset(buf, 0, size);
    strncpy(buf, fname.toAscii().constData(), size - 1);
  }
  return buf;
}
