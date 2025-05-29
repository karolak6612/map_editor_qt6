#include "Sprite.h"
#include <QBuffer>
#include <QColor>
#include <QPainter>

Sprite::Sprite() {
    // Initialize with a null image or a default small transparent image
    m_image = QImage(1, 1, QImage::Format_ARGB32_Premultiplied);
    m_image.fill(Qt::transparent);
}

Sprite::~Sprite() {
    // QImage is implicitly shared, so no explicit deletion of m_image data is needed here
    // unless it was allocated with 'new'.
}

// Basic draw implementation, can be overridden by subclasses
// void Sprite::draw(QPainter* painter, int x, int y, int width, int height) { // Already declared pure virtual
//     if (!painter || m_image.isNull()) {
//         return;
//     }
//     if (width == -1) {
//         width = m_image.width();
//     }
//     if (height == -1) {
//         height = m_image.height();
//     }
//     painter->drawImage(QRect(x, y, width, height), m_image);
// }

bool Sprite::load(const QString& path) {
    if (m_image.load(path)) {
        // Optionally convert to a preferred format after loading
        // m_image = m_image.convertToFormat(QImage::Format_ARGB32_Premultiplied);
        return true;
    }
    return false;
}

bool Sprite::loadFromData(const QByteArray& data, const char* format) {
    if (m_image.loadFromData(data, format)) {
        // Optionally convert to a preferred format after loading
        // m_image = m_image.convertToFormat(QImage::Format_ARGB32_Premultiplied);
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
        // Ensure m_image is never truly null to avoid crashes, assign a default small image
        m_image = QImage(1, 1, QImage::Format_ARGB32_Premultiplied);
        m_image.fill(Qt::transparent);
    }
}

QImage Sprite::scale(const QSize& size, Qt::AspectRatioMode aspectRatioMode, Qt::TransformationMode transformMode) {
    if (m_image.isNull()) {
        return QImage(); // Return a null QImage if there's no image to scale
    }
    return m_image.scaled(size, aspectRatioMode, transformMode);
}

void Sprite::setTransparency(int alpha) {
    if (m_image.isNull()) { // Check if m_image is null before using it
        m_image = QImage(1,1,QImage::Format_ARGB32_Premultiplied); // Ensure it's valid
        m_image.fill(Qt::transparent);
    }
    
    if (!m_image.hasAlphaChannel()) {
        // If no alpha channel, convert it to a format that has one.
        // Format_ARGB32_Premultiplied is a common choice for images with transparency.
        m_image = m_image.convertToFormat(QImage::Format_ARGB32_Premultiplied);
    }

    // Create a new image with the desired alpha
    QImage tempImage = QImage(m_image.size(), QImage::Format_ARGB32_Premultiplied);
    tempImage.fill(Qt::transparent); // Start with a transparent image

    QPainter painter(&tempImage);
    painter.setCompositionMode(QPainter::CompositionMode_Source);
    // Ensure alpha is within valid range for QPainter::setOpacity (0.0 to 1.0)
    double opacity = qBound(0.0, static_cast<double>(alpha) / 255.0, 1.0);
    painter.setOpacity(opacity);
    painter.drawImage(0, 0, m_image);
    painter.end();

    m_image = tempImage;
}

// This is a simplified version. A more accurate wxImage::SetMaskColour equivalent
// would involve iterating pixels and setting those matching 'color' to transparent.
void Sprite::setMaskColor(const QColor& color, bool enable) {
    if (m_image.isNull()) {
        return;
    }

    if (enable) {
        // Convert to a format with an alpha channel if it doesn't have one
        if (!m_image.hasAlphaChannel()) {
            m_image = m_image.convertToFormat(QImage::Format_ARGB32_Premultiplied);
        }
        // Create a QBitmap mask
        // For QImage, createMaskFromColor is not a direct method.
        // We need to iterate or use QPixmap intermediate for advanced masking.
        // A simple approach for QImage:
        QImage new_image = m_image.convertToFormat(QImage::Format_ARGB32_Premultiplied); // Ensure alpha channel
        for (int y = 0; y < new_image.height(); ++y) {
            for (int x = 0; x < new_image.width(); ++x) {
                if (QColor(new_image.pixel(x, y)) == color) {
                    new_image.setPixelColor(x, y, Qt::transparent);
                }
            }
        }
        m_image = new_image;

    } else {
        // Disabling mask: This is complex. If the original pixels were preserved, they could be restored.
        // For now, this makes the image opaque again by removing alpha channel or setting alpha to full.
        // This simplified version does not restore original pixel colors if they were overwritten by transparency.
        if (m_image.hasAlphaChannel()) {
             m_image = m_image.convertToFormat(QImage::Format_RGB32); // Or some other opaque format
        }
    }
}
