#include "componentitem.h"
#include "componentregistry.h"
#include "componentport.h"
#include "connectionitem.h"
#include "componentcodemanager.h"
#include <QPainter>
#include <QFont>
#include <QFontMetrics>
#include <QGraphicsSceneContextMenuEvent>
#include <QMenu>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>
#include <QJsonArray>
#include <QGraphicsScene>
#include <QTimer>
#include <QPointer>

ComponentItem::ComponentItem(
    const QString& typeId,
    ComponentRegistry* registry,
    ComponentItem* parentComponentItem,
    QGraphicsItem* parent)
    : QGraphicsObject(parent)
    , m_typeId(typeId)
    , m_registry(registry)
    , m_parentComponentItem(parentComponentItem)
{
    setFlags(ItemIsMovable | ItemIsSelectable | ItemSendsGeometryChanges);

    m_meta = registry->currentMeta(typeId);
    m_color = registry->getColor(typeId);
    connect(registry, &ComponentRegistry::metaReady, this, &ComponentItem::onMetaReady);

    loadIcon();
    rebuildLayout();
}

ComponentItem::~ComponentItem()
{
    performCleanup();
}

QRectF ComponentItem::boundingRect() const
{
    return QRectF(QPointF(0, 0), m_size);
}

void ComponentItem::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*)
{
    painter->setRenderHint(QPainter::Antialiasing);

    if (m_expanded) {
        QColor bgColor = m_color;
        bgColor.setAlpha(40);
        QColor headerColor = m_color;

        QPen borderPen(m_color.darker(120), 2);
        painter->setPen(borderPen);
        painter->setBrush(bgColor);
        painter->drawRoundedRect(boundingRect().adjusted(1, 1, -1, -1), 8, 8);

        // header
        QRectF headerRect(0, 0, m_size.width(), HEADER_HEIGHT);
        painter->setBrush(headerColor);
        painter->setPen(Qt::NoPen);
        painter->drawRoundedRect(headerRect, 8, 8);
        painter->drawRect(0, HEADER_HEIGHT - 4, m_size.width(), 4);

        // icon
        if (m_iconLoaded) {
            QRectF iconRect(PADDING, (HEADER_HEIGHT - ICON_SIZE) / 2, ICON_SIZE, ICON_SIZE);
            m_icon.paint(painter, iconRect.toRect(), Qt::AlignCenter, QIcon::Normal, QIcon::On);
        }

        // name
        QFont font = painter->font();
        font.setBold(true);
        font.setPointSize(10);
        painter->setFont(font);
        painter->setPen(Qt::black);
        qreal textX = PADDING + (m_iconLoaded ? ICON_SIZE + ICON_TEXT_GAP : 0);
        painter->drawText(QRectF(textX, 0, m_size.width() - textX - PADDING, HEADER_HEIGHT),
                          Qt::AlignLeft | Qt::AlignVCenter, m_meta.className);

    } else {
        // background
        painter->setBrush(m_color);
        painter->setPen(QPen(Qt::darkGray, 1.5));
        painter->drawRoundedRect(boundingRect(), 6, 6);

        // header
        QRectF headerRect(0, 0, m_size.width(), HEADER_HEIGHT);
        painter->setBrush(m_color);
        painter->setPen(Qt::NoPen);
        painter->drawRoundedRect(headerRect, 6, 6);
        painter->drawRect(0, HEADER_HEIGHT - 6, m_size.width(), 6);

        // icon
        if (m_iconLoaded) {
            QRectF iconRect(PADDING, (HEADER_HEIGHT - ICON_SIZE) / 2, ICON_SIZE, ICON_SIZE);
            m_icon.paint(painter, iconRect.toRect(), Qt::AlignCenter, QIcon::Normal, QIcon::On);
        }

        // name
        QFont font = painter->font();
        font.setBold(true);
        font.setPointSize(10);
        painter->setFont(font);
        painter->setPen(Qt::black);

        qreal textX = PADDING + (m_iconLoaded ? ICON_SIZE + ICON_TEXT_GAP : 0);
        painter->drawText(QRectF(textX, 0, m_size.width() - textX - PADDING, HEADER_HEIGHT),
                          Qt::AlignLeft | Qt::AlignVCenter, m_meta.className);
    }
}

