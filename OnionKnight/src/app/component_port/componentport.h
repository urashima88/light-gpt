#pragma once

#include <QGraphicsEllipseItem>
#include <QGraphicsTextItem>
#include <QString>

class ComponentItem;

class ComponentPort: public QGraphicsEllipseItem
{
public:
    enum PortType { Input, Output };

    ComponentPort(const QString& name, PortType type, ComponentItem* parentBlock);
    virtual ~ComponentPort() = default;

    QString portName() const { return m_name; }
    PortType portType() const { return m_type; }

    void adjustLabelPosition();

private:
    QString m_name;
    PortType m_type;
    QGraphicsTextItem* m_label;
};
