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

#include <iostream>
#include <QtGui/QGraphicsView>
#include <QtGui/QGraphicsItem>
#include <QtGui/QRubberBand>

class UserInitAreaTableModel;
class UserInitArea;

class EMMPMGraphicsView : public QGraphicsView
{
    Q_OBJECT

  public:
    EMMPMGraphicsView(QWidget *parent = NULL);

    void setUserInitAreaTableModel(UserInitAreaTableModel* userInitAreaTableModel)
    {
      m_UserInitAreaTableModel = userInitAreaTableModel;
    }
//  void setEmMpmGui(EmMpmGui* gui);

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

    void mousePressEvent(QMouseEvent *);
    void mouseMoveEvent(QMouseEvent *);
    void mouseReleaseEvent(QMouseEvent *);




  public slots:
    void zoomIn() { scale(1.1, 1.1); }
    void zoomOut() { scale(1.0/1.1, 1.0/1.1); }

    void fitToWindow()
    {
      QRectF r = scene()->sceneRect();
      fitInView(r, Qt::KeepAspectRatio);
    }

    void setZoomIndex(int index)
    {
      if (index == 3)
      {
        resetMatrix();
        resetTransform();
      }
      else
      {
        resetMatrix();
        resetTransform();
        scale(m_ZoomFactors[index], m_ZoomFactors[index]);
      }
    }

    //TODO Need to implement setCompositeMode(int)
    void setCompositeMode(int mode) {
      std::cout << "TODO:// Need to implement setCompositeMode(int)" << std::endl;
    }

    void addUserInitArea(bool b) { m_AddUserInitArea = b; }


  signals:
   void fireImageFileLoaded(const QString &filename);
   void fireUserInitAreaAdded();

  protected:
    void loadImageFile(const QString &filename);
    void addNewInitArea(const QPolygonF &box);

  private:
   QGraphicsItem* m_ImageGraphicsItem;
   bool           m_AddUserInitArea;
   QRubberBand*   m_RubberBand;
   QPoint         m_MouseClickOrigin;
   float          m_ZoomFactors[10];
   UserInitAreaTableModel* m_UserInitAreaTableModel;

   EMMPMGraphicsView(const EMMPMGraphicsView&); // Copy Constructor Not Implemented
   void operator=(const EMMPMGraphicsView&); // Operator '=' Not Implemented
};

#endif /* QFSDROPPABLEGRAPHICSVIEW_H_ */
