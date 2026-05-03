#include "workfieldview.h"
#include "componentregistry.h"
#include "componentitem.h"
#include "componentport.h"
#include "connectionitem.h"
#include <QWheelEvent>
#include <QMouseEvent>
#include <QScrollBar>
#include <qmimedata.h>

WorkFieldView::WorkFieldView(ComponentRegistry* registry, QWidget* parent)
    : QGraphicsView(parent)
    , m_registry(registry)
{
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);

    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setRenderHint(QPainter::Antialiasing, true);
    setViewportUpdateMode(QGraphicsView::FullViewportUpdate);

    setFocusPolicy(Qt::StrongFocus);

    setAcceptDrops(true);
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
    if (event->button() == Qt::MiddleButton) {
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
    if (event->button() == Qt::MiddleButton && m_isPanning) {
        m_isPanning = false;
        setCursor(Qt::ArrowCursor);
        event->accept();
        return;
    }
    QGraphicsView::mouseReleaseEvent(event);
}

void WorkFieldView::dragEnterEvent(QDragEnterEvent* event)
{
    if (event->mimeData()->hasFormat(QStringLiteral("application/x-component-type"))) {
        event->acceptProposedAction();
    } else {
        QGraphicsView::dragEnterEvent(event);
    }
}

void WorkFieldView::dragMoveEvent(QDragMoveEvent* event)
{
    if (event->mimeData()->hasFormat(QStringLiteral("application/x-component-type"))) {
        event->acceptProposedAction();
    } else {
        QGraphicsView::dragMoveEvent(event);
    }
}

void WorkFieldView::dropEvent(QDropEvent* event)
{
    if (!event->mimeData()->hasFormat(QStringLiteral("application/x-component-type"))) {
        QGraphicsView::dropEvent(event);
        return;
    }

    const QString typeId = QString::fromUtf8(
        event->mimeData()->data(QStringLiteral("application/x-component-type")));

    if (typeId.isEmpty()) {
        event->ignore();
        return;
    }

    QPointF scenePos = mapToScene(event->position().toPoint());
    emit componentDropped(typeId, scenePos);
    event->acceptProposedAction();
}

void WorkFieldView::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Delete) {
        QList<QGraphicsItem*> selected = scene()->selectedItems();
        for (auto* item : selected) {
            if (auto* conn = dynamic_cast<ConnectionItem*>(item)) {
                ComponentPort* sp = conn->startPort();
                ComponentPort* ep = conn->endPort();
                if (sp) {
                    ComponentItem* owner = sp->ownerComponent();
                    if (owner) owner->onConnectionRemoved(conn);
                }
                if (ep) {
                    ComponentItem* owner = ep->ownerComponent();
                    if (owner) owner->onConnectionRemoved(conn);
                }
                if (conn->scene()) conn->scene()->removeItem(conn);
                delete conn;
            }
        }
        selected = scene()->selectedItems();
        for (auto* item : selected) {
            if (auto* comp = dynamic_cast<ComponentItem*>(item)) {
                comp->deleteComponent();
            }
        }
        event->accept();
        return;
    }
    QGraphicsView::keyPressEvent(event);
}