#ifndef QT_GAMESPRITE_H
#define QT_GAMESPRITE_H

#include "Sprite.h"     // Base class
#include "Animator.h"   // For m_animator
#include "SpriteLight.h" // For m_spriteLight
#include <QPixmap>
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

    void setFrameDimensions(int width, int height);
    void setAnimationLayout(int layers, int patternsX, int patternsY, int patternsZ, int framesPerPattern); // framesPerPattern means total frames for one x/y/z pattern combination
    void setDrawingAttributes(qint16 drawHeight, qint16 drawOffsetX, qint16 drawOffsetY);
    void setSpriteLight(const SpriteLight& light);
    void configureAnimator(int startFrame, int loopCount, bool isAsync, const QVector<Animator::FrameDuration>& durations);

    // --- Overridden from Sprite ---
    void drawTo(QPainter* painter, const QRect& targetScreenRect, const QRect& sourceSpriteRect) override;
    void drawTo(QPainter* painter, const QPoint& targetPos, int sourceX = 0, int sourceY = 0, int sourceWidth = -1, int sourceHeight = -1) override;

    // GameSprite specific drawing, includes animation frame selection
    // Corresponds to original GameSprite::DrawTo, but parameters are more Qt-like
    // patternX, patternY, patternZ are for selecting which animation sequence (e.g. direction, addon)
    void drawAnimated(QPainter* painter, const QPoint& targetPos,
                      int patternX = 0, int patternY = 0, int patternZ = 0, int layer = 0);

    // TODO: Add DrawOutfit method equivalent if needed, taking an Outfit struct
    // virtual void drawOutfit(QPainter* painter, const QPoint& targetPos, const Outfit& outfit, int patternX = 0, ...);

    void unload() override;

    int width() const override;  // Width of a single sprite frame
    int height() const override; // Height of a single sprite frame

    // --- GameSprite Specific Methods ---
    Animator* getAnimator() { return &m_animator; } // Allow external access if needed
    const Animator* getAnimator() const { return &m_animator; }

    SpriteLight getLightInfo() const { return m_spriteLight; }
    qint16 getDrawHeight() const { return m_drawHeight; }
    qint16 getDrawOffsetX() const { return m_drawOffsetX; }
    qint16 getDrawOffsetY() const { return m_drawOffsetY; }

    int getFrameWidth() const { return m_frameWidth; }
    int getFrameHeight() const { return m_frameHeight; }
    int getLayers() const { return m_layers; }
    int getPatternXCount() const { return m_patternX; }
    int getPatternYCount() const { return m_patternY; }
    int getPatternZCount() const { return m_patternZ; }
    int getFramesPerPattern() const { return m_framesPerPattern; } // Total frames for one X,Y,Z pattern

protected:
    QRect calculateFrameRect(int frameIndex, int patternX, int patternY, int patternZ, int layer) const;

private:
    QPixmap m_spriteSheet;
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

    SpriteLight m_spriteLight;
};

#endif // QT_GAMESPRITE_H
