#ifndef MONKEYWIDGET_H
#define MONKEYWIDGET_H

#include <QWidget>

namespace Ui {
class MonkeyWidget;
}

class MonkeyWidget : public QWidget {
    Q_OBJECT
public:
    explicit MonkeyWidget(QWidget *parent = 0);
    ~MonkeyWidget();
private slots:
    void on_btStart_clicked();
private:
    Ui::MonkeyWidget *ui;
};

#endif // MONKEYWIDGET_H
