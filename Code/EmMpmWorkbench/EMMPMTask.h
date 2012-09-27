/*
The Original EM/MPM algorithm was developed by Mary L. Comer and is distributed
under the BSD License.
Copyright (c) <2010>, <Mary L. Comer>
All rights reserved.

[1] Comer, Mary L., and Delp, Edward J.,  ÒThe EM/MPM Algorithm for Segmentation
of Textured Images: Analysis and Further Experimental Results,Ó IEEE Transactions
on Image Processing, Vol. 9, No. 10, October 2000, pp. 1731-1744.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

Redistributions of source code must retain the above copyright notice, this list
of conditions and the following disclaimer.

Redistributions in binary form must reproduce the above copyright notice, this
list of conditions and the following disclaimer in the documentation and/or
other materials provided with the distribution.

Neither the name of <Mary L. Comer> nor the names of its contributors may be
used to endorse or promote products derived from this software without specific
prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2010, Michael A. Jackson. BlueQuartz Software
//  All rights reserved.
//  BSD License: http://www.opensource.org/licenses/bsd-license.html
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _EMMPM_TASK_H_
#define _EMMPM_TASK_H_

#include <QtCore/QObject>
#include <QtCore/QThread>
#include <QtCore/QString>
#include <QtGui/QImage>

#include "QtSupport/ProcessQueueTask.h"

//-- EMMPM Lib Includes
#include "EMMPMLib/EMMPMLib.h"
#include "EMMPMLib/Core/EMMPM.h"
#include "EMMPMLib/Core/EMMPM_Data.h"
#include "EMMPMLib/Common/StatsDelegate.h"



#define UPDATE_PROGRESS(m, p)\
  emit progressMessage( (m) );\
  emit updateProgress( (p) );

/**
* @class EMMPMTask EMMPMTask.h EmMpm/GUI/EMMPMTask.h
* @brief This is the wrapper code for the code. This is called as a "worker" class from a separate thread
* of execution in order to not lock up the GUI.
* @author Michael A. Jackson for BlueQuartz Software
* @date Dec 20, 2009
* @version 1.0
*/
class EMMPMTask : public ProcessQueueTask, public StatsDelegate
{

  Q_OBJECT;

  public:
    EMMPMTask(QObject* parent = 0);
    virtual ~EMMPMTask();

    /**
     * @brief StatsDelegate Implementation
     * @param data The Incoming data from the EM/MPM Process
     */
    virtual void reportProgress(EMMPM_Data::Pointer data);

    EMMPM_Data::Pointer getEMMPM_Data();

    virtual void run();

    void segmentImage(int i);

    MXA_INSTANCE_PROPERTY(QVector<double>, ImageHistogram)

    MXA_INSTANCE_STRING_PROPERTY(OutputStatsFile)

  public slots:

    /**
     * @brief Slot to receive a signal to cancel the operation
     */
    void cancel();


  private:

    EMMPM_Data::Pointer m_data;
    AIMImage::Pointer m_OriginalImage;
    AIMImage::Pointer m_SegmentedImage;

    char* copyStringToNewBuffer(const QString &fname);



    EMMPMTask(const EMMPMTask&); // Copy Constructor Not Implemented
    void operator=(const EMMPMTask&); // Operator '=' Not Implemented

};





#endif /* _EMMPM_TASK_H_ */
