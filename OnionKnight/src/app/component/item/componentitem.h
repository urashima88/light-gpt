#pragma once

#include "componentmeta.h"
#include <QGraphicsObject>
#include <QVector>
#include <QPixmap>

class ComponentRegistry;
class ComponentPort;

class ComponentItem: public QGraphicsObject
{
    Q_OBJECT

public:
    ComponentItem(const QString& typeId, ComponentRegistry* registry,
                   QGraphicsItem* parent = nullptr);

    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option,
               QWidget* widget) override;
    QString typeId() const { return m_typeId; }
    ComponentMeta currentMeta() const { return m_meta; }

signals:
    void positionChanged();

protected:
    QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;
    void contextMenuEvent(QGraphicsSceneContextMenuEvent* event) override;

private slots:
    void onMetaReady(const QString& id, const ComponentMeta& meta);

private:
    void rebuildLayout();
    void clearPorts();
    QSizeF calculateRequiredSize() const;
    void loadIcon();

    QString m_typeId;
    ComponentRegistry* m_registry;
    ComponentMeta m_meta;

    QVector<ComponentPort*> m_inputPorts;
    QVector<ComponentPort*> m_outputPorts;

    QSizeF m_size;
    QPixmap m_iconPixmap;
    bool m_iconLoaded = false;

    static constexpr qreal HEADER_HEIGHT = 28.0;
    static constexpr qreal PORT_MARGIN = 8.0;
    static constexpr qreal PORT_SPACING = 20.0;
    static constexpr qreal BLOCK_MIN_WIDTH = 120.0;
    static constexpr qreal PADDING = 8.0;
    static constexpr qreal ICON_SIZE = 18.0;
    static constexpr qreal ICON_TEXT_GAP = 4.0;
};
