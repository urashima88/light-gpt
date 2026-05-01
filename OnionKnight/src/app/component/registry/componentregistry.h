#pragma once

#include "componentmeta.h"
#include <QObject>
#include <QMap>
#include <QFileSystemWatcher>
#include <QTimer>

class ComponentInspector;

class ComponentRegistry: public QObject
{
    Q_OBJECT

public:
    explicit ComponentRegistry(ComponentInspector* inspector,
                               QFileSystemWatcher* watcher,
                               QObject* parent = nullptr);

    struct ComponentEntry {
        QString modulePath;
        QString className;
        QString virtualPath;
        QString iconPath;
        ComponentMeta meta;
        bool pending = false;
    };

    void registerComponent(
        const QString& typeId,
        const QString& modulePath,
        const QString& virtualPath,
        const QString& iconPath,
        const QString& className
    );
    void changeComponentSource(
        const QString& typeId,
        const QString& newModulePath,
        const QString& newVirtualPath,
        const QString& newIconPath,
        const QString& newClassName
    );

    ComponentMeta currentMeta(const QString& typeId) const;

    const QMap<QString, ComponentEntry>& allEntries() const;
    QString getIconPath(const QString& className) const;

signals:
    void metaReady(const QString& typeId, const ComponentMeta& meta);
    void metaError(const QString& typeId, const QString& error);

private slots:
    void onInspectorMetaReady(const QString& tag, const ComponentMeta& meta);
    void onInspectorError(const QString& tag, const QString& error);
    void onFileChanged(const QString& path);

private:
    void requestMetaFromSource(
        const QString& typeId,
        const QString& modulePath,
        const QString& virtualPath,
        const QString& iconPath,
        const QString& className
    );

    ComponentInspector* m_inspector;
    QFileSystemWatcher* m_watcher;
    QMap<QString, ComponentEntry> m_entries;
};
