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
//#include "UserInitAreaTableModel.h"
#include "UserInitAreaDialog.h"

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
EMMPMGraphicsView::EMMPMGraphicsView(QWidget *parent)
: QGraphicsView(parent),
  m_ImageGraphicsItem(NULL),
  m_UserInitAreaVector(NULL),
  m_UseColorTable(false)
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
  m_composition_mode = QPainter::CompositionMode_SourceOver;
  m_OverlayTransparency = 1.0f; // Fully opaque

  m_ColorTable.resize(256);
  m_GrayScaleTable.resize(256);
  for (quint32 i = 0; i < 256; ++i)
  {
    m_GrayScaleTable[i] = qRgb(i, i, i);
    m_ColorTable[i] = qRgb(i, i, i);
  }

}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EMMPMGraphicsView::setOverlayTransparency(float f)
{
  m_OverlayTransparency = f;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EMMPMGraphicsView::useColorTable(bool b)
{
  m_UseColorTable = b;
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
  if (index == 9)
  {
    QRectF r = scene()->sceneRect();
    fitInView(r, Qt::KeepAspectRatio);
  }
  else
  {
    QTransform transform;
    transform.scale(m_ZoomFactors[index], m_ZoomFactors[index]);
    setTransform(transform);
  }

}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EMMPMGraphicsView::userInitAreaUpdated(UserInitArea* uia)
{
  // Update the Color Table for this new user init area
  for (quint32 i = 0; i < 256; ++i)
  {
    m_ColorTable[i] = qRgb(i, i, i);
  }

  qint32 size = m_UserInitAreaVector->size();
  UserInitArea* u = NULL;
  for (qint32 i = 0; i < size; ++i)
  {
    u = m_UserInitAreaVector->at(i);
    if (NULL != u)
    {
      int index = u->getEmMpmGrayLevel();
      m_ColorTable[index] = u->getColor().rgb();
    }
  }
  updateDisplay();
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
      if (ext.compare("tif") == 0
          || ext.compare("tiff") == 0
          || ext.compare("jpg") == 0
          || ext.compare("jpeg") == 0
          || ext.compare("png") == 0
          || ext.compare("bmp") == 0)
      {
        m_MainGui->openBaseImageFile(fName);
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
QImage& EMMPMGraphicsView::blend(QImage& src, QImage& dst, float opacity)
{
    if (src.width() <= 0 || src.height() <= 0)
        return dst;
    if (dst.width() <= 0 || dst.height() <= 0)
        return dst;

    if (src.width() != dst.width() || src.height() != dst.height()) {
#ifndef NDEBUG
        std::cerr << "WARNING: ImageEffect::blend : src and destination images are not the same size\n";
#endif
        return dst;
    }

    if (opacity < 0.0 || opacity > 1.0) {
#ifndef NDEBUG
        std::cerr << "WARNING: ImageEffect::blend : invalid opacity. Range [0, 1]\n";
#endif
        return dst;
    }

    if (src.depth() != 32) src = src.convertToFormat(QImage::Format_ARGB32);
    if (dst.depth() != 32) dst = dst.convertToFormat(QImage::Format_ARGB32);

    int pixels = src.width() * src.height();
    {
#ifdef WORDS_BIGENDIAN   // ARGB (skip alpha)
        register unsigned char *data1 = (unsigned char *)dst.bits() + 1;
        register unsigned char *data2 = (unsigned char *)src.bits() + 1;
#else                    // BGRA
        register unsigned char *data1 = (unsigned char *)dst.bits();
        register unsigned char *data2 = (unsigned char *)src.bits();
#endif

        for (register int i=0; i<pixels; i++)
        {
#ifdef WORDS_BIGENDIAN
            *data1 += (unsigned char)((*(data2++) - *data1) * opacity);
            data1++;
            *data1 += (unsigned char)((*(data2++) - *data1) * opacity);
            data1++;
            *data1 += (unsigned char)((*(data2++) - *data1) * opacity);
            data1++;
#else
            *data1 += (unsigned char)((*(data2++) - *data1) * opacity);
            data1++;
            *data1 += (unsigned char)((*(data2++) - *data1) * opacity);
            data1++;
            *data1 += (unsigned char)((*(data2++) - *data1) * opacity);
            data1++;
#endif
            data1++; // skip alpha
            data2++;
        }
    }

    return dst;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EMMPMGraphicsView::updateDisplay()
{
 // QVector<QRgb > colorTable(256);
  if (m_OverlayImage.isNull() == false)
  {
    if (m_UseColorTable == true)
    {
      m_OverlayImage.setColorTable(m_ColorTable);
    }
    else
    {
      m_OverlayImage.setColorTable(m_GrayScaleTable);
    }
  }

//  std::cout << "EMMPMGraphicsView::updateDisplay()" << std::endl;
  QPainter painter;
  QSize pSize(0, 0);
  if (m_BaseImage.isNull() == false)
  {
   pSize = m_BaseImage.size();
  }
  QImage paintImage(pSize, QImage::Format_ARGB32_Premultiplied);
  QPoint point(0, 0);
  painter.begin(&paintImage);
  painter.setPen(Qt::NoPen);
  if (m_ImageDisplayType == EmMpm_Constants::OriginalImage)
  {
    painter.drawImage(point, m_BaseImage);
  }
  else if (m_ImageDisplayType == EmMpm_Constants::SegmentedImage)
  {
    painter.drawImage(point, m_OverlayImage);
  }
  else if (m_ImageDisplayType == EmMpm_Constants::CompositedImage)
  {

    if (m_composition_mode == QPainter::CompositionMode_SourceOver)
    {
      QImage img = m_OverlayImage.copy(0, 0, m_OverlayImage.width(), m_OverlayImage.height());
      img = blend(m_BaseImage, img, m_OverlayTransparency);
      painter.drawImage(point, img);
    }
    else
    {
      painter.drawImage(point, m_BaseImage);
      painter.setCompositionMode(m_composition_mode);
      painter.drawImage(point, m_OverlayImage);
    }
  }
  painter.end();
  m_CompositedImage = paintImage;

  QGraphicsPixmapItem *pixItem = qgraphicsitem_cast<QGraphicsPixmapItem*> (m_ImageGraphicsItem);
  pixItem->setPixmap(QPixmap::fromImage(paintImage));

  this->update();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EMMPMGraphicsView::setImageDisplayType(EmMpm_Constants::ImageDisplayType displayType)
{
  m_ImageDisplayType = displayType;
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

  m_BaseImage.convertToFormat(QImage::Format_ARGB32_Premultiplied);

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
  this->updateDisplay();
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
    std::cout << "Error Loading image: " << filename.toStdString() << std::endl;
    return;
  }
  QVector<QRgb > colorTable(256);
  for (quint32 i = 0; i < 256; ++i)
  {
    colorTable[i] = qRgb(i, i, i);
  }

  if (m_UseColorTable)
  {
    qint32 size = m_UserInitAreaVector->size();
    UserInitArea* u = NULL;
    for(qint32 i = 0; i < size; ++i)
    {
      u = m_UserInitAreaVector->at(i);
      if (NULL != u) {
        int index = u->getEmMpmGrayLevel();
        colorTable[index] = u->getColor().rgb();
      }
    }
  }

  m_OverlayImage.setColorTable(colorTable);
  m_OverlayImage.convertToFormat(QImage::Format_ARGB32_Premultiplied);

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
  //centerOn(m_ImageGraphicsItem);

  m_ImageDisplayType = EmMpm_Constants::CompositedImage;
  emit fireOverlayImageFileLoaded(filename);
  updateDisplay();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EMMPMGraphicsView::setOverlayImage(QImage image)
{
  m_OverlayImage = image;
// Convert to an Premultiplied Image for faster rendering
  m_OverlayImage.convertToFormat(QImage::Format_ARGB32_Premultiplied);

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
  updateDisplay();
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
//void EMMPMGraphicsView::setBaseImage(QImage image)
//{
//  m_BaseImage = image;
//}

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
//void EMMPMGraphicsView::setOverlayImage(QImage image)
//{
//  m_OverlayImage = image;
//}

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
    if (scene())
    {
      QList<QGraphicsItem *> selected;
      selected = scene()->selectedItems();
      if (selected.count() == 0)
      {
        emit fireUserInitAreaLostFocus();
      }
    }
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
  QRectF brect = polygon.boundingRect();

  UserInitArea* userInitArea = new UserInitArea(m_UserInitAreaVector->count(), brect);

  // Line Color
  userInitArea->setPen(QPen(QColor(225, 225, 225, UIA::Alpha)));
  // Fill Color
  userInitArea->setBrush(QBrush(QColor(28, 28, 200, UIA::Alpha)));
  userInitArea->setParentItem(m_ImageGraphicsItem);
  userInitArea->setZValue(1);
  userInitArea->setCacheMode(QGraphicsItem::DeviceCoordinateCache);
  double mu = 0.0;
  double sig = 0.0;
  int err = calculateMuSigma(userInitArea, mu, sig);
  userInitArea->setMu(mu);
  userInitArea->setSigma(sig);

  // Show a dialog to let the user set the values
  UserInitAreaDialog initDialog;
  initDialog.getUserInitAreaWidget()->setUserInitArea(userInitArea);
  int ret =initDialog.exec();
  if (ret == QDialog::Accepted)
  {
    m_UserInitAreaVector->push_back(userInitArea);

    connect (userInitArea, SIGNAL(fireUserInitAreaAboutToDelete(UserInitArea*)),
             m_MainGui, SLOT(deleteUserInitArea(UserInitArea*)), Qt::DirectConnection);

    connect (userInitArea, SIGNAL (fireUserInitAreaUpdated(UserInitArea*)),
             m_MainGui, SLOT(userInitAreaUpdated(UserInitArea*)), Qt::QueuedConnection);

    connect (userInitArea, SIGNAL(fireUserInitAreaSelected(UserInitArea*)),
             m_MainGui, SLOT(userInitAreaSelected(UserInitArea*)), Qt::QueuedConnection);

    connect (userInitArea, SIGNAL (fireUserInitAreaUpdated(UserInitArea*)),
             this, SLOT(userInitAreaUpdated(UserInitArea*)), Qt::QueuedConnection);

    // Update the Color Table for this new user init area
    for (quint32 i = 0; i < 256; ++i)
    {
      m_ColorTable[i] = qRgb(i, i, i);
    }

    qint32 size = m_UserInitAreaVector->size();
    UserInitArea* u = NULL;
    for(qint32 i = 0; i < size; ++i)
    {
      u = m_UserInitAreaVector->at(i);
      if (NULL != u) {
        int index = u->getEmMpmGrayLevel();
        m_ColorTable[index] = u->getColor().rgb();
      }
    }

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
void EMMPMGraphicsView::setCompositeMode(EmMpm_Constants::CompositeType mode)
{
  m_ImageDisplayType = EmMpm_Constants::CompositedImage;
  switch(mode)
  {
    case EmMpm_Constants::Exclusion: m_composition_mode = QPainter::CompositionMode_Exclusion; break;
    case EmMpm_Constants::Difference: m_composition_mode = QPainter::CompositionMode_Difference; break;
    case EmMpm_Constants::Alpha_Blend:
      m_composition_mode = QPainter::CompositionMode_SourceOver;
      break;
#if 0
    case 2: m_composition_mode = QPainter::CompositionMode_Plus; break;
    case 3: m_composition_mode = QPainter::CompositionMode_Multiply; break;
    case 4: m_composition_mode = QPainter::CompositionMode_Screen; break;
    case 5: m_composition_mode = QPainter::CompositionMode_Darken; break;
    case 6: m_composition_mode = QPainter::CompositionMode_Lighten; break;
    case 7: m_composition_mode = QPainter::CompositionMode_ColorDodge; break;
    case 8: m_composition_mode = QPainter::CompositionMode_ColorBurn; break;
    case 9: m_composition_mode = QPainter::CompositionMode_HardLight; break;
    case 10: m_composition_mode = QPainter::CompositionMode_SoftLight; break;

    case 12: m_composition_mode = QPainter::CompositionMode_Destination; break;
    case 13: m_composition_mode = QPainter::CompositionMode_Source; break;
    case 14: m_composition_mode = QPainter::CompositionMode_DestinationOver; break;
    case 15: m_composition_mode = QPainter::CompositionMode_SourceIn; break;
    case 16: m_composition_mode = QPainter::CompositionMode_DestinationIn; break;
    case 17: m_composition_mode = QPainter::CompositionMode_DestinationOut; break;
    case 18: m_composition_mode = QPainter::CompositionMode_SourceAtop; break;
    case 19: m_composition_mode = QPainter::CompositionMode_DestinationAtop; break;
    case 20: m_composition_mode = QPainter::CompositionMode_Overlay; break;
    case 21: m_composition_mode = QPainter::CompositionMode_Clear; break;
#endif
  default:
    m_composition_mode = QPainter::CompositionMode_Exclusion; break;
  }

  this->setImageDisplayType(m_ImageDisplayType);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int EMMPMGraphicsView::calculateMuSigma(UserInitArea* uia, double &mu, double &sig)
{
  if (NULL == uia)
  {
    return -1;
  }
  QPoint p = uia->pos().toPoint();
  QRect b = uia->boundingRect().toAlignedRect();

  QPoint upLeft(b.x() + p.x(), b.y() + p.y());
  QPoint lowRight(b.x() + p.x() + b.width(), b.y() + p.y() + b.height());

  QImage image = getBaseImage();
  qint32 height = image.height();
  qint32 width = image.width();
  QRgb rgbPixel;
  int gray;
  qint32 index;

  int xStart = b.x() + p.x();
  int xEnd = b.x() + p.x() + b.width();
  int yStart = b.y() + p.y();
  int yEnd = b.y() + p.y() + b.height();

  mu = 0.0;
  sig = 0.0;
  //Calculate Mu
  for (qint32 y = yStart; y < yEnd; y++)
  {
    for (qint32 x = xStart; x < xEnd; x++)
    {
      index = (y * width) + x;
      rgbPixel = image.pixel(x, y);
      gray = qGray(rgbPixel);
      mu += gray;
    }
  }
  mu /= ((yEnd - yStart) * (xEnd - xStart));

  // Calculate Sigma
  for (qint32 y = yStart; y < yEnd; y++)
  {
    for (qint32 x = xStart; x < xEnd; x++)
    {
      index = (y * width) + x;
      rgbPixel = image.pixel(x, y);
      gray = qGray(rgbPixel);
      sig += (gray - mu) * (gray - mu);
    }
  }
  sig /= ((yEnd - yStart) * (xEnd - xStart));
  //Calculate Std Dev (Squart Root of Variance)
  sig = sqrt(sig);
  return 0;
}
