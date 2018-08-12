#include "logswidget.h"
#include "ui_logswidget.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QVector>
#include "adb.h"

LogsWidget::LogsWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::LogsWidget)
    , mTimerTop(this)
    , mLogFile(0)
{
    ui->setupUi(this);
    ui->tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableWidget->horizontalHeader()->setStretchLastSection(true);
    ui->tableWidget->setColumnCount(2);
    ui->tableWidget->setHorizontalHeaderLabels(
        QStringList() << tr("Metric") << tr("Process")
    );
    ui->radioMem->setChecked(true);
    ui->spinTopInterval->setValue(10);
    connect(&mTimerTop, SIGNAL(timeout()), this, SLOT(timerTop()));
    connect(&mProcLogcat, SIGNAL(readyRead()), this, SLOT(logReadReady()));
}

LogsWidget::~LogsWidget() {
    delete ui;
}

// tick of metrics getter timer

void LogsWidget::timerTop() {
    bool failed = true;

    // prepare command
    QStringList argv;
    argv << "shell";
    argv << QString("top -n 1 -s %1")
                   .arg(ui->radioCPU->isChecked() ? "cpu" : "rss");

    // execute it
    QString all(adb().run(argv));
    if (!all.isEmpty()){
        QStringList sl = all.split("\r\n");
        // find headers
        int i;
        bool found = false;
        for (i = 0; i < sl.size(); ++i) {
            if (sl[i].contains("PID")) {
                found = true;
                ++i;
                break;
            }
        }
        if (found) {
            // filter
            QVector<int> twis;
            QString fmt = ui->lineTopFilter->text();
            for (; i < sl.size(); ++i) {
                QStringList items(sl[i].split(" ", QString::SkipEmptyParts));
                if (items.size() < 10) continue;
                if (fmt.isEmpty() || items[9].contains(fmt)) {
                    twis.push_back(i);
                }
            }

            ui->tableWidget->setRowCount(twis.size());
            i = ui->radioCPU->isChecked() ? 2 : 6;
            for (int row = 0; row < twis.size(); ++row) {
                // split string by ' '
                QStringList items(sl[twis[row]].split(" ", QString::SkipEmptyParts));
                // create items
                QTableWidgetItem * twi;
                twi = new QTableWidgetItem(items[i]);
                twi->setFlags(Qt::ItemIsEnabled);
                twi->setTextAlignment(Qt::AlignCenter);
                ui->tableWidget->setItem(row, 0, twi);
                twi = new QTableWidgetItem(items[9]);
                twi->setFlags(Qt::ItemIsEnabled);
                twi->setTextAlignment(Qt::AlignCenter);
                ui->tableWidget->setItem(row, 1, twi);
            }
            failed = false;
        }
    }

    if (failed) {
        mTimerTop.stop();
    }
}

// LogCat logs getting start

void LogsWidget::on_btLogStart_clicked() {
    QString fileName(ui->lineLogPath->text());
    if (fileName.isEmpty()) return;

    ui->btLogStart->setEnabled(false);
    ui->btLogStop->setEnabled(true);

    mProcLogcat.close();
    if (mLogFile) {
        mLogFile->close();
        delete mLogFile;
        mLogFile = 0;
    }

    mLogFile = new QFile(fileName);
    if (!mLogFile->open(QIODevice::WriteOnly)) {
        delete mLogFile;
        mLogFile = 0;
        QMessageBox::warning(this, tr("Error"),
            QString("Can't create file '%1'.").arg(fileName));
        return;
    }

    mProcLogcat.setProcessChannelMode(QProcess::MergedChannels);
    mProcLogcat.start(adb().path(), QStringList() << "logcat");

    if (mProcLogcat.waitForStarted() == false ||
        mProcLogcat.isOpen() == false) {
        mLogFile->close();
        delete mLogFile;
        mLogFile = 0;
        QMessageBox::warning(this,
            tr("Error"),
            mProcLogcat.errorString());
    }
}

// LogCat logs getting stop

void LogsWidget::on_btLogStop_clicked() {
    mProcLogcat.close();
    ui->btLogStart->setEnabled(true);
    ui->btLogStop->setEnabled(false);
}

// file opena for LogCat logs

void LogsWidget::on_btLogPathBrowse_clicked() {
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save file..."));
    if (fileName.isEmpty()) return;
    ui->lineLogPath->setText(fileName);
}

// metrics request interval

void LogsWidget::on_spinTopInterval_valueChanged(int value) {
    mTimerTop.setInterval(value * 1000);
}

// start of metrics getting

void LogsWidget::on_btMetrStart_clicked() {
    ui->btMetrStart->setEnabled(false);
    ui->btMetrStop->setEnabled(true);
    if (mTimerTop.isActive()) return;
    mTimerTop.start();
    timerTop();
}

// end of metrics getting

void LogsWidget::on_btMetrStop_clicked() {
    mTimerTop.stop();
    ui->btMetrStart->setEnabled(true);
    ui->btMetrStop->setEnabled(false);
}

// ready to read from logcat process

void LogsWidget::logReadReady() {
    if (mLogFile) {
        mLogFile->write(mProcLogcat.readAll());
    }
}
