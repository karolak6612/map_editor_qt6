#ifndef EDITORSPRITE_H
#define EDITORSPRITE_H

#include "Sprite.h"
#include <QMap> // For managing different sizes if needed
#include <QPainter> // For QPainter

// In wxwidgets, EditorSprite held multiple wxBitmaps for different sizes.
// Here, we can either store multiple QImages or one base QImage and scale on demand.
// For simplicity, starting with one base QImage from Sprite parent class.
// If specific pre-scaled versions are needed, this class can be expanded.

enum EditorSpriteSize { // Similar to SpriteSize in wxwidgets, but for editor specific needs
    EDITOR_SPRITE_SIZE_16x16,
    EDITOR_SPRITE_SIZE_32x32,
    EDITOR_SPRITE_SIZE_64x64, // Example sizes
    EDITOR_SPRITE_SIZE_COUNT
};


class EditorSprite : public Sprite {
public:
    EditorSprite();
    // Constructor to load a base image, or multiple sizes
    EditorSprite(const QString& basePath); // Load base image
    EditorSprite(const QMap<EditorSpriteSize, QString>& paths); // Load specific sizes
    EditorSprite(const QMap<EditorSpriteSize, QImage>& images);


    ~EditorSprite() override;

    // Override draw if EditorSprite has different drawing logic (e.g., choosing a pre-scaled image)
    void draw(QPainter* painter, int x, int y, int width = -1, int height = -1) override;
    
    // Method to get a specific size (could return a scaled version or a pre-loaded one)
    QImage getImageForSize(EditorSpriteSize size);

    // Load specific sizes if not done at construction
    bool loadSize(EditorSpriteSize size_enum, const QString& path);
    bool setImageForSize(EditorSpriteSize size_enum, const QImage& image);


private:
    // Option 1: Store multiple pre-scaled images (closer to original wxEditorSprite)
    QMap<EditorSpriteSize, QImage> m_sized_images;

    // Option 2: Store one high-resolution image and scale down (simpler, uses Sprite::m_image)
    // If using Option 2, m_sized_images might not be needed, or could be a cache.

    // Helper to get a QSize from enum
    QSize getQSizeForEnum(EditorSpriteSize size_enum) const;
};

#endif // EDITORSPRITE_H
