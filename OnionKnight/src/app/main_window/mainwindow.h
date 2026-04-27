#pragma once

#include <QMainWindow>

class WorkField;
class WorkFieldView;

class MainWindow: public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

private:
    WorkField* m_workField = nullptr;
    WorkFieldView*  m_workFieldView = nullptr;

    void initScene();
};
