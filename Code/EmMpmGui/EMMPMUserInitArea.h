#ifndef _EMMPM_USER_INIT_AREA_H_
#define _EMMPM_USER_INIT_AREA_H_


#include <QtGui/QBrush>
#include <QtGui/QGraphicsPolygonItem>
#include <QtGui/QPen>


class QDataStream;
class QGraphicsScene;
class QGraphicsSceneMouseEvent;
class QKeyEvent;

const int EMMPMUserInitAreaType = QGraphicsItem::UserType + 1;

class EMMPMUserInitArea : public QObject, public QGraphicsRectItem
{
    Q_OBJECT
#ifdef ANIMATE_ALIGNMENT
    Q_PROPERTY(QPointF pos READ pos WRITE setPos)
#endif
    Q_PROPERTY(QBrush brush READ brush WRITE setBrush)
    Q_PROPERTY(QPen pen READ pen WRITE setPen)
//    Q_PROPERTY(double angle READ angle WRITE setAngle)
//    Q_PROPERTY(double shearHorizontal READ shearHorizontal WRITE setShearHorizontal)
//    Q_PROPERTY(double shearVertical READ shearVertical WRITE setShearVertical)

public:
    enum {Type = EMMPMUserInitAreaType};

    explicit EMMPMUserInitArea(const QRectF &rect, QGraphicsScene *scene);
    int type() const { return Type; }

//    double angle() const { return m_angle; }
//    double shearHorizontal() const { return m_shearHorizontal; }
//    double shearVertical() const { return m_shearVertical; }

signals:
    void dirty();

public slots:
    void setPen(const QPen &pen);
    void setBrush(const QBrush &brush);
//    void setAngle(double angle);
//    void setShearHorizontal(double shearHorizontal)
//        { setShear(shearHorizontal, m_shearVertical); }
//    void setShearVertical(double shearVertical)
//        { setShear(m_shearHorizontal, shearVertical); }
//    void setShear(double shearHorizontal, double shearVertical);

protected:
    QVariant itemChange(GraphicsItemChange change,
                        const QVariant &value);
    void keyPressEvent(QKeyEvent *event);
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);

private:
    void updateTransform();

    bool m_resizing;
//    double m_angle;
//    double m_shearHorizontal;
//    double m_shearVertical;
};


QDataStream &operator<<(QDataStream &out, const EMMPMUserInitArea &boxItem);
QDataStream &operator>>(QDataStream &in, EMMPMUserInitArea &boxItem);


#endif // _EMMPM_USER_INIT_AREA_H_
