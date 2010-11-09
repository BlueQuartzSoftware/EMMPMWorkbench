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

#include <QtCore/QDir>
#include <QtCore/QLocale>


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
EMMPMTask::EMMPMTask(QObject* parent) :
  ProcessQueueTask(parent),
  m_Beta(0.1f),
  m_Gamma(0.0f),
  m_EmIterations(5),
  m_MpmIterations(5),
  m_UseSimulatedAnnealing(false)
{

}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
EMMPMTask::~EMMPMTask()
{
 // std::cout << "   ~EMMPMTask()" << std::endl;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EMMPMTask::run()
{
  
//  std::cout << "  EMMPMTask::run()" << std::endl;
  UPDATE_PROGRESS(QString("Starting Segmentation"), 0);

  // Load the image from the input file
  QImage image(getInputFilePath());
  m_OriginalImage = convertQImageToGrayScaleAIMImage(image);
  if (NULL == m_OriginalImage.RAW_PTR())
  {
    std::cout << "Error loading image '" << getInputFilePath().toStdString() << "'" << std::endl;
 //   emit finished();
    emit taskFinished(this);
    return;
  }

  // Allocate memory for the segmented image
  m_SegmentedImage = AIMImage::New();
  m_SegmentedImage->allocateSameSizeImage(m_OriginalImage);
  if (NULL == m_SegmentedImage)
  {
    std::cout << "Error Allocating Memory for segmented Image." << std::endl;
 //   emit finished();
    emit taskFinished(this);
    return;
  }


  execute<uint8_t, uint8_t>( m_OriginalImage->getImageBuffer(), m_SegmentedImage->getImageBuffer());

  if (isCanceled() == false)
  {
    // Save the image to the output file:
    qint32 width = m_SegmentedImage->getImagePixelWidth();
    qint32 height = m_SegmentedImage->getImagePixelHeight();
    quint8* dataPointer = m_SegmentedImage->getImageBuffer();
    QImage outImage(width, height, QImage::Format_Indexed8);
    QVector<QRgb> colorTable(256);
    for (quint32 i = 0; i < 256; ++i)
    {
      colorTable[i] = qRgb(i, i, i);
    }
    outImage.setColorTable(colorTable);
    qint32 index;
    for (qint32 j=0; j<height; j++)
    {
      for (qint32 i =0; i<width; i++)
      {
        index = (j * width) + i;
        outImage.setPixel(i, j, dataPointer[index]);
      }
    }
    bool ok = outImage.save(getOutputFilePath() );
    if (false == ok)
    {
      std::cout << "Error saving segmented image to '" << getOutputFilePath().toStdString() << "'" << std::endl;
    }
    // Clean up memory usage
    m_SegmentedImage = AIMImage::NullPointer();
    m_OriginalImage = AIMImage::NullPointer();
  }
  // Notify observers that we are finished
//  emit finished();
  emit taskFinished(this);
 // std::cout << "  EmMpm Task Finished." << std::endl;
}



void EMMPMTask::setBeta(float beta)
{
  m_Beta = beta;
}
void EMMPMTask::setGamma(float gamma)
{
 m_Gamma = gamma;
}
void EMMPMTask::setEmIterations(int emIterations)
{
  m_EmIterations = emIterations;
}
void EMMPMTask::setMpmIterations(int mpmIterations)
{
  m_MpmIterations = mpmIterations;
}
void EMMPMTask::setNumberOfClasses(int numClasses)
{
 m_NumberOfClasses = numClasses;
}

void EMMPMTask::useSimulatedAnnealing()
{
  m_UseSimulatedAnnealing = true;
}

