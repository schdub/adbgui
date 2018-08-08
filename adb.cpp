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

// ctor

adb::adb() {
    // initialize static variable with path to adb executable
    if (sAdbPath.isEmpty()) {
//		QString path = QString("%1/adb.exe").arg(QDir::currentPath());
//        if (QFile::exists(path)) {
//            sAdbPath = path;
//        } else {
			QSettings s("adbgui.ini", QSettings::IniFormat);
			QString path = s.value("adbPath").toString();
            if (!path.isEmpty()) {
                sAdbPath = QString("%1/adb%2").arg(path).arg(ADB_EXT);
				if (!QFile::exists(sAdbPath)) {
					sAdbPath.clear();
				}
			}
//		}
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
                sAdbPath = QString("%1/platform-tools/adb").arg(envpath);
            }
        }
        // lets suppose that path to adb in your PATH variable
        if (sAdbPath.isEmpty()) {
            sAdbPath.append("adb");
        }

//        if (sAdbPath.isEmpty()) {
//            QMessageBox::warning(NULL, QObject::tr("Ошибка"),
//                QObject::tr("Не известен путь к adb.\n"
//                            "Укажите где он находится в ключе adbPath файла adbgui.ini,\n"
//                            "например, adbPath=c:/android/sdk/platform-tools"));
//        }
    }
}

// adb path

const QString& adb::path() const {
    return sAdbPath;
}

// run adb process

QString adb::run(const QStringList &argv, bool ignoreErrors) {
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
	}

    // reading process output
    QString out(proc.readAll());
    if (!ignoreErrors) {
        // check for errors
        if (!ok || out.contains("error") || out.contains("Failure")) {
            if (out.isEmpty())
                out = QObject::tr("Parsing data from adb process failed.");
            QMessageBox::warning(NULL, QObject::tr("Error"), out);
            return QString();
        }
    }
    return out;
}
