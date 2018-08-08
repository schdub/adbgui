#include "monkeywidget.h"
#include "ui_monkeywidget.h"
#include "adb.h"
#include <QMessageBox>

MonkeyWidget::MonkeyWidget(QWidget *parent)
: QWidget(parent)
, ui(new Ui::MonkeyWidget)
{
    ui->setupUi(this);
}

MonkeyWidget::~MonkeyWidget() {
    delete ui;
}

// start monkey test

void MonkeyWidget::on_btStart_clicked() {
    // prepare command
    QStringList argv;
    argv << "shell";
    argv << "monkey";
    if (ui->checkPackage->isChecked()) {
        QString pack(ui->linePackage->text().trimmed());
        if (!pack.isEmpty()) {
            argv[1].append(QString(" -p %1").arg(pack));
        }
    }
    if (ui->checkSysKeys->isChecked()) {
        argv[1].append(QString(" --pct-syskeys 0"));
    }
    argv[1].append(QString(" --throttle %1 %2")
                   .arg(ui->spinClickInterval->value())
                   .arg(ui->spinClickCount->value()));
    // ... run it and get its output
    ui->lineResults->clear();
    ui->lineResults->setPlainText(adb().run(argv, true));
}
