#include "mainwindow.h"
#include "workfield.h"
#include "workfieldview.h"
#include "componentinspector.h"
#include "componentregistry.h"
#include "componentitem.h"

#include <QFileInfo>
#include <QCoreApplication>


class ComponentItem;

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    setWindowTitle(tr("OnionKnight"));
    resize(1200, 800);

    QString appDir = QCoreApplication::applicationDirPath();
    QDir root(appDir);
    root.cdUp();
    root.cdUp();
    root.cdUp();

    initRegistry(root);
    initScene();
    setupLayout(root);

    auto* componentItem = new ComponentItem("Linear", m_registry);
    m_workField->addItem(componentItem);
    QPointF viewCenter = m_workFieldView->mapToScene(
        m_workFieldView->viewport()->rect().center());
    componentItem->setPos(viewCenter - QPointF(60, 30));
    componentItem->animateAppearance();
}

MainWindow::~MainWindow() {}

void MainWindow::initScene()
{
    m_workField = new WorkField(this);
    m_workFieldView = new WorkFieldView(m_registry, this);
    m_workFieldView->setScene(m_workField);

    setCentralWidget(m_workFieldView);
}

void MainWindow::initRegistry(const QDir& root)
{
    QString pythonExe = root.filePath("venv/Scripts/python.exe");
    QString inspectorScript = root.filePath("OnionKnight/scripts/component_inspector.py");

    ComponentInspector* inspector = new ComponentInspector(pythonExe, inspectorScript);
    QFileSystemWatcher* watcher = new QFileSystemWatcher();

    m_registry = new ComponentRegistry(inspector, watcher, this);

    m_registry->registerComponent("Linear",
                                  root.filePath("lib/ml/layers/linear/linear.py"),
                                  "lib/ml/layers/linear",
                                  root.filePath("OnionKnight/icons/linear.svg"),
                                  "CastedLinear");
}

void MainWindow::setupLayout(const QDir& root) {
    QString componentsTabPath = root.filePath("OnionKnight/icons/components_tab_icon.svg");

    setCentralWidget(m_workFieldView);

    m_tabPanel = new TabPanel(m_registry, componentsTabPath, this);

    const int panelWidth = 200;
    m_tabPanel->setFixedWidth(panelWidth);

    int parentHeight = this->height();
    int panelHeight = static_cast<int>(parentHeight * 0.7);
    m_tabPanel->setFixedHeight(panelHeight);

    int x = 10;
    int y = (parentHeight - panelHeight) / 2;
    m_tabPanel->move(x, y);

    m_tabPanel->raise();
}

void MainWindow::resizeEvent(QResizeEvent* event)
{
    QMainWindow::resizeEvent(event);
    if (m_tabPanel) {
        int parentHeight = this->height();
        int panelHeight = static_cast<int>(parentHeight * 0.7);
        m_tabPanel->setFixedHeight(panelHeight);
        int y = (parentHeight - panelHeight) / 2;
        m_tabPanel->move(10, y);
    }
}