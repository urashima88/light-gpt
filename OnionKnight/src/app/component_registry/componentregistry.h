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

    void registerComponent(const QString& typeId, const QString& modulePath, const QString& className);
    void changeComponentSource(const QString& typeId, const QString& newModulePath, const QString& newClassName);

    ComponentMeta currentMeta(const QString& typeId) const;

    void setIconBasePath(const QString& path);
    QString getIconPath(const QString& className) const;

signals:
    void metaReady(const QString& typeId, const ComponentMeta& meta);
    void metaError(const QString& typeId, const QString& error);

private slots:
    void onInspectorMetaReady(const QString& tag, const ComponentMeta& meta);
    void onInspectorError(const QString& tag, const QString& error);
    void onFileChanged(const QString& path);

private:
    struct ComponentEntry {
        QString modulePath;
        QString className;
        ComponentMeta meta;
        bool pending = false;
    };

    void requestMetaFromSource(const QString& typeId, const QString& modulePath, const QString& className);

    ComponentInspector* m_inspector;
    QFileSystemWatcher* m_watcher;
    QMap<QString, ComponentEntry> m_entries;
    QString m_iconBasePath;
};
