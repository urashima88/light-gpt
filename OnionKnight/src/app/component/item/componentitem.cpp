#include "componentitem.h"
#include "componentregistry.h"
#include "componentport.h"
#include <QPainter>
#include <QFont>
#include <QFontMetrics>
#include <QSvgRenderer>
#include <QGraphicsSceneContextMenuEvent>
#include <QMenu>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>

ComponentItem::ComponentItem(const QString& typeId, ComponentRegistry* registry, QGraphicsItem* parent)
    : QGraphicsObject(parent)
    , m_typeId(typeId)
    , m_registry(registry)
{
    setFlags(ItemIsMovable | ItemIsSelectable | ItemSendsGeometryChanges);

    m_meta = registry->currentMeta(typeId);
    connect(registry, &ComponentRegistry::metaReady, this, &ComponentItem::onMetaReady);

    loadIcon();
    rebuildLayout();
}

QRectF ComponentItem::boundingRect() const
{
    return QRectF(QPointF(0, 0), m_size);
}

void ComponentItem::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*)
{
    painter->setRenderHint(QPainter::Antialiasing);

    // background
    painter->setBrush(QColor(41, 163, 25));
    painter->setPen(QPen(Qt::darkGray, 1.5));
    painter->drawRoundedRect(boundingRect(), 6, 6);

    // header
    QRectF headerRect(0, 0, m_size.width(), HEADER_HEIGHT);
    painter->setBrush(QColor(41, 163, 25));
    painter->setPen(Qt::NoPen);
    painter->drawRoundedRect(headerRect, 6, 6);
    painter->drawRect(0, HEADER_HEIGHT - 6, m_size.width(), 6);

    // icon
    if (m_iconLoaded) {
        painter->drawPixmap(PADDING, (HEADER_HEIGHT - ICON_SIZE) / 2, m_iconPixmap);
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

QVariant ComponentItem::itemChange(GraphicsItemChange change, const QVariant& value)
{
    if (change == ItemPositionHasChanged) {
        emit positionChanged();
    }
    return QGraphicsObject::itemChange(change, value);
}

void ComponentItem::contextMenuEvent(QGraphicsSceneContextMenuEvent* event)
{
    QMenu menu;
    menu.addAction("Configure...")->setEnabled(false);
    menu.addAction("Delete");
    menu.exec(event->screenPos());
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
    clearPorts();

    m_size = calculateRequiredSize();

    if (const int n = m_meta.outputs.size()) {
        const qreal totalWidth = (n - 1) * PORT_SPACING;
        const qreal startX = (m_size.width() - totalWidth) / 2.0;
        for (int i = 0; i < n; ++i) {
            auto *port = new ComponentPort(m_meta.outputs[i].name, ComponentPort::Output, this);
            port->setPos(startX + i * PORT_SPACING, 0.0);
            m_outputPorts.append(port);
        }
    }

    if (const int n = m_meta.inputs.size()) {
        const qreal totalWidth = (n - 1) * PORT_SPACING;
        const qreal startX = (m_size.width() - totalWidth) / 2.0;
        for (int i = 0; i < n; ++i) {
            auto *port = new ComponentPort(m_meta.inputs[i].name, ComponentPort::Input, this);
            port->setPos(startX + i * PORT_SPACING, m_size.height());
            m_inputPorts.append(port);
        }
    }

    update();
}

void ComponentItem::clearPorts()
{
    for (auto* p : m_inputPorts) { delete p; }
    for (auto* p : m_outputPorts) { delete p; }
    m_inputPorts.clear();
    m_outputPorts.clear();
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
    }
    QString iconPath = m_registry->getIconPath(m_typeId);
    if (iconPath.isEmpty()) {
        m_iconLoaded = false;
        return;
    }

    QSvgRenderer svg(iconPath);
    if (svg.isValid()) {
        m_iconPixmap = QPixmap(ICON_SIZE, ICON_SIZE);
        m_iconPixmap.fill(Qt::transparent);
        QPainter p(&m_iconPixmap);
        svg.render(&p);
        m_iconLoaded = true;
    } else {
        m_iconLoaded = false;
    }
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