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
  m_UserInitAreaVector(NULL)
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
  m_OverlayTransparency = 1.0f; // Fully opaque
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EMMPMGraphicsView::setOverlayTransparency(float f)
{
  m_OverlayTransparency = f;
  setImageDisplayType(m_ImageDisplayType);
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
      if (ext.compare("tif") == 0
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

// Nice and fast direct pixel manipulation
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

#ifdef USE_SSE2_INLINE_ASM
    if ( KCPUInfo::haveExtension( KCPUInfo::IntelSSE2 ) && pixels > 16 ) {
        Q_UINT16 alpha = Q_UINT16( opacity * 256.0 );
        KIE8Pack packedalpha = { { alpha, alpha, alpha, 0,
                                   alpha, alpha, alpha, 0 } };

        // Prepare the XMM6 and XMM7 registers for unpacking and blending
        __asm__ __volatile__(
        "pxor      %%xmm7, %%xmm7\n\t" // Zero out XMM7 for unpacking
        "movdqu      (%0), %%xmm6\n\t" // Set up alpha * 256 in XMM6
        : : "r"(&packedalpha), "m"(packedalpha) );

        Q_UINT32 *data1 = reinterpret_cast<Q_UINT32*>( src.bits() );
        Q_UINT32 *data2 = reinterpret_cast<Q_UINT32*>( dst.bits() );

        // Check how many pixels we need to process to achieve 16 byte alignment
        int offset = (16 - (Q_UINT32( data2 ) & 0x0f)) / 4;

        // The main loop processes 4 pixels / iteration
        int remainder = (pixels - offset) % 4;
        pixels -= remainder;

        // Alignment loop
        for ( int i = 0; i < offset; i++ ) {
            __asm__ __volatile__(
            "movd       (%1,%2,4),    %%xmm1\n\t"  // Load one dst pixel to XMM1
            "punpcklbw     %%xmm7,    %%xmm1\n\t"  // Unpack the pixel
            "movd       (%0,%2,4),    %%xmm0\n\t"  // Load one src pixel to XMM0
            "punpcklbw     %%xmm7,    %%xmm0\n\t"  // Unpack the pixel
            "psubw         %%xmm1,    %%xmm0\n\t"  // Subtract dst from src
            "pmullw        %%xmm6,    %%xmm0\n\t"  // Multiply the result with alpha * 256
            "psllw             $8,    %%xmm1\n\t"  // Multiply dst with 256
            "paddw         %%xmm1,    %%xmm0\n\t"  // Add dst to result
            "psrlw             $8,    %%xmm0\n\t"  // Divide by 256
            "packuswb      %%xmm1,    %%xmm0\n\t"  // Pack the pixel to a dword
            "movd          %%xmm0, (%1,%2,4)\n\t"  // Write the pixel to the image
            : : "r"(data1), "r"(data2), "r"(i) );
        }

        // Main loop
        for ( int i = offset; i < pixels; i += 4 ) {
            __asm__ __volatile__(
            // Load 4 src pixels to XMM0 and XMM2 and 4 dst pixels to XMM1 and XMM3
            "movq       (%0,%2,4),    %%xmm0\n\t"  // Load two src pixels to XMM0
            "movq       (%1,%2,4),    %%xmm1\n\t"  // Load two dst pixels to XMM1
            "movq      8(%0,%2,4),    %%xmm2\n\t"  // Load two src pixels to XMM2
            "movq      8(%1,%2,4),    %%xmm3\n\t"  // Load two dst pixels to XMM3

            // Prefetch the pixels for the iteration after the next one
            "prefetchnta 32(%0,%2,4)        \n\t"
            "prefetchnta 32(%1,%2,4)        \n\t"

            // Blend the first two pixels
            "punpcklbw     %%xmm7,    %%xmm1\n\t"  // Unpack the dst pixels
            "punpcklbw     %%xmm7,    %%xmm0\n\t"  // Unpack the src pixels
            "psubw         %%xmm1,    %%xmm0\n\t"  // Subtract dst from src
            "pmullw        %%xmm6,    %%xmm0\n\t"  // Multiply the result with alpha * 256
            "psllw             $8,    %%xmm1\n\t"  // Multiply dst with 256
            "paddw         %%xmm1,    %%xmm0\n\t"  // Add dst to the result
            "psrlw             $8,    %%xmm0\n\t"  // Divide by 256

            // Blend the next two pixels
            "punpcklbw     %%xmm7,    %%xmm3\n\t"  // Unpack the dst pixels
            "punpcklbw     %%xmm7,    %%xmm2\n\t"  // Unpack the src pixels
            "psubw         %%xmm3,    %%xmm2\n\t"  // Subtract dst from src
            "pmullw        %%xmm6,    %%xmm2\n\t"  // Multiply the result with alpha * 256
            "psllw             $8,    %%xmm3\n\t"  // Multiply dst with 256
            "paddw         %%xmm3,    %%xmm2\n\t"  // Add dst to the result
            "psrlw             $8,    %%xmm2\n\t"  // Divide by 256

            // Write the pixels back to the image
            "packuswb      %%xmm2,    %%xmm0\n\t"  // Pack the pixels to a double qword
            "movdqa        %%xmm0, (%1,%2,4)\n\t"  // Store the pixels
            : : "r"(data1), "r"(data2), "r"(i) );
        }

        // Cleanup loop
        for ( int i = pixels; i < pixels + remainder; i++ ) {
            __asm__ __volatile__(
            "movd       (%1,%2,4),    %%xmm1\n\t"  // Load one dst pixel to XMM1
            "punpcklbw     %%xmm7,    %%xmm1\n\t"  // Unpack the pixel
            "movd       (%0,%2,4),    %%xmm0\n\t"  // Load one src pixel to XMM0
            "punpcklbw     %%xmm7,    %%xmm0\n\t"  // Unpack the pixel
            "psubw         %%xmm1,    %%xmm0\n\t"  // Subtract dst from src
            "pmullw        %%xmm6,    %%xmm0\n\t"  // Multiply the result with alpha * 256
            "psllw             $8,    %%xmm1\n\t"  // Multiply dst with 256
            "paddw         %%xmm1,    %%xmm0\n\t"  // Add dst to result
            "psrlw             $8,    %%xmm0\n\t"  // Divide by 256
            "packuswb      %%xmm1,    %%xmm0\n\t"  // Pack the pixel to a dword
            "movd          %%xmm0, (%1,%2,4)\n\t"  // Write the pixel to the image
            : : "r"(data1), "r"(data2), "r"(i) );
        }
    } else
#endif // USE_SSE2_INLINE_ASM

#ifdef USE_MMX_INLINE_ASM
    if ( KCPUInfo::haveExtension( KCPUInfo::IntelMMX ) && pixels > 1 ) {
        Q_UINT16 alpha = Q_UINT16( opacity * 256.0 );
        KIE4Pack packedalpha = { { alpha, alpha, alpha, 0 } };

        // Prepare the MM6 and MM7 registers for blending and unpacking
        __asm__ __volatile__(
        "pxor       %%mm7,   %%mm7\n\t"      // Zero out MM7 for unpacking
        "movq        (%0),   %%mm6\n\t"      // Set up alpha * 256 in MM6
        : : "r"(&packedalpha), "m"(packedalpha) );

        Q_UINT32 *data1 = reinterpret_cast<Q_UINT32*>( src.bits() );
        Q_UINT32 *data2 = reinterpret_cast<Q_UINT32*>( dst.bits() );

        // The main loop processes 2 pixels / iteration
        int remainder = pixels % 2;
        pixels -= remainder;

        // Main loop
        for ( int i = 0; i < pixels; i += 2 ) {
            __asm__ __volatile__(
            // Load 2 src pixels to MM0 and MM2 and 2 dst pixels to MM1 and MM3
            "movd        (%0,%2,4),     %%mm0\n\t"  // Load the 1st src pixel to MM0
            "movd        (%1,%2,4),     %%mm1\n\t"  // Load the 1st dst pixel to MM1
            "movd       4(%0,%2,4),     %%mm2\n\t"  // Load the 2nd src pixel to MM2
            "movd       4(%1,%2,4),     %%mm3\n\t"  // Load the 2nd dst pixel to MM3

            // Blend the first pixel
            "punpcklbw       %%mm7,     %%mm0\n\t"  // Unpack the src pixel
            "punpcklbw       %%mm7,     %%mm1\n\t"  // Unpack the dst pixel
            "psubw           %%mm1,     %%mm0\n\t"  // Subtract dst from src
            "pmullw          %%mm6,     %%mm0\n\t"  // Multiply the result with alpha * 256
            "psllw              $8,     %%mm1\n\t"  // Multiply dst with 256
            "paddw           %%mm1,     %%mm0\n\t"  // Add dst to the result
            "psrlw              $8,     %%mm0\n\t"  // Divide by 256

            // Blend the second pixel
            "punpcklbw       %%mm7,     %%mm2\n\t"  // Unpack the src pixel
            "punpcklbw       %%mm7,     %%mm3\n\t"  // Unpack the dst pixel
            "psubw           %%mm3,     %%mm2\n\t"  // Subtract dst from src
            "pmullw          %%mm6,     %%mm2\n\t"  // Multiply the result with alpha * 256
            "psllw              $8,     %%mm3\n\t"  // Multiply dst with 256
            "paddw           %%mm3,     %%mm2\n\t"  // Add dst to the result
            "psrlw              $8,     %%mm2\n\t"  // Divide by 256

            // Write the pixels back to the image
            "packuswb        %%mm2,     %%mm0\n\t"  // Pack the pixels to a qword
            "movq            %%mm0, (%1,%2,4)\n\t"  // Store the pixels
            : : "r"(data1), "r"(data2), "r"(i) );
        }

        // Blend the remaining pixel (if there is one)
        if ( remainder ) {
             __asm__ __volatile__(
            "movd             (%0),     %%mm0\n\t"  // Load one src pixel to MM0
            "punpcklbw       %%mm7,     %%mm0\n\t"  // Unpack the src pixel
            "movd             (%1),     %%mm1\n\t"  // Load one dst pixel to MM1
            "punpcklbw       %%mm7,     %%mm1\n\t"  // Unpack the dst pixel
            "psubw           %%mm1,     %%mm0\n\t"  // Subtract dst from src
            "pmullw          %%mm6,     %%mm0\n\t"  // Multiply the result with alpha * 256
            "psllw              $8,     %%mm1\n\t"  // Multiply dst with 256
            "paddw           %%mm1,     %%mm0\n\t"  // Add dst to result
            "psrlw              $8,     %%mm0\n\t"  // Divide by 256
            "packuswb        %%mm0,     %%mm0\n\t"  // Pack the pixel to a dword
            "movd            %%mm0,      (%1)\n\t"  // Write the pixel to the image
            : : "r"(data1 + pixels), "r"(data2 + pixels) );
        }

        // Empty the MMX state
        __asm__ __volatile__("emms");
    } else
#endif // USE_MMX_INLINE_ASM

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
void EMMPMGraphicsView::setImageDisplayType(int displayType)
{
  m_ImageDisplayType = (EmMpm_Constants::ImageDisplayType)displayType;
  QPainter painter;
  QImage paintImage(m_BaseImage.size(), QImage::Format_ARGB32_Premultiplied);
  QPoint point(0, 0);
  painter.begin(&paintImage);
  painter.setPen(Qt::NoPen);
  if (displayType == EmMpm_Constants::OriginalImage)
  {
    painter.drawImage(point, m_BaseImage);
  }
  else if (displayType == EmMpm_Constants::SegmentedImage)
  {
    painter.drawImage(point, m_OverlayImage);
  }
  else if (displayType == EmMpm_Constants::CompositedImage)
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

  m_ImageDisplayType = EmMpm_Constants::SegmentedImage;

  setImageDisplayType(m_ImageDisplayType);

  emit fireOverlayImageFileLoaded(filename);
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
 // centerOn(m_ImageGraphicsItem);

  m_ImageDisplayType = EmMpm_Constants::SegmentedImage;

  setImageDisplayType(m_ImageDisplayType);

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

  // Show a dialog to let the user set the values
  UserInitAreaDialog initDialog;
  initDialog.getUserInitAreaWidget()->setUserInitArea(userInitArea);
  int ret =initDialog.exec();
  if (ret == QDialog::Accepted)
  {
    m_UserInitAreaVector->push_back(userInitArea);

    connect (userInitArea, SIGNAL(fireUserInitAreaAboutToDelete(UserInitArea*)),
             m_MainGui, SLOT(deleteUserInitArea(UserInitArea*)), Qt::QueuedConnection);

    connect (userInitArea, SIGNAL (fireUserInitAreaUpdated(UserInitArea*)),
             m_MainGui, SLOT(userInitAreaUpdated(UserInitArea*)), Qt::QueuedConnection);

    connect (userInitArea, SIGNAL(fireUserInitAreaSelected(UserInitArea*)),
             m_MainGui, SLOT(userInitAreaSelected(UserInitArea*)), Qt::QueuedConnection);

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
    case 11: m_composition_mode = QPainter::CompositionMode_SourceOver; break;


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
  default:
    m_composition_mode = QPainter::CompositionMode_Exclusion; break;
  }

  this->setImageDisplayType(m_ImageDisplayType);
}



