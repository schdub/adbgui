#ifndef ADB_H
#define ADB_H
#include <QString>
#include <QStringList>

class adb {
public:
    adb();
    QString run(const QStringList & argv = QStringList(), bool ignoreErrors = false);
    const QString& path() const;
private:
    static QString sAdbPath;
};

#ifdef Q_OS_WIN
#define ADB_EXT  ".exe"
#define ADB_PATH(s) s.replace('/', '\\')
#else
#define ADB_EXT ""
#define ADB_PATH(s) s
#endif

#endif // ADB_H
