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
    // Defer pixmap creation until actually needed for better performance
    m_spriteSheet = QPixmap(); // Clear existing pixmap
    m_pixmapNeedsUpdate = true;
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

    // Ensure pixmap is available
    ensurePixmapReady();

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

void GameSprite::ensurePixmapReady() {
    if (m_pixmapNeedsUpdate && !m_spriteImage.isNull()) {
        m_spriteSheet = QPixmap::fromImage(m_spriteImage);
        m_pixmapNeedsUpdate = false;
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

    // Ensure pixmap is available
    ensurePixmapReady();

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

    // Implement proper Tibia outfit colorization
    // Tibia uses specific color palettes and HSV adjustments for outfit colorization
    if (outfit.lookHead > 0 || outfit.lookBody > 0 || outfit.lookLegs > 0 || outfit.lookFeet > 0) {

        // Convert to 32-bit format for better color manipulation
        if (colorizedImage.format() != QImage::Format_ARGB32_Premultiplied) {
            colorizedImage = colorizedImage.convertToFormat(QImage::Format_ARGB32_Premultiplied);
        }

        // Apply Tibia-style outfit colorization
        for (int y = 0; y < colorizedImage.height(); ++y) {
            for (int x = 0; x < colorizedImage.width(); ++x) {
                QRgb pixel = colorizedImage.pixel(x, y);

                // Skip transparent pixels
                if (qAlpha(pixel) == 0) {
                    continue;
                }

                // Extract RGB components
                int red = qRed(pixel);
                int green = qGreen(pixel);
                int blue = qBlue(pixel);
                int alpha = qAlpha(pixel);

                // Apply Tibia outfit colorization algorithm
                QRgb colorizedPixel = applyTibiaOutfitColor(red, green, blue, alpha, outfit);
                colorizedImage.setPixel(x, y, colorizedPixel);
            }
        }
    }

    return colorizedImage;
}

QRgb GameSprite::applyTibiaOutfitColor(int red, int green, int blue, int alpha, const Outfit& outfit) const {
    // Tibia outfit colorization works by mapping grayscale values to specific colors
    // This is a simplified version of the Tibia colorization algorithm

    // Calculate grayscale value for determining which color channel to use
    int grayscale = qGray(red, green, blue);

    // Determine which outfit color to apply based on pixel intensity
    QColor targetColor;

    if (grayscale >= 192) {
        // Bright pixels -> Head color
        if (outfit.lookHead > 0) {
            targetColor = getTibiaOutfitColor(outfit.lookHead);
        } else {
            return qRgba(red, green, blue, alpha); // No change
        }
    } else if (grayscale >= 128) {
        // Medium pixels -> Body color
        if (outfit.lookBody > 0) {
            targetColor = getTibiaOutfitColor(outfit.lookBody);
        } else {
            return qRgba(red, green, blue, alpha); // No change
        }
    } else if (grayscale >= 64) {
        // Dark pixels -> Legs color
        if (outfit.lookLegs > 0) {
            targetColor = getTibiaOutfitColor(outfit.lookLegs);
        } else {
            return qRgba(red, green, blue, alpha); // No change
        }
    } else {
        // Very dark pixels -> Feet color
        if (outfit.lookFeet > 0) {
            targetColor = getTibiaOutfitColor(outfit.lookFeet);
        } else {
            return qRgba(red, green, blue, alpha); // No change
        }
    }

    // Apply the target color while preserving the original intensity
    float intensity = grayscale / 255.0f;
    int newRed = static_cast<int>(targetColor.red() * intensity);
    int newGreen = static_cast<int>(targetColor.green() * intensity);
    int newBlue = static_cast<int>(targetColor.blue() * intensity);

    return qRgba(qBound(0, newRed, 255),
                 qBound(0, newGreen, 255),
                 qBound(0, newBlue, 255),
                 alpha);
}

QColor GameSprite::getTibiaOutfitColor(int colorId) const {
    // Tibia outfit color palette (simplified version)
    // In a full implementation, this would be loaded from game data files
    static const QColor tibiaColors[] = {
        QColor(255, 255, 255), // 0 - White
        QColor(255, 255, 204), // 1 - Light Yellow
        QColor(255, 255, 153), // 2 - Yellow
        QColor(255, 255, 102), // 3 - Dark Yellow
        QColor(255, 255, 51),  // 4 - Gold
        QColor(255, 204, 153), // 5 - Light Orange
        QColor(255, 153, 102), // 6 - Orange
        QColor(255, 102, 51),  // 7 - Dark Orange
        QColor(255, 51, 0),    // 8 - Red Orange
        QColor(255, 0, 0),     // 9 - Red
        QColor(204, 0, 0),     // 10 - Dark Red
        QColor(153, 0, 0),     // 11 - Very Dark Red
        QColor(102, 0, 0),     // 12 - Brown Red
        QColor(255, 204, 255), // 13 - Light Pink
        QColor(255, 153, 255), // 14 - Pink
        QColor(255, 102, 255), // 15 - Dark Pink
        QColor(255, 51, 255),  // 16 - Purple Pink
        QColor(255, 0, 255),   // 17 - Magenta
        QColor(204, 0, 204),   // 18 - Dark Magenta
        QColor(153, 0, 153),   // 19 - Purple
        QColor(102, 0, 102),   // 20 - Dark Purple
        QColor(204, 204, 255), // 21 - Light Blue
        QColor(153, 153, 255), // 22 - Blue
        QColor(102, 102, 255), // 23 - Dark Blue
        QColor(51, 51, 255),   // 24 - Very Dark Blue
        QColor(0, 0, 255),     // 25 - Pure Blue
        QColor(0, 0, 204),     // 26 - Navy Blue
        QColor(0, 0, 153),     // 27 - Dark Navy
        QColor(0, 0, 102),     // 28 - Very Dark Navy
        QColor(204, 255, 255), // 29 - Light Cyan
        QColor(153, 255, 255), // 30 - Cyan
        QColor(102, 255, 255), // 31 - Dark Cyan
        QColor(51, 255, 255),  // 32 - Very Dark Cyan
        QColor(0, 255, 255),   // 33 - Pure Cyan
        QColor(0, 204, 204),   // 34 - Teal
        QColor(0, 153, 153),   // 35 - Dark Teal
        QColor(0, 102, 102),   // 36 - Very Dark Teal
        QColor(204, 255, 204), // 37 - Light Green
        QColor(153, 255, 153), // 38 - Green
        QColor(102, 255, 102), // 39 - Dark Green
        QColor(51, 255, 51),   // 40 - Very Dark Green
        QColor(0, 255, 0),     // 41 - Pure Green
        QColor(0, 204, 0),     // 42 - Forest Green
        QColor(0, 153, 0),     // 43 - Dark Forest
        QColor(0, 102, 0),     // 44 - Very Dark Forest
        QColor(255, 255, 204), // 45 - Light Lime
        QColor(255, 255, 153), // 46 - Lime
        QColor(255, 255, 102), // 47 - Dark Lime
        QColor(255, 255, 51),  // 48 - Very Dark Lime
        QColor(255, 255, 0),   // 49 - Pure Yellow
        QColor(204, 204, 0),   // 50 - Dark Yellow
        QColor(153, 153, 0),   // 51 - Brown Yellow
        QColor(102, 102, 0),   // 52 - Dark Brown Yellow
        QColor(255, 204, 153), // 53 - Light Brown
        QColor(255, 153, 102), // 54 - Brown
        QColor(255, 102, 51),  // 55 - Dark Brown
        QColor(255, 51, 0),    // 56 - Very Dark Brown
        QColor(204, 102, 51),  // 57 - Chocolate
        QColor(153, 76, 38),   // 58 - Dark Chocolate
        QColor(102, 51, 25),   // 59 - Very Dark Chocolate
        QColor(51, 25, 13),    // 60 - Black Brown
        QColor(204, 204, 204), // 61 - Light Gray
        QColor(153, 153, 153), // 62 - Gray
        QColor(102, 102, 102), // 63 - Dark Gray
        QColor(51, 51, 51),    // 64 - Very Dark Gray
        QColor(0, 0, 0),       // 65 - Black
    };

    const int maxColors = sizeof(tibiaColors) / sizeof(tibiaColors[0]);

    if (colorId >= 0 && colorId < maxColors) {
        return tibiaColors[colorId];
    }

    // Default to white for invalid color IDs
    return QColor(255, 255, 255);
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


