#include "imagewidget.h"

#include <QPainter>
#include <QWheelEvent>
#include <QDebug>
#include <QMouseEvent>

#include "adb.h"

ImageWidget::ImageWidget(QWidget *parent)
    : QWidget(parent)
    , mPixmap(nullptr)
    , mZoom(1)
{}

ImageWidget::~ImageWidget() {
    if (mPixmap) delete mPixmap;
}

QSize ImageWidget::sizeHint() const {
    if (mPixmap) {
        return mPixmap->size();
    } else {
        return QSize();
    }
}

void ImageWidget::setPixmap(QPixmap * p) {
    if (mPixmap) {
        delete mPixmap;
    } else {
        // adjust zoom only for 1st time
        if (p->size().width() > width()) {
            mZoom = width();
            mZoom /= p->size().width();
            qDebug() << mZoom << p->size().width() << width();
        }
    }

    mPixmap = p;
    update();
}

QPixmap* ImageWidget::pixmap() {
    return mPixmap;
}

void ImageWidget::paintEvent(QPaintEvent *ev) {
   QPainter p(this);
   if (pixmap() != NULL) {
       int w = pixmap()->width();
       int h = pixmap()->height();
       QPixmap map = pixmap()->scaled(w*mZoom,h*mZoom,Qt::KeepAspectRatio);
       resize(sizeHint() * mZoom);
       p.drawPixmap(0, 0, map);
   }
}

void ImageWidget::wheelEvent (QWheelEvent * e) {
   int x = e->pos( ).x();
   int y = e->pos( ).y();

   if (e->delta() != 0) {
       double delta = e->delta();
       delta /= 10000;
       mZoom += delta;
   }

   qDebug() << "ZOOM" << e->delta() << mZoom;
}

void ImageWidget::mousePressEvent(QMouseEvent *e) {

    if (mZoom < 0) {
        return;
    }

    int x = e->x();
    int y = e->y();
    int newX = x / mZoom;
    int newY = y / mZoom;
    qDebug() << __FUNCTION__ << x << y << " -> " << newX << newY;

    if (newX > 0 && newX < pixmap()->size().width()) {
        if (newY >0 && newY < pixmap()->size().height()) {
            //    "shell input tap "100 100
            QStringList argv;
            argv << "shell";
            argv << "input";
            argv << "tap";
            argv << QString::number( newX );
            argv << QString::number( newY );
            QByteArray out(adb().run(argv));
        }
    }
}
