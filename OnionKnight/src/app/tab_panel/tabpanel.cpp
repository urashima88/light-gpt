#include "tabpanel.h"
#include "componenttabwidget.h"

#include <QLabel>
#include <QIcon>
#include <QApplication>
#include <QStyle>
#include <QVBoxLayout>

TabPanel::TabPanel(
    ComponentRegistry* registry,
    const QString& componentsTabIconPath,
    QWidget* parent
)
    : QWidget(parent)
    , m_componentsTabIconPath(componentsTabIconPath)
{
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    m_tabWidget = new QTabWidget(this);
    m_tabWidget->setTabPosition(QTabWidget::North);
    m_tabWidget->setDocumentMode(true);

    setupTabs(registry);
    layout->addWidget(m_tabWidget);
}

void TabPanel::setupTabs(ComponentRegistry* registry)
{
    QIcon componentsTabIcon(m_componentsTabIconPath);
    if (componentsTabIcon.isNull())
        componentsTabIcon = style()->standardIcon(QStyle::SP_FileDialogContentsView);

    auto* componentsTab = new ComponentTabWidget(registry);
    m_tabWidget->addTab(componentsTab, componentsTabIcon, tr("Components"));

}

