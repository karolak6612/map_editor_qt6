#ifndef QT_GAMESPRITE_H
#define QT_GAMESPRITE_H

#include "Sprite.h"     // Base class
#include "Animator.h"   // For m_animator
#include "SpriteLight.h" // For m_spriteLight
#include <QPixmap>
#include <QImage>
#include <QString>
#include <QVector> // For potential animation data storage if not all in Animator

// Forward declarations
class QPainter;
struct Outfit; // Assuming Outfit struct will be defined elsewhere, similar to wxwidgets

class GameSprite : public Sprite {
    Q_OBJECT

public:
    // Constructor might take sprite sheet path, dimensions, animation layout, etc.
    // For now, a basic one. Detailed loading will be handled by a SpriteManager later.
    explicit GameSprite(QObject *parent = nullptr);
    // A more complete constructor might look like:
    // explicit GameSprite(const QString& spriteSheetPath,
    //                     int frameWidth, int frameHeight,
    //                     int numFrames, int numLayers, int numPatternsX, int numPatternsY, int numPatternsZ,
    //                     const SpriteLight& lightInfo, qint16 drawHeight, qint16 drawOffsetX, qint16 drawOffsetY,
    //                     const Animator::FrameDuration& defaultFrameDuration, // Or QVector<Animator::FrameDuration>
    //                     QObject *parent = nullptr);

    ~GameSprite() override;

    // --- Configuration Methods (called by SpriteManager or similar) ---
    void setSpriteSheet(const QPixmap& sheet);
    void setSpriteSheet(const QString& path); // Loads from file/resource
    void setImage(const QImage& image); // Set sprite sheet from QImage

    void setFrameDimensions(int width, int height);
    void setAnimationLayout(int layers, int patternsX, int patternsY, int patternsZ, int framesPerPattern); // framesPerPattern means total frames for one x/y/z pattern combination
    void setDrawingAttributes(qint16 drawHeight, qint16 drawOffsetX, qint16 drawOffsetY);
    void setSpriteLight(const SpriteLight& light);
    void configureAnimator(int startFrame, int loopCount, bool isAsync, const QVector<Animator::FrameDuration>& durations);

    // --- Overridden from Sprite ---
    void drawTo(QPainter* painter, const QRect& targetScreenRect, const QRect& sourceSpriteRect) override;
    void drawTo(QPainter* painter, const QPoint& targetPos, int sourceX = 0, int sourceY = 0, int sourceWidth = -1, int sourceHeight = -1) override;
    void drawTo(QPainter* painter, const QPoint& targetPos, SpriteSize size) override;

    // GameSprite specific drawing, includes animation frame selection
    // Corresponds to original GameSprite::DrawTo, but parameters are more Qt-like
    // patternX, patternY, patternZ are for selecting which animation sequence (e.g. direction, addon)
    void drawAnimated(QPainter* painter, const QPoint& targetPos,
                      int patternX = 0, int patternY = 0, int patternZ = 0, int layer = 0);

    // Outfit rendering support (matching wxWidgets DrawOutfit functionality)
    void drawOutfit(QPainter* painter, const QPoint& targetPos, const Outfit& outfit,
                   int patternX = 0, int patternY = 0, int patternZ = 0, int layer = 0);
    void drawOutfit(QPainter* painter, const QPoint& targetPos, SpriteSize size, const Outfit& outfit);

    void unload() override;

    int width() const override;  // Width of a single sprite frame
    int height() const override; // Height of a single sprite frame
    int width(SpriteSize size) const override;   // Width for specific size
    int height(SpriteSize size) const override;  // Height for specific size

    bool isLoaded() const override;

    // Animation interface overrides
    int getFrameCount() const override;
    int getCurrentFrame() const override;
    void setCurrentFrame(int frame) override;

    // --- GameSprite Specific Methods ---
    Animator* getAnimator() { return &m_animator; } // Allow external access if needed
    const Animator* getAnimator() const { return &m_animator; }

    SpriteLight getLightInfo() const { return m_spriteLight; }
    qint16 getDrawHeight() const { return m_drawHeight; }
    qint16 getDrawOffsetX() const { return m_drawOffsetX; }
    qint16 getDrawOffsetY() const { return m_drawOffsetY; }

    // Additional wxWidgets compatibility methods
    quint16 getMinimapColor() const { return m_minimapColor; }
    void setMinimapColor(quint16 color) { m_minimapColor = color; }

    // Frame indexing (matching wxWidgets getIndex method)
    int getIndex(int width, int height, int layer, int patternX, int patternY, int patternZ, int frame) const;

    int getFrameWidth() const { return m_frameWidth; }
    int getFrameHeight() const { return m_frameHeight; }
    int getLayers() const { return m_layers; }
    int getPatternXCount() const { return m_patternX; }
    int getPatternYCount() const { return m_patternY; }
    int getPatternZCount() const { return m_patternZ; }
    int getFramesPerPattern() const { return m_framesPerPattern; } // Total frames for one X,Y,Z pattern

    // --- QImage-related methods for compatibility with CreatureSpriteManager ---
    QImage getImage() const; // Get the sprite sheet as QImage
    QImage getSpritePart(int frameIndex, int patternX, int patternY, int patternZ, int layer, int width = 0, int height = 0) const;
    QImage colorizeSpritePart(const QImage& sourceImage, const Outfit& outfit) const;

    // --- Compatibility members for CreatureSpriteManager ---
    QVector<QImage> sprite_parts; // For compatibility with existing code
    int width = 1;     // Number of sprite images horizontally
    int height = 1;    // Number of sprite images vertically
    int layers = 1;    // Number of layers
    int frames = 1;    // Number of animation frames
    int pattern_x = 1; // Number of pattern variations X
    int pattern_y = 1; // Number of pattern variations Y
    int pattern_z = 1; // Number of pattern variations Z

    // Sprite caching and cleanup
    void clean(qint64 currentTime); // Clean old cached sprites
    void clearOutfitCache(); // Clear all cached outfit sprites

protected:
    QRect calculateFrameRect(int frameIndex, int patternX, int patternY, int patternZ, int layer) const;

    // Helper methods for outfit rendering
    QString generateOutfitCacheKey(const Outfit& outfit, int patternX, int patternY, int patternZ, int layer) const;
    QPixmap createOutfitSprite(const Outfit& outfit, int patternX, int patternY, int patternZ, int layer) const;

private:
    QPixmap m_spriteSheet;
    QImage m_spriteImage; // QImage version for manipulation
    Animator m_animator;

    // Sprite sheet layout and properties
    int m_frameWidth = 0;
    int m_frameHeight = 0;
    int m_layers = 1;          // Number of blend layers
    int m_patternX = 1;        // e.g., creature directions
    int m_patternY = 1;        // e.g., creature addons
    int m_patternZ = 1;        // e.g., other variations
    int m_framesPerPattern = 1; // Number of animation frames for a single pattern combination

    // Drawing attributes
    qint16 m_drawHeight = 0;    // For Z-ordering or elevation effect
    qint16 m_drawOffsetX = 0;
    qint16 m_drawOffsetY = 0;
    quint16 m_minimapColor = 0; // Minimap color for this sprite

    SpriteLight m_spriteLight;

    // Sprite caching and cleanup
    mutable QMap<QString, QPixmap> m_outfitCache; // Cache for outfit-colorized sprites
    qint64 m_lastAccessTime = 0; // For cleanup management
};

#endif // QT_GAMESPRITE_H
