#include "GameSprite.h"
#include "Outfit.h" // For Outfit struct
#include <QPainter>
#include <QDebug> // For warnings
#include <QDateTime> // For timestamp management

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
    m_spriteImage = sheet.toImage(); // Keep QImage version for manipulation
    // Potentially reset animator or update based on new sheet, if layout changes
}

void GameSprite::setSpriteSheet(const QString& path) {
    if (!path.isEmpty()) {
        if (!m_spriteSheet.load(path)) {
            qWarning() << "GameSprite: Failed to load sprite sheet from" << path;
        } else {
            m_spriteImage = m_spriteSheet.toImage(); // Keep QImage version
        }
    } else {
        m_spriteSheet = QPixmap(); // Clear if path is empty
        m_spriteImage = QImage(); // Clear QImage too
    }
}

void GameSprite::setImage(const QImage& image) {
    m_spriteImage = image;
    m_spriteSheet = QPixmap::fromImage(image);
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
    m_spriteImage = QImage(); // Release image data
    sprite_parts.clear(); // Clear compatibility sprite parts
    m_outfitCache.clear(); // Clear outfit cache
    m_animator.reset(); // Reset animator state
}

int GameSprite::width() const {
    return m_frameWidth;
}

int GameSprite::height() const {
    return m_frameHeight;
}

int GameSprite::width(SpriteSize size) const {
    // For GameSprite, size scaling is handled during drawing
    // Return the base frame width regardless of size
    return m_frameWidth;
}

int GameSprite::height(SpriteSize size) const {
    // For GameSprite, size scaling is handled during drawing
    // Return the base frame height regardless of size
    return m_frameHeight;
}

bool GameSprite::isLoaded() const {
    return !m_spriteSheet.isNull() && m_frameWidth > 0 && m_frameHeight > 0;
}

int GameSprite::getFrameCount() const {
    return m_framesPerPattern;
}

int GameSprite::getCurrentFrame() const {
    return m_animator.getCurrentFrame();
}

void GameSprite::setCurrentFrame(int frame) {
    m_animator.setCurrentFrame(frame);
}

