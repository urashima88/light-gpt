#include "workfield.h"
#include <QPainter>
#include <QtMath>

WorkField::WorkField(QObject *parent)
    : QGraphicsScene(parent)
{
    setSceneRect(-50000, -50000, 100000, 100000);
}

void WorkField::drawBackground(QPainter* painter, const QRectF& rect)
{

    painter->fillRect(rect, QColor(0, 0, 0));
    const qreal scaleX = painter->worldTransform().m11();
    const qreal scaleY = painter->worldTransform().m22();
    const qreal scale = (qAbs(scaleX) + qAbs(scaleY)) * 0.5;

    qreal gridStep = BASE_GRID_SIZE;
    constexpr qreal TARGET_PIXEL_SPACING = 30.0;
    while (gridStep * scale < TARGET_PIXEL_SPACING) {
        gridStep *= 2.0;
    }
    while (gridStep * scale > TARGET_PIXEL_SPACING * 2.0) {
        gridStep /= 2.0;
    }
    if (gridStep < BASE_GRID_SIZE)
        gridStep = BASE_GRID_SIZE;

    const qreal left = qFloor(rect.left() / gridStep) * gridStep;
    const qreal top = qFloor(rect.top() / gridStep) * gridStep;

    painter->setPen(Qt::NoPen);
    painter->setBrush(m_gridColor);

    for (qreal x = left; x <= rect.right(); x += gridStep) {
        for (qreal y = top; y <= rect.bottom(); y += gridStep) {
            painter->drawEllipse(QPointF(x, y), m_pointRadius, m_pointRadius);
        }
    }
}