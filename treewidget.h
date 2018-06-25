#ifndef TREEWIDGET_H
#define TREEWIDGET_H

#include <QTreeWidget>

class TreeWidget : public QTreeWidget {
    Q_OBJECT
public:
    explicit TreeWidget(QWidget *parent = 0);
    ~TreeWidget();
    static QTreeWidgetItem* createChild(const QString &text, const QString &path, bool isRoot = false);
    static bool isFake(QTreeWidgetItem* item);
private:
    QStringList mimeTypes() const;
    bool dropMimeData(QTreeWidgetItem *parent, int index, const QMimeData *data, Qt::DropAction action);
};

#endif // TREEWIDGET_H
