#ifndef IMAGEWIDGET_H
#define IMAGEWIDGET_H

#include <QWidget>
#include <QPixmap>

class ImageWidget : public QWidget {
    Q_OBJECT

    QPixmap *mPixmap;
    double mZoom;

public:
    explicit ImageWidget(QWidget *parent = nullptr);
    ~ImageWidget();

    void setPixmap(QPixmap * p);
    QPixmap* pixmap();

    QSize sizeHint() const;

protected:
    void paintEvent(QPaintEvent *ev);
    void wheelEvent(QWheelEvent * e);
    void mousePressEvent(QMouseEvent * e);

signals:

};

#endif // IMAGEWIDGET_H
