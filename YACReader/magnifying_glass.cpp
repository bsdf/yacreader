#include "magnifying_glass.h"
#include "viewer.h"
#include "configuration.h"
#include "shortcuts_manager.h"

#include <QScrollBar>

MagnifyingGlass::MagnifyingGlass(int w, int h, QWidget *parent)
    : QLabel(parent), zoomLevel(0.5)
{
    setup(QSize(w, h));
}

MagnifyingGlass::MagnifyingGlass(const QSize &size, QWidget *parent)
    : QLabel(parent), zoomLevel(0.5)
{
    setup(size);
}

void MagnifyingGlass::setup(const QSize &size)
{
    resize(size);
    setScaledContents(true);
    setMouseTracking(true);
    setCursor(QCursor(QBitmap(1, 1), QBitmap(1, 1)));
}

void MagnifyingGlass::mouseMoveEvent(QMouseEvent *event)
{
    updateImage();
    event->accept();
}

void MagnifyingGlass::updateImage(int x, int y)
{
    // image section augmented
    int zoomWidth = static_cast<int>(width() * zoomLevel);
    int zoomHeight = static_cast<int>(height() * zoomLevel);
    auto *const p = qobject_cast<const Viewer *>(parentWidget());
    int currentPos = p->verticalScrollBar()->sliderPosition();
    const QPixmap image = p->pixmap();
    int iWidth = image.width();
    int iHeight = image.height();
    float wFactor = static_cast<float>(iWidth) / p->widget()->width();
    float hFactor = static_cast<float>(iHeight) / p->widget()->height();
    zoomWidth *= wFactor;
    zoomHeight *= hFactor;
    if (p->verticalScrollBar()->minimum() == p->verticalScrollBar()->maximum()) {
        int xp = static_cast<int>(((x - p->widget()->pos().x()) * wFactor) - zoomWidth / 2);
        int yp = static_cast<int>((y - p->widget()->pos().y() + currentPos) * hFactor - zoomHeight / 2);
        int xOffset = 0;
        int yOffset = 0;
        int zw = zoomWidth;
        int zh = zoomHeight;
        // int wOffset,hOffset=0;
        bool outImage = false;
        if (xp < 0) {
            xOffset = -xp;
            xp = 0;
            zw = zw - xOffset;
            outImage = true;
        }
        if (yp < 0) {
            yOffset = -yp;
            yp = 0;
            zh = zh - yOffset;
            outImage = true;
        }

        if (xp + zoomWidth >= image.width()) {
            zw -= xp + zw - image.width();
            outImage = true;
        }
        if (yp + zoomHeight >= image.height()) {
            zh -= yp + zh - image.height();
            outImage = true;
        }
        if (outImage) {
            QImage img(zoomWidth, zoomHeight, QImage::Format_RGB32);
            img.setDevicePixelRatio(devicePixelRatioF());
            img.fill(Configuration::getConfiguration().getBackgroundColor());
            if (zw > 0 && zh > 0) {
                QPainter painter(&img);
                painter.drawPixmap(xOffset, yOffset, image.copy(xp, yp, zw, zh));
            }
            setPixmap(QPixmap().fromImage(img));
        } else
            setPixmap(image.copy(xp, yp, zoomWidth, zoomHeight));
    } else {
        int xp = static_cast<int>(((x - p->widget()->pos().x()) * wFactor) - zoomWidth / 2);
        int yp = static_cast<int>((y + currentPos) * hFactor - zoomHeight / 2);
        int xOffset = 0;
        int yOffset = 0;
        int zw = zoomWidth;
        int zh = zoomHeight;
        // int wOffset,hOffset=0;
        bool outImage = false;
        if (xp < 0) {
            xOffset = -xp;
            xp = 0;
            zw = zw - xOffset;
            outImage = true;
        }
        if (yp < 0) {
            yOffset = -yp;
            yp = 0;
            zh = zh - yOffset;
            outImage = true;
        }

        if (xp + zoomWidth >= image.width()) {
            zw -= xp + zw - image.width();
            outImage = true;
        }
        if (yp + zoomHeight >= image.height()) {
            zh -= yp + zh - image.height();
            outImage = true;
        }
        if (outImage) {
            QImage img(zoomWidth, zoomHeight, QImage::Format_RGB32);
            img.setDevicePixelRatio(devicePixelRatioF());
            img.fill(Configuration::getConfiguration().getBackgroundColor());
            if (zw > 0 && zh > 0) {
                QPainter painter(&img);
                painter.drawPixmap(xOffset, yOffset, image.copy(xp, yp, zw, zh));
            }
            setPixmap(QPixmap().fromImage(img));
        } else
            setPixmap(image.copy(xp, yp, zoomWidth, zoomHeight));
    }
    move(static_cast<int>(x - float(width()) / 2), static_cast<int>(y - float(height()) / 2));
}

