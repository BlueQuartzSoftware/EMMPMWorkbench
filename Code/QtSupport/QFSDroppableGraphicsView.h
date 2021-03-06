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

#ifndef QFSDROPPABLEGRAPHICSVIEW_H_
#define QFSDROPPABLEGRAPHICSVIEW_H_

#include <QtGui/QGraphicsView>

/**
* @class QFSDroppableGraphicsView QFSDroppableGraphicsView.h QtSupport/QFSDroppableGraphicsView.h
* @brief A QGraphicsView object that handles files being dropped on it from the
* filesystem. After the file is dropped the signal "loadImageFileRequested(QString)" is
* emitted. Hooking up to this signal with a slot will allow the developer to open
* the file. The complete path to the file is containted in the QString argument.
* @author Michael A. Jackson for BlueQuartz Software
* @date Apr 5, 2010
* @version 1.0
*/
class QFSDroppableGraphicsView : public QGraphicsView
{
    Q_OBJECT

  public:
    QFSDroppableGraphicsView(QWidget *parent = NULL);

    /**
     * @brief Over-riding implementation from base class
     * @param event QDragEnterEvent Event fired when dragging enters the QGraphicsView
     */
    void dragEnterEvent(QDragEnterEvent *event);

    /**
     * @brief Over-riding implementation from base class
     * @param event QDropEvent Event fired when object is dropped on QGraphicsView
     */
    void dropEvent(QDropEvent *event);

    /**
     * @brief Over-riding implementation from base class
     * @param event QDragLeaveEvent Event fired when dragging leaves QGraphicsView
     */
    void dragLeaveEvent(QDragLeaveEvent *event);

  protected:

  signals:
   void loadImageFileRequested(const QString &filename);

};

#endif /* QFSDROPPABLEGRAPHICSVIEW_H_ */
