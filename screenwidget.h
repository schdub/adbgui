#ifndef SCREENWIDGET_H
#define SCREENWIDGET_H

#include <QWidget>
#include <QPixmap>
#include <QTimer>

#include "imagewidget.h"

namespace Ui {
class ScreenWidget;
}

class ScreenWidget : public QWidget {
    Q_OBJECT

    ImageWidget * mImageWidget;

public:
    explicit ScreenWidget(QWidget *parent = nullptr);
    ~ScreenWidget();

    void screenShot();

public slots:
    void onTimer();

protected:
    void showEvent(QShowEvent *);
    void hideEvent(QHideEvent *);

private:
    Ui::ScreenWidget *ui;
    QPixmap mPixmap;
    QTimer *mTimer;
};

#endif // SCREENWIDGET_H
