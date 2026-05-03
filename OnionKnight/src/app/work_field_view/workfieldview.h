#pragma once

#include <QGraphicsView>

class ComponentRegistry;

class WorkFieldView: public QGraphicsView
{
    Q_OBJECT

public:
    explicit WorkFieldView(ComponentRegistry* registry, QWidget* parent = nullptr);

signals:
    void componentDropped(const QString& typeId, QPointF scenePos);

protected:
    void wheelEvent(QWheelEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;

    void dragEnterEvent(QDragEnterEvent* event) override;
    void dragMoveEvent(QDragMoveEvent* event) override;
    void dropEvent(QDropEvent* event) override;

    void keyPressEvent(QKeyEvent* event) override;

private:
    ComponentRegistry* m_registry;
    bool m_isPanning = false;
    QPoint m_lastMousePos;
};
