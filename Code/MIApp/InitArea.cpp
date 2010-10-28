/* --------------------------------------------------------------------------*
* This source code has been cleared for public release by the                *
* US Air Force 88th Air Base Wing Public Affairs Office under                *
* case number 88ABW-2010-4857 on Sept. 7, 2010.                              *
* -------------------------------------------------------------------------- */
/****************************************************************************
**
** Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the demonstration applications of the Qt Toolkit.
**
** Commercial Usage
** Licensees holding valid Qt Commercial licenses may use this file in
** accordance with the Qt Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Nokia.
**
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License versions 2.0 or 3.0 as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file.  Please review the following information
** to ensure GNU General Public Licensing requirements will be met:
** http://www.fsf.org/licensing/licenses/info/GPLv2.html and
** http://www.gnu.org/copyleft/gpl.html.  In addition, as a special
** exception, Nokia gives you certain additional rights. These rights
** are described in the Nokia Qt GPL Exception version 1.3, included in
** the file GPL_EXCEPTION.txt in this package.
**
** Qt for Windows(R) Licensees
** As a special exception, Nokia, as the sole copyright holder for Qt
** Designer, grants users of the Qt/Eclipse Integration plug-in the
** right for the Qt/Eclipse Integration to link to functionality
** provided by Qt Designer and its related libraries.
**
** If you are unsure which license is appropriate for your use, please
** contact the sales department at qt-sales@nokia.com.
**
****************************************************************************/

#include "InitArea.h"
#include <iostream>
#define printf

InitArea::InitArea(QWidget *widget, PointShape shape)
    : QObject(widget)
{
    m_widget = widget;
    widget->installEventFilter(this);

//    m_connectionType = CurveConnection;
//    m_sortType = NoSort;
    m_shape = shape;
    m_pointPen = QPen(QColor(255, 255, 255, 191), 1);
    m_connectionPen = QPen(QColor(255, 255, 255, 127), 2);
    m_pointBrush = QBrush(QColor(191, 191, 191, 127));
    m_pointSize = QSize(8, 8);

    m_editable = true;
    m_enabled = true;

    connect(this, SIGNAL(pointChanged(const QPointF &)),
            m_widget, SLOT(update()));
}


void InitArea::setEnabled(bool enabled)
{
    if (m_enabled != enabled) {
        m_enabled = enabled;
        m_widget->update();
    }
}


bool InitArea::eventFilter(QObject *object, QEvent *event)
{
    if (object == m_widget && m_enabled) {
        switch (event->type()) {

        case QEvent::MouseButtonPress:
        {
            QMouseEvent *me = (QMouseEvent *) event;

            QPointF clickPos = me->pos();
          //  int index = -1;
          //  for (int i=0; i<m_point.size(); ++i) {
                QPainterPath path;
                if (m_shape == CircleShape)
                    path.addEllipse(pointBoundingRect());
                else
                    path.addRect(pointBoundingRect());

                if (path.contains(clickPos)) {
                  //  index = i;
                    break;
                }
           // }

            if (me->button() == Qt::LeftButton) {
//                if (index == -1) {
                    if (!m_editable)
                        return false;
                    int pos = 0;
//                    // Insert sort for x or y
//                    if (m_sortType == XSort) {
//                        for (int i=0; i<m_point.size(); ++i)
//                            if (m_point.at(i).x() > clickPos.x()) {
//                                pos = i;
//                                break;
//                            }
//                    } else if (m_sortType == YSort) {
//                        for (int i=0; i<m_point.size(); ++i)
//                            if (m_point.at(i).y() > clickPos.y()) {
//                                pos = i;
//                                break;
//                            }
//                    }
//
//                    m_point.insert(pos, clickPos);
//                    m_locks.insert(pos, 0);
//                    m_currentIndex = pos;
                    firePointChange();
//                } else {
//                    m_currentIndex = index;
//                }
                return true;

            } else if (me->button() == Qt::RightButton) {
                if (m_editable) {
//                    if (m_locks[index] == 0) {
//                        m_locks.remove(index);
//                        m_point.remove(index);
//                    }
                    //TODO: Delete the point by sending a signal?
                    firePointChange();
                    return true;
                }
            }

        }
        break;

        case QEvent::MouseButtonRelease:

            break;

        case QEvent::MouseMove:
            std::cout << "QEvent::MouseMove" << std::endl;
                movePoint( ((QMouseEvent *)event)->pos());
            break;

        case QEvent::Resize:
        {
          //std::cout << "QEvent::Resize" << std::endl;
            QResizeEvent *e = (QResizeEvent *) event;
            if (e->oldSize().width() == 0 || e->oldSize().height() == 0)
                break;
            qreal stretch_x = e->size().width() / qreal(e->oldSize().width());
            qreal stretch_y = e->size().height() / qreal(e->oldSize().height());
     //       for (int i=0; i<m_point.size(); ++i) {
                QPointF p = m_point;
                movePoint( QPointF(p.x() * stretch_x, p.y() * stretch_y), false);
       //     }

            firePointChange();
            break;
        }

        case QEvent::Paint:
        {
            QWidget *that_widget = m_widget;
            m_widget = 0;
            QApplication::sendEvent(object, event);
            m_widget = that_widget;
            paintPoints();
            return true;
        }
        default:
            break;
        }
    }

    return false;
}


