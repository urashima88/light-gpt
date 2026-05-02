#include "componenttabwidget.h"
#include "componenttreemodel.h"
#include "componentfilterproxymodel.h"

#include <QVBoxLayout>
#include <QLineEdit>
#include <QTreeView>
#include <QHeaderView>

ComponentTabWidget::ComponentTabWidget(ComponentRegistry* registry, QWidget* parent)
    : QWidget(parent)
    , m_registry(registry)
{
    setupUi();
    m_model->rebuild();
    connectSearch();
}

void ComponentTabWidget::refresh()
{
    m_model->rebuild();
    m_treeView->collapseAll();
    for (int i = 0; i < m_proxyModel->rowCount(); ++i)
        m_treeView->expand(m_proxyModel->index(i, 0));
}

void ComponentTabWidget::setupUi()
{
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(4, 4, 4, 4);
    layout->setSpacing(2);

    // search field
    m_searchEdit = new QLineEdit(this);
    m_searchEdit->setPlaceholderText(tr("Search components..."));
    m_searchEdit->setClearButtonEnabled(true);
    layout->addWidget(m_searchEdit);

    // component tree
    m_treeView = new QTreeView(this);
    m_treeView->setHeaderHidden(true);
    m_treeView->setIndentation(12);
    m_treeView->setIconSize(QSize(20, 20));
    m_treeView->setAnimated(true);
    m_treeView->setExpandsOnDoubleClick(true);

    // drag & drop settings
    m_treeView->setDragEnabled(true);
    m_treeView->setDragDropMode(QAbstractItemView::DragOnly);
    m_treeView->setSelectionMode(QAbstractItemView::SingleSelection);

    layout->addWidget(m_treeView);

    m_model = new ComponentTreeModel(m_registry, QIcon(), QIcon(), this);
    m_proxyModel = new ComponentFilterProxyModel(this);
    m_proxyModel->setSourceModel(m_model);
    m_treeView->setModel(m_proxyModel);

    setMinimumWidth(220);
}

void ComponentTabWidget::connectSearch()
{
    connect(m_searchEdit, &QLineEdit::textChanged, this, [this](const QString& text) {
        m_proxyModel->setFilterString(text);
        if (text.isEmpty()) {
            m_treeView->collapseAll();
            for (int i = 0; i < m_proxyModel->rowCount(); ++i)
                m_treeView->expand(m_proxyModel->index(i, 0));
        } else {
            m_treeView->expandAll();
        }
    });
}