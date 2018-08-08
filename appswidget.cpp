#include "appswidget.h"
#include "ui_appswidget.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QDebug>
#include <QTreeWidgetItem>
#include "adb.h"

AppsWidget::AppsWidget(QWidget *parent)
: QWidget(parent)
, ui(new Ui::AppsWidget)
{
    ui->setupUi(this);
    ui->treeWidget->header()->setVisible(false);
    on_btSdCardRefresh_clicked();
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

// expanding TreeView item

void AppsWidget::on_treeWidget_itemExpanded(QTreeWidgetItem *item) {
    if (!TreeWidget::isFake(item)) {
        return;
    }

    // delete 'fake' item
    QTreeWidgetItem *twi = item->child(0);
    item->removeChild(twi);
    delete twi;

    // current path
    QString path(item->data(0, Qt::UserRole).toString());

    // prepare command for getting contents of given path
    QStringList argv;
    argv << "shell";
    argv << QString("ls -la %1").arg(path);

    // execute ...
    QString out(adb().run(argv));
    if (out.isEmpty()) return;

    // parse output and adding items into TV
    foreach (const QString & s, out.split("\r\n", QString::SkipEmptyParts)) {
        if (s.size() <= 58) continue;
        int i = 58;
        while (i >= 0 && s[i] != ' ') --i;
        if (i) ++i;
        twi = TreeWidget::createChild(s.mid(i), s[0] == 'd' ? path : QString());
        item->addChild(twi);
    }
}

// user hit refresh button for TreeView

void AppsWidget::on_btSdCardRefresh_clicked() {
    ui->treeWidget->clear();
    QTreeWidgetItem *twi = TreeWidget::createChild("sdcard", "", true);
    ui->treeWidget->insertTopLevelItem(0, twi);
}

// saving file

void AppsWidget::on_btSave_clicked() {
    QTreeWidgetItem * twi = ui->treeWidget->currentItem();

    // 1) item selected in tree?
    // 2) this is 'fake' item?
    // 3) this item has parent?
    // 4) item - directory?

    if (!twi                    ||                 // 1
        TreeWidget::isFake(twi) ||                 // 2
        !twi->parent()          ||                 // 3
        !twi->data(0, Qt::UserRole).isNull())      // 4
        return;

    // lets coose when we should save file

    QString localPath = QFileDialog::getSaveFileName(this,
        tr("Saving file..."),
        twi->text(0));
    if (localPath.isEmpty()) return;

    // path on device

    QString remotePath(QString("%1%2")
        .arg(twi->parent()->data(0, Qt::UserRole).toString())
        .arg(twi->text(0)));

    // prepare command

    QStringList argv;
    argv << "pull";
    argv << remotePath;
    argv << ADB_PATH(localPath);

    // execute adb

    QString out(adb().run(argv));
    if (!out.isEmpty()) {
        QMessageBox::information(this,
            tr("Copying"),
            QString("Executed with result:\n%1").arg(out));
    }
}
