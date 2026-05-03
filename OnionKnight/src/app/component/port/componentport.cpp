#include "componentport.h"
#include "componentitem.h"
#include "connectionitem.h"
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QPainter>
#include <QPainterPath>
#include <QCursor>
#include <QPen>
#include <QBrush>
#include <QFont>
#include <QGraphicsSceneMouseEvent>

ComponentPort::ComponentPort(const QString& name, PortType type, ComponentItem* parentItem)
    : QGraphicsEllipseItem(parentItem)
    , m_name(name)
    , m_type(type)
    , m_owner(parentItem)
{
    setRect(-4, -4, 8, 8);
    setPen(QPen(Qt::darkGray, 1.5));
    setBrush(QBrush(type == Input ? QColor(100, 200, 100) : QColor(200, 100, 100)));
    setCursor(Qt::CrossCursor);
    setAcceptHoverEvents(true);
    setZValue(1);

    m_label = new QGraphicsTextItem(name, this);
    QFont font = m_label->font();
    font.setPointSize(7);
    m_label->setFont(font);
    m_label->setDefaultTextColor(Qt::white);
    adjustLabelPosition();
}

void ComponentPort::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        m_dragging = true;
        startNewConnection();
        event->accept();
    } else {
        QGraphicsEllipseItem::mousePressEvent(event);
    }
}

void ComponentPort::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
    if (m_dragging && m_tempCurve) {
        updateTempCurve(event->scenePos());
        event->accept();
    } else {
        QGraphicsEllipseItem::mouseMoveEvent(event);
    }
}

void ComponentPort::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
    if (m_dragging && m_tempCurve) {
        finishConnection(event->scenePos());
        m_dragging = false;
        event->accept();
    } else {
        QGraphicsEllipseItem::mouseReleaseEvent(event);
    }
}

void ComponentPort::startNewConnection()
{
    if (!scene()) return;
    m_tempCurve = new QGraphicsPathItem();
    QPen pen(Qt::white, 2.5, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    m_tempCurve->setPen(pen);
    m_tempCurve->setZValue(0);
    scene()->addItem(m_tempCurve);
}

void ComponentPort::updateTempCurve(const QPointF& scenePos)
{
    if (!m_tempCurve) return;
    QPointF start = mapToScene(0, 0);
    QPointF end = scenePos;
    QPainterPath path;
    path.moveTo(start);
    qreal dx = qAbs(end.x() - start.x()) * 0.5;
    QPointF ctrl1(start.x() + dx, start.y());
    QPointF ctrl2(end.x() - dx, end.y());
    path.cubicTo(ctrl1, ctrl2, end);
    m_tempCurve->setPath(path);
}

void ComponentPort::finishConnection(const QPointF& scenePos)
{
    if (m_tempCurve) {
        if (scene())
            scene()->removeItem(m_tempCurve);
        delete m_tempCurve;
        m_tempCurve = nullptr;
    }

    if (!scene()) return;

    QList<QGraphicsItem*> items = scene()->items(scenePos);
    ComponentPort* targetPort = nullptr;
    for (auto* item : items) {
        if (auto* port = dynamic_cast<ComponentPort*>(item)) {
            if (port != this) {
                targetPort = port;
                break;
            }
        }
    }
    if (!targetPort) return;

    ComponentItem* sourceComp = ownerComponent();
    ComponentItem* targetComp = targetPort->ownerComponent();
    if (!sourceComp || !targetComp || sourceComp == targetComp) return;

    bool valid = false;

    bool sourceIsAncestor = sourceComp->isAncestorOf(targetComp);
    bool targetIsAncestor = targetComp->isAncestorOf(sourceComp);

    if (sourceIsAncestor) {
        valid = (m_type == Input && targetPort->portType() == Input ||
                 (m_type == Output && targetPort->portType() == Input));
    } else if (targetIsAncestor) {
        valid = (m_type == Output && targetPort->portType() == Output) ||
                (m_type == Output && targetPort->portType() == Input);
    } else {
        valid = (m_type == Output && targetPort->portType() == Input);
    }

    if (!valid) return;

    for (auto* conn : sourceComp->outgoingConnections()) {
        if (conn->endPort() == targetPort)
            return;
    }

    ConnectionItem* conn = new ConnectionItem(this, targetPort);
    scene()->addItem(conn);
    sourceComp->onConnectionAdded(conn);
    targetComp->onConnectionAdded(conn);
}

void ComponentPort::cancelConnection()
{
    if (m_tempCurve) {
        scene()->removeItem(m_tempCurve);
        delete m_tempCurve;
        m_tempCurve = nullptr;
    }
    m_dragging = false;
}

void ComponentPort::adjustLabelPosition()
{
    const qreal labelWidth = m_label->boundingRect().width();
    const qreal labelHeight = m_label->boundingRect().height();
    const qreal gap = 6.0;
    const qreal radius = 4.0;

    const qreal xCenter = -labelWidth / 2.0;

    if (m_type == Input) {
        m_label->setPos(xCenter, radius + gap);
    } else {
        m_label->setPos(xCenter, -(labelHeight + radius + gap));
    }
}

QVariant ComponentPort::itemChange(GraphicsItemChange change, const QVariant& value)
{
    if (change == ItemPositionHasChanged && scene()) {
        ComponentItem* owner = ownerComponent();
        if (owner) {
            for (auto* conn : owner->outgoingConnections()) {
                if (conn->startPort() == this || conn->endPort() == this)
                    conn->updatePath();
            }
            for (auto* conn : owner->incomingConnections()) {
                if (conn->startPort() == this || conn->endPort() == this)
                    conn->updatePath();
            }
        }
    }
    return QGraphicsEllipseItem::itemChange(change, value);
}