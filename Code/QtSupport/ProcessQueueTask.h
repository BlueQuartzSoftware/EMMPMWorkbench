
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

#ifndef PROCESSQUEUETASK_H_
#define PROCESSQUEUETASK_H_

#include <QtCore/QObject>
#include <QtCore/QThread>
#include <QtCore/QString>
#include <QtCore/QList>
#include <QtCore/QPair>
#include <QtGui/QImage>

#include "MXA/MXA.h"
#include "MXA/Common/MXASetGetMacros.h"

#include "AIM/Common/AIMImage.h"

#include "EMMPMLib/EMMPMLib.h"
#include "EMMPMLib/Common/Observer.h"
#include "EMMPMLib/Core/EMMPM_Data.h"



/**
 * @class ProcessQueueTask ProcessQueueTask.h QtSupport/ProcessQueueTask.h
 * @brief This class is a QThread derived class which is meant to be subclassed
 * by the developer and code implemented that the developer would run on a
 * separate thread. The developer should implement the "run()" method in their own
 * class which contains the code to be executed. When the code is complete the developer
 * should call the 'emit finished()' just before exiting the 'run' method. This way
 * the proper signals are sent and dialogs and other objects are updated correctly.
 * The developer can also emit various signals that indicate progress of the code.
 * @author Michael A. Jackson for BlueQuartz Software
 * @date Jul 26, 2010
 * @version 1.0
 */
class ProcessQueueTask : public QThread , public Observer
{
    Q_OBJECT;

  public:
    ProcessQueueTask(QObject* parent = 0);
    virtual ~ProcessQueueTask();

    /**
     * @brief Is the task canceled
     * @return
     */
    bool isCanceled();

    typedef QPair<QString, QString>        InputOutputFilePair;
    typedef QList<InputOutputFilePair>     InputOutputFilePairList;

    MXA_INSTANCE_PROPERTY(bool, Debug);
    MXA_INSTANCE_PROPERTY(InputOutputFilePairList, InputOutputFilePairList)

    MXA_INSTANCE_PROPERTY(bool, FeedBackInitialization);
    MXA_INSTANCE_PROPERTY(bool, SaveHistogram);

    /**
     * @brief Converts a QImage to a Gray Scale (8 bit) AIMImage object
     * @param image The input QImage
     * @return An AIMImage smart pointer. The Wrapped pointer will be NULL if there
     * was an error during the conversion.
     */
    AIMImage::Pointer convertQImageToGrayScaleAIMImage(QImage image);


    /**
     * @brief Either prints a message or sends the message to the User Interface
     * @param message The message to print
     * @param progress The progress of the GrainGenerator normalized to a value between 0 and 100
     */
    virtual void updateProgressAndMessage(const char* message, int value)
    {
      emit updateProgress(value);
      emit progressMessage(QString(message));
    }

    /**
     * @brief This method reports progress such that a user interface element such
     * as a progress bar could be updated. It is assumed the value will fluctuate
     * between 0 and 100.
     * @param value
     */
    virtual void pipelineProgress(int value)
    {
      emit updateProgress(value);
    }

    /**
     * @brief This message reports some human readable message suitable for display
     * on a GUI or printed to a console or possibly saved to a log file
     * @param message
     */
    virtual void pipelineProgressMessage(const char* message)
    {
      emit progressMessage(QString(message));
    }

    /**
     * @brief This message reports some human readable message suitable for display
     * on a GUI or printed to a console or possibly saved to a log file
     * @param message
     */
    virtual void pipelineWarningMessage(const char* message)
    {
      emit warningMessage(QString(message));
    }

    /**
     * @brief This message reports some human readable message suitable for display
     * on a GUI or printed to a console or possibly saved to a log file
     * @param message
     */
    virtual void pipelineErrorMessage(const char* message)
    {
      emit errorMessage(QString(message));
    }

    /**
     * @brief This method is called from the run() method just before exiting and
     * signals the end of the pipeline execution
     */
    virtual void pipelineFinished()
    {
      emit finished(this);
    }

    /**
     * Qt Signals for connections
     */
    signals:
      void progressMessage(QString message);
      void warningMessage(QString message);
      void errorMessage(QString message);
      void updateProgress(int value);
      void finished(QObject *o);
      void updateImageAvailable(QImage image);
      void histogramsAboutToBeUpdated();
      void updateHistogramAvailable(QVector<real_t> values);
      void imageStarted(QString s);

    public slots:

      /**
       * @brief Slot to receive a signal to cancel the operation
       */
      void cancel();

      /**
       * @brief Pure Virtual method which should be implemented by sub-classes. The
       * run method represents the entry point into this class.
       */
      virtual void run() = 0;

    private:
      bool m_Cancel;

      ProcessQueueTask(const ProcessQueueTask&); // Copy Constructor Not Implemented
      void operator=(const ProcessQueueTask&); // Operator '=' Not Implemented


};

#endif /* PROCESSQUEUETASK_H_ */
