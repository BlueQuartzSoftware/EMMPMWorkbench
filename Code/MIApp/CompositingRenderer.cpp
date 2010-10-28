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
#include "CompositingRenderer.h"

#include <QMouseEvent>
#include <qmath.h>

#include <MXA/Common/LogTime.h>

#include <iostream>

const int alpha = 155;

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
CompositingRenderer::CompositingRenderer(QWidget *parent) :
  QWidget(parent),
  zoomFactor(1.0),
  imageModified(true)
{
  m_NewInitArea = false;
  m_current_object = Image;
  m_composition_mode = QPainter::CompositionMode_Exclusion;
  m_movingImagePos = QPoint(0, 0);
  m_RubberBand = NULL;

  setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void CompositingRenderer::setFixedImage(QPixmap fixedImage)
{
  std::cout << logTime() << "setFixedImage()" << std::endl;
  _fixedImage = fixedImage;
  QSize size(_fixedImage.size().width() * zoomFactor, _fixedImage.size().height() * zoomFactor);
  m_image = _fixedImage.scaled(size, Qt::KeepAspectRatio, Qt::SmoothTransformation);
  imageModified = true;
  setMinimumSize(m_image.size());
  setBaseSize(m_image.size());
  QRectF r = geometry();
  setGeometry(r.topLeft().x(), r.topLeft().y(), m_image.size().width(), m_image.size().height());
  resize(m_image.size());
  updateGeometry();
  update();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void CompositingRenderer::setMovingImage(QPixmap movingImage)
{
  std::cout << logTime() << "setMovingImage()" << std::endl;
  _movingImage = movingImage;
  QSize size(_movingImage.size().width() * zoomFactor, _movingImage.size().height() * zoomFactor);
  m_top_image = _movingImage.scaled(size, Qt::KeepAspectRatio, Qt::SmoothTransformation);
  imageModified = true;
  update();
}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void CompositingRenderer::setZoomFactor(double zoom)
{
  //std::cout << logTime() << "setZoomFactor()" << std::endl;
  this->zoomFactor = zoom;
  setFixedImage(_fixedImage);
  setMovingImage(_movingImage);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int CompositingRenderer::saveToFile(QString &filename)
{
  int err = 0;
  QPainter painter;
  QImage image(this->size(), QImage::Format_ARGB32_Premultiplied);

  painter.begin(&image);
  paint(&painter);
  painter.end();
//  hoverPts->paintPoints(image);
  err = (int) (image.save(filename));
  return err;
}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void CompositingRenderer::paint(QPainter *painter)
{
  //std::cout << logTime() << "  R3DCompositionRenderer::paint()" << std::endl;
  // using a QPixmap
  if (imageModified)
  {
    // Draw the fixed Image first
    painter->setPen(Qt::NoPen);
    painter->drawPixmap(rect(), m_image);
    imageModified = false;
  }
  else
  {
    // Draw the fixed Image first
    painter->setPen(Qt::NoPen);
    painter->drawPixmap(rect(), m_image);
  }
    // Draw the moving image next
    painter->setCompositionMode(m_composition_mode);
    QPointF center(m_top_image.width() / qreal(2), m_top_image.height() / qreal(2));
//    painter->translate(ctrlPoints.at(0) );
//    painter->rotate(m_rotation);
//    painter->translate(-ctrlPoints.at(0) );
    painter->drawPixmap(m_movingImagePos, m_top_image);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void CompositingRenderer::paintEvent(QPaintEvent *e)
{
 // std::cout << logTime() << " R3DCompositionRenderer::paintEvent" << std::endl;
  QPainter painter;
  painter.begin(this);
  paint(&painter);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void CompositingRenderer::resizeEvent(QResizeEvent *e)
{
  std::cout << logTime() << "R3DCompositionRenderer::resizeEvent" << std::endl;
  QWidget::resizeEvent(e);
}

void CompositingRenderer::mousePressEvent(QMouseEvent *event)
{
  if (m_NewInitArea == true)
  {
    m_MouseClickOrigin = event->pos();
    if (!m_RubberBand) m_RubberBand = new QRubberBand(QRubberBand::Rectangle, this);
    m_RubberBand->setGeometry(QRect(m_MouseClickOrigin, QSize()));
    m_RubberBand->show();
  }
}

 void CompositingRenderer::mouseMoveEvent(QMouseEvent *event)
 {
   if (m_NewInitArea == true) {
     m_RubberBand->setGeometry(QRect(m_MouseClickOrigin, event->pos()).normalized());
   }
 }

 void CompositingRenderer::mouseReleaseEvent(QMouseEvent *event)
 {
   if (m_NewInitArea == true) {
     m_RubberBand->hide();
     QRect box = QRect(m_MouseClickOrigin, event->pos()).normalized();
     addNewInitArea(box);
     m_NewInitArea = false;
   }

 }

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void CompositingRenderer::addNewInitArea(const QRect &box)
{

  InitArea* initArea = new InitArea(this, InitArea::RectangleShape);

  initArea->setEditable(true);
  initArea->setPointSize(box.size());
  initArea->setShapeBrush(QBrush(QColor(151, 0, 0, alpha)));
  initArea->setShapePen(QPen(QColor(255, 100, 50, alpha)));
  //initArea->setConnectionPen(QPen(QColor(151, 0, 0, 50)));
  //hoverPts->setBoundingRect(m_top_image.rect());

  initArea->setPoint(box.center());
  std::cout << "InitArea.Size(); " << box.size().width() << ", " << box.size().height() << std::endl;
  std::cout << "InitArea.Center.pos(): " << box.center().x() << ", " << box.center().y() << std::endl;
  m_InitAreas << initArea;
//  connect(initArea, SIGNAL(pointChanged(const QPointF&)),
//          this, SLOT(updateInitArea(const QPointF&)));

}

#if 0
// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void CompositingRenderer::mousePressEvent(QMouseEvent *e)
{
  std::cout << "mousePressedEvent: pos=" << e->pos().x() << "," << e->pos().y() << std::endl;
  QRectF movImageRect = QRectF(m_movingImagePos, m_top_image.size());
  if (movImageRect.contains(e->pos()))
  {
    std::cout << "  circle contains... " << std::endl;
    movImageRect.translate(-m_top_image.size().width()/2, -m_top_image.size().height()/2);
    m_current_object = Image;
    m_offset = movImageRect.center() - e->pos();
  }
  else
  {
    m_current_object = NoObject;
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void CompositingRenderer::mouseMoveEvent(QMouseEvent *e)
{
  // std::cout << "mouseMoveEvent" << std::endl;
  if (m_current_object == Image) setMovingImagePos(e->pos() + m_offset);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void CompositingRenderer::mouseReleaseEvent(QMouseEvent *)
{
  // std::cout << "mouseReleaseEvent" << std::endl;
  m_current_object = NoObject;
  updateMovingImagePos();
}
#endif


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
//void CompositingRenderer::updateCtrlPoints(const QPolygonF &points)
//{
//    ctrlPoints = points;
//}

void CompositingRenderer::updateInitArea(const QPointF &point)
{

}
