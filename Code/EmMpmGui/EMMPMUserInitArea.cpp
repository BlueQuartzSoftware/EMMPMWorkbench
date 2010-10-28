/*
    Copyright (c) 2009-10 Qtrac Ltd. All rights reserved.

    This program or module is free software: you can redistribute it
    and/or modify it under the terms of the GNU General Public License
    as published by the Free Software Foundation, either version 3 of
    the License, or (at your option) any later version. It is provided
    for educational purposes and is distributed in the hope that it will
    be useful, but WITHOUT ANY WARRANTY; without even the implied
    warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See
    the GNU General Public License for more details.
*/

//#include "alt_key.hpp"
//#include "aqp.hpp"
//#include "global.hpp"

#include "EMMPMUserInitArea.h"

#include <QtGui/QGraphicsScene>
#include <QtGui/QGraphicsSceneMouseEvent>
#include <QtGui/QKeyEvent>
#include <QtGui/QCursor>


EMMPMUserInitArea::EMMPMUserInitArea(const QRectF &rect_, QGraphicsScene *scene)
    : QObject(), QGraphicsRectItem(), m_resizing(false)
{
    setFlags(QGraphicsItem::ItemIsSelectable|
#if QT_VERSION >= 0x040600
             QGraphicsItem::ItemSendsGeometryChanges|
#endif
             QGraphicsItem::ItemIsMovable|
             QGraphicsItem::ItemIsFocusable);
    setPos(rect_.center());
//    setRect(QRectF(QPointF(-rect_.width() / 2.0,
//                           -rect_.height() / 2.0), rect_.size()));
    setRect(rect_);
    scene->clearSelection();
    scene->addItem(this);
    setSelected(true);
    setFocus();
}


void EMMPMUserInitArea::setPen(const QPen &pen_)
{
    if (isSelected() && pen_ != pen()) {
        QGraphicsRectItem::setPen(pen_);
        emit dirty();
    }
}


void EMMPMUserInitArea::setBrush(const QBrush &brush_)
{
    if (isSelected() && brush_ != brush()) {
        QGraphicsRectItem::setBrush(brush_);
        emit dirty();
    }
}


#if 0
void EMMPMUserInitArea::setAngle(double angle)
{
  if (isSelected() && !qFuzzyCompare(m_angle, angle))
  {
    m_angle = angle;
    updateTransform();
  }
}

void EMMPMUserInitArea::setShear(double shearHorizontal, double shearVertical)
{
  if (isSelected() &&
      (!qFuzzyCompare(m_shearHorizontal, shearHorizontal) ||
          !qFuzzyCompare(m_shearVertical, shearVertical)))
  {
    m_shearHorizontal = shearHorizontal;
    m_shearVertical = shearVertical;
    updateTransform();
  }
}
#endif



void EMMPMUserInitArea::updateTransform()
{
    QTransform transform;
//    transform.shear(m_shearHorizontal, m_shearVertical);
//    transform.rotate(m_angle);
    setTransform(transform);
}


QVariant EMMPMUserInitArea::itemChange(GraphicsItemChange change,
                             const QVariant &value)
{
//    if (isDirtyChange(change))
//        emit dirty();
    return QGraphicsRectItem::itemChange(change, value);
}


void EMMPMUserInitArea::keyPressEvent(QKeyEvent *event)
{
    if (event->modifiers() & Qt::ShiftModifier ||
        event->modifiers() & Qt::ControlModifier) {
        bool move = event->modifiers() & Qt::ControlModifier;
        bool changed = true;
        double dx1 = 0.0;
        double dy1 = 0.0;
        double dx2 = 0.0;
        double dy2 = 0.0;
        switch (event->key()) {
            case Qt::Key_Left:
                if (move)
                    dx1 = -1.0;
                dx2 = -1.0;
                break;
            case Qt::Key_Right:
                if (move)
                    dx1 = 1.0;
                dx2 = 1.0;
                break;
            case Qt::Key_Up:
                if (move)
                    dy1 = -1.0;
                dy2 = -1.0;
                break;
            case Qt::Key_Down:
                if (move)
                    dy1 = 1.0;
                dy2 = 1.0;
                break;
            default:
                changed = false;
        }
        if (changed) {
            setRect(rect().adjusted(dx1, dy1, dx2, dy2));
            event->accept();
            emit dirty();
            return;
        }
    }
    QGraphicsRectItem::keyPressEvent(event);
}


void EMMPMUserInitArea::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->modifiers() & Qt::ShiftModifier) {
        m_resizing = true;
        setCursor(Qt::SizeAllCursor);
    }
    else
        QGraphicsRectItem::mousePressEvent(event);
}


void EMMPMUserInitArea::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (m_resizing) {
#ifdef ALTERNATIVE_RESIZING
        qreal dx = event->pos().x() - event->lastPos().x();
        qreal dy = event->pos().y() - event->lastPos().y();
        setRect(rect().adjusted(0, 0, dx, dy).normalized());
#else
        QRectF rectangle(rect());
        if (event->pos().x() < rectangle.x())
            rectangle.setBottomLeft(event->pos());
        else
            rectangle.setBottomRight(event->pos());
        setRect(rectangle);
#endif
        scene()->update();
    }
    else
        QGraphicsRectItem::mouseMoveEvent(event);
}


void EMMPMUserInitArea::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if (m_resizing) {
        m_resizing = false;
        setCursor(Qt::ArrowCursor);
        emit dirty();
    }
    else
        QGraphicsRectItem::mouseReleaseEvent(event);
}


QDataStream &operator<<(QDataStream &out, const EMMPMUserInitArea &boxItem)
{
    out << boxItem.pos()
//        << boxItem.angle()
//        << boxItem.shearHorizontal() << boxItem.shearVertical()
        << boxItem.zValue() << boxItem.rect() << boxItem.pen()
        << boxItem.brush();
    return out;
}


QDataStream &operator>>(QDataStream &in, EMMPMUserInitArea &boxItem)
{
    QPointF position;
    double angle;
    double shearHorizontal;
    double shearVertical;
    double z;
    QRectF rect;
    QPen pen;
    QBrush brush;
    in >> position >> angle >> shearHorizontal >> shearVertical >> z
       >> rect >> pen >> brush;
    boxItem.setPos(position);
//    boxItem.setAngle(angle);
//    boxItem.setShear(shearHorizontal, shearVertical);
    boxItem.setZValue(z);
    boxItem.setRect(rect);
    boxItem.setPen(pen);
    boxItem.setBrush(brush);
    return in;
}
