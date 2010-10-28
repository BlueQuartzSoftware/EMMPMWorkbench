#ifndef _EMMPM_USER_INIT_AREA_H_
#define _EMMPM_USER_INIT_AREA_H_

#include <QtCore/QRectF>
#include <QtGui/QGraphicsItem>
#include <QtGui/QBrush>
#include <QtGui/QPen>


class EMMPMUserInitArea : public QGraphicsItem
{
public:
    enum { Type = UserType + 1 };

    EMMPMUserInitArea(const QRectF &bounds);

    virtual ~EMMPMUserInitArea();

    virtual QRectF boundingRect() const;
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

    void setPen(const QPen &pen) { m_Pen = pen; }
    void setBrush(const QBrush &brush) { m_Brush = brush; }
protected:
    virtual EMMPMUserInitArea *createNew(const QRectF &bounds)
    {
      return new EMMPMUserInitArea(bounds);
    }
    virtual void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    virtual void hoverMoveEvent(QGraphicsSceneHoverEvent *event);
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
    virtual void keyPressEvent(QKeyEvent *event);
   // virtual void wheelEvent(QGraphicsSceneWheelEvent *event);
    virtual int type() const;
    bool isInResizeArea(const QPointF &pos);

    static void duplicateSelectedItems(QGraphicsScene *scene);
    static void deleteSelectedItems(QGraphicsScene *scene);
    static void growSelectedItems(QGraphicsScene *scene);
    static void shrinkSelectedItems(QGraphicsScene *scene);

private:
 //   int m_size;
//    QTime m_startTime;
    bool m_isResizing;
    QRectF m_BoundingRect;
    QPen m_Pen;
    QBrush m_Brush;
};

#endif // _EMMPM_USER_INIT_AREA_H_
