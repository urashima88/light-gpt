#include "mainwindow.h"
#include "workfield.h"
#include "workfieldview.h"
#include "componentinspector.h"
#include "componentregistry.h"
#include "componentitem.h"

#include <QFileInfo>
#include <QCoreApplication>
#include <QDir>

class ComponentItem;

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    setWindowTitle(tr("OnionKnight"));
    resize(1200, 800);

    initScene();
    initRegistry();

    auto* componentItem = new ComponentItem("Linear", m_componentRegistry);
    m_workField->addItem(componentItem);
    QPointF viewCenter = m_workFieldView->mapToScene(
        m_workFieldView->viewport()->rect().center());
    componentItem->setPos(viewCenter - QPointF(60, 30));
}

MainWindow::~MainWindow() {}

void MainWindow::initScene()
{
    m_workField = new WorkField(this);
    m_workFieldView = new WorkFieldView(this);
    m_workFieldView->setScene(m_workField);

    setCentralWidget(m_workFieldView);
}

void MainWindow::initRegistry()
{
    QString appDir = QCoreApplication::applicationDirPath();
    QDir root(appDir);
    root.cdUp();
    root.cdUp();
    root.cdUp();

    QString pythonExe = root.filePath("venv/Scripts/python.exe");
    QString inspectorScript = root.filePath("OnionKnight/scripts/component_inspector.py");

    ComponentInspector* inspector = new ComponentInspector(pythonExe, inspectorScript);
    QFileSystemWatcher* watcher = new QFileSystemWatcher();

    m_componentRegistry = new ComponentRegistry(inspector, watcher, this);
    m_componentRegistry->setIconBasePath(root.filePath("OnionKnight/icons"));

    m_componentRegistry->registerComponent("Linear",
                                  root.filePath("lib/ml/layers/linear/linear.py"),
                                  "CastedLinear");
}