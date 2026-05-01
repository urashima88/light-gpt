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
    void setupTabs(ComponentRegistry* registry);

    QTabWidget* m_tabWidget;
    QString m_componentsTabIconPath;
};
