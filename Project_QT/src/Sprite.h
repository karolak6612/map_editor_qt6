#ifndef QT_SPRITE_H
#define QT_SPRITE_H

#include <QObject>
#include <QRect>
#include <QPoint>

// Forward declarations
class QPainter;
class QPixmap;
class QImage;

/**
 * @brief Sprite size enumeration for different rendering scales
 *
 * These sizes correspond to the original wxWidgets implementation
 * and are used for multi-resolution sprite rendering.
 */
enum SpriteSize {
    SPRITE_SIZE_16x16 = 0,
    SPRITE_SIZE_32x32 = 1,
    SPRITE_SIZE_64x64 = 2,
    SPRITE_SIZE_COUNT = 3
};

/**
 * @brief Animation direction for sprite animations
 */
enum AnimationDirection {
    ANIMATION_FORWARD = 0,
    ANIMATION_BACKWARD = 1
};

/**
 * @brief Common sprite constants
 */
enum SpriteConstants {
    SPRITE_PIXELS = 32,                    // Default sprite size in pixels
    ITEM_FRAME_DURATION = 500             // Default animation frame duration in ms
};

/**
 * @brief Abstract base class for all sprite types in the Qt map editor
 *
 * This class provides the core interface for sprite rendering and resource management.
 * It supports both simple sprites (EditorSprite) and complex animated sprites (GameSprite).
 *
 * The interface is designed to be compatible with the original wxWidgets implementation
 * while leveraging Qt's graphics capabilities.
 */
class Sprite : public QObject {
    Q_OBJECT

public:
    explicit Sprite(QObject *parent = nullptr) : QObject(parent) {}
    virtual ~Sprite() = default;

    // --- Core Drawing Interface (Pure Virtual) ---

    /**
     * @brief Draw sprite to target rectangle with source rectangle specification
     * @param painter The QPainter to draw with
     * @param targetScreenRect Target rectangle on screen/widget
     * @param sourceSpriteRect Source rectangle from sprite data
     */
    virtual void drawTo(QPainter* painter, const QRect& targetScreenRect, const QRect& sourceSpriteRect) = 0;

    /**
     * @brief Draw sprite to target position with optional source specification
     * @param painter The QPainter to draw with
     * @param targetPos Target position on screen/widget
     * @param sourceX Source X coordinate (default: 0)
     * @param sourceY Source Y coordinate (default: 0)
     * @param sourceWidth Source width (-1 for full width)
     * @param sourceHeight Source height (-1 for full height)
     */
    virtual void drawTo(QPainter* painter, const QPoint& targetPos, int sourceX = 0, int sourceY = 0, int sourceWidth = -1, int sourceHeight = -1) = 0;

    /**
     * @brief Draw sprite at target position with specific size
     * @param painter The QPainter to draw with
     * @param targetPos Target position on screen/widget
     * @param size Sprite size to use for rendering
     */
    virtual void drawTo(QPainter* painter, const QPoint& targetPos, SpriteSize size) = 0;

    // --- Resource Management (Pure Virtual) ---

    /**
     * @brief Unload sprite resources to free memory
     *
     * This method should release any cached pixmaps, images, or other
     * graphics resources held by the sprite.
     */
    virtual void unload() = 0;

    // --- Dimension Interface (Pure Virtual) ---

    /**
     * @brief Get the width of a single sprite frame
     * @return Width in pixels
     */
    virtual int width() const = 0;

    /**
     * @brief Get the height of a single sprite frame
     * @return Height in pixels
     */
    virtual int height() const = 0;

    /**
     * @brief Get the width for a specific sprite size
     * @param size The sprite size to query
     * @return Width in pixels for the specified size
     */
    virtual int width(SpriteSize size) const = 0;

    /**
     * @brief Get the height for a specific sprite size
     * @param size The sprite size to query
     * @return Height in pixels for the specified size
     */
    virtual int height(SpriteSize size) const = 0;

    // --- Animation Interface (Virtual with default implementations) ---

    /**
     * @brief Get the number of animation frames
     * @return Number of frames (1 for static sprites)
     */
    virtual int getFrameCount() const { return 1; }

    /**
     * @brief Get the current animation frame
     * @return Current frame index
     */
    virtual int getCurrentFrame() const { return 0; }

    /**
     * @brief Set the current animation frame
     * @param frame Frame index to set
     */
    virtual void setCurrentFrame(int frame) { Q_UNUSED(frame); }

    /**
     * @brief Check if sprite has animation
     * @return True if sprite is animated
     */
    virtual bool isAnimated() const { return getFrameCount() > 1; }

    // --- Utility Methods (Virtual with default implementations) ---

    /**
     * @brief Check if sprite data is loaded
     * @return True if sprite has valid data
     */
    virtual bool isLoaded() const = 0;

    /**
     * @brief Get sprite size in pixels for a given SpriteSize enum
     * @param size The sprite size enum
     * @return Size in pixels
     */
    static int getSizeInPixels(SpriteSize size) {
        switch (size) {
            case SPRITE_SIZE_16x16: return 16;
            case SPRITE_SIZE_32x32: return 32;
            case SPRITE_SIZE_64x64: return 64;
            default: return 32;
        }
    }

protected:
    // --- Protected Interface for Derived Classes ---

    /**
     * @brief Validate drawing parameters
     * @param painter The painter to validate
     * @param targetPos The target position
     * @return True if parameters are valid
     */
    bool validateDrawingParameters(QPainter* painter, const QPoint& targetPos) const;

    /**
     * @brief Calculate source rectangle with defaults
     * @param sourceX Source X coordinate
     * @param sourceY Source Y coordinate
     * @param sourceWidth Source width (-1 for default)
     * @param sourceHeight Source height (-1 for default)
     * @param defaultWidth Default width to use
     * @param defaultHeight Default height to use
     * @return Calculated source rectangle
     */
    QRect calculateSourceRect(int sourceX, int sourceY, int sourceWidth, int sourceHeight, int defaultWidth, int defaultHeight) const;

private:
    // Disallow copy and assignment
    Sprite(const Sprite&) = delete;
    Sprite& operator=(const Sprite&) = delete;
};

#endif // QT_SPRITE_H
