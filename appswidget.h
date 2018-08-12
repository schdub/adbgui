#ifndef APPSWIDGET_H
#define APPSWIDGET_H

#include <QWidget>

namespace Ui {
class AppsWidget;
}

class AppsWidget : public QWidget {
    Q_OBJECT
public:
    explicit AppsWidget(QWidget *parent = 0);
    ~AppsWidget();
private slots:
    void on_btBrowseToInstall_clicked();
    void on_btApkInstall_clicked();
    void on_btAppsRefresh_clicked();

private:
    Ui::AppsWidget *ui;
};

#endif // APPSWIDGET_H