QVariant ComponentItem::itemChange(GraphicsItemChange change, const QVariant& value)
{
    if (change == ItemPositionHasChanged) {
        for (auto* conn : m_outgoingConns) conn->updatePath();
        for (auto* conn : m_incomingConns) conn->updatePath();

        std::function<void(ComponentItem*)> updateChildrenConns = [&](ComponentItem* item) {
            for (auto* child : item->childItems()) {
                for (auto* conn : child->outgoingConnections()) conn->updatePath();
                for (auto* conn : child->incomingConnections()) conn->updatePath();
                updateChildrenConns(child);
            }
        };

        updateChildrenConns(this);

        emit positionChanged();
    }
    return QGraphicsObject::itemChange(change, value);
}

void ComponentItem::contextMenuEvent(QGraphicsSceneContextMenuEvent* event)
{
    QMenu menu;
    QAction* expandAction = menu.addAction(m_expanded ? tr("Collapse") : tr("Expand"));
    menu.addSeparator();
    QAction* deleteAction = menu.addAction(tr("Delete"));

    QAction* selected = menu.exec(event->screenPos());
    if (selected == expandAction) {
        setExpanded(!m_expanded);
    } else if (selected == deleteAction) {
        deleteComponent();
    }
}

void ComponentItem::onMetaReady(const QString& id, const ComponentMeta& meta)
{
    if (id != m_typeId)
        return;

    m_meta = meta;
    loadIcon();
    rebuildLayout();
}

void ComponentItem::rebuildLayout()
{
    prepareGeometryChange();
    m_size = m_expanded ? m_expandedSizeHint : calculateRequiredSize();

    const int expectedIn = m_meta.inputs.size();
    const int expectedOut = m_meta.outputs.size();

    if (m_inputPorts.size() != expectedIn || m_outputPorts.size() != expectedOut) {
        clearPorts();
        if (expectedOut > 0) {
            const qreal totalWidth = (expectedOut - 1) * PORT_SPACING;
            const qreal startX = (m_size.width() - totalWidth) / 2.0;
            for (int i = 0; i < expectedOut; ++i) {
                auto* port = new ComponentPort(m_meta.outputs[i].name, ComponentPort::Output, this);
                port->setPos(startX + i * PORT_SPACING, 0.0);
                m_outputPorts.append(port);
            }
        }
        if (expectedIn > 0) {
            const qreal totalWidth = (expectedIn - 1) * PORT_SPACING;
            const qreal startX = (m_size.width() - totalWidth) / 2.0;
            for (int i = 0; i < expectedIn; ++i) {
                auto* port = new ComponentPort(m_meta.inputs[i].name, ComponentPort::Input, this);
                port->setPos(startX + i * PORT_SPACING, m_size.height());
                m_inputPorts.append(port);
            }
        }
    } else {
        if (expectedOut > 0) {
            const qreal totalWidth = (expectedOut - 1) * PORT_SPACING;
            const qreal startX = (m_size.width() - totalWidth) / 2.0;
            for (int i = 0; i < expectedOut; ++i) {
                m_outputPorts[i]->setPos(startX + i * PORT_SPACING, 0.0);
            }
        }
        if (expectedIn > 0) {
            const qreal totalWidth = (expectedIn - 1) * PORT_SPACING;
            const qreal startX = (m_size.width() - totalWidth) / 2.0;
            for (int i = 0; i < expectedIn; ++i) {
                m_inputPorts[i]->setPos(startX + i * PORT_SPACING, m_size.height());
            }
        }
    }
    update();
}

