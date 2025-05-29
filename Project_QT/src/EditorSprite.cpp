#include "EditorSprite.h"
#include <QPainter>

EditorSprite::EditorSprite() : Sprite() {
    // Default constructor, m_image in base class is initialized
}

EditorSprite::EditorSprite(const QString& basePath) : Sprite() {
    Sprite::load(basePath); // Load the base image using Sprite's load method
    // If this base image is meant to be a specific size, store it additionally
    // For example, if it's the 32x32 version by default:
    // if (!m_image.isNull()) {
    //     m_sized_images.insert(EDITOR_SPRITE_SIZE_32x32, m_image);
    // }
}

EditorSprite::EditorSprite(const QMap<EditorSpriteSize, QString>& paths) : Sprite() {
    bool first = true;
    for (auto it = paths.constBegin(); it != paths.constEnd(); ++it) {
        QImage img;
        if (img.load(it.value())) {
            m_sized_images.insert(it.key(), img);
            if (first) {
                Sprite::setImage(img); // Set the base image to the first successfully loaded one
                first = false;
            }
        } else {
            // Handle error: log or skip
        }
    }
    if (m_image.isNull() && !m_sized_images.isEmpty()) {
        // Fallback if the first image failed but others succeeded
        Sprite::setImage(m_sized_images.first());
    }
}

EditorSprite::EditorSprite(const QMap<EditorSpriteSize, QImage>& images) : Sprite() {
    m_sized_images = images;
    if (!m_sized_images.isEmpty()) {
        Sprite::setImage(m_sized_images.first()); // Set base image to the first one
    }
}


EditorSprite::~EditorSprite() {
    // m_sized_images will be cleared automatically
}

void EditorSprite::draw(QPainter* painter, int x, int y, int width, int height) {
    if (!painter) return;

    // This draw method could try to find the best-fit pre-scaled image
    // or fall back to scaling the base m_image from the Sprite class.

    // Example: If a specific width/height is requested, try to find a matching pre-scaled version.
    // This is a simplified logic. A more robust approach would check aspect ratio or closest size.
    QImage imageToDraw;

    if (width != -1 && height != -1) {
        for (auto it = m_sized_images.constBegin(); it != m_sized_images.constEnd(); ++it) {
            QSize s = getQSizeForEnum(it.key());
            if (s.width() == width && s.height() == height) {
                imageToDraw = it.value();
                break;
            }
        }
    }

    if (imageToDraw.isNull()) {
        // If no specific sized image matched or no specific size requested, use base class draw
        // (which uses m_image and scales if width/height are different from m_image's dimensions)
        // Sprite::draw(painter, x, y, width, height); // This would call the pure virtual base.
        // Instead, draw the m_image directly if no specific sized image is found.
         if (!m_image.isNull()) {
            painter->drawImage(QRect(x, y, width == -1 ? m_image.width() : width, 
                                          height == -1 ? m_image.height() : height), 
                               m_image);
        }

    } else {
        // Draw the found pre-scaled image without further scaling by the painter here,
        // unless width/height specifically override the found image's dimensions.
        painter->drawImage(QRect(x, y, width == -1 ? imageToDraw.width() : width,
                                      height == -1 ? imageToDraw.height() : height),
                           imageToDraw);
    }
}

QImage EditorSprite::getImageForSize(EditorSpriteSize size_enum) {
    if (m_sized_images.contains(size_enum)) {
        return m_sized_images.value(size_enum);
    }
    // If not found, scale from the base image (m_image)
    if (!m_image.isNull()) {
        QSize targetQSize = getQSizeForEnum(size_enum);
        if (targetQSize.isValid()) {
            // Cache the scaled version
            QImage scaled = m_image.scaled(targetQSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
            m_sized_images.insert(size_enum, scaled);
            return scaled;
        }
    }
    return QImage(); // Return null if no base image or invalid size
}

bool EditorSprite::loadSize(EditorSpriteSize size_enum, const QString& path) {
    QImage img;
    if (img.load(path)) {
        m_sized_images.insert(size_enum, img);
        // If the base m_image is null and this is the first image loaded, set it as base
        if (m_image.isNull()) {
            Sprite::setImage(img);
        }
        return true;
    }
    return false;
}

bool EditorSprite::setImageForSize(EditorSpriteSize size_enum, const QImage& image) {
    if (image.isNull()) return false;
    m_sized_images.insert(size_enum, image);
    if (m_image.isNull()) {
        Sprite::setImage(image);
    }
    return true;
}


QSize EditorSprite::getQSizeForEnum(EditorSpriteSize size_enum) const {
    switch (size_enum) {
        case EDITOR_SPRITE_SIZE_16x16: return QSize(16, 16);
        case EDITOR_SPRITE_SIZE_32x32: return QSize(32, 32);
        case EDITOR_SPRITE_SIZE_64x64: return QSize(64, 64);
        default: return QSize(); // Invalid or undefined
    }
}
