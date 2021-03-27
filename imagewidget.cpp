#include "imagewidget.h"

#include <QPainter>
#include <QWheelEvent>
#include <QDebug>
#include <QMouseEvent>
#include <QDateTime>

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

void ImageWidget::setPixmap(const QByteArray & picBinary) {
    bool firstLoad = false;
    if (mPixmap == nullptr) {
        firstLoad = true;
        mPixmap = new QPixmap;
    }

    if (!mPixmap->loadFromData(picBinary, "PNG")) {
        qWarning() << "loadFromData failed. Possible corrupted data.";
    } else {
        if (firstLoad) {
            // adjust zoom only for 1st time
            if (mPixmap->size().width() > width()) {
                mZoom = width();
                mZoom /= mPixmap->size().width();
                qDebug() << mZoom << mPixmap->size().width() << width();
            }
        }
    }

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

void ImageWidget::mousePressEvent(QMouseEvent * e) {
    mPressPos = e->pos();
    mPressTime = QDateTime::currentMSecsSinceEpoch();
}

void ImageWidget::mouseReleaseEvent(QMouseEvent * e) {
    // zoom isn't
    if (mZoom < 0) {
        qDebug() << "zoom factor is invalid.";
        return;
    }

    // original size of our phone screen
    const QSize & origSize = pixmap()->size();

    // press and release mouse positions isn't same
    if (mPressPos == e->pos()) {
        // then we're about to send tap event

        // lets translate our 2D coordinates to phone coordinates
        // using zoom factor
        int newX = e->x() / mZoom;
        int newY = e->y() / mZoom;

        // coordinates can't be larger than original phone screen
        if (newX > 0 && newX < origSize.width() && newY > 0 && newY < origSize.height()) {
            // perform tap at given translated position
            adb::tap(newX, newY);
        } else {
            qDebug() << "tapping failed for" << e->x() << e->y() << " -> " << newX << newY;
        }
    } else {
        //  ... or swipe

        // calculate time between press and release mouse events
        qint64 msec = QDateTime::currentMSecsSinceEpoch() - mPressTime;
        // prepare start and end press 2D coodinates by translating it to android coodrinates
        int startX = mPressPos.x() / mZoom;
        int startY = mPressPos.y() / mZoom;
        int endX = e->x() / mZoom;
        int endY = e->y() / mZoom;
        bool ok = false;
        // checking translated coordinates not large original android coordinates
        if (startX > 0 && startX < origSize.width() && startY > 0 && startY < origSize.height()) {
            if (endX > 0 && endX < origSize.width() && endY > 0 && endY < origSize.height()) {
                // and finally perform swipe
                adb::swipe( startX, startY, endX, endY, msec );
                ok = true;
            }
        }

        if (!ok) {
            qDebug() << "swapping failed for"
                     << mPressPos << "->" << "(" << startX << startY << ")"
                     << e->pos() << "->" << "("  << endX << endY << ")"
                     << msec;
        }
    }
}
