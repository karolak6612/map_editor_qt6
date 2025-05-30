#include "GameSprite.h"
#include <QPainter>
#include <QDebug> // For warnings

// Basic constructor
GameSprite::GameSprite(QObject *parent)
    : Sprite(parent),
      m_animator(this), // Animator is a QObject, set parent if desired or manage lifetime separately
      m_frameWidth(0), m_frameHeight(0),
      m_layers(1), m_patternX(1), m_patternY(1), m_patternZ(1), m_framesPerPattern(1),
      m_drawHeight(0), m_drawOffsetX(0), m_drawOffsetY(0) {
    // m_spriteLight is default constructed
}

GameSprite::~GameSprite() {
    unload();
}

// --- Configuration Methods ---
void GameSprite::setSpriteSheet(const QPixmap& sheet) {
    m_spriteSheet = sheet;
    // Potentially reset animator or update based on new sheet, if layout changes
}

void GameSprite::setSpriteSheet(const QString& path) {
    if (!path.isEmpty()) {
        if (!m_spriteSheet.load(path)) {
            qWarning() << "GameSprite: Failed to load sprite sheet from" << path;
        }
    } else {
        m_spriteSheet = QPixmap(); // Clear if path is empty
    }
}

void GameSprite::setFrameDimensions(int width, int height) {
    m_frameWidth = qMax(0, width);
    m_frameHeight = qMax(0, height);
}

void GameSprite::setAnimationLayout(int layers, int patternsX, int patternsY, int patternsZ, int framesPerPattern) {
    m_layers = qMax(1, layers); // Must have at least 1 layer
    m_patternX = qMax(1, patternsX);
    m_patternY = qMax(1, patternsY);
    m_patternZ = qMax(1, patternsZ);
    m_framesPerPattern = qMax(1, framesPerPattern);
    
    // Reconfigure animator based on new layout (total frames for one pattern combination)
    // The animator itself doesn't know about patterns, just total frames for its current sequence
    m_animator.setup(m_framesPerPattern, -1, 0, false); // Default setup, can be reconfigured by configureAnimator
}

void GameSprite::setDrawingAttributes(qint16 drawHeight, qint16 drawOffsetX, qint16 drawOffsetY) {
    m_drawHeight = drawHeight;
    m_drawOffsetX = drawOffsetX;
    m_drawOffsetY = drawOffsetY;
}

void GameSprite::setSpriteLight(const SpriteLight& light) {
    m_spriteLight = light;
}

void GameSprite::configureAnimator(int startFrame, int loopCount, bool isAsync, const QVector<Animator::FrameDuration>& durations) {
    m_animator.setup(m_framesPerPattern, startFrame, loopCount, isAsync);
    if (!durations.isEmpty()) {
        m_animator.setFrameDurations(durations);
    }
}

// --- Overridden from Sprite ---

// Generic drawTo - tries to draw the current animated frame if possible, or a default portion.
// This might be too ambiguous for GameSprite. drawAnimated is more specific.
void GameSprite::drawTo(QPainter* painter, const QRect& targetScreenRect, const QRect& sourceSpriteRect) {
    if (!painter || m_spriteSheet.isNull() || m_frameWidth == 0 || m_frameHeight == 0) {
        return;
    }
    // This version of drawTo is a bit tricky for an animated sprite if sourceSpriteRect is arbitrary
    // and not aligned with animation frames.
    // For simplicity, let's assume sourceSpriteRect refers to a portion of the *entire spritesheet*,
    // or if it's empty, we draw the current animated frame scaled to targetScreenRect.
    if (sourceSpriteRect.isNull() || !sourceSpriteRect.isValid()) {
        int currentFrame = m_animator.getCurrentFrameIndex();
        // Assuming default pattern/layer for this generic call
        QRect calculatedSourceRect = calculateFrameRect(currentFrame, 0, 0, 0, 0); 
        if (calculatedSourceRect.isValid()) {
            painter->drawPixmap(targetScreenRect, m_spriteSheet, calculatedSourceRect);
        }
    } else {
        painter->drawPixmap(targetScreenRect, m_spriteSheet, sourceSpriteRect);
    }
}

// Generic drawTo - similar ambiguity. Let's draw the top-left frame or current animated one.
void GameSprite::drawTo(QPainter* painter, const QPoint& targetPos, int sourceX, int sourceY, int sourceWidth, int sourceHeight) {
    if (!painter || m_spriteSheet.isNull() || m_frameWidth == 0 || m_frameHeight == 0) {
        return;
    }

    QRect sourceRect;
    if (sourceWidth > 0 && sourceHeight > 0) {
        sourceRect.setRect(sourceX, sourceY, sourceWidth, sourceHeight);
    } else { // Draw current animated frame of default pattern
        int currentFrame = m_animator.getCurrentFrameIndex();
        sourceRect = calculateFrameRect(currentFrame, 0, 0, 0, 0);
    }
    
    if (sourceRect.isValid()) {
        QPoint finalTargetPos = targetPos + QPoint(m_drawOffsetX, m_drawOffsetY);
        painter->drawPixmap(finalTargetPos, m_spriteSheet, sourceRect);
    }
}

