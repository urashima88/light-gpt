#pragma once

#include <QGraphicsView>

class WorkFieldView: public QGraphicsView
{
    Q_OBJECT

public:
    explicit WorkFieldView(QWidget* parent = nullptr);

protected:
    void wheelEvent(QWheelEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;

private:
    bool m_isPanning = false;
    QPoint m_lastMousePos;
};
