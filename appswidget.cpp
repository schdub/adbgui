#include "appswidget.h"
#include "ui_appswidget.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QDebug>
#include "adb.h"

AppsWidget::AppsWidget(QWidget *parent)
: QWidget(parent)
, ui(new Ui::AppsWidget)
{
    ui->setupUi(this);
}

AppsWidget::~AppsWidget() {
    delete ui;
}

// set .apk file for installation

void AppsWidget::on_btBrowseToInstall_clicked() {
    QString fileName = QFileDialog::getOpenFileName(this,
         tr("Open file..."), QString(), tr("APK files (*.apk)"));
    if (fileName.isEmpty()) return;
    ui->editApkToInstall->setText(fileName);
}

// click on button 'Installation'

void AppsWidget::on_btApkInstall_clicked() {
    QString pathToInstall(ui->editApkToInstall->text());
    if (pathToInstall.isEmpty()) return;

    // prepare command for adb

    QStringList argv;
    argv << "install";
    if (ui->checkReinstall->isChecked()) argv << "-r";
    argv << ADB_PATH(pathToInstall);

    // run adb process

    QString out(adb().run(argv));
    if (!out.isEmpty() && out.endsWith("Success")) {
        ui->editApkToInstall->clear();
        QMessageBox::information(this,
            tr("Install APK"),
            QString("Executed with result:\n%1").arg(out));
    }
}

// refres list of installed applications

void AppsWidget::on_btAppsRefresh_clicked() {
    QStringList argv;
    argv << "shell";
    argv << "pm list packages";
    QString out(adb().run(argv));
    if (!out.isEmpty()) {
        QListWidget * lv = ui->listInstalledApps;
        lv->clear();
        foreach (const QString & s, out.split("\r\n", QString::SkipEmptyParts)) {
            new QListWidgetItem(s.mid(8).simplified(), lv);
        }
    }
}
