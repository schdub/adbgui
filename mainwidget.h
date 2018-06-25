#ifndef MAINWIDGET_H
#define MAINWIDGET_H

#include <QWidget>

namespace Ui {
class MainWidget;
}

class MainWidget : public QWidget {
    Q_OBJECT
public:
    explicit MainWidget(QWidget *parent = 0);
    ~MainWidget();
private slots:
    void on_btRefresh_clicked();
private:
    Ui::MainWidget *ui;
};

#endif // MAINWIDGET_H