void ComponentItem::clearPorts()
{
    QVector<ConnectionItem*> outConns = m_outgoingConns;
    QVector<ConnectionItem*> inConns = m_incomingConns;

    for (auto* conn : outConns) {
        if (conn->scene()) conn->scene()->removeItem(conn);
        conn->startPort()->ownerComponent()->onConnectionRemoved(conn);
        conn->endPort()->ownerComponent()->onConnectionRemoved(conn);
        delete conn;
    }
    for (auto* conn : inConns) {
        if (conn->scene()) conn->scene()->removeItem(conn);
        conn->startPort()->ownerComponent()->onConnectionRemoved(conn);
        conn->endPort()->ownerComponent()->onConnectionRemoved(conn);
        delete conn;
    }

    for (auto* p : m_inputPorts) { delete p; }
    for (auto* p : m_outputPorts) { delete p; }
    m_inputPorts.clear();
    m_outputPorts.clear();
    m_outgoingConns.clear();
    m_incomingConns.clear();
}

QSizeF ComponentItem::calculateRequiredSize() const
{
    QFont font;
    font.setBold(true);
    font.setPointSize(10);
    QFontMetrics fm(font);

    qreal textWidth = fm.horizontalAdvance(m_meta.className);
    qreal iconPart = m_iconLoaded ? (ICON_SIZE + ICON_TEXT_GAP) : 0;
    qreal widthByTitle = PADDING + iconPart + textWidth + PADDING;

    int maxPorts = qMax(m_meta.inputs.size(), m_meta.outputs.size());
    qreal widthByPorts = 0.0;
    if (maxPorts > 0) {
        widthByPorts = 2 * PORT_MARGIN + (maxPorts - 1) * PORT_SPACING;
    }
    qreal width = qMax(widthByTitle, qMax(widthByPorts, BLOCK_MIN_WIDTH));
    qreal height = HEADER_HEIGHT + (maxPorts > 0 ? 4.0 : 2 * PADDING);

    return QSizeF(width, height);
}

void ComponentItem::loadIcon()
{
    if (!m_registry) {
        m_iconLoaded = false;
        return;
    }
    QString iconPath = m_registry->getIconPath(m_typeId);
    if (iconPath.isEmpty()) {
        m_iconLoaded = false;
        return;
    }

    m_icon = QIcon(iconPath);
    m_iconLoaded = !m_icon.isNull();
}

void ComponentItem::animateAppearance()
{
    if (graphicsEffect() == nullptr) {
        auto* effect = new QGraphicsOpacityEffect(this);
        effect->setOpacity(0.0);
        setGraphicsEffect(effect);
    }

    if (auto* oldAnim = findChild<QPropertyAnimation*>()) {
        oldAnim->stop();
        oldAnim->deleteLater();
    }

    auto* anim = new QPropertyAnimation(graphicsEffect(), "opacity", this);
    anim->setDuration(250);
    anim->setStartValue(0.0);
    anim->setEndValue(1.0);
    anim->start(QAbstractAnimation::DeleteWhenStopped);
}

void ComponentItem::setExpanded(bool expanded)
{
    if (m_expanded == expanded) return;
    m_expanded = expanded;
    rebuildLayout();

    for (auto* child : m_childItems) {
        child->setVisible(expanded);
        for (auto* port : child->m_inputPorts)
            port->setVisible(expanded);
        for (auto* port : child->m_outputPorts)
            port->setVisible(expanded);
        for (auto* conn : child->outgoingConnections())
            conn->setVisible(expanded);
        for (auto* conn : child->incomingConnections())
            conn->setVisible(expanded);
    }

    for (auto* child : m_childItems) {
        for (auto* conn : child->outgoingConnections()) conn->updatePath();
        for (auto* conn : child->incomingConnections()) conn->updatePath();
    }
    for (auto* conn : m_outgoingConns) conn->updatePath();
    for (auto* conn : m_incomingConns) conn->updatePath();

    update();
}

void ComponentItem::addChild(ComponentItem* child)
{
    if (!child || m_childItems.contains(child))
        return;

    child->setParentComponentItem(this);
    child->setParentItem(this);
    child->setVisible(m_expanded);
    m_childItems.append(child);
}

void ComponentItem::removeChild(ComponentItem* child)
{
    if (!m_childItems.removeOne(child))
        return;

    child->setParentComponentItem(nullptr);
    child->setParentItem(nullptr);
    child->setVisible(true);
}

