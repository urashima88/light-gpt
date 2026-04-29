#include "componentport.h"
#include "componentitem.h"
#include <QPen>
#include <QBrush>
#include <QFont>

ComponentPort::ComponentPort(const QString& name, PortType type, ComponentItem* parentBlock)
    : QGraphicsEllipseItem(QRectF(-4, -4, 8, 8), parentBlock)
    , m_name(name)
    , m_type(type)
{
    setPen(QPen(Qt::darkGray, 1.5));
    setBrush(QBrush(type == Input ? QColor(100, 200, 100) : QColor(200, 100, 100)));
    setAcceptHoverEvents(true);

    m_label = new QGraphicsTextItem(name, this);
    QFont font = m_label->font();
    font.setPointSize(7);
    m_label->setFont(font);
    m_label->setDefaultTextColor(Qt::white);
    adjustLabelPosition();
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