#pragma once

#include <QGraphicsScene>

class WorkField: public QGraphicsScene {
    Q_OBJECT

public:
    explicit WorkField(QObject* parent = nullptr);

protected:
    void drawBackground(QPainter* painter, const QRectF& rect) override;

private:
    static constexpr qreal BASE_GRID_SIZE = 20.0;
    QColor m_gridColor {255, 255, 255};
    qreal m_pointRadius = 1.0;
};
