#include "mainwidget.h"
#include "ui_mainwidget.h"
#include "appswidget.h"
#include "logswidget.h"
#include "monkeywidget.h"
#include "fswidget.h"
#include "adb.h"

MainWidget::MainWidget(QWidget *parent)
: QWidget(parent)
, ui(new Ui::MainWidget) {
    ui->setupUi(this);
    ui->tabWidget->addTab(new FSWidget(),     tr("Files"));
    ui->tabWidget->addTab(new AppsWidget(),   tr("Apps"));
    ui->tabWidget->addTab(new LogsWidget(),   tr("Logs"));
    ui->tabWidget->addTab(new MonkeyWidget(), tr("Advanced"));
    on_btRefresh_clicked();
}

MainWidget::~MainWidget() {
    delete ui;
}

// refresh devices list

void MainWidget::on_btRefresh_clicked() {
    QString out(adb().run(QStringList() << "devices"));
    if (!out.isEmpty()) {
        int i = 0;
        QStringList sl(out.split("\n", QString::SkipEmptyParts));
        for (;i < sl.size() && !sl[i].startsWith("List of devices attached"); ++i);
        ui->cbDevices->clear();
        int current = -1;
        for (++i; i < sl.size(); ++i) {
            ui->cbDevices->addItem(sl[i]);
            if (current == -1 && sl[i].endsWith("\tdevice")) {
                current = ui->cbDevices->count()-1;
            }
        }
        ui->cbDevices->setCurrentIndex(current);
    }
}
