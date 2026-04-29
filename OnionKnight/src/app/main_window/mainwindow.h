#pragma once

#include "componentregistry.h"
#include <QMainWindow>

class WorkField;
class WorkFieldView;
class ComponentItem;

class MainWindow: public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

private:
    WorkField* m_workField = nullptr;
    WorkFieldView*  m_workFieldView = nullptr;
    ComponentRegistry* m_componentRegistry = nullptr;

    void initScene();
    void initRegistry();
};
