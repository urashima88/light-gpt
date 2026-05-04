#pragma once

#include <QGraphicsEllipseItem>
#include <QGraphicsPathItem>
#include <QGraphicsTextItem>
#include <QString>

class ConnectionItem;
class ComponentItem;

class ComponentPort: public QGraphicsEllipseItem
{
public:
    enum PortType { Input, Output };

    ComponentPort(const QString& name, PortType type, ComponentItem* parentItem);

    QString portName() const { return m_name; }
    PortType portType() const { return m_type; }
    ComponentItem* ownerComponent() const { return m_owner; }

    void adjustLabelPosition();

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;

    QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;

private:
    QString m_name;
    PortType m_type;
    QGraphicsTextItem* m_label;
    ComponentItem* m_owner;
    QGraphicsPathItem* m_tempCurve = nullptr;
    bool m_dragging = false;

    friend class ConnectionItem;

    void startNewConnection();
    void updateTempCurve(const QPointF& scenePos);
    void finishConnection(const QPointF& scenePos);
    void cancelConnection();
};
