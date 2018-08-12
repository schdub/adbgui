#ifndef FSWIDGET_H
#define FSWIDGET_H

#include <QWidget>

namespace Ui {
class FSWidget;
}

class QTreeWidgetItem;

class FSWidget : public QWidget {
    Q_OBJECT
public:
    explicit FSWidget(QWidget *parent = 0);
    ~FSWidget();
private slots:
    void on_btSdCardRefresh_clicked();
    void on_btSave_clicked();
    void on_treeWidget_itemExpanded(QTreeWidgetItem *item);

private:
    Ui::FSWidget *ui;
};

#endif // FSWIDGET_H
