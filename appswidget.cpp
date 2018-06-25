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

// открыть .apk файл для инсталляции

void AppsWidget::on_btBrowseToInstall_clicked() {
    QString fileName = QFileDialog::getOpenFileName(this,
         tr("Открыть файл..."), QString(), tr("Файлы APK (*.apk)"));
    if (fileName.isEmpty()) return;
    ui->editApkToInstall->setText(fileName);
}

// нажатие кнопки исталляция

void AppsWidget::on_btApkInstall_clicked() {
    QString pathToInstall(ui->editApkToInstall->text());
    if (pathToInstall.isEmpty()) return;

    // формируем команду для adb

    QStringList argv;
    argv << "install";
    if (ui->checkReinstall->isChecked()) argv << "-r";
    argv << ADB_PATH(pathToInstall);

    // запускаем процесс adb

    QString out(adb().run(argv));
    if (!out.isEmpty() && out.endsWith("Success")) {
        ui->editApkToInstall->clear();
        QMessageBox::information(this,
            tr("Установка APK"),
            QString("Выполнено с результатом:\n%1").arg(out));
    }
}

// обновляем список установленных программ

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

// открытие элемента TreeView

void AppsWidget::on_treeWidget_itemExpanded(QTreeWidgetItem *item) {
    if (!TreeWidget::isFake(item)) {
        return;
    }

    // удаляем фейковый элемент
    QTreeWidgetItem *twi = item->child(0);
    item->removeChild(twi);
    delete twi;

    // получаем текущий путь
    QString path(item->data(0, Qt::UserRole).toString());

    // формируем команду для запуска получения текущей директории
    QStringList argv;
    argv << "shell";
    argv << QString("ls -la %1").arg(path);

    // подгружаем содержимое этой директории
    QString out(adb().run(argv));
    if (out.isEmpty()) return;

    // парсим и добавляем элементы
    foreach (const QString & s, out.split("\r\n", QString::SkipEmptyParts)) {
        if (s.size() <= 58) continue;
        int i = 58;
        while (i >= 0 && s[i] != ' ') --i;
        if (i) ++i;
        twi = TreeWidget::createChild(s.mid(i), s[0] == 'd' ? path : QString());
        item->addChild(twi);
    }
}

// пользователь нажал обновить TreeView

void AppsWidget::on_btSdCardRefresh_clicked() {
    ui->treeWidget->clear();
    QTreeWidgetItem *twi = TreeWidget::createChild("sdcard", "", true);
    ui->treeWidget->insertTopLevelItem(0, twi);
}

// пользователь нажал сохранить файл

void AppsWidget::on_btSave_clicked() {
    QTreeWidgetItem * twi = ui->treeWidget->currentItem();

    // 1) выделен ли элемент в дереве?
    // 2) это временный элемент?
    // 3) у элемента есть родитель?
    // 4) элемент - директория?

    if (!twi                    ||                 // 1
        TreeWidget::isFake(twi) ||                 // 2
        !twi->parent()          ||                 // 3
        !twi->data(0, Qt::UserRole).isNull())      // 4
        return;

    // выбираем куда сохранить файл

    QString localPath = QFileDialog::getSaveFileName(this,
        tr("Куда сохранять?"),
        twi->text(0));
    if (localPath.isEmpty()) return;

    // путь на стороне adb

    QString remotePath(QString("%1%2")
        .arg(twi->parent()->data(0, Qt::UserRole).toString())
        .arg(twi->text(0)));

    // подготавливаем команду

    QStringList argv;
    argv << "pull";
    argv << remotePath;
    argv << ADB_PATH(localPath);

    // запускаем adb

    QString out(adb().run(argv));
    if (!out.isEmpty()) {
        QMessageBox::information(this,
            tr("Копирование"),
            QString("Завершилось с результатом:\n%1").arg(out));
    }
}
