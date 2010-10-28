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

#include "EMMPMUserInitArea.h"


#include <QtGui/QGraphicsScene>
#include <QtGui/QGraphicsSceneMouseEvent>
#include <QtGui/QPainter>
#include <QtGui/QStyleOption>
#include <QtGui/QMenu>
#include <QtGui/QKeyEvent>

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
EMMPMUserInitArea::EMMPMUserInitArea(const QRectF &bounds) :
m_isResizing(false)
{
    setFlag(QGraphicsItem::ItemIsMovable, true);
    setFlag(QGraphicsItem::ItemIsSelectable, true);
    setFlag(QGraphicsItem::ItemIsFocusable, true);
    setAcceptHoverEvents(true);
    setPos(bounds.x(), bounds.y());
    m_BoundingRect = bounds;


 //   m_startTime = QTime::currentTime();
}

EMMPMUserInitArea::~EMMPMUserInitArea()
{
}

QRectF EMMPMUserInitArea::boundingRect() const
{
    return m_BoundingRect;
}

void EMMPMUserInitArea::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *)
{
    if (option->state & QStyle::State_Selected)
    {
        painter->setRenderHint(QPainter::Antialiasing, true);
        if (option->state & QStyle::State_HasFocus)
            painter->setPen(Qt::yellow);
        else
            painter->setPen(Qt::white);
        painter->drawRect(boundingRect());

        int m_size = m_BoundingRect.width();
        painter->drawLine(m_size / 2 - 9, m_size / 2, m_size / 2, m_size / 2 - 9);
        painter->drawLine(m_size / 2 - 6, m_size / 2, m_size / 2, m_size / 2 - 6);
        painter->drawLine(m_size / 2 - 3, m_size / 2, m_size / 2, m_size / 2 - 3);

        painter->setRenderHint(QPainter::Antialiasing, false);
    }
    else // Just draw the outline
    {
      painter->setPen(m_Pen);
      painter->setBrush(m_Brush);
      painter->drawRect(m_BoundingRect);
    }




}

void EMMPMUserInitArea::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    if (!isSelected() && scene()) {
        scene()->clearSelection();
        setSelected(true);
    }

    QMenu menu;
    QAction *delAction = menu.addAction("Delete");
    QAction *newAction = menu.addAction("New");
    QAction *growAction = menu.addAction("Grow");
    QAction *shrinkAction = menu.addAction("Shrink");

    QAction *selectedAction = menu.exec(event->screenPos());

    if (selectedAction == delAction)
        deleteSelectedItems(scene());
    else if (selectedAction == newAction)
        duplicateSelectedItems(scene());
    else if (selectedAction == growAction)
        growSelectedItems(scene());
    else if (selectedAction == shrinkAction)
        shrinkSelectedItems(scene());
}

void EMMPMUserInitArea::duplicateSelectedItems(QGraphicsScene *scene)
{
    if (!scene)
        return;
//TODO This needs to be fully implemeted
#if 0
    QList<QGraphicsItem *> selected;
    selected = scene->selectedItems();

    foreach (QGraphicsItem *item, selected)
    {
      EMMPMUserInitArea *itemBase = qgraphicsitem_cast<EMMPMUserInitArea *>(item);
      if (itemBase)
      scene->addItem(itemBase->createNew(itemBase->m_size, itemBase->pos().x() + itemBase->m_size, itemBase->pos().y()));
    }
#endif

}

void EMMPMUserInitArea::deleteSelectedItems(QGraphicsScene *scene)
{
    if (!scene)
        return;

    QList<QGraphicsItem *> selected;
    selected = scene->selectedItems();

    foreach (QGraphicsItem *item, selected) {
        EMMPMUserInitArea *itemBase = qgraphicsitem_cast<EMMPMUserInitArea *>(item);
        if (itemBase)
            delete itemBase;
    }
}

void EMMPMUserInitArea::growSelectedItems(QGraphicsScene *scene)
{
    if (!scene)
        return;
//TODO: This needs to be fully implemented
#if 0
    QList<QGraphicsItem *> selected;
    selected = scene->selectedItems();

    foreach (QGraphicsItem *item, selected)
    {
      EMMPMUserInitArea *itemBase = qgraphicsitem_cast<EMMPMUserInitArea *>(item);
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

void EMMPMUserInitArea::shrinkSelectedItems(QGraphicsScene *scene)
{
    if (!scene)
        return;

    //TODO: This needs to be fully implemented
#if 0
    QList<QGraphicsItem *> selected;
    selected = scene->selectedItems();

    foreach (QGraphicsItem *item, selected)
    {
      EMMPMUserInitArea *itemBase = qgraphicsitem_cast<EMMPMUserInitArea *>(item);
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

void EMMPMUserInitArea::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (m_isResizing) {
        int dx = int(2.0 * event->pos().x());
        int dy = int(2.0 * event->pos().y());
        prepareGeometryChange();
        m_BoundingRect.setWidth(m_BoundingRect.width() + dy);
        m_BoundingRect.setHeight(m_BoundingRect.height() + dx);
#if 0
        m_size = (dx > dy ? dx : dy);
        if (m_size < MIN_ITEM_SIZE)
        m_size = MIN_ITEM_SIZE;
        else if (m_size > MAX_ITEM_SIZE)
        m_size = MAX_ITEM_SIZE;
#endif

    } else {
        QGraphicsItem::mouseMoveEvent(event);
    }
}

void EMMPMUserInitArea::hoverMoveEvent(QGraphicsSceneHoverEvent *event)
{
    if (m_isResizing || (isInResizeArea(event->pos()) && isSelected()))
        setCursor(Qt::SizeFDiagCursor);
    else
        setCursor(Qt::ArrowCursor);
    QGraphicsItem::hoverMoveEvent(event);
}

void EMMPMUserInitArea::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    static qreal z = 0.0;
    setZValue(z += 1.0);
    if (event->button() == Qt::LeftButton && isInResizeArea(event->pos())) {
        m_isResizing = true;
    } else {
        QGraphicsItem::mousePressEvent(event);
    }
}

void EMMPMUserInitArea::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && m_isResizing) {
        m_isResizing = false;
    } else {
        QGraphicsItem::mouseReleaseEvent(event);
    }
}

void EMMPMUserInitArea::keyPressEvent(QKeyEvent *event)
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

#if 0
void EMMPMUserInitArea::wheelEvent(QGraphicsSceneWheelEvent *event)
{
  prepareGeometryChange();
  m_size = int(m_size * exp(-event->delta() / 600.0));
  if (m_size > MAX_ITEM_SIZE)
  m_size = MAX_ITEM_SIZE;
  else if (m_size < MIN_ITEM_SIZE)
  m_size = MIN_ITEM_SIZE;
}
#endif


int EMMPMUserInitArea::type() const
{
    return Type;
}


bool EMMPMUserInitArea::isInResizeArea(const QPointF &pos)
{
  //TODO: Fix this with real areas
  int m_size = m_BoundingRect.width();
    return (-pos.y() < pos.x() - m_size + 9);
}
