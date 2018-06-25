#include "mainwindow.h"
#include "mainwidget.h"

MainWindow::MainWindow(QWidget *parent)
: QMainWindow(parent) {
    setFixedSize(800, 600);
    setCentralWidget(new MainWidget(this));
}

MainWindow::~MainWindow() {
}
