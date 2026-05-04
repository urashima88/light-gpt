#pragma once

#include <QWidget>
#include <QTabWidget>

class ComponentRegistry;

class TabPanel: public QWidget
{
    Q_OBJECT

public:
    explicit TabPanel(
        ComponentRegistry* registry,
        const QString& componentsTabIconPath,
        QWidget* parent = nullptr
    );

private:
    QTabWidget* m_tabWidget;
    QString m_componentsTabIconPath;

    void setupTabs(ComponentRegistry* registry);
};
