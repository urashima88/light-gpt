#include "connectionitem.h"
#include "componentport.h"
#include "componentitem.h"
#include <QPainterPath>
#include <QGraphicsScene>
#include <QMenu>
#include <QKeyEvent>
#include <QGraphicsSceneMouseEvent>
#include <QTimer>

ConnectionItem::ConnectionItem(ComponentPort* startPort, ComponentPort* endPort,
                               QGraphicsItem* parent)
    : QGraphicsPathItem(parent)
    , m_startPort(startPort)
    , m_endPort(endPort)
{
    Q_ASSERT(m_startPort && m_endPort);
    setPen(QPen(Qt::white, 2.5, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    setBrush(Qt::NoBrush);
    setZValue(0.5);
    setFlag(QGraphicsItem::ItemIsSelectable, true);
    updatePath();
}

void ConnectionItem::updatePath()
{
    if (!m_startPort || !m_endPort) return;

    QPointF start = m_startPort->scenePos();
    QPointF end = m_endPort->scenePos();

    QPainterPath path;
    path.moveTo(start);
    qreal dx = qAbs(end.x() - start.x()) * 0.5;
    QPointF ctrl1(start.x() + dx, start.y());
    QPointF ctrl2(end.x() - dx, end.y());
    path.cubicTo(ctrl1, ctrl2, end);
    setPath(path);
}

void ConnectionItem::setConnectionColor(const QColor& color)
{
    QPen p = pen();
    p.setColor(color);
    setPen(p);
}

void ConnectionItem::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    if (event->modifiers() & Qt::ControlModifier) {
        if (auto* scene = this->scene()) {
            scene->removeItem(this);
        }
        QTimer::singleShot(0, [this]() { delete this; });
        m_startPort->ownerComponent()->onConnectionRemoved(this);
        m_endPort->ownerComponent()->onConnectionRemoved(this);
        event->accept();
        return;
    }
    QGraphicsPathItem::mousePressEvent(event);
}

void ConnectionItem::contextMenuEvent(QGraphicsSceneContextMenuEvent* event)
{
    QMenu menu;
    QAction* deleteAction = menu.addAction("Delete");
    QAction* selected = menu.exec(event->screenPos());
    if (selected == deleteAction) {
        if (auto* scene = this->scene()) {
            scene->removeItem(this);
        }
        QTimer::singleShot(0, [this]() { delete this; });
        m_startPort->ownerComponent()->onConnectionRemoved(this);
        m_endPort->ownerComponent()->onConnectionRemoved(this);
    }
}