#include "componentregistry.h"
#include "componentinspector.h"
#include <QFileInfo>

ComponentRegistry::ComponentRegistry(ComponentInspector* inspector,
                                     QFileSystemWatcher* watcher,
                                     QObject* parent)
    : m_inspector(inspector)
    , m_watcher(watcher)
    , QObject(parent)
{
    m_inspector->setParent(this);
    m_watcher->setParent(this);

    connect(m_inspector, &ComponentInspector::metaReady,
            this, &ComponentRegistry::onInspectorMetaReady);
    connect(m_inspector, &ComponentInspector::errorOccured,
            this, &ComponentRegistry::onInspectorError);
    connect(m_watcher, &QFileSystemWatcher::fileChanged,
            this, &ComponentRegistry::onFileChanged);
}

void ComponentRegistry::registerComponent(const QString& typeId,
                                          const QString& modulePath,
                                          const QString& virtualPath,
                                          const QString& iconPath,
                                          const QString& className)
{
    ComponentEntry& entry = m_entries[typeId];
    if (entry.modulePath == modulePath && entry.virtualPath == virtualPath &&
        entry.iconPath == iconPath && entry.className == className &&
        (entry.pending || entry.meta.isValid()))
        return;

    entry.modulePath = modulePath;
    entry.virtualPath = virtualPath;
    entry.iconPath = iconPath;
    entry.className = className;
    entry.meta = ComponentMeta();
    entry.pending = true;

    m_inspector->inspect(modulePath, virtualPath, iconPath, className, typeId);
    if (QFileInfo::exists(modulePath))
        m_watcher->addPath(modulePath);
}

void ComponentRegistry::changeComponentSource(const QString& typeId,
                                              const QString& newModulePath,
                                              const QString& newVirtualPath,
                                              const QString& newIconPath,
                                              const QString& newClassName)
{
    if (!m_entries.contains(typeId))
        return;

    ComponentEntry& entry = m_entries[typeId];
    if (!entry.modulePath.isEmpty() && m_watcher->files().contains(entry.modulePath))
        m_watcher->removePath(entry.modulePath);

    entry.modulePath = newModulePath;
    entry.virtualPath = newVirtualPath;
    entry.iconPath = newIconPath;
    entry.className = newClassName;
    entry.meta = ComponentMeta();
    entry.pending = true;

    m_inspector->inspect(newModulePath, newVirtualPath, newIconPath, newClassName, typeId);

    if (QFileInfo::exists(newModulePath))
        m_watcher->addPath(newModulePath);
}

ComponentMeta ComponentRegistry::currentMeta(const QString& typeId) const
{
    return m_entries.value(typeId).meta;
}

void ComponentRegistry::requestMetaFromSource(const QString& typeId,
                                              const QString& modulePath,
                                              const QString& virtualPath,
                                              const QString& iconPath,
                                              const QString& className)
{
    m_entries[typeId].pending = true;
    m_inspector->inspect(modulePath, virtualPath, className, typeId);
}

void ComponentRegistry::onInspectorMetaReady(const QString& tag, const ComponentMeta& meta)
{
    if (!m_entries.contains(tag))
        return;

    ComponentEntry& entry = m_entries[tag];
    entry.meta = meta;
    entry.pending = false;
    emit metaReady(tag, meta);
}

void ComponentRegistry::onInspectorError(const QString& tag, const QString& error)
{
    if (!m_entries.contains(tag))
        return;

    m_entries[tag].pending = false;
    emit metaError(tag, error);
}

void ComponentRegistry::onFileChanged(const QString& path)
{
    QString typeId;
    for (auto it = m_entries.begin(); it != m_entries.end(); ++it) {
        if (it.value().modulePath == path) {
            typeId = it.key();
            break;
        }
    }
    if (typeId.isEmpty())
        return;

    const ComponentEntry& entry = m_entries[typeId];

    requestMetaFromSource(typeId, entry.modulePath, entry.virtualPath, entry.iconPath, entry.className);

    if (!m_watcher->files().contains(path) && QFileInfo::exists(path)) {
        m_watcher->addPath(path);
    }
}

QString ComponentRegistry::getIconPath(const QString& typeId) const
{
    return m_entries[typeId].iconPath;
}

const QMap<QString, ComponentRegistry::ComponentEntry>& ComponentRegistry::allEntries() const {
    return m_entries;
}