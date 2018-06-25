#ifndef LOGSWIDGET_H
#define LOGSWIDGET_H

#include <QWidget>
#include <QTimer>
#include <QProcess>
#include <QFile>

namespace Ui {
class LogsWidget;
}

class LogsWidget : public QWidget {
    Q_OBJECT
public:
    explicit LogsWidget(QWidget *parent = 0);
    ~LogsWidget();
private slots:
    void on_btLogStart_clicked();
    void on_btLogStop_clicked();
    void on_btLogPathBrowse_clicked();
    void on_spinTopInterval_valueChanged(int value);
    void on_btMetrStart_clicked();
    void on_btMetrStop_clicked();
    void timerTop();
    void logReadReady();
private:
    Ui::LogsWidget* ui;
    QProcess mProcLogcat;
    QTimer mTimerTop;
    QFile *mLogFile;
};

#endif // LOGSWIDGET_H
