#include "componenttreemodel.h"
#include "componentregistry.h"
#include <QApplication>
#include <QStyle>
#include <QFile>
#include <QMimeData>
#include <QStringList>
#include <QPixmap>
#include <QPainter>

ComponentTreeModel::ComponentTreeModel(
    ComponentRegistry* registry,
    const QIcon& folderIcon = QIcon() ,
    const QIcon& componentDefaultIcon = QIcon(),
    QObject* parent)
    : QStandardItemModel(parent)
    , m_registry(registry)
    , m_folderIcon(folderIcon)
    , m_componentDefaultIcon(componentDefaultIcon)
{
    if (m_folderIcon.isNull())
        m_folderIcon = qApp->style()->standardIcon(QStyle::SP_DirIcon);

    if (m_componentDefaultIcon.isNull())
        m_componentDefaultIcon = qApp->style()->standardIcon(QStyle::SP_FileIcon);
}

void ComponentTreeModel::rebuild()
{
    clear();
    m_pathIndex.clear();
    setHorizontalHeaderLabels({tr("Components")});

    const auto& entries = m_registry->allEntries();
    for (auto it = entries.begin(); it != entries.end(); ++it) {
        insertEntry(it.key(), it.value().virtualPath, it.value().className);
    }

    sortAll();
}

void ComponentTreeModel::insertEntry(const QString& typeId,
                                     const QString& virtualPath,
                                     const QString& className)
{
    const QStringList parts = virtualPath.split('/', Qt::SkipEmptyParts);
    if (parts.isEmpty())
        return;

    QStandardItem* parent = nullptr;
    QStringList currentPath;

    for (const QString& part : parts) {
        currentPath.append(part);
        QStandardItem* folder = m_pathIndex.value(currentPath);
        if (!folder) {
            folder = new QStandardItem(m_folderIcon, part);
            folder->setData(QStringLiteral("folder"), NodeRole);
            folder->setFlags(Qt::ItemIsEnabled);
            if (parent) {
                parent->appendRow(folder);
            } else {
                invisibleRootItem()->appendRow(folder);
            }
            m_pathIndex.insert(currentPath, folder);
        }
        parent = folder;
    }

    QIcon icon = componentIcon(typeId);
    auto* compItem = new QStandardItem(icon, className);
    compItem->setData(className, ComponentRole);
    compItem->setData(QStringLiteral("component"), NodeRole);
    compItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled);

    if (parent) {
        parent->appendRow(compItem);
    } else {
        invisibleRootItem()->appendRow(compItem);
    }
}

QString ComponentTreeModel::componentTypeId(const QModelIndex& index) const
{
    if (!index.isValid())
        return {};
    QStandardItem* item = itemFromIndex(index);
    if (item->data(NodeRole).toString() == QLatin1String("component"))
        return item->data(ComponentRole).toString();

    return {};
}

QIcon ComponentTreeModel::componentIcon(const QString& typeId) const
{
    auto it = m_iconCache.find(typeId);
    if (it != m_iconCache.end())
        return *it;

    QString path = m_registry->getIconPath(typeId);
    QIcon icon;
    if (QFile::exists(path)) {
        QPixmap pix(path);
        QPixmap whitePix(pix.size());
        whitePix.fill(Qt::transparent);
        QPainter painter(&whitePix);
        painter.setCompositionMode(QPainter::CompositionMode_Source);
        painter.drawPixmap(0, 0, pix);
        painter.setCompositionMode(QPainter::CompositionMode_SourceIn);
        painter.fillRect(whitePix.rect(), Qt::white);
        painter.end();
        icon = QIcon(whitePix);
    } else {
        icon = m_componentDefaultIcon;
    }

    m_iconCache.insert(typeId, icon);
    return icon;
}

QMimeData* ComponentTreeModel::mimeData(const QModelIndexList& indexes) const
{
    auto* mimeData = new QMimeData();
    QStringList types;
    for (const QModelIndex& idx : indexes) {
        if (idx.isValid() && idx.column() == 0) {
            QString tid = componentTypeId(idx);
            if (!tid.isEmpty())
                types.append(tid);
        }
    }

    if (!types.isEmpty()) {
        mimeData->setData(QStringLiteral("application/x-component-type"),
                          types.join(QLatin1Char(',')).toUtf8());
    }

    return mimeData;
}

QStringList ComponentTreeModel::mimeTypes() const
{
    return { QStringLiteral("application/x-component-type") };
}

void ComponentTreeModel::sortAll()
{
    std::function<void(QStandardItem*)> sortRecursive = [&](QStandardItem* parent) {
        parent->sortChildren(0);
        for (int i = 0; i < parent->rowCount(); ++i) {
            QStandardItem* child = parent->child(i);
            if (child->data(NodeRole).toString() == QLatin1String("folder"))
                sortRecursive(child);
        }
    };

    QStandardItem* root = invisibleRootItem();
    root->sortChildren(0);
    for (int i = 0; i < root->rowCount(); ++i) {
        QStandardItem* child = root->child(i);
        if (child->data(NodeRole).toString() == QLatin1String("folder"))
            sortRecursive(child);
    }
}
