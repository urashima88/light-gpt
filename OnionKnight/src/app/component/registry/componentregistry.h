#pragma once

#include "componentmeta.h"
#include <QObject>
#include <QMap>
#include <QFileSystemWatcher>
#include <QTimer>
#include <QColor>

class ComponentCodeManager;

class ComponentRegistry: public QObject
{
    Q_OBJECT

public:
    explicit ComponentRegistry(ComponentCodeManager* codeManager,
                               QFileSystemWatcher* watcher,
                               QObject* parent = nullptr);

    struct ComponentEntry {
        QString filePath;
        QString className;
        QString virtualPath;
        QString iconPath;
        QColor color;
        ComponentMeta meta;
        bool pending = false;
    };

    void registerComponent(
        const QString& typeId,
        const QString& filePath,
        const QString& virtualPath,
        const QString& iconPath,
        const QString& className,
        const QColor& color
    );
    void changeComponentSource(
        const QString& typeId,
        const QString& newFilePath,
        const QString& newVirtualPath,
        const QString& newIconPath,
        const QString& newClassName,
        const QColor& newColor
    );

    ComponentMeta currentMeta(const QString& typeId) const;

    const QMap<QString, ComponentEntry>& allEntries() const;
    QString getIconPath(const QString& typeId) const;
    QColor getColor(const QString& typeId) const;

signals:
    void metaReady(const QString& typeId, const ComponentMeta& meta);
    void metaError(const QString& typeId, const QString& error);

private slots:
    void onCodeManagerMetaReady(const QString& typeId, const ComponentMeta& meta);
    void onCodeManagerError(const QString& typeId, const QString& error);
    void onFileChanged(const QString& path);

private:
    void requestMetaFromSource(
        const QString& typeId,
        const QString& filePath,
        const QString& virtualPath,
        const QString& iconPath,
        const QString& className,
        const QColor& color
    );

    ComponentCodeManager* m_codeManager;
    QFileSystemWatcher* m_watcher;
    QMap<QString, ComponentEntry> m_entries;
};