void ComponentItem::onConnectionAdded(ConnectionItem* conn)
{
    if (conn->startPort()->ownerComponent() == this) {
        m_outgoingConns.append(conn);
    } else if (conn->endPort()->ownerComponent() == this) {
        m_incomingConns.append(conn);
    }

    ComponentItem* parentContainer = dynamic_cast<ComponentItem*>(parentItem());
    if (parentContainer && parentContainer->isExpanded())
        parentContainer->updateCodeFromChildren();

    emit connectionAdded(conn);
}

void ComponentItem::onConnectionRemoved(ConnectionItem* conn)
{
    m_outgoingConns.removeAll(conn);
    m_incomingConns.removeAll(conn);
    if (!m_isBeingDeleted) {
        QTimer::singleShot(0, this, [this]() {
            if (m_isBeingDeleted) return;
            ComponentItem* parentContainer = dynamic_cast<ComponentItem*>(parentItem());
            if (parentContainer && parentContainer->isExpanded())
                parentContainer->updateCodeFromChildren();
        });
    }
    emit connectionRemoved(conn);
}

void ComponentItem::updateCodeFromChildren()
{
    if (!m_codeManager || !m_expanded) return;

    QJsonArray childrenJson;
    for (auto* child : m_childItems) {
        QJsonObject childObj;
        childObj["var"] = child->variableName();
        childObj["type"] = child->typeId();

        QJsonObject kwargs;
        const ComponentMeta meta = child->currentMeta();
        for (const auto& param : meta.constructorParams) {
            QVariant defVal = param.hasDefault ?
                                  ComponentMeta::parseDefaultValue(param.defaultValue, param.type) :
                                  QVariant();
            kwargs[param.name] = QJsonValue::fromVariant(defVal);
        }
        for (auto it = child->customParams().begin(); it != child->customParams().end(); ++it) {
            kwargs[it.key()] = QJsonValue::fromVariant(it.value());
        }
        childObj["kwargs"] = kwargs;
        childrenJson.append(childObj);
    }

    QJsonArray connectionsJson;
    // all outgoing connections from a container are routed to its children
    for (auto* conn : m_outgoingConns) {
        ComponentPort* startPort = conn->startPort();
        ComponentPort* endPort = conn->endPort();
        ComponentItem* targetComp = endPort->ownerComponent();
        if (targetComp && targetComp != this && isAncestorOf(targetComp)) {
            QString toVar = targetComp->variableName();
            if (!toVar.isEmpty()) {
                QJsonObject connObj;
                connObj["from"] = QStringLiteral("input");
                connObj["to"] = toVar;
                connectionsJson.append(connObj);
            }
        }
    }

    // all incoming connections from a container are routed from children
    for (auto* conn : m_incomingConns) {
        ComponentPort* startPort = conn->startPort();
        ComponentPort* endPort = conn->endPort();
        ComponentItem* sourceComp = startPort->ownerComponent();
        if (sourceComp && sourceComp != this && isAncestorOf(sourceComp)) {
            QString fromVar = sourceComp->variableName();
            if (!fromVar.isEmpty()) {
                QJsonObject connObj;
                connObj["from"] = fromVar;
                connObj["to"] = QStringLiteral("output");
                connectionsJson.append(connObj);
            }
        }
    }

    // connections between child components
    for (auto* child : m_childItems) {
        for (auto* conn : child->outgoingConnections()) {
            ComponentPort* startPort = conn->startPort();
            ComponentPort* endPort = conn->endPort();
            if (startPort->ownerComponent() == this || endPort->ownerComponent() == this)
                continue;
            QString fromVar = startPort->ownerComponent()->variableName();
            QString toVar = endPort->ownerComponent()->variableName();
            if (!fromVar.isEmpty() && !toVar.isEmpty()) {
                QJsonObject connObj;
                connObj["from"] = fromVar;
                connObj["to"] = toVar;
                connectionsJson.append(connObj);
            }
        }
    }


    QJsonObject changes;
    changes["children"] = childrenJson;
    changes["set_forward_connections"] = connectionsJson;

    ComponentMeta parentMeta = m_parentComponentItem->currentMeta();
    m_codeManager->applyChanges(parentMeta.filePath,
                                parentMeta.className,
                                changes);
}

