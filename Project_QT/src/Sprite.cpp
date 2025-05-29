#include "Sprite.h"
#include <QBuffer>
#include <QColor>
#include <QPainter>
#include <QPixmap> // For getPixmap() implementation

Sprite::Sprite() {
    m_image = QImage(1, 1, QImage::Format_ARGB32_Premultiplied);
    m_image.fill(Qt::transparent);
}

Sprite::~Sprite() {
}

bool Sprite::load(const QString& path) {
    if (m_image.load(path)) {
        return true;
    }
    return false;
}

bool Sprite::loadFromData(const QByteArray& data, const char* format) {
    if (m_image.loadFromData(data, format)) {
        return true;
    }
    return false;
}

QImage Sprite::getImage() const {
    return m_image;
}

void Sprite::setImage(const QImage& image) {
    m_image = image;
    if (m_image.isNull()){
        m_image = QImage(1, 1, QImage::Format_ARGB32_Premultiplied);
        m_image.fill(Qt::transparent);
    }
}

QPixmap Sprite::getPixmap() const {
    if (m_image.isNull()) {
        return QPixmap(); 
    }
    return QPixmap::fromImage(m_image);
}

QImage Sprite::scale(const QSize& size, Qt::AspectRatioMode aspectRatioMode, Qt::TransformationMode transformMode) {
    if (m_image.isNull()) {
        return QImage(); 
    }
    return m_image.scaled(size, aspectRatioMode, transformMode);
}

void Sprite::setTransparency(int alpha) {
    if (m_image.isNull()) { 
        m_image = QImage(1,1,QImage::Format_ARGB32_Premultiplied); 
        m_image.fill(Qt::transparent);
    }
    
    if (!m_image.hasAlphaChannel()) {
        m_image = m_image.convertToFormat(QImage::Format_ARGB32_Premultiplied);
    }

    QImage tempImage = QImage(m_image.size(), QImage::Format_ARGB32_Premultiplied);
    tempImage.fill(Qt::transparent); 

    QPainter painter(&tempImage);
    painter.setCompositionMode(QPainter::CompositionMode_Source);
    double opacity = qBound(0.0, static_cast<double>(alpha) / 255.0, 1.0);
    painter.setOpacity(opacity);
    painter.drawImage(0, 0, m_image);
    painter.end();

    m_image = tempImage;
}

void Sprite::setMaskColor(const QColor& color, bool enable) {
    if (m_image.isNull()) {
        return;
    }

    if (enable) {
        if (!m_image.hasAlphaChannel()) {
            m_image = m_image.convertToFormat(QImage::Format_ARGB32_Premultiplied);
        }
        QImage new_image = m_image.convertToFormat(QImage::Format_ARGB32_Premultiplied); 
        for (int y = 0; y < new_image.height(); ++y) {
            for (int x = 0; x < new_image.width(); ++x) {
                if (QColor(new_image.pixel(x, y)) == color) {
                    new_image.setPixelColor(x, y, Qt::transparent);
                }
            }
        }
        m_image = new_image;

    } else {
        if (m_image.hasAlphaChannel()) {
             m_image = m_image.convertToFormat(QImage::Format_RGB32); 
        }
    }
}
