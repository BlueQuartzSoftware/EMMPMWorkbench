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


#include "EMMPMGraphicsView.h"

#include <QtCore/QFileInfo>
#include <QtCore/QUrl>
#include <QtGui/QWidget>
#include <QDragEnterEvent>
#include <QDragLeaveEvent>
#include <QDropEvent>
#include <QtDebug>
#include <QtGui/QPixmap>
#include <QtGui/QGraphicsPolygonItem>

#include "EmMpmGui.h"
#include "UserInitArea.h"
#include "UserInitAreaTableModel.h"
#include "UserInitAreaDialog.h"

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
EMMPMGraphicsView::EMMPMGraphicsView(QWidget *parent)
: QGraphicsView(parent),
  m_ImageGraphicsItem(NULL),
  m_UserInitAreaTableModel(NULL)
{
  setAcceptDrops(true);
  setDragMode(RubberBandDrag);
  m_AddUserInitArea = false;

  m_ZoomFactors[0] = 0.1f;
  m_ZoomFactors[1] = 0.25f;
  m_ZoomFactors[2] = 0.5f;
  m_ZoomFactors[3] = 1.0f;
  m_ZoomFactors[4] = 1.250f;
  m_ZoomFactors[5] = 1.500f;
  m_ZoomFactors[6] = 2.000f;
  m_ZoomFactors[7] = 4.000f;
  m_ZoomFactors[8] = 6.000f;
  m_ZoomFactors[9] = -1.0f;
  m_MainGui = NULL;
  m_RubberBand = NULL;
  m_ImageDisplayType = EmMpm_Constants::OriginalImage;
  m_composition_mode = QPainter::CompositionMode_Exclusion;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EMMPMGraphicsView::fitToWindow()
{
  QRectF r = scene()->sceneRect();
  fitInView(r, Qt::KeepAspectRatio);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EMMPMGraphicsView::addUserInitArea(bool b)
{
  m_AddUserInitArea = b;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EMMPMGraphicsView::zoomIn()
{
  scale(1.1, 1.1);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EMMPMGraphicsView::zoomOut()
{
  scale(1.0 / 1.1, 1.0 / 1.1);
}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EMMPMGraphicsView::setZoomIndex(int index)
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

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EMMPMGraphicsView::dragEnterEvent(QDragEnterEvent *event)
{
 // qWarning("QFSDroppableGraphicsView::dragEnterEvent(QDragEnterEvent *event)");
  // accept just text/uri-list mime format
  if (event->mimeData()->hasFormat("text/uri-list"))
  {
    event->acceptProposedAction();
  }
  this->setStyleSheet("border: 1px solid green;");
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EMMPMGraphicsView::dragLeaveEvent(QDragLeaveEvent *event)
{
//  qWarning("QFSDroppableGraphicsView::dragLeaveEvent(QDragLeaveEvent *event)");
  this->setStyleSheet("");
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EMMPMGraphicsView::dropEvent(QDropEvent *event)
{
  this->setStyleSheet("");
//  qWarning("QFSDroppableGraphicsView::dropEvent(QDropEvent *event)");
  QList<QUrl> urlList;
  QString fName;
  QFileInfo info;

  if (event->mimeData()->hasUrls())
  {
    urlList = event->mimeData()->urls(); // returns list of QUrls
    // if just text was dropped, urlList is empty (size == 0)

    if ( urlList.size() > 0) // if at least one QUrl is present in list
    {
      fName = urlList[0].toLocalFile(); // convert first QUrl to local path
      info.setFile( fName ); // information about file
      QString ext = info.suffix().toLower();
      if (ext.compare("tif") == 0\
          || ext.compare("tiff") == 0
          || ext.compare("jpg") == 0
          || ext.compare("jpeg") == 0
          || ext.compare("png") == 0
          || ext.compare("bmp") == 0)
      {
        loadBaseImageFile(fName);
      }
    }
  }
  event->acceptProposedAction();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QImage EMMPMGraphicsView::getCompositedImage()
{
  return m_CompositedImage;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EMMPMGraphicsView::setImageDisplayType(int displayType)
{
  m_ImageDisplayType = (EmMpm_Constants::ImageDisplayType)displayType;
  QImage base;
  QImage overlay;
  if (displayType == EmMpm_Constants::OriginalImage)
  {
    base = m_BaseImage;
  }
  else if (displayType == EmMpm_Constants::SegmentedImage)
  {
    base = m_OverlayImage;
  }
  else if (displayType == EmMpm_Constants::CompositedImage)
  {
    base = m_BaseImage;
    overlay = m_OverlayImage;
  }

  QPainter painter;
  QImage paintImage(m_BaseImage.size(), QImage::Format_ARGB32_Premultiplied);
  QPoint point(0, 0);
  painter.begin(&paintImage);
  painter.setPen(Qt::NoPen);

  if (overlay.isNull() == false) { painter.drawImage(point, overlay); }
  painter.setCompositionMode(m_composition_mode);
  painter.drawImage(point, base);
  painter.end();
  m_CompositedImage = paintImage;

  QGraphicsPixmapItem *pixItem = qgraphicsitem_cast<QGraphicsPixmapItem*> (m_ImageGraphicsItem);
  pixItem->setPixmap(QPixmap::fromImage(paintImage));

  this->update();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EMMPMGraphicsView::loadBaseImageFile(const QString &filename)
{
  m_BaseImage = QImage(filename);
  if (m_BaseImage.isNull() == true)
  {
    return;
  }
  QVector<QRgb > colorTable(256);
  for (quint32 i = 0; i < 256; ++i)
  {
    colorTable[i] = qRgb(i, i, i);
  }
  m_BaseImage.setColorTable(colorTable);

  QGraphicsScene* gScene = scene();
  if (gScene == NULL)
  {
    gScene = new QGraphicsScene(this);
    setScene(gScene);
  }
  if (NULL == m_ImageGraphicsItem) {
    m_ImageGraphicsItem = gScene->addPixmap(QPixmap::fromImage(m_BaseImage));
  }
  m_ImageGraphicsItem->setAcceptDrops(true);
  m_ImageGraphicsItem->setZValue(-1);
  QRectF rect = m_ImageGraphicsItem->boundingRect();
  gScene->setSceneRect(rect);
  centerOn(m_ImageGraphicsItem);

  setImageDisplayType(m_ImageDisplayType);

  emit fireBaseImageFileLoaded(filename);
}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EMMPMGraphicsView::loadOverlayImageFile(const QString &filename)
{

  m_OverlayImage = QImage(filename);
  if (m_OverlayImage.isNull() == true)
  {
    return;
  }
  QVector<QRgb > colorTable(256);
  for (quint32 i = 0; i < 256; ++i)
  {
    colorTable[i] = qRgb(i, i, i);
  }
  m_OverlayImage.setColorTable(colorTable);


  QGraphicsScene* gScene = scene();
  if (gScene == NULL)
  {
    gScene = new QGraphicsScene(this);
    setScene(gScene);
  }

  // If the GraphicsScene Item does not exist yet lets make one. This would happen
  // if the user loads a segmented image first.
  if (NULL == m_ImageGraphicsItem) {
    m_ImageGraphicsItem = gScene->addPixmap(QPixmap::fromImage(m_OverlayImage));
  }
  m_ImageGraphicsItem->setAcceptDrops(true);
  m_ImageGraphicsItem->setZValue(-1);
  QRectF rect = m_ImageGraphicsItem->boundingRect();
  gScene->setSceneRect(rect);
  centerOn(m_ImageGraphicsItem);

  m_ImageDisplayType = EmMpm_Constants::SegmentedImage;

  setImageDisplayType(m_ImageDisplayType);

  emit fireOverlayImageFileLoaded(filename);
}



// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QImage EMMPMGraphicsView::getBaseImage()
{
  return m_BaseImage;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QImage EMMPMGraphicsView::getOverlayImage()
{
  return m_OverlayImage;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EMMPMGraphicsView::mousePressEvent(QMouseEvent *event)
{
  if (m_AddUserInitArea == true)
  {
    m_MouseClickOrigin = event->pos();
    if (!m_RubberBand) m_RubberBand = new QRubberBand(QRubberBand::Rectangle, this);
    m_RubberBand->setGeometry(QRect(m_MouseClickOrigin, QSize()));
    m_RubberBand->show();
  }
  else
  {
  QGraphicsView::mousePressEvent(event);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EMMPMGraphicsView::mouseMoveEvent(QMouseEvent *event)
{
 if (m_AddUserInitArea == true && m_RubberBand != NULL) {
   m_RubberBand->setGeometry(QRect(m_MouseClickOrigin, event->pos()).normalized());
 }
 else
 {
 QGraphicsView::mouseMoveEvent(event);
 }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EMMPMGraphicsView::mouseReleaseEvent(QMouseEvent *event)
{
 if (m_AddUserInitArea == true)
 {
   m_RubberBand->hide();
   QRect box = QRect(m_MouseClickOrigin, event->pos()).normalized();
   QPolygonF sceneBox = mapToScene(box);
   addNewInitArea(sceneBox);
   m_AddUserInitArea = false;
 }
 else
 {
   QGraphicsView::mouseReleaseEvent(event);
 }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EMMPMGraphicsView::setEmMpmGui(EmMpmGui* gui)
{
  m_MainGui = gui;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EMMPMGraphicsView::addNewInitArea(const QPolygonF &polygon)
{
  QGraphicsScene* gScene = scene();
  if (gScene == NULL)
  {
    gScene = new QGraphicsScene(this);
    setScene(gScene);
  }
  QRectF brect = polygon.boundingRect();

  UserInitArea* userInitArea = new UserInitArea(m_UserInitAreaTableModel->rowCount(), brect);

  // Line Color
  userInitArea->setPen(QPen(QColor(225, 225, 225, UIA::Alpha)));
  // Fill Color
  userInitArea->setBrush(QBrush(QColor(28, 28, 200, UIA::Alpha)));
  userInitArea->setParentItem(m_ImageGraphicsItem);
  userInitArea->setZValue(1);
  userInitArea->setCacheMode(QGraphicsItem::DeviceCoordinateCache);

  // Show a dialog to let the user set the values
  UserInitAreaDialog initDialog(userInitArea);
  int ret =initDialog.exec();
  if (ret == QDialog::Accepted)
  {
    m_UserInitAreaTableModel->addUserInitArea(userInitArea);


    connect (userInitArea, SIGNAL(fireUserInitAreaUpdated(UserInitArea*)),
             m_UserInitAreaTableModel, SLOT(updateUserInitArea(UserInitArea*)), Qt::QueuedConnection);
    connect (userInitArea, SIGNAL (fireUserInitAreaUpdated(UserInitArea*)),
             m_MainGui, SLOT(userInitAreaUpdated(UserInitArea*)), Qt::QueuedConnection);
    connect (userInitArea, SIGNAL(fireUserInitAreaAboutToDelete(UserInitArea*)),
             m_MainGui, SLOT(deleteUserInitArea(UserInitArea*)) );
    connect (userInitArea, SIGNAL(fireUserInitAreaDeleted(UserInitArea*)),
             m_UserInitAreaTableModel, SLOT(deleteUserInitArea(UserInitArea*)));

    emit fireUserInitAreaAdded(userInitArea);
  }
  else
  {
    delete userInitArea;
  }


 }

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EMMPMGraphicsView::setCompositeMode(int mode) {

  switch(mode)
  {
    case 0: m_composition_mode = QPainter::CompositionMode_Exclusion; break;
    case 1: m_composition_mode = QPainter::CompositionMode_Difference; break;
    case 2: m_composition_mode = QPainter::CompositionMode_Plus; break;
    case 3: m_composition_mode = QPainter::CompositionMode_Multiply; break;
    case 4: m_composition_mode = QPainter::CompositionMode_Screen; break;
    case 5: m_composition_mode = QPainter::CompositionMode_Darken; break;
    case 6: m_composition_mode = QPainter::CompositionMode_Lighten; break;
    case 7: m_composition_mode = QPainter::CompositionMode_ColorDodge; break;
    case 8: m_composition_mode = QPainter::CompositionMode_ColorBurn; break;
    case 9: m_composition_mode = QPainter::CompositionMode_HardLight; break;
    case 10: m_composition_mode = QPainter::CompositionMode_SoftLight; break;

    case 11: m_composition_mode = QPainter::CompositionMode_Source; break;
    case 12: m_composition_mode = QPainter::CompositionMode_Destination; break;
    case 13: m_composition_mode = QPainter::CompositionMode_SourceOver; break;
    case 14: m_composition_mode = QPainter::CompositionMode_DestinationOver; break;
    case 15: m_composition_mode = QPainter::CompositionMode_SourceIn; break;
    case 16: m_composition_mode = QPainter::CompositionMode_DestinationIn; break;
    case 17: m_composition_mode = QPainter::CompositionMode_DestinationOut; break;
    case 18: m_composition_mode = QPainter::CompositionMode_SourceAtop; break;
    case 19: m_composition_mode = QPainter::CompositionMode_DestinationAtop; break;
    case 20: m_composition_mode = QPainter::CompositionMode_Overlay; break;
    case 21: m_composition_mode = QPainter::CompositionMode_Clear; break;
  default:
    m_composition_mode = QPainter::CompositionMode_Exclusion; break;
  }

  this->setImageDisplayType(m_ImageDisplayType);
}



