#include "screenwidget.h"
#include "ui_screenwidget.h"
#include "imagewidget.h"

#include <QDebug>
#include <QShowEvent>
#include <QHideEvent>
#include <QPixmap>

#include "adb.h"

ScreenWidget::ScreenWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ScreenWidget),
    mTimer(nullptr)
{
    ui->setupUi(this);

    mTimer = new QTimer(this);
    mTimer->setInterval(500);
    connect(mTimer, SIGNAL(timeout()), this, SLOT(onTimer()));
}

ScreenWidget::~ScreenWidget() {
    delete ui;
}

// adb exec-out screencap -p > screen.png && xdg-open screen.png

void ScreenWidget::hideEvent(QHideEvent *) {
    qDebug() << __FUNCTION__;
    mTimer->stop();
}

void ScreenWidget::showEvent(QShowEvent *) {
    qDebug() << __FUNCTION__;
    mTimer->start(100);
}

void ScreenWidget::onTimer() {
    screenShot();
}

void ScreenWidget::screenShot() {
    qDebug() << __FUNCTION__;

    QStringList argv;
    argv << "exec-out";
    argv << "screencap";
    argv << "-p";
    QByteArray out(adb().run(argv));

    if (out.size() == 0) {
        // error of parsing output from ADB
        mTimer->stop();
    } else {
        ui->widget->setPixmap( out );
    }
}