void MagnifyingGlass::updateImage()
{
    if (isVisible()) {
        QPoint p = QPoint(cursor().pos().x(), cursor().pos().y());
        p = this->parentWidget()->mapFromGlobal(p);
        updateImage(p.x(), p.y());
    }
}
void MagnifyingGlass::wheelEvent(QWheelEvent *event)
{
    switch (event->modifiers()) {
    // size
    case Qt::NoModifier:
        if (event->angleDelta().y() < 0)
            sizeUp();
        else
            sizeDown();
        break;
    // size height
    case Qt::ControlModifier:
        if (event->angleDelta().y() < 0)
            heightUp();
        else
            heightDown();
        break;
    // size width
    case Qt::AltModifier:
        if (event->angleDelta().y() < 0)
            widthUp();
        else
            widthDown();
        break;
    // zoom level
    case Qt::ShiftModifier:
        if (event->angleDelta().y() < 0)
            zoomIn();
        else
            zoomOut();
        break;
    default:
        break; // Never propagate a wheel event to the parent widget, even if we ignore it.
    }
    event->setAccepted(true);
}
void MagnifyingGlass::zoomIn()
{
    if (zoomLevel > 0.2f) {
        zoomLevel -= 0.025f;
        updateImage();
    }
}

void MagnifyingGlass::zoomOut()
{
    if (zoomLevel < 0.9f) {
        zoomLevel += 0.025f;
        updateImage();
    }
}

static constexpr auto maxRelativeDimension = 0.9;

void MagnifyingGlass::sizeUp()
{
    if (width() < parentWidget()->width() * maxRelativeDimension)
        resizeAndUpdate(width() + 30, height() + 15);
}

void MagnifyingGlass::sizeDown()
{
    if (width() > 175)
        resizeAndUpdate(width() - 30, height() - 15);
}

void MagnifyingGlass::heightUp()
{
    if (height() < parentWidget()->height() * maxRelativeDimension)
        resizeAndUpdate(width(), height() + 15);
}

void MagnifyingGlass::heightDown()
{
    if (height() > 80)
        resizeAndUpdate(width(), height() - 15);
}

void MagnifyingGlass::widthUp()
{
    if (width() < parentWidget()->width() * maxRelativeDimension)
        resizeAndUpdate(width() + 30, height());
}

void MagnifyingGlass::widthDown()
{
    if (width() > 175)
        resizeAndUpdate(width() - 30, height());
}

void MagnifyingGlass::resizeAndUpdate(int w, int h)
{
    resize(w, h);
    updateImage();
}

void MagnifyingGlass::keyPressEvent(QKeyEvent *event)
{
    bool validKey = false;

    int _key = event->key();
    Qt::KeyboardModifiers modifiers = event->modifiers();

    if (modifiers & Qt::ShiftModifier)
        _key |= Qt::SHIFT;
    if (modifiers & Qt::ControlModifier)
        _key |= Qt::CTRL;
    if (modifiers & Qt::MetaModifier)
        _key |= Qt::META;
    if (modifiers & Qt::AltModifier)
        _key |= Qt::ALT;

    QKeySequence key(_key);

    if (key == ShortcutsManager::getShortcutsManager().getShortcut(SIZE_UP_MGLASS_ACTION_Y)) {
        sizeUp();
        validKey = true;
    }

    else if (key == ShortcutsManager::getShortcutsManager().getShortcut(SIZE_DOWN_MGLASS_ACTION_Y)) {
        sizeDown();
        validKey = true;
    }

    else if (key == ShortcutsManager::getShortcutsManager().getShortcut(ZOOM_IN_MGLASS_ACTION_Y)) {
        zoomIn();
        validKey = true;
    }

    else if (key == ShortcutsManager::getShortcutsManager().getShortcut(ZOOM_OUT_MGLASS_ACTION_Y)) {
        zoomOut();
        validKey = true;
    }

    if (validKey) {
        event->setAccepted(true);
    }
}
