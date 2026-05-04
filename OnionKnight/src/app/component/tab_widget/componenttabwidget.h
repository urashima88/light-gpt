#pragma once

#include <QWidget>

class QLineEdit;
class QTreeView;
class ComponentTreeModel;
class ComponentFilterProxyModel;
class ComponentRegistry;

class ComponentTabWidget: public QWidget
{
    Q_OBJECT

public:
    explicit ComponentTabWidget(ComponentRegistry* registry, QWidget* parent = nullptr);

    void refresh();

private:
    ComponentRegistry* m_registry;
    QLineEdit* m_searchEdit;
    QTreeView* m_treeView;
    ComponentTreeModel* m_model;
    ComponentFilterProxyModel* m_proxyModel;

    void setupUi();
    void connectSearch();
};
