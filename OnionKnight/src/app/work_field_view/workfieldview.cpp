#include "workfieldview.h"
#include <QWheelEvent>
#include <QMouseEvent>
#include <QScrollBar>

WorkFieldView::WorkFieldView(QWidget* parent)
    : QGraphicsView(parent)
{
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);

    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setRenderHint(QPainter::Antialiasing, true);
    setViewportUpdateMode(QGraphicsView::BoundingRectViewportUpdate);

    setFocusPolicy(Qt::StrongFocus);
}

void WorkFieldView::wheelEvent(QWheelEvent* event)
{
    constexpr qreal SCALE_FACTOR = 1.15;
    if (event->angleDelta().y() > 0) {
        scale(SCALE_FACTOR, SCALE_FACTOR);
    } else {
        scale(1.0 / SCALE_FACTOR, 1.0 / SCALE_FACTOR);
    }
    event->accept();
}

void WorkFieldView::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        QGraphicsItem* item = itemAt(event->pos());
        if (item == nullptr) {
            m_isPanning = true;
            m_lastMousePos = event->pos();
            setCursor(Qt::ClosedHandCursor);
            event->accept();
            return;
        }
    }
    QGraphicsView::mousePressEvent(event);
}

void WorkFieldView::mouseMoveEvent(QMouseEvent* event)
{
    if (m_isPanning) {
        QPoint delta = event->pos() - m_lastMousePos;
        m_lastMousePos = event->pos();
        horizontalScrollBar()->setValue(horizontalScrollBar()->value() - delta.x());
        verticalScrollBar()->setValue(verticalScrollBar()->value() - delta.y());
        event->accept();
        return;
    }
    QGraphicsView::mouseMoveEvent(event);
}

void WorkFieldView::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton && m_isPanning) {
        m_isPanning = false;
        setCursor(Qt::ArrowCursor);
        event->accept();
        return;
    }
    QGraphicsView::mouseReleaseEvent(event);
}