// GameSprite specific drawing
void GameSprite::drawAnimated(QPainter* painter, const QPoint& targetPos, 
                              int patternX, int patternY, int patternZ, int layer) {
    if (!painter || m_spriteSheet.isNull() || m_frameWidth == 0 || m_frameHeight == 0) {
        return;
    }

    // Note: The Animator currently is configured for m_framesPerPattern.
    // If different patterns could have different frame counts or timings, Animator would need
    // to be reconfigured or GameSprite would need multiple Animator instances.
    // For now, assume one Animator configuration applies to all patterns.
    int currentFrame = m_animator.getCurrentFrameIndex();
    QRect sourceRect = calculateFrameRect(currentFrame, patternX, patternY, patternZ, layer);

    if (sourceRect.isValid()) {
        QPoint finalTargetPos = targetPos + QPoint(m_drawOffsetX, m_drawOffsetY);
        painter->drawPixmap(finalTargetPos, m_spriteSheet, sourceRect);
    } else {
        //qWarning() << "GameSprite::drawAnimated: Calculated invalid sourceRect";
    }
}

void GameSprite::unload() {
    m_spriteSheet = QPixmap(); // Release pixmap data
}

int GameSprite::width() const {
    return m_frameWidth;
}

int GameSprite::height() const {
    return m_frameHeight;
}

// Protected helper method
QRect GameSprite::calculateFrameRect(int frameIndex, int patternX, int patternY, int patternZ, int layer) const {
    if (m_spriteSheet.isNull() || m_frameWidth == 0 || m_frameHeight == 0 ||
        m_framesPerPattern == 0 || m_patternX == 0 || m_patternY == 0 || m_patternZ == 0 || m_layers == 0) {
        return QRect(); // Invalid setup
    }

    // Validate inputs to prevent going out of bounds
    int safeLayer = qBound(0, layer, m_layers - 1);
    int safePatternZ = qBound(0, patternZ, m_patternZ - 1);
    int safePatternY = qBound(0, patternY, m_patternY - 1);
    int safePatternX = qBound(0, patternX, m_patternX - 1);
    int safeFrameIndex = qBound(0, frameIndex, m_framesPerPattern - 1);

    // Calculate how many full "animation strips" (one full animation for one pattern x/y/z/layer)
    // are on the sheet horizontally before our target pattern begins.
    // This assumes a sprite sheet layout where animations for different patterns/layers are laid out sequentially.
    // The exact layout of the sprite sheet (how layers, patterns, and frames are arranged) is critical.
    // Original wxGameSprite::getIndex implies a specific order:
    // ((((((frame % frames) * pattern_z + p_z) * pattern_y + p_y) * pattern_x + p_x) * layers + l) * height_in_sprites + h_idx) * width_in_sprites + w_idx
    // This means 'width_in_sprites' and 'height_in_sprites' were the dimensions of one single sprite image (usually 1x1 for items/creatures).
    // 'frames' was total frames for one pattern.
    // 'height' and 'width' members of GameSprite were number of sprite *images* (usually 1x1).
    
    // Let's assume the sprite sheet is laid out as a grid of frames.
    // Total frames horizontally on the sheet: m_spriteSheet.width() / m_frameWidth
    // Total frames vertically on the sheet: m_spriteSheet.height() / m_frameHeight

    // Simplified calculation assuming sprite sheet contains frames for ONE specific combination
    // of (layer, patternZ, patternY, patternX) arranged in a grid, and framesPerPattern refers to this.
    // Or, if framesPerPattern is the number of frames in one "row" of the sprite sheet for this pattern.
    
    // More robust: The sheet contains ALL patterns, layers, frames.
    // Indexing similar to original:
    // Start with the base index for the pattern and layer
    long patternStartIndex = 0;
    // patternStartIndex = (safeLayer * m_patternZ * m_patternY * m_patternX +
    //                      safePatternZ * m_patternY * m_patternX +
    //                      safePatternY * m_patternX +
    //                      safePatternX) * m_framesPerPattern;
    
    // Let total_patterns_per_layer = m_patternX * m_patternY * m_patternZ
    // Let total_frames_per_layer = total_patterns_per_layer * m_framesPerPattern
    // base_frame_offset_for_layer = safeLayer * total_frames_per_layer
    // base_frame_offset_for_pattern_z = safePatternZ * m_patternY * m_patternX * m_framesPerPattern
    // base_frame_offset_for_pattern_y = safePatternY * m_patternX * m_framesPerPattern
    // base_frame_offset_for_pattern_x = safePatternX * m_framesPerPattern
    // global_frame_index = base_for_layer + base_for_pz + base_for_py + base_for_px + safeFrameIndex

    // This calculation matches the original GameSprite::getIndex logic if width=1, height=1 (i.e. each "sprite" is one frame)
    // and GameSprite::frames was m_framesPerPattern.
    // GameSprite::height/width (num sprite images) are m_frameHeight/m_frameWidth in pixels for Qt.
    // GameSprite::layers, pattern_x/y/z, frames are m_layers, m_patternX/Y/Z, m_framesPerPattern.

    long globalFrameIndex = 0;
    globalFrameIndex += safeLayer;
    globalFrameIndex = globalFrameIndex * m_patternZ + safePatternZ;
    globalFrameIndex = globalFrameIndex * m_patternY + safePatternY;
    globalFrameIndex = globalFrameIndex * m_patternX + safePatternX;
    globalFrameIndex = globalFrameIndex * m_framesPerPattern + safeFrameIndex;


    int framesPerRow = m_spriteSheet.width() / m_frameWidth;
    if (framesPerRow == 0) return QRect(); // Cannot determine layout

    int frameX = (globalFrameIndex % framesPerRow) * m_frameWidth;
    int frameY = (globalFrameIndex / framesPerRow) * m_frameHeight;

    if (frameX + m_frameWidth > m_spriteSheet.width() || frameY + m_frameHeight > m_spriteSheet.height()) {
        //qWarning() << "GameSprite::calculateFrameRect: Calculated frame is out of sprite sheet bounds.";
        return QRect(); // Out of bounds
    }

    return QRect(frameX, frameY, m_frameWidth, m_frameHeight);
}
