#include "treewidget.h"

#include <QDebug>
#include <QDragEnterEvent>
#include <QMimeData>
#include <QFileInfo>
#include "adb.h"

static QString sFakeName(":$FAKE$:");

TreeWidget::TreeWidget(QWidget *parent)
: QTreeWidget(parent) {
    setAcceptDrops(true);
}

TreeWidget::~TreeWidget() {
}

// поддерживаемые для drop типы mime

QStringList TreeWidget::mimeTypes() const {
    static QStringList ret = QStringList() << "text/uri-list";
    return ret;
}

// над деревом произошло событие drop, удовлетворяющее
// нашему заявленному mimeTypes()

bool TreeWidget::dropMimeData(
    QTreeWidgetItem *parent,
    int index,
    const QMimeData *data,
    Qt::DropAction action
) {
    Q_UNUSED(index);
    Q_UNUSED(action);

    // формируем командную строку adb для добавления файла

    QStringList argv;
    argv << "push";                                   // команда
    argv << "";                                       // локальный путь
    argv << parent->data(0, Qt::UserRole).toString(); // путь на устройстве

    bool ok = true;
    bool dontAppend = TreeWidget::isFake(parent);

    QTreeWidgetItem * twi;
    foreach (const QUrl &url, data->urls()) {
        // получаем путь
        argv[1] = url.toLocalFile();
        qDebug() << "Dropped file:" << argv[1];

        // если это не файл, то начинаем следующую итерацию

        QFileInfo fi(argv[1]);
        if (!fi.isFile()) continue;

        // запускаем процесс копирования

        ADB_PATH(argv[1]);
        QString out(adb().run(argv));
        if (out.isEmpty()) {
            // что-то пошло не так
            ok = false;
            break;
        }

        // если указатель на предка - временный элемент, то
        // ничего добавлять и не нужно

        if (dontAppend) continue;

        // иначе добавляем элемент в дерево

        twi = createChild(fi.fileName(), fi.isFile() ? QString() : argv[2]);
        parent->addChild(twi);
    }
    return ok;
}

// создаем новый элемент дерева

QTreeWidgetItem* TreeWidget::createChild(const QString & text, const QString & path, bool isRoot) {
    QTreeWidgetItem * item = new QTreeWidgetItem((QTreeWidget*)NULL);
    item->setText(0, text);
    if (path.isEmpty() && !isRoot) {
        // это файл
        item->setIcon(0, QIcon(":/img/Document-Blank-icon.png"));
    } else {
        // это директория
        item->setIcon(0, QIcon(":/img/Places-folder-blue-icon.png"));
        item->setData(0, Qt::UserRole, QString("%1/%2/").arg(path).arg(text));
        // добавляем элементу временный под-элемент
        // чтобы можно было попытаться открыть его
        QTreeWidgetItem * fake = new QTreeWidgetItem((QTreeWidget*)NULL);
        fake->setText(0, sFakeName);
        item->addChild(fake);
    }
    return item;
}

// это временный элемент?

bool TreeWidget::isFake(QTreeWidgetItem* item) {
    return (item->childCount() == 1 &&
            item->child(0)->text(0).compare(sFakeName) == 0);
}
