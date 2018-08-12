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
    argv << QString("ls -lA 2>/dev/null %1").arg(path);

    // execute ...
    QString out(adb().run(argv, true));
    if (out.isEmpty()) return;

    // parse output and adding items into TV
    foreach (const QString & s, out.split("\n", QString::SkipEmptyParts)) {
        // ignore error messages
        if (s.startsWith("ls: ")) continue;

        // split up every row
        QVector<QStringRef> rowItems = s.splitRef(" ", QString::SkipEmptyParts);

        // not enough peaces?
        if (rowItems.size() < 8) continue;

        // skip current and parent dirs
        bool isDir = (rowItems[0].at(0) == 'd');
        if (isDir && (rowItems[7] == "." || rowItems[7] == ".."))
            continue;

        // first and last indexes of name
        int lastIndex = s.size();
        int firstIndex = rowItems[7].position();

        bool isLink = (rowItems[0].at(0) == 'l');
#if (0)
        // cut name of link
        if (isLink)
            lastIndex = s.lastIndexOf(" -> ");
#endif
        // create tree view item
        QString itemName(s.mid(firstIndex, lastIndex - firstIndex));
        twi = TreeWidget::createChild(itemName, isDir ? path : QString());
        twi->setTextColor(0, isLink ? Qt::blue : Qt::black);
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
