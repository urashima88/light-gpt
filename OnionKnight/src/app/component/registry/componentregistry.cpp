#include "componentregistry.h"
#include "componentcodemanager.h"
#include <QFileInfo>

ComponentRegistry::ComponentRegistry(ComponentCodeManager* codeManager,
                                     QFileSystemWatcher* watcher,
                                     QObject* parent)
    : m_codeManager(codeManager)
    , m_watcher(watcher)
    , QObject(parent)
{
    m_codeManager->setParent(this);
    m_watcher->setParent(this);

    connect(m_codeManager, &ComponentCodeManager::metaReady,
            this, &ComponentRegistry::onCodeManagerMetaReady);
    connect(m_codeManager, &ComponentCodeManager::errorOccurred,
            this, &ComponentRegistry::onCodeManagerError);
    connect(m_watcher, &QFileSystemWatcher::fileChanged,
            this, &ComponentRegistry::onFileChanged);
}

void ComponentRegistry::registerComponent(const QString& typeId,
                                          const QString& filePath,
                                          const QString& virtualPath,
                                          const QString& iconPath,
                                          const QString& className,
                                          const QColor& color)
{
    ComponentEntry& entry = m_entries[typeId];
    if (entry.filePath == filePath && entry.virtualPath == virtualPath &&
        entry.iconPath == iconPath && entry.className == className &&
        entry.color == color &&
        (entry.pending || entry.meta.isValid()))
        return;

    entry.filePath = filePath;
    entry.virtualPath = virtualPath;
    entry.iconPath = iconPath;
    entry.className = className;
    entry.color = color;
    entry.meta = ComponentMeta();
    entry.pending = true;
    m_codeManager->inspectComponent(filePath, virtualPath, iconPath, color, className, typeId);
    if (QFileInfo::exists(filePath))
        m_watcher->addPath(filePath);
}

void ComponentRegistry::changeComponentSource(const QString& typeId,
                                              const QString& newFilePath,
                                              const QString& newVirtualPath,
                                              const QString& newIconPath,
                                              const QString& newClassName,
                                              const QColor& newColor)
{
    if (!m_entries.contains(typeId))
        return;

    ComponentEntry& entry = m_entries[typeId];
    if (!entry.filePath.isEmpty() && m_watcher->files().contains(entry.filePath))
        m_watcher->removePath(entry.filePath);

    entry.filePath = newFilePath;
    entry.virtualPath = newVirtualPath;
    entry.iconPath = newIconPath;
    entry.className = newClassName;
    entry.color = newColor;
    entry.meta = ComponentMeta();
    entry.pending = true;

    m_codeManager->inspectComponent(newFilePath, newVirtualPath, newIconPath, newColor, newClassName, typeId);

    if (QFileInfo::exists(newFilePath))
        m_watcher->addPath(newFilePath);
}

ComponentMeta ComponentRegistry::currentMeta(const QString& typeId) const
{
    return m_entries.value(typeId).meta;
}

void ComponentRegistry::requestMetaFromSource(const QString& typeId,
                                              const QString& filePath,
                                              const QString& virtualPath,
                                              const QString& iconPath,
                                              const QString& className,
                                              const QColor& color)
{
    m_entries[typeId].pending = true;
    m_codeManager->inspectComponent(filePath, virtualPath, iconPath, color, className, typeId);
}

void ComponentRegistry::onCodeManagerMetaReady(const QString& typeId, const ComponentMeta& meta)
{
    if (!m_entries.contains(typeId))
        return;

    ComponentEntry& entry = m_entries[typeId];
    entry.meta = meta;
    entry.pending = false;
    emit metaReady(typeId, meta);
}

void ComponentRegistry::onCodeManagerError(const QString& typeId, const QString& error)
{
    if (!m_entries.contains(typeId))
        return;

    m_entries[typeId].pending = false;
    emit metaError(typeId, error);
}

void ComponentRegistry::onFileChanged(const QString& path)
{
    QString typeId;
    for (auto it = m_entries.begin(); it != m_entries.end(); ++it) {
        if (it.value().filePath == path) {
            typeId = it.key();
            break;
        }
    }
    if (typeId.isEmpty())
        return;

    const ComponentEntry& entry = m_entries[typeId];

    requestMetaFromSource(typeId, entry.filePath, entry.virtualPath, entry.iconPath, entry.className, entry.color);

    if (!m_watcher->files().contains(path) && QFileInfo::exists(path)) {
        m_watcher->addPath(path);
    }
}

QString ComponentRegistry::getIconPath(const QString& typeId) const
{
    return m_entries[typeId].iconPath;
}

QColor ComponentRegistry::getColor(const QString& typeId) const
{

    return m_entries[typeId].color;
}

const QMap<QString, ComponentRegistry::ComponentEntry>& ComponentRegistry::allEntries() const {
    return m_entries;
}