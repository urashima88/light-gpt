#include "mainwindow.h"
#include "workfield.h"
#include "workfieldview.h"
#include "componentregistry.h"
#include "componentcodemanager.h"
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

    connect(m_workFieldView, &WorkFieldView::componentDropped, this, &MainWindow::onComponentDropped);

    auto* model = new ComponentItem(QStringLiteral("Model"), m_registry);
    model->setVariableName(QStringLiteral("model"));
    model->setComponentCodeManager(m_codeManager);
    model->setExpanded(true);
    m_workField->addItem(model);
    model->setPos(-100, -100);
    model->animateAppearance();
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
    QString editorScript = root.filePath("OnionKnight/scripts/component_code_editor.py");

    ComponentCodeManager* m_codeManager = new ComponentCodeManager(pythonExe, editorScript, this);

    QFileSystemWatcher* watcher = new QFileSystemWatcher();

    m_registry = new ComponentRegistry(m_codeManager, watcher, this);

    m_registry->registerComponent(QStringLiteral("Linear"),
                                  root.filePath("lib/ml/layers/linear/linear.py"),
                                  QStringLiteral("lib/ml/layers/linear"),
                                  root.filePath("OnionKnight/icons/linear.svg"),
                                  QStringLiteral("CastedLinear"),
                                  QColor(41, 163, 25));

    m_registry->registerComponent(QStringLiteral("Model"),
                                  root.filePath("models/model.py"),
                                  QStringLiteral("models"),
                                  root.filePath("OnionKnight/icons/model.svg"),
                                  QStringLiteral("Model"),
                                  QColor(0, 0, 205));
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

void MainWindow::onComponentDropped(const QString& typeId, QPointF scenePos)
{
    ComponentItem* container = nullptr;
    const QList<QGraphicsItem*> items = m_workField->items(scenePos, Qt::IntersectsItemBoundingRect);
    for (auto* item : items) {
        if (auto* comp = dynamic_cast<ComponentItem*>(item)) {
            if (comp->isExpanded()) {
                container = comp;
                break;
            }
        }
    }

    auto* newItem = new ComponentItem(typeId, m_registry);
    if (!newItem) return;

    if (container) {
        container->addChild(newItem);

        const QString baseName = typeId.toLower();
        int count = 0;
        for (auto* child : container->childItems()) {
            if (child->typeId() == typeId)
                ++count;
        }
        newItem->setVariableName(QStringLiteral("%1%2").arg(baseName).arg(count + 1));

        const QPointF localPos = container->mapFromScene(scenePos);
        newItem->setPos(localPos);

        newItem->animateAppearance();

        container->updateCodeFromChildren();
    } else {
        m_workField->addItem(newItem);
        newItem->setPos(scenePos);
        newItem->animateAppearance();
    }
}
