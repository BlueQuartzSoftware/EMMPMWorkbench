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
  m_ImageGraphicsItem = NULL;

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
        loadImageFile(fName);
      }
    }
  }
  event->acceptProposedAction();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EMMPMGraphicsView::loadImageFile(const QString &filename)
{
  QGraphicsScene* gScene = scene();


  if (gScene == NULL)
  {
    gScene = new QGraphicsScene(this);
    setScene(gScene);
  }

  if (NULL != m_ImageGraphicsItem)
  {
    QList<QGraphicsItem* > items;
    items = m_ImageGraphicsItem->children();
    foreach (QGraphicsItem *item, items)
      {
        UserInitArea *itemBase = qgraphicsitem_cast<UserInitArea * > (item);
        if (itemBase)
        {
          m_MainGui->deleteUserInitArea(itemBase);
          gScene->removeItem(itemBase);
          m_UserInitAreaTableModel->deleteUserInitArea(itemBase);
          delete itemBase;
        }

      }

    gScene->removeItem(m_ImageGraphicsItem); //Remove the image that is displaying
    m_ImageGraphicsItem->setParentItem(NULL); // Set the parent to NULL
    delete m_ImageGraphicsItem; // Delete the object
  }


  m_CurrentImage = QImage(filename);
  if (m_CurrentImage.isNull() == true)
  {
    //TODO: Show some sort of error message
    return;
  }
  QVector<QRgb> colorTable(256);
  for (quint32 i = 0; i < 256; ++i)
  {
    colorTable[i] = qRgb(i, i, i);
  }
  m_CurrentImage.setColorTable(colorTable);

  QPixmap imagePixmap = QPixmap::fromImage(m_CurrentImage);
  m_ImageGraphicsItem = gScene->addPixmap(imagePixmap); // Add the new image into the display
  m_ImageGraphicsItem->setAcceptDrops(true);
  m_ImageGraphicsItem->setZValue(-1);
  QRectF rect = m_ImageGraphicsItem->boundingRect();
  gScene->setSceneRect(rect);
  centerOn(m_ImageGraphicsItem);

  emit fireImageFileLoaded(filename);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QImage EMMPMGraphicsView::getCurrentImage()
{
  return m_CurrentImage;
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
