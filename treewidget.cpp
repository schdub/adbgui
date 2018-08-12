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

// supported for drop mime types

QStringList TreeWidget::mimeTypes() const {
    static QStringList ret = QStringList() << "text/uri-list";
    return ret;
}

// drop event above widget

bool TreeWidget::dropMimeData(
    QTreeWidgetItem *parent,
    int index,
    const QMimeData *data,
    Qt::DropAction action
) {
    Q_UNUSED(index);
    Q_UNUSED(action);

    // prepare command

    QStringList argv;
    argv << "push";                                   // command
    argv << "";                                       // local path
    argv << parent->data(0, Qt::UserRole).toString(); // remote path

    bool ok = true;
    bool dontAppend = TreeWidget::isFake(parent);

    QTreeWidgetItem * twi;
    foreach (const QUrl &url, data->urls()) {
        // local path
        argv[1] = url.toLocalFile();
        qDebug() << "Dropped file:" << argv[1];

        // working only with files

        QFileInfo fi(argv[1]);
        if (!fi.isFile()) continue;

        // execute process

        ADB_PATH(argv[1]);
        QString out(adb().run(argv));
        if (out.isEmpty()) {
            // something wrong
            ok = false;
            break;
        }

        // make sure that parent isn't 'fake' item

        if (dontAppend) continue;

        // create sub-item for parent

        twi = createChild(fi.fileName(), fi.isFile() ? QString() : argv[2]);
        parent->addChild(twi);
    }
    return ok;
}

// create new item

QTreeWidgetItem* TreeWidget::createChild(const QString & text, const QString & path, bool isRoot) {
    QTreeWidgetItem * item = new QTreeWidgetItem((QTreeWidget*)NULL);
    item->setText(0, text.isEmpty() ? path : text);
    if (path.isEmpty() && !isRoot) {
        // this is file
        item->setIcon(0, QIcon(":/img/Document-Blank-icon.png"));
    } else {
        // this is directory
        const QString & data = ( text.isEmpty() ? path : QString("%1/%2/").arg(path).arg(text) );
        item->setIcon(0, QIcon(":/img/Places-folder-blue-icon.png"));
        item->setData(0, Qt::UserRole, data);
        // adding 'fake' sub-item
        QTreeWidgetItem * fake = new QTreeWidgetItem((QTreeWidget*)NULL);
        fake->setText(0, sFakeName);
        item->addChild(fake);
    }
    return item;
}

// this is 'fake' item?

bool TreeWidget::isFake(QTreeWidgetItem* item) {
    return (item->childCount() == 1 &&
            item->child(0)->text(0).compare(sFakeName) == 0);
}
