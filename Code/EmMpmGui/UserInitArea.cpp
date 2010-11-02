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
#include <iostream>


#include <QtGui/QGraphicsScene>
#include <QtGui/QGraphicsSceneMouseEvent>
#include <QtGui/QPainter>
#include <QtGui/QStyleOption>
#include <QtGui/QMenu>
#include <QtGui/QKeyEvent>
#include <QtCore/QRect>


#include "UserInitArea.h"
#include "UserInitAreaDialog.h"


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
UserInitArea::UserInitArea(int userIndex, const QPolygonF &polygon,  QGraphicsItem *parent) :
QGraphicsPolygonItem(polygon, parent),
m_EmMpmClass(userIndex)
{
  setFlag(QGraphicsItem::ItemIsMovable, true);
  setFlag(QGraphicsItem::ItemIsSelectable, true);
  setFlag(QGraphicsItem::ItemIsFocusable, true);
  //  setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
  setAcceptHoverEvents(true);
  m_isResizing = false;
  m_CurrentResizeHandle = UserInitArea::NO_CTRL_POINT;
  ctrlPointSize = 7.0f;
  m_GrayLevel = 255/16 * userIndex;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
UserInitArea::~UserInitArea()
{

}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void UserInitArea::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
  painter->setRenderHint(QPainter::Antialiasing, true);
  const int alpha = 128;
  if (option->state & QStyle::State_HasFocus)
  {
    painter->setPen(QPen(QColor(255, 25, 25, alpha), 3.0));
    painter->setBrush(brush());
  }
  else
  {
    painter->setPen(pen());
    painter->setBrush(brush());
  }

  painter->drawRect(boundingRect());
  if (option->state & QStyle::State_Selected)
  {
    float x = boundingRect().x();
    float y = boundingRect().y();
    float w = boundingRect().width();
    float h = boundingRect().height();

    painter->setPen(QPen(QColor(255, 25, 25, alpha)));
    painter->setBrush( QBrush(QColor(255, 25, 25, alpha)));
    //Upper Left
    painter->drawRect((int)x, (int)y, (int)ctrlPointSize, (int)ctrlPointSize);
    //Upper Right
    painter->drawRect((int)x + (int)w - (int)ctrlPointSize, (int)y, (int)ctrlPointSize, (int)ctrlPointSize);
    // Lower Right
    painter->drawRect((int)x + (int)w - (int)ctrlPointSize, (int)y + (int)h - (int)ctrlPointSize, (int)ctrlPointSize, (int)ctrlPointSize);
    // Lower Left
    painter->drawRect((int)x, (int)y + (int)h - (int)ctrlPointSize, (int)ctrlPointSize, (int)ctrlPointSize);
  }

  painter->setRenderHint(QPainter::Antialiasing, false);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void UserInitArea::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    if (!isSelected() && scene()) {
        scene()->clearSelection();
        setSelected(true);
    }

    QMenu menu;
    QAction *delAction = menu.addAction("Delete");
    QAction *propertiesAction = menu.addAction("Properties");
//    QAction *growAction = menu.addAction("Grow");
//    QAction *shrinkAction = menu.addAction("Shrink");

    QAction *selectedAction = menu.exec(event->screenPos());

    if (selectedAction == delAction)
        deleteSelectedItems(scene());
    else if (selectedAction == propertiesAction)
        propertiesSelectedItems(scene());
//    else if (selectedAction == growAction)
//        growSelectedItems(scene());
//    else if (selectedAction == shrinkAction)
//        shrinkSelectedItems(scene());
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void UserInitArea::propertiesSelectedItems(QGraphicsScene *scene)
{
    if (!scene)
        return;

    QList<QGraphicsItem *> selected;
    selected = scene->selectedItems();

    foreach (QGraphicsItem *item, selected)
    {
      UserInitArea *itemBase = qgraphicsitem_cast<UserInitArea *>(item);
      if (itemBase) {
        UserInitAreaDialog about(itemBase);
        about.exec();
      }
    }


}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void UserInitArea::duplicateSelectedItems(QGraphicsScene *scene)
{
    if (!scene)
        return;
//TODO This needs to be fully implemeted
#if 0
    QList<QGraphicsItem *> selected;
    selected = scene->selectedItems();

    foreach (QGraphicsItem *item, selected)
    {
      UserInitArea *itemBase = qgraphicsitem_cast<UserInitArea *>(item);
      if (itemBase)
      scene->addItem(itemBase->createNew(itemBase->m_size, itemBase->pos().x() + itemBase->m_size, itemBase->pos().y()));
    }
#endif

}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void UserInitArea::deleteSelectedItems(QGraphicsScene *scene)
{
    if (!scene)
        return;

    QList<QGraphicsItem *> selected;
    selected = scene->selectedItems();

    foreach (QGraphicsItem *item, selected) {
        UserInitArea *itemBase = qgraphicsitem_cast<UserInitArea *>(item);
        if (itemBase) {
            emit itemBase->fireUserInitAreaDeleted(itemBase);
            delete itemBase;
        }
    }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void UserInitArea::growSelectedItems(QGraphicsScene *scene)
{
    if (!scene)
        return;
//TODO: This needs to be fully implemented
#if 0
    QList<QGraphicsItem *> selected;
    selected = scene->selectedItems();

    foreach (QGraphicsItem *item, selected)
    {
      UserInitArea *itemBase = qgraphicsitem_cast<UserInitArea *>(item);
      if (itemBase)
      {
        itemBase->prepareGeometryChange();
        itemBase->m_size *= 2;
        if (itemBase->m_size > MAX_ITEM_SIZE)
        itemBase->m_size = MAX_ITEM_SIZE;
      }
    }
#endif

}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void UserInitArea::shrinkSelectedItems(QGraphicsScene *scene)
{
    if (!scene)
        return;

    //TODO: This needs to be fully implemented
#if 0
    QList<QGraphicsItem *> selected;
    selected = scene->selectedItems();

    foreach (QGraphicsItem *item, selected)
    {
      UserInitArea *itemBase = qgraphicsitem_cast<UserInitArea *>(item);
      if (itemBase)
      {
        itemBase->prepareGeometryChange();
        itemBase->m_size /= 2;
        if (itemBase->m_size < MIN_ITEM_SIZE)
        itemBase->m_size = MIN_ITEM_SIZE;
      }
    }
#endif

}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void UserInitArea::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
  static qreal z = 0.0;
  setZValue(z += 1.0);
  m_CurrentResizeHandle = isInResizeArea(event->pos());
  if (event->button() == Qt::LeftButton && m_CurrentResizeHandle != UserInitArea::NO_CTRL_POINT)
  {
  //  std::cout << "mousePressEvent m_isResizing = true" << std::endl;
    m_isResizing = true;
  }
  else
  {
    QGraphicsItem::mousePressEvent(event);
    emit fireUserInitAreaSelected(this);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void UserInitArea::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && m_isResizing) {
        m_isResizing = false;
    } else {
        QGraphicsItem::mouseReleaseEvent(event);
    }
    emit fireUserInitAreaUpdated(this);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void UserInitArea::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
  if (m_isResizing)
  {
    //      std::cout << "mouseMoveEvent m_isResizing = true" << std::endl;
    QPointF lastPos = event->lastScenePos();
    QPointF pos = event->scenePos();
    float deltaX = pos.x() - lastPos.x();
    float deltaY = pos.y() - lastPos.y();
    float x = boundingRect().x();
    float y = boundingRect().y();
    float w = boundingRect().width();
    float h = boundingRect().height();
    //        std::cout << "Delta(): " << deltaX << ", " << deltaY << std::endl;
  //  std::cout << "newRect: " << x << ", " << y << " (" << w << " x " << h << ")" << std::endl;
    QRectF newRect = boundingRect();
    // Move the upper left corner as it is grown
    if (m_CurrentResizeHandle == UserInitArea::UPPER_LEFT_CTRL_POINT)
    {
      newRect.setX(x + deltaX);
      newRect.setY(y + deltaY);
      newRect.setWidth(w - deltaX);
      newRect.setHeight(h - deltaY);
    }
    else if (m_CurrentResizeHandle == UserInitArea::UPPER_RIGHT_CTRL_POINT)
    {
      newRect.setY(y + deltaY);
      newRect.setWidth(w + deltaX);
    }
    else if (m_CurrentResizeHandle == UserInitArea::LOWER_LEFT_CTRL_POINT)
    {
      newRect.setX(x + deltaX);
      newRect.setHeight(h + deltaY);
    }
    else if (m_CurrentResizeHandle == UserInitArea::LOWER_RIGHT_CTRL_POINT)
    {
      newRect.setWidth(w + deltaX);
      newRect.setHeight(h + deltaY);
    }
    prepareGeometryChange();
    setPolygon(QPolygonF(newRect));
  }
  else
  {
    QGraphicsItem::mouseMoveEvent(event);
  }
  emit fireUserInitAreaUpdated(this);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void UserInitArea::hoverMoveEvent(QGraphicsSceneHoverEvent *event)
{
  UserInitArea::CTRL_POINTS pt = isInResizeArea(event->pos());
  if (m_isResizing || pt != UserInitArea::NO_CTRL_POINT && isSelected() )
  {

    if (pt == UPPER_LEFT_CTRL_POINT || pt == LOWER_RIGHT_CTRL_POINT)
    {
      setCursor(Qt::SizeFDiagCursor);
    }
    else if (pt == UPPER_RIGHT_CTRL_POINT || pt == LOWER_LEFT_CTRL_POINT)
    {
      setCursor(Qt::SizeBDiagCursor);
    }
  }
  else
  {
    setCursor(Qt::ArrowCursor);
  }
  QGraphicsItem::hoverMoveEvent(event);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void UserInitArea::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_Delete:
        deleteSelectedItems(scene());
        break;
    case Qt::Key_Insert:
        duplicateSelectedItems(scene());
        break;
    case Qt::Key_Plus:
        growSelectedItems(scene());
        break;
    case Qt::Key_Minus:
        shrinkSelectedItems(scene());
        break;
    default:
        QGraphicsItem::keyPressEvent(event);
        break;
    }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
UserInitArea::CTRL_POINTS UserInitArea::isInResizeArea(const QPointF &pos)
{
  float x = boundingRect().x();
  float y = boundingRect().y();
  float w = boundingRect().width();
  float h = boundingRect().height();


  QRectF upLeft(x, y, ctrlPointSize, ctrlPointSize);
  QRectF upRight(x + w - ctrlPointSize, y, ctrlPointSize, ctrlPointSize);
  QRectF lowRight(x + w - ctrlPointSize, y + h - ctrlPointSize, ctrlPointSize, ctrlPointSize);
  QRectF lowLeft(x, y + h - ctrlPointSize, ctrlPointSize, ctrlPointSize);

  if (upLeft.contains(pos))
  {
//    std::cout << "UPPER_LEFT_CTRL_POINT" << std::endl;
    return UPPER_LEFT_CTRL_POINT;
  }
  if (upRight.contains(pos))   {
//    std::cout << "UPPER_RIGHT_CTRL_POINT" << std::endl;
    return UPPER_RIGHT_CTRL_POINT;
  }
  if (lowRight.contains(pos))   {
 //   std::cout << "LOWER_RIGHT_CTRL_POINT" << std::endl;
    return LOWER_RIGHT_CTRL_POINT;
  }
  if (lowLeft.contains(pos))   {
//    std::cout << "LOWER_LEFT_CTRL_POINT" << std::endl;
    return LOWER_LEFT_CTRL_POINT;
  }
  return NO_CTRL_POINT;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int UserInitArea::type() const
{
    return Type;
}
