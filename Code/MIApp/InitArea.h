

#ifndef _INIT_AREA_H_
#define _INIT_AREA_H_

#include <QtGui>

QT_FORWARD_DECLARE_CLASS(QBypassWidget)

class InitArea : public QObject
{
    Q_OBJECT
public:
    enum PointShape {
        CircleShape,
        RectangleShape
    };

//    enum LockType {
//        LockToLeft   = 0x01,
//        LockToRight  = 0x02,
//        LockToTop    = 0x04,
//        LockToBottom = 0x08
//    };
//
//    enum SortType {
//        NoSort,
//        XSort,
//        YSort
//    };
//
//    enum ConnectionType {
//        NoConnection,
//        LineConnection,
//        CurveConnection
//    };

    InitArea(QWidget *widget, PointShape shape);

    bool eventFilter(QObject *object, QEvent *event);

    void paintPoints();
    void paintPoints(QImage &image);

    inline QRectF boundingRect() const;
    void setBoundingRect(const QRectF &boundingRect) { m_bounds = boundingRect; }

    QPointF point() const { return m_point; }
    void setPoint(const QPointF &point) { m_point = point;}

    QSizeF pointSize() const { return m_pointSize; }
    void setPointSize(const QSizeF &size) { m_pointSize = size; }

//    SortType sortType() const { return m_sortType; }
//    void setSortType(SortType sortType) { m_sortType = sortType; }
//
//    ConnectionType connectionType() const { return m_connectionType; }
//    void setConnectionType(ConnectionType connectionType) { m_connectionType = connectionType; }
//
//    void setConnectionPen(const QPen &pen) { m_connectionPen = pen; }
    void setShapePen(const QPen &pen) { m_pointPen = pen; }
    void setShapeBrush(const QBrush &brush) { m_pointBrush = brush; }

 //   void setPointLock(int pos, LockType lock) { m_locks[pos] = lock; }

    void setEditable(bool editable) { m_editable = editable; }
    bool editable() const { return m_editable; }

    void movePoint(const QPointF &newPos, bool emitChange = true);
    void unboundedMovePoint( const QPointF &newPos, bool emitChange);

public slots:
    void setEnabled(bool enabled);
    void setDisabled(bool disabled) { setEnabled(!disabled); }

signals:
    void pointChanged(const QPointF &point);

public:
    void firePointChange();

private:
    inline QRectF pointBoundingRect() const;


    QWidget *m_widget;

    QPointF m_point;
    QRectF m_bounds;
    PointShape m_shape;
//    SortType m_sortType;
//    ConnectionType m_connectionType;
//
//    QVector<uint> m_locks;

    QSizeF m_pointSize;
 //   int m_currentIndex;
    bool m_editable;
    bool m_enabled;

    QPen m_pointPen;
    QBrush m_pointBrush;
    QPen m_connectionPen;
};


inline QRectF InitArea::pointBoundingRect() const
{
    QPointF p = m_point;
    qreal w = m_pointSize.width();
    qreal h = m_pointSize.height();
    qreal x = p.x() - w / 2;
    qreal y = p.y() - h / 2;
    return QRectF(x, y, w, h);
}

inline QRectF InitArea::boundingRect() const
{
    if (m_bounds.isEmpty())
        return m_widget->rect();
    else
        return m_bounds;
}

#endif // _INIT_AREA_H_
