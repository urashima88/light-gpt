#pragma once

#include <QGraphicsPathItem>
#include <QPen>

class ComponentPort;

class ConnectionItem: public QGraphicsPathItem
{

public:
    ConnectionItem(ComponentPort* startPos, ComponentPort* endPort,
                   QGraphicsItem* parent = nullptr);

    void updatePath();
    ComponentPort* startPort() const { return m_startPort; }
    ComponentPort* endPort() const { return m_endPort; }

    void setConnectionColor(const QColor& color);

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    void contextMenuEvent(QGraphicsSceneContextMenuEvent* event) override;

private:
    ComponentPort* m_startPort;
    ComponentPort* m_endPort;
};