void ComponentItem::detachFromParent()
{
    ComponentItem* parentContainer = dynamic_cast<ComponentItem*>(parentItem());
    if (!parentContainer)
        return;

    QPointF scenePos = mapToScene(0, 0);

    QVector<ConnectionItem*> allConns = m_outgoingConns + m_incomingConns;
    for (ConnectionItem* conn : allConns) {
        ComponentItem* other = (conn->startPort()->ownerComponent() == this)
        ? conn->endPort()->ownerComponent()
        : conn->startPort()->ownerComponent();

        if (conn->scene())
            conn->scene()->removeItem(conn);

        onConnectionRemoved(conn);
        other->onConnectionRemoved(conn);
        delete conn;
    }

    parentContainer->removeChild(this);
    setPos(scenePos);

    QPointer<ComponentItem> weakParent(parentContainer);
    QTimer::singleShot(0, this, [weakParent]() {
        if (weakParent)
            weakParent->updateCodeFromChildren();
    });
}

void ComponentItem::checkParentContainer()
{
    if (!scene()) return;

    QList<QGraphicsItem*> items = scene()->items(mapToScene(boundingRect()), Qt::IntersectsItemBoundingRect);
    ComponentItem* newContainer = nullptr;
    for (auto* item : items) {
        if (auto* comp = dynamic_cast<ComponentItem*>(item)) {
            if (comp != this && comp->isExpanded()) {
                newContainer = comp;
                break;
            }
        }
    }

    ComponentItem* currentContainer = dynamic_cast<ComponentItem*>(parentItem());

    if (newContainer == currentContainer)
        return;

    if (!newContainer && currentContainer) {
        detachFromParent();
        return;
    }

    if (currentContainer) {
        detachFromParent();
    }

    if (newContainer) {
        QPointF scenePos = mapToScene(0, 0);
        newContainer->addChild(this);

        if (variableName().isEmpty()) {
            QString baseName = typeId().toLower();
            int num = 1;
            bool exists = true;
            while (exists) {
                QString candidate = QString("%1%2").arg(baseName).arg(num);
                exists = false;
                for (auto* child : newContainer->childItems()) {
                    if (child != this && child->variableName() == candidate) {
                        exists = true;
                        break;
                    }
                }
                if (!exists) {
                    setVariableName(candidate);
                }
                ++num;
            }
        }
        setPos(newContainer->mapFromScene(scenePos));
        newContainer->updateCodeFromChildren();
    }
}

void ComponentItem::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
    QGraphicsObject::mouseReleaseEvent(event);
    QTimer::singleShot(0, this, &ComponentItem::checkParentContainer);
}

bool ComponentItem::isAncestorOf(ComponentItem* descendant) const {
    ComponentItem* current = descendant->m_parentComponentItem;
    while (current) {
        if (current == this) return true;
        current = current->m_parentComponentItem;
    }
    return false;
}

void ComponentItem::deleteComponent()
{
    performCleanup();
    deleteLater();
}

void ComponentItem::performCleanup()
{
    if (m_isBeingDeleted) return;
    m_isBeingDeleted = true;

    QVector<ConnectionItem*> allConns = m_outgoingConns + m_incomingConns;
    for (auto* conn : allConns) {
        ComponentItem* other = (conn->startPort()->ownerComponent() == this)
        ? conn->endPort()->ownerComponent()
        : conn->startPort()->ownerComponent();
        if (other && !other->m_isBeingDeleted)
            other->onConnectionRemoved(conn);
        if (conn->scene())
            conn->scene()->removeItem(conn);
        delete conn;
    }
    m_outgoingConns.clear();
    m_incomingConns.clear();

    while (!m_childItems.isEmpty()) {
        ComponentItem* child = m_childItems.first();
        child->performCleanup();
        delete child;
    }

    if (ComponentItem* parentContainer = dynamic_cast<ComponentItem*>(parentItem())) {
        parentContainer->removeChild(this);
        if (!parentContainer->m_isBeingDeleted)
            parentContainer->updateCodeFromChildren();
    } else {
        if (scene())
            scene()->removeItem(this);
    }
}