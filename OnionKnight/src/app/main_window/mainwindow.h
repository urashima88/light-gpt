#pragma once

#include "componentregistry.h"
#include "tabpanel.h"
#include <QMainWindow>
#include <QDir>

class WorkField;
class WorkFieldView;
class ComponentItem;
class ComponentCodeManager;

class MainWindow: public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

protected:
    void resizeEvent(QResizeEvent* event) override;

private:
    WorkField* m_workField = nullptr;
    WorkFieldView*  m_workFieldView = nullptr;
    ComponentRegistry* m_registry = nullptr;
    ComponentCodeManager* m_codeManager = nullptr;
    TabPanel* m_tabPanel;

    void initScene();
    void initRegistry(const QDir& root);
    void setupLayout(const QDir& root);

    void onComponentDropped(const QString& typeId, QPointF scenePos);
    void createModelContainer(const QString& filePath, const QString& className);
};
