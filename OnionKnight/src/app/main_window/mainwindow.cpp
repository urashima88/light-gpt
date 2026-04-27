#include "mainwindow.h"
#include "workfield.h"
#include "workfieldview.h"

#include <QStatusBar>
#include <QMenuBar>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    setWindowTitle(tr("OnionKnight"));
    resize(1200, 800);

    initScene();
}

MainWindow::~MainWindow()
{
    delete m_workField;
    delete m_workFieldView;
}

void MainWindow::initScene()
{
    m_workField = new WorkField(this);
    m_workFieldView = new WorkFieldView(this);
    m_workFieldView->setScene(m_workField);

    setCentralWidget(m_workFieldView);
}