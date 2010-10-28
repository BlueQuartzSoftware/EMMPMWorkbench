/* --------------------------------------------------------------------------*
* This source code has been cleared for public release by the                *
* US Air Force 88th Air Base Wing Public Affairs Office under                *
* case number 88ABW-2010-4857 on Sept. 7, 2010.                              *
* -------------------------------------------------------------------------- */
///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007, 2010 Michael A. Jackson for BlueQuartz Software
//  All rights reserved.
//  BSD License: http://www.opensource.org/licenses/bsd-license.html
//
//  This code was written under United States Air Force Contract number
//                           F33615-03-C-5220
//
///////////////////////////////////////////////////////////////////////////////

#ifndef R3DCOMPOSITIONRENDERER_H_
#define R3DCOMPOSITIONRENDERER_H_

//-- R3D Includes
#include <MIApp/InitArea.h>
#include <QFSDroppableGraphicsView.h>

#include <QtCore/QEvent>
#include <QtGui/QWidget>
#include <QtGui/QPainter>
#include <QtGui/QRubberBand>


QT_FORWARD_DECLARE_CLASS(QPushButton)
QT_FORWARD_DECLARE_CLASS(QRadioButton)

/**
* @class R3DCompositionRenderer R3DCompositionRenderer.h R3D/GuiAlignment/R3DCompositionRenderer.h
* @brief Renders 2 images from an MXA file so the user can manually align the images
* @author Michael A. Jackson for BlueQuartz Software
* @date Apr 24, 2009
* @version 1.0
*/
class CompositingRenderer : public QWidget
{
    Q_OBJECT

    enum ObjectType { NoObject, Image };

public:
    CompositingRenderer(QWidget *parent);

    virtual void paint(QPainter *);

    void mousePressEvent(QMouseEvent *);
    void mouseMoveEvent(QMouseEvent *);
    void mouseReleaseEvent(QMouseEvent *);

    void setMovingImagePos(const QPointF &pos);

    void setFixedImage(QPixmap image);
    void setMovingImage(QPixmap image);

    void setZoomFactor(double zoom);
    double getZoomFactor() { return zoomFactor; }

//    void setFixedImageGrid(int x, int y);
//    void setMovingImageGrid(int x, int y);

    void init(QPixmap fixedImage, QPixmap movingImage);
 //   void setMovingImageOffset(float x, float y);

 //   void setRotation(qreal r);

    void addNewInitArea(const QRect &area);

    QList<InitArea*> getInitAreas();

    int saveToFile(QString &filename);

    signals:
//      void updateOffsets(int x, int y);
//      void rotationChanged(int rotation);
//      void updateCenterOfRotation(int x, int y);

  public slots:

    void setNewInitArea() { m_NewInitArea = true; }




    void setClearMode() { m_composition_mode = QPainter::CompositionMode_Clear; update(); }
    void setSourceMode() { m_composition_mode = QPainter::CompositionMode_Source; update(); }
    void setDestinationMode() { m_composition_mode = QPainter::CompositionMode_Destination; update(); }
    void setSourceOverMode() { m_composition_mode = QPainter::CompositionMode_SourceOver; update(); }
    void setDestinationOverMode() { m_composition_mode = QPainter::CompositionMode_DestinationOver; update(); }
    void setSourceInMode() { m_composition_mode = QPainter::CompositionMode_SourceIn; update(); }
    void setDestInMode() { m_composition_mode = QPainter::CompositionMode_DestinationIn; update(); }
    void setSourceOutMode() { m_composition_mode = QPainter::CompositionMode_SourceOut; update(); }
    void setDestOutMode() { m_composition_mode = QPainter::CompositionMode_DestinationOut; update(); }
    void setSourceAtopMode() { m_composition_mode = QPainter::CompositionMode_SourceAtop; update(); }
    void setDestAtopMode() { m_composition_mode = QPainter::CompositionMode_DestinationAtop; update(); }
    void setXorMode() { m_composition_mode = QPainter::CompositionMode_Xor; update(); }
    void setPlusMode() { m_composition_mode = QPainter::CompositionMode_Plus; update(); }
    void setMultiplyMode() { m_composition_mode = QPainter::CompositionMode_Multiply; update(); }
    void setScreenMode() { m_composition_mode = QPainter::CompositionMode_Screen; update(); }
    void setOverlayMode() { m_composition_mode = QPainter::CompositionMode_Overlay; update(); }
    void setDarkenMode() { m_composition_mode = QPainter::CompositionMode_Darken; update(); }
    void setLightenMode() { m_composition_mode = QPainter::CompositionMode_Lighten; update(); }
    void setColorDodgeMode() { m_composition_mode = QPainter::CompositionMode_ColorDodge; update(); }
    void setColorBurnMode() { m_composition_mode = QPainter::CompositionMode_ColorBurn; update(); }
    void setHardLightMode() { m_composition_mode = QPainter::CompositionMode_HardLight; update(); }
    void setSoftLightMode() { m_composition_mode = QPainter::CompositionMode_SoftLight; update(); }
    void setDifferenceMode() { m_composition_mode = QPainter::CompositionMode_Difference; update(); }
    void setExclusionMode() { m_composition_mode = QPainter::CompositionMode_Exclusion; update(); }

//    void setFixedGridEnabled(bool enabled) { _enableFixedGrid = enabled; update(); }
//    void setMovingGridEnabled(bool enabled) { _enableMovingGrid = enabled; update(); }

//    void updateCtrlPoints(const QPolygonF &);
//    void changeRotation(double rotation);

    void updateInitArea(const QPointF &);

  protected:
    void paintEvent(QPaintEvent *);
    void resizeEvent(QResizeEvent *);

private:
  //  void updateMovingImagePos();
//    void drawFixedImageGrid(QPainter &p);
//    void drawMovingImageGrid(QPainter &p);
//    void initHoverPoints();

    QPainter::CompositionMode m_composition_mode;

    QPixmap m_image;
    QPixmap _fixedImage;
    QPixmap m_top_image;
    QPixmap _movingImage;

    QPointF m_movingImagePos;
    QPointF m_offset;

    ObjectType m_current_object;
    double zoomFactor;
//    int fixedGridX;
//    int fixedGridY;
//
//    int movingGridX;
//    int movingGridY;

//    bool _enableFixedGrid;
//    bool _enableMovingGrid;

    bool imageModified;

//    QPolygonF ctrlPoints;
//    HoverPoints* hoverPts;
//    qreal m_rotation;

    bool m_NewInitArea;
    QList<InitArea*> m_InitAreas;

    QRubberBand* m_RubberBand;
    QPoint m_MouseClickOrigin;

};


#endif /* R3DCOMPOSITIONRENDERER_H_ */
