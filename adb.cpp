#include "adb.h"
#include <QString>
#include <QProcess>
#include <QMessageBox>
#include <QApplication>
#include <QSettings>
#include <QDebug>
#include <QFile>
#include <QDir>

QString adb::sAdbPath;
QString adb::sDevice;

// ctor

adb::adb() {
    // initialize static variable with path to adb executable
    if (sAdbPath.isEmpty()) {
        // maybe user specified in INI configuration
        QSettings s("adbgui.ini", QSettings::IniFormat);
        QString path = s.value("adbPath").toString();
        if (!path.isEmpty()) {
            sAdbPath = QString("%1/adb%2").arg(path).arg(ADB_EXT);
            if (!QFile::exists(sAdbPath)) {
                sAdbPath.clear();
            }
        }
        // try to find path to android SDK installation in environment variables
        // https://developer.android.com/studio/command-line/variables
        if (sAdbPath.isEmpty()) {
            const char * vars[] = {
                "ANDROID_SDK_ROOT",
                "ANDROID_HOME",
                "ANDROID_ROOT"
            };
            char * envpath = NULL;
            for (int i = 0, ie = sizeof(vars)/sizeof(*vars); i < ie && !(envpath = getenv(vars[i])); ++i);
            if (envpath) {
                sAdbPath = QString("%1/platform-tools/adb%2").arg(envpath).arg(ADB_EXT);
            }
        }
    }
    // lets suppose that path to adb in your PATH variable
    if (sAdbPath.isEmpty()) {
        sAdbPath.append(QString("adb%1").arg(ADB_EXT));
    }
}

// adb path

const QString& adb::path() const {
    return sAdbPath;
}

// run adb process

QByteArray adb::run(QStringList argv, bool ignoreErrors) {
    // current device to work with
    if (!sDevice.isEmpty()) {
        argv.insert(0, "-s");
        argv.insert(1, sDevice);
    }

    // create process
    QProcess proc;
    proc.setProcessChannelMode(QProcess::MergedChannels);
    proc.start(sAdbPath, argv);

    // waiting while starting
	bool ok = proc.waitForStarted();
    if (ok) {
        // waiting finished and don't forget to process gui events
		int i = 0;
		while (!proc.waitForFinished(500) && i<30) {
			qApp->processEvents();
			++i;
		}
		ok = (i < 30 && proc.exitCode() == 0);
        if (proc.exitCode() != 0) {
            qDebug() << "exitCode =" << proc.exitCode();
        }
	}

    // reading process output
    QByteArray out(proc.readAll());
    if (ignoreErrors) {
        ok = out.size() > 0;
    } else {
        // check for errors
        if (!ok || out.contains("error") || out.contains("Failure")) {
            if (out.isEmpty())
                out = QObject::tr("Parsing data from adb process failed.").toLatin1();
            QMessageBox::warning(NULL, QObject::tr("Error"), out);
            return QByteArray();
        }
    }
    return out;
}

void adb::setDevice(const QString & deviceName) {
    sDevice = deviceName;
}

QByteArray adb::tap(int x, int y) {
    QStringList argv;
    argv << "shell";
    argv << "input";
    argv << "tap";
    argv << QString::number( x );
    argv << QString::number( y );
    QByteArray out(adb().run(argv));
    return out;
}

QByteArray adb::swipe(int startX, int startY, int endX, int endY, qint64 msec) {
    QStringList argv;
    argv << "shell";
    argv << "input";
    argv << "swipe";
    argv << QString::number( startX );
    argv << QString::number( startY );
    argv << QString::number( endX );
    argv << QString::number( endY );
    argv << QString::number( msec );
    QByteArray out(adb().run(argv));
    return out;
}