void InitArea::paintPoints()
{
    QPainter p;
    p.begin(m_widget);
    p.setRenderHint(QPainter::Antialiasing);

  //  if (m_connectionPen.style() != Qt::NoPen && m_connectionType != NoConnection) { }

    p.setPen(m_pointPen);
    p.setBrush(m_pointBrush);

   // for (int i=0; i<m_point.size(); ++i)
    {
        QRectF bounds = pointBoundingRect();
        if (m_shape == CircleShape)
            p.drawEllipse(bounds);
        else
            p.drawRect(bounds);
    }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void InitArea::paintPoints(QImage &image)
{
  QPainter p;
  p.begin(&image);
  p.setRenderHint(QPainter::Antialiasing);

  //if (m_connectionPen.style() != Qt::NoPen && m_connectionType != NoConnection) { }

  p.setPen(m_pointPen);
  p.setBrush(m_pointBrush);

 // for (int i=0; i<m_point.size(); ++i)
  {
      QRectF bounds = pointBoundingRect();
      if (m_shape == CircleShape)
          p.drawEllipse(bounds);
      else
          p.drawRect(bounds);
  }
  p.end();
}

static QPointF bound_point(const QPointF &point, const QRectF &bounds)
{
    QPointF p = point;

#if 0
    qreal left = bounds.left();
    qreal right = bounds.right();
    qreal top = bounds.top();
    qreal bottom = bounds.bottom();

    if (p.x() < left )
    {
      p.setX(left);
    }
    else if (p.x() > right )
    {
      p.setX(right);
    }

    if (p.y() < top )
    {
      p.setY(top);
    }
    else if (p.y() > bottom )
    {
      p.setY(bottom);
    }
#endif


    return p;
}

#if 0
void InitArea::setPoints(const QPolygonF &points)
{
  m_point.clear();
  for (int i=0; i<points.size(); ++i)
  m_point << bound_point(points.at(i), boundingRect(), 0);

  m_locks.clear();
  if (m_point.size() > 0)
  {
    m_locks.resize(m_point.size());
    m_locks.fill(0);
  }
}
#endif


// -----------------------------------------------------------------------------
//  index - The index into the QPloygonF to change
//  newPos - A 2D point in Pixels relative to the upper left corner of the image
//  emitChange - Should we emit an update through a signal
// -----------------------------------------------------------------------------
void InitArea::movePoint( const QPointF &newPos, bool emitChange)
{
 // std::cout << "HoverPoints::movePoint(): [" << index << "] " << newPos.x() << ", " << newPos.y() << std::endl;
  m_point = bound_point(newPos, boundingRect());
  if (emitChange)
      firePointChange();
}

// -----------------------------------------------------------------------------
//  index - The index into the QPloygonF to change
//  newPos - A 2D point in Pixels relative to the upper left corner of the image
//  emitChange - Should we emit an update through a signal
// -----------------------------------------------------------------------------
void InitArea::unboundedMovePoint( const QPointF &newPos, bool emitChange)
{
  //std::cout << "HoverPoints::unboundedMovePoint(): [" << index << "] " << newPos.x() << ", " << newPos.y() << std::endl;
  m_point = newPos;
  if (emitChange)
      firePointChange();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
inline static bool x_less_than(const QPointF &p1, const QPointF &p2)
{
    return p1.x() < p2.x();
}


inline static bool y_less_than(const QPointF &p1, const QPointF &p2)
{
    return p1.y() < p2.y();
}

void InitArea::firePointChange()
{
  //std::cout << "HoverPoints::firePointChange()" << std::endl;
#if 0
  if (m_sortType != NoSort)
  {

    QPointF oldCurrent;
    if (m_currentIndex != -1)
    {
      oldCurrent = m_point[m_currentIndex];
    }

    if (m_sortType == XSort)
    qSort(m_point.begin(), m_point.end(), x_less_than);
    else if (m_sortType == YSort)
    qSort(m_point.begin(), m_point.end(), y_less_than);

    // Compensate for changed order...
    if (m_currentIndex != -1)
    {
      for (int i = 0; i < m_point.size(); ++i)
      {
        if (m_point[i] == oldCurrent)
        {
          m_currentIndex = i;
          break;
        }
      }
    }
    //  printf(" - firePointChange(), current=%d\n", m_currentIndex);
  }
#endif


  //     for (int i=0; i<m_point.size(); ++i) {
  //         printf(" - point(%2d)=[%.2f, %.2f], lock=%d\n",
  //                i, m_point.at(i).x(), m_point.at(i).y(), m_locks.at(i));
  //     }
 // std::cout << "About to emit 'pointsChanged' " << std::endl;
  emit pointChanged(m_point);
}
