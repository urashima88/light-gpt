#pragma once

#include <QStandardItemModel>
#include <QIcon>
#include <QHash>
#include <QMimeData>
#include <QFile>

class ComponentRegistry;

class ComponentTreeModel: public QStandardItemModel
{
    Q_OBJECT

    enum ItemRoles {
        NodeRole = Qt::UserRole,
        ComponentRole = Qt::UserRole + 1
    };

public:
    explicit ComponentTreeModel(
        ComponentRegistry* registry,
        const QIcon& folderIcon,
        const QIcon& componentDefaultIcon,
        QObject* parent = nullptr
    );

    void rebuild();

    QString componentTypeId(const QModelIndex& index) const;

    QMimeData* mimeData(const QModelIndexList& indexes) const override;
    QStringList mimeTypes() const override;

private:
    void insertEntry(const QString& typeId, const QString& virtualPath, const QString& className);
    void sortAll();

    QIcon componentIcon(const QString& className) const;

    ComponentRegistry* m_registry;
    QIcon m_folderIcon;
    QIcon m_componentDefaultIcon;

    mutable QHash<QString, QIcon> m_iconCache;
    QHash<QStringList, QStandardItem*> m_pathIndex;
};


