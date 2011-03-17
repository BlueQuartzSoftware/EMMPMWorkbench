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

#ifndef _EMMPM_GRAPHICS_VIEW_H_
#define _EMMPM_GRAPHICS_VIEW_H_

#include <iostream>

#include <QtCore/QVector>
#include <QtGui/QGraphicsView>
#include <QtGui/QGraphicsItem>
#include <QtGui/QRubberBand>

//class UserInitAreaTableModel;
class UserInitArea;
class EmMpmGui;

namespace EmMpm_Constants {

enum ImageDisplayType {
  OriginalImage = 0,
  SegmentedImage,
  CompositedImage,
};

}

class EMMPMGraphicsView : public QGraphicsView
{
    Q_OBJECT;

  public:

    EMMPMGraphicsView( QWidget *parent = NULL);

    void setUserInitAreaTableModel(QVector<UserInitArea*>* userInitAreaVector)
    {
      m_UserInitAreaVector = userInitAreaVector;
    }

    void setEmMpmGui(EmMpmGui* gui);

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

    void mousePressEvent( QMouseEvent* event );
    void mouseMoveEvent( QMouseEvent* event );
    void mouseReleaseEvent( QMouseEvent* event );

    //void setBaseImage(QImage image);
    QImage getBaseImage();

    //void setOverlayImage(QImage image);
    QImage getOverlayImage();

    QImage getCompositedImage();

    void loadBaseImageFile(const QString &filename);
    void loadOverlayImageFile(const QString &filename);

    QImage& blend(QImage& src, QImage& dst, float opacity);


  public slots:
    void zoomIn();
    void zoomOut();

    void setOverlayTransparency(float f);

    void fitToWindow();

    void setZoomIndex(int index);

    void setImageDisplayType(int displayType);

    void setCompositeMode(int mode);

    void addUserInitArea(bool b);

    void setOverlayImage(QImage image);

  signals:
   void fireBaseImageFileLoaded(const QString &filename);
   void fireOverlayImageFileLoaded(const QString &filename);
   void fireUserInitAreaAdded(UserInitArea* uia);
   void fireUserInitAreaLostFocus();

  protected:

    void addNewInitArea(const QPolygonF &box);

  private:
   QGraphicsItem* m_ImageGraphicsItem;
   QImage         m_BaseImage;
   QImage         m_OverlayImage;
   QImage         m_CompositedImage;

   bool           m_AddUserInitArea;
   QRubberBand*   m_RubberBand;
   QPoint         m_MouseClickOrigin;
   float          m_ZoomFactors[10];
  // UserInitAreaTableModel* m_UserInitAreaTableModel;
   QVector<UserInitArea*>*      m_UserInitAreaVector;

   EmMpmGui*      m_MainGui;
   EmMpm_Constants::ImageDisplayType           m_ImageDisplayType;
   bool           m_ShowOverlayImage;
   QPainter::CompositionMode m_composition_mode;
   float          m_OverlayTransparency;

   EMMPMGraphicsView(const EMMPMGraphicsView&); // Copy Constructor Not Implemented
   void operator=(const EMMPMGraphicsView&); // Operator '=' Not Implemented
};

#endif /* _EMMPM_GRAPHICS_VIEW_H_ */
