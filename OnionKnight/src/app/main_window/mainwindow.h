#pragma once

#include "componentregistry.h"
#include "tabpanel.h"
#include <QMainWindow>
#include <QDir>

class WorkField;
class WorkFieldView;
class ComponentItem;

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
    TabPanel* m_tabPanel;

    void initScene();
    void initRegistry(const QDir& root);
    void setupLayout(const QDir& root);
};
