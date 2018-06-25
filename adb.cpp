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

// конструктор

adb::adb() {
    // инициализируем статическую переменную с путем к процессу adb
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

        // если путь к ADB не указан, то предполагаем, что он указан в PATH
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

// путь к файлу adb

const QString& adb::path() const {
    return sAdbPath;
}

// запуск процесса

QString adb::run(const QStringList &argv, bool ignoreErrors) {
    // создаем объект процесса
    QProcess proc;
    proc.setProcessChannelMode(QProcess::MergedChannels);
    proc.start(sAdbPath, argv);

    // ждем пока стартует
	bool ok = proc.waitForStarted();
    if (ok) {
        // ожидаем завершения, "прокачивая" gui события
		int i = 0;
		while (!proc.waitForFinished(500) && i<30) {
			qApp->processEvents();
			++i;
		}
		ok = (i < 30 && proc.exitCode() == 0);
	}

    // запускаем процесс
    QString out(proc.readAll());
    if (!ignoreErrors) {
        // проверяем есть ли ошибки
        if (!ok || out.contains("error") || out.contains("Failure")) {
            if (out.isEmpty())
                out = QObject::tr("Получение данных из процесса adb завершилось неудачей.");
            // показываем сообщение об ошибке
            QMessageBox::warning(NULL, QObject::tr("Ошибка"), out);
            return QString();
        }
    }
    // иначе возвращаем результат
    return out;
}
