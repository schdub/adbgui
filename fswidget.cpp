#include "fswidget.h"
#include "ui_fswidget.h"

#include <QTreeWidgetItem>
#include <QFileDialog>
#include <QMessageBox>
#include <QDebug>
#include "adb.h"

FSWidget::FSWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FSWidget)
{
    ui->setupUi(this);
    ui->treeWidget->header()->setVisible(false);
    on_btSdCardRefresh_clicked();
}

FSWidget::~FSWidget() {
    delete ui;
}

// expanding TreeView item

void FSWidget::on_treeWidget_itemExpanded(QTreeWidgetItem *item) {
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
    argv << QString("ls -la 2>/dev/null %1").arg(path);

    // execute ...
    QString out(adb().run(argv, true));
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

void FSWidget::on_btSdCardRefresh_clicked() {
    ui->treeWidget->clear();
    QTreeWidgetItem *twi = TreeWidget::createChild("", "/", true);
    ui->treeWidget->insertTopLevelItem(0, twi);
}

// saving file

void FSWidget::on_btSave_clicked() {
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