void GameSprite::drawTo(QPainter* painter, const QPoint& targetPos, SpriteSize size) {
    if (!validateDrawingParameters(painter, targetPos) || !isLoaded()) {
        return;
    }

    // Get current frame from animator
    int currentFrame = getCurrentFrame();

    // Use default pattern values for this simple draw method
    QRect frameRect = calculateFrameRect(currentFrame, 0, 0, 0, 0);
    if (!frameRect.isValid()) {
        return;
    }

    // Calculate target size based on SpriteSize
    int targetSize = Sprite::getSizeInPixels(size);
    QRect targetRect(targetPos, QSize(targetSize, targetSize));

    // Draw with scaling if necessary
    painter->drawPixmap(targetRect, m_spriteSheet, frameRect);
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

// --- QImage-related methods for compatibility ---

QImage GameSprite::getImage() const {
    return m_spriteImage;
}

QImage GameSprite::getSpritePart(int frameIndex, int patternX, int patternY, int patternZ, int layer, int width, int height) const {
    Q_UNUSED(width);
    Q_UNUSED(height);

    if (m_spriteImage.isNull()) {
        return QImage();
    }

    QRect frameRect = calculateFrameRect(frameIndex, patternX, patternY, patternZ, layer);
    if (!frameRect.isValid()) {
        return QImage();
    }

    return m_spriteImage.copy(frameRect);
}

QImage GameSprite::colorizeSpritePart(const QImage& sourceImage, const Outfit& outfit) const {
    if (sourceImage.isNull()) {
        return QImage();
    }

    // Create a copy of the source image for colorization
    QImage colorizedImage = sourceImage.copy();

    // Simple colorization based on outfit colors
    // This is a basic implementation - more sophisticated colorization may be needed
    if (outfit.lookHead > 0 || outfit.lookBody > 0 || outfit.lookLegs > 0 || outfit.lookFeet > 0) {
        // Apply color transformations based on outfit
        // For now, just apply a simple color overlay
        for (int y = 0; y < colorizedImage.height(); ++y) {
            for (int x = 0; x < colorizedImage.width(); ++x) {
                QColor pixel = colorizedImage.pixelColor(x, y);
                if (pixel.alpha() > 0) { // Only colorize non-transparent pixels
                    // Simple colorization - this would need to be more sophisticated
                    // based on the actual outfit colorization algorithm from wxwidgets
                    if (outfit.lookHead > 0) {
                        pixel.setRed(qMin(255, pixel.red() + outfit.lookHead));
                    }
                    if (outfit.lookBody > 0) {
                        pixel.setGreen(qMin(255, pixel.green() + outfit.lookBody));
                    }
                    if (outfit.lookLegs > 0) {
                        pixel.setBlue(qMin(255, pixel.blue() + outfit.lookLegs));
                    }
                    colorizedImage.setPixelColor(x, y, pixel);
                }
            }
        }
    }

    return colorizedImage;
}

// --- New Methods for Enhanced GameSprite Functionality ---

void GameSprite::drawOutfit(QPainter* painter, const QPoint& targetPos, const Outfit& outfit,
                           int patternX, int patternY, int patternZ, int layer) {
    if (!validateDrawingParameters(painter, targetPos) || !isLoaded()) {
        return;
    }

    // Update last access time for cleanup
    m_lastAccessTime = QDateTime::currentMSecsSinceEpoch();

    // Generate cache key for this outfit combination
    QString cacheKey = generateOutfitCacheKey(outfit, patternX, patternY, patternZ, layer);

    // Check if we have a cached version
    QPixmap outfitPixmap;
    if (m_outfitCache.contains(cacheKey)) {
        outfitPixmap = m_outfitCache.value(cacheKey);
    } else {
        // Create new outfit sprite and cache it
        outfitPixmap = createOutfitSprite(outfit, patternX, patternY, patternZ, layer);
        if (!outfitPixmap.isNull()) {
            m_outfitCache.insert(cacheKey, outfitPixmap);
        }
    }

    if (!outfitPixmap.isNull()) {
        QPoint finalTargetPos = targetPos + QPoint(m_drawOffsetX, m_drawOffsetY);
        painter->drawPixmap(finalTargetPos, outfitPixmap);
    }
}

void GameSprite::drawOutfit(QPainter* painter, const QPoint& targetPos, SpriteSize size, const Outfit& outfit) {
    if (!validateDrawingParameters(painter, targetPos) || !isLoaded()) {
        return;
    }

    // Use current frame and default patterns for this simple method
    int currentFrame = getCurrentFrame();

    // Generate cache key
    QString cacheKey = generateOutfitCacheKey(outfit, 0, 0, 0, 0) + QString("_size%1_frame%2").arg(static_cast<int>(size)).arg(currentFrame);

    QPixmap outfitPixmap;
    if (m_outfitCache.contains(cacheKey)) {
        outfitPixmap = m_outfitCache.value(cacheKey);
    } else {
        // Create outfit sprite
        outfitPixmap = createOutfitSprite(outfit, 0, 0, 0, 0);
        if (!outfitPixmap.isNull()) {
            // Scale to requested size
            int targetSize = Sprite::getSizeInPixels(size);
            outfitPixmap = outfitPixmap.scaled(targetSize, targetSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
            m_outfitCache.insert(cacheKey, outfitPixmap);
        }
    }

    if (!outfitPixmap.isNull()) {
        painter->drawPixmap(targetPos, outfitPixmap);
    }
}

int GameSprite::getIndex(int width, int height, int layer, int patternX, int patternY, int patternZ, int frame) const {
    // Calculate sprite index based on layout (matching wxWidgets algorithm)
    // This assumes sprites are laid out in a specific order in the sprite sheet

    // Validate parameters
    if (width <= 0 || height <= 0 || layer < 0 || patternX < 0 || patternY < 0 || patternZ < 0 || frame < 0) {
        return -1;
    }

    // Calculate index based on sprite sheet layout
    // Formula matches the original wxWidgets implementation
    int index = 0;
    index += frame;
    index += patternZ * m_framesPerPattern;
    index += patternY * m_patternZ * m_framesPerPattern;
    index += patternX * m_patternY * m_patternZ * m_framesPerPattern;
    index += layer * m_patternX * m_patternY * m_patternZ * m_framesPerPattern;
    index += height * m_layers * m_patternX * m_patternY * m_patternZ * m_framesPerPattern;
    index += width * m_frameHeight * m_layers * m_patternX * m_patternY * m_patternZ * m_framesPerPattern;

    return index;
}

void GameSprite::clean(qint64 currentTime) {
    // Clean old cached outfit sprites (similar to wxWidgets clean method)
    const qint64 maxAge = 30000; // 30 seconds

    QMutableMapIterator<QString, QPixmap> it(m_outfitCache);
    while (it.hasNext()) {
        it.next();
        // For simplicity, remove all entries older than maxAge from last access
        if (currentTime - m_lastAccessTime > maxAge) {
            it.remove();
        }
    }
}

void GameSprite::clearOutfitCache() {
    m_outfitCache.clear();
}

QString GameSprite::generateOutfitCacheKey(const Outfit& outfit, int patternX, int patternY, int patternZ, int layer) const {
    return QString("outfit_%1_%2_%3_%4_%5_%6_%7_%8_%9_%10_%11_%12")
        .arg(outfit.lookType)
        .arg(outfit.lookHead)
        .arg(outfit.lookBody)
        .arg(outfit.lookLegs)
        .arg(outfit.lookFeet)
        .arg(outfit.lookAddon)
        .arg(patternX)
        .arg(patternY)
        .arg(patternZ)
        .arg(layer)
        .arg(getCurrentFrame())
        .arg(m_animator.getCurrentFrameIndex());
}

QPixmap GameSprite::createOutfitSprite(const Outfit& outfit, int patternX, int patternY, int patternZ, int layer) const {
    // Get the base sprite part
    QImage baseSpriteImage = getSpritePart(getCurrentFrame(), patternX, patternY, patternZ, layer);
    if (baseSpriteImage.isNull()) {
        return QPixmap();
    }

    // Apply outfit colorization
    QImage colorizedImage = colorizeSpritePart(baseSpriteImage, outfit);
    if (colorizedImage.isNull()) {
        return QPixmap::fromImage(baseSpriteImage); // Fallback to uncolorized
    }

    return QPixmap::fromImage(colorizedImage);
}

#include "GameSprite.moc"
