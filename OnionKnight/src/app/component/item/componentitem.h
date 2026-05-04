#pragma once

#include "componentmeta.h"
#include <QGraphicsObject>
#include <QVector>
#include <QIcon>

class ComponentRegistry;
class ComponentPort;
class ConnectionItem;
class ComponentCodeManager;

class ComponentItem: public QGraphicsObject
{
    Q_OBJECT

public:
    ComponentItem(
        const QString& typeId,
        ComponentRegistry* registry,
        ComponentItem* parentComponentItem = nullptr,
        QGraphicsItem* parent = nullptr
    );

    ~ComponentItem() override;

    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option,
               QWidget* widget) override;
    void animateAppearance();

    ComponentMeta currentMeta() const { return m_meta; }
    QString typeId() const { return m_typeId; }

    void setComponentCodeManager(ComponentCodeManager* codeManager) { m_codeManager = codeManager; }
    ComponentCodeManager* componentCodeManager() const { return m_codeManager; }

    void setParentComponentItem(ComponentItem* parentComponentItem) { m_parentComponentItem = parentComponentItem; }

    bool isExpanded() const { return m_expanded; }
    void setExpanded(bool expanded);

    void setVariableName(const QString& name) { m_variableName = name; }
    QString variableName() const { return m_variableName; }

    void setCustomParams(const QVariantMap& params) { m_customParams = params; }
    QVariantMap customParams() const { return m_customParams; }

    void addChild(ComponentItem* child);
    void removeChild(ComponentItem* child);
    QVector<ComponentItem*> childItems() const { return m_childItems; }

    void onConnectionAdded(ConnectionItem* conn);
    void onConnectionRemoved(ConnectionItem* conn);
    QVector<ConnectionItem*> outgoingConnections() const { return m_outgoingConns; }
    QVector<ConnectionItem*> incomingConnections() const { return m_incomingConns; }

    void updateCodeFromChildren();
    void checkParentContainer();
    void detachFromParent();
    bool isAncestorOf(ComponentItem* descendant) const;
    void deleteComponent();

signals:
    void positionChanged();
    void connectionAdded(ConnectionItem* conn);
    void connectionRemoved(ConnectionItem* conn);

protected:
    QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;
    void contextMenuEvent(QGraphicsSceneContextMenuEvent* event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;

private slots:
    void onMetaReady(const QString& id, const ComponentMeta& meta);

private:
    static constexpr qreal HEADER_HEIGHT = 28.0;
    static constexpr qreal PORT_MARGIN = 8.0;
    static constexpr qreal PORT_SPACING = 20.0;
    static constexpr qreal BLOCK_MIN_WIDTH = 120.0;
    static constexpr qreal PADDING = 8.0;
    static constexpr qreal ICON_SIZE = 18.0;
    static constexpr qreal ICON_TEXT_GAP = 4.0;

    QSizeF m_size;
    QIcon m_icon;
    bool m_iconLoaded = false;
    QColor m_color;
    bool m_expanded = false;
    QSizeF m_expandedSizeHint {500, 400};

    QString m_variableName;
    QVariantMap m_customParams;
    bool m_isBeingDeleted = false;

    ComponentRegistry* m_registry;
    ComponentItem* m_parentComponentItem;
    ComponentMeta m_meta;
    QString m_typeId;
    ComponentCodeManager* m_codeManager = nullptr;

    QVector<ComponentPort*> m_inputPorts;
    QVector<ComponentPort*> m_outputPorts;

    QVector<ConnectionItem*> m_outgoingConns;
    QVector<ConnectionItem*> m_incomingConns;

    QVector<ComponentItem*> m_childItems;

    void rebuildLayout();
    void clearPorts();
    QSizeF calculateRequiredSize() const;
    void loadIcon();
    void performCleanup();
};
