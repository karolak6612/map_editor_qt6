#include "EditorSprite.h"
#include <QPainter>
#include <QDebug>
#include <QImageReader>

EditorSprite::EditorSprite(const QString& path16x16, const QString& path32x32, const QString& path64x64, QObject *parent)
    : Sprite(parent), m_defaultSize(SPRITE_SIZE_32x32), m_scalingCacheEnabled(true) {
    loadFromPaths(path16x16, path32x32, path64x64);

    // Task 014: Create fallback sprites if loading completely failed
    if (m_pixmaps.isEmpty()) {
        qWarning() << "EditorSprite: All sprite loading failed for paths, creating fallback sprites";
        m_pixmaps.insert(SPRITE_SIZE_16x16, createFallbackPixmap(SPRITE_SIZE_16x16));
        m_pixmaps.insert(SPRITE_SIZE_32x32, createFallbackPixmap(SPRITE_SIZE_32x32));
        m_pixmaps.insert(SPRITE_SIZE_64x64, createFallbackPixmap(SPRITE_SIZE_64x64));
    }
}

EditorSprite::EditorSprite(const QPixmap& pixmap16x16, const QPixmap& pixmap32x32, const QPixmap& pixmap64x64, QObject *parent)
    : Sprite(parent), m_defaultSize(SPRITE_SIZE_32x32), m_scalingCacheEnabled(true) {
    if (!pixmap16x16.isNull()) {
        m_pixmaps.insert(SPRITE_SIZE_16x16, pixmap16x16);
    }
    if (!pixmap32x32.isNull()) {
        m_pixmaps.insert(SPRITE_SIZE_32x32, pixmap32x32);
    }
    if (!pixmap64x64.isNull()) {
        m_pixmaps.insert(SPRITE_SIZE_64x64, pixmap64x64);
    }

    // Set default size based on available pixmaps
    if (!m_pixmaps.contains(SPRITE_SIZE_32x32)) {
        if (m_pixmaps.contains(SPRITE_SIZE_16x16)) {
            m_defaultSize = SPRITE_SIZE_16x16;
        } else if (m_pixmaps.contains(SPRITE_SIZE_64x64)) {
            m_defaultSize = SPRITE_SIZE_64x64;
        }
    }
}

EditorSprite::EditorSprite(EditorSpriteId spriteId, QObject *parent)
    : Sprite(parent), m_defaultSize(SPRITE_SIZE_32x32), m_scalingCacheEnabled(true) {
    const EditorSpriteResource* resource = getEditorSpriteResource(spriteId);
    if (resource) {
        loadFromPaths(
            resource->path16x16 ? QString(resource->path16x16) : QString(),
            resource->path32x32 ? QString(resource->path32x32) : QString(),
            resource->path64x64 ? QString(resource->path64x64) : QString()
        );
    } else {
        qWarning() << "EditorSprite: Invalid sprite ID" << spriteId;
    }

    // Task 014: Create fallback sprites if loading completely failed
    if (m_pixmaps.isEmpty()) {
        qWarning() << "EditorSprite: All sprite loading failed for ID" << spriteId << ", creating fallback sprites";
        m_pixmaps.insert(SPRITE_SIZE_16x16, createFallbackPixmap(SPRITE_SIZE_16x16));
        m_pixmaps.insert(SPRITE_SIZE_32x32, createFallbackPixmap(SPRITE_SIZE_32x32));
        m_pixmaps.insert(SPRITE_SIZE_64x64, createFallbackPixmap(SPRITE_SIZE_64x64));
    }
}

EditorSprite::EditorSprite(const EditorSpriteResource& resource, QObject *parent)
    : Sprite(parent), m_defaultSize(SPRITE_SIZE_32x32), m_scalingCacheEnabled(true) {
    loadFromPaths(
        resource.path16x16 ? QString(resource.path16x16) : QString(),
        resource.path32x32 ? QString(resource.path32x32) : QString(),
        resource.path64x64 ? QString(resource.path64x64) : QString()
    );

    // Task 014: Create fallback sprites if loading completely failed
    if (m_pixmaps.isEmpty()) {
        qWarning() << "EditorSprite: All sprite loading failed for resource, creating fallback sprites";
        m_pixmaps.insert(SPRITE_SIZE_16x16, createFallbackPixmap(SPRITE_SIZE_16x16));
        m_pixmaps.insert(SPRITE_SIZE_32x32, createFallbackPixmap(SPRITE_SIZE_32x32));
        m_pixmaps.insert(SPRITE_SIZE_64x64, createFallbackPixmap(SPRITE_SIZE_64x64));
    }
}

EditorSprite::~EditorSprite() {
    unload(); // Ensure pixmaps are cleared
}

void EditorSprite::drawTo(QPainter* painter, const QRect& targetScreenRect, const QRect& sourceSpriteRect) {
    if (!painter) {
        return;
    }

    // Task 014: Enhanced fallback handling
    if (m_pixmaps.isEmpty()) {
        qWarning() << "EditorSprite::drawTo (QRect, QRect): No pixmaps loaded, drawing fallback placeholder";
        drawFallbackPlaceholder(painter, targetScreenRect);
        return;
    }

    const QPixmap* bestPixmap = getBestFitPixmap(targetScreenRect.size());
    if (!bestPixmap) {
        qWarning() << "EditorSprite::drawTo (QRect, QRect): No suitable pixmap found, drawing fallback placeholder";
        drawFallbackPlaceholder(painter, targetScreenRect);
        return;
    }

    // If sourceSpriteRect is invalid or full, use the whole pixmap
    if (sourceSpriteRect.isNull() || sourceSpriteRect.width() <= 0 || sourceSpriteRect.height() <= 0 ||
        (sourceSpriteRect.width() == bestPixmap->width() && sourceSpriteRect.height() == bestPixmap->height() && sourceSpriteRect.topLeft() == QPoint(0,0) )) {
        painter->drawPixmap(targetScreenRect, *bestPixmap);
    } else {
        painter->drawPixmap(targetScreenRect, *bestPixmap, sourceSpriteRect);
    }
}

void EditorSprite::drawTo(QPainter* painter, const QPoint& targetPos, int sourceX, int sourceY, int sourceWidth, int sourceHeight) {
    if (!painter) {
        return;
    }

    // Task 014: Enhanced fallback handling
    if (m_pixmaps.isEmpty()) {
        qWarning() << "EditorSprite::drawTo (QPoint, int...): No pixmaps loaded, drawing fallback placeholder";
        int defaultSize = Sprite::getSizeInPixels(m_defaultSize);
        QRect targetRect(targetPos, QSize(defaultSize, defaultSize));
        drawFallbackPlaceholder(painter, targetRect);
        return;
    }

    const QPixmap* bestPixmap = getBestFitPixmap(m_defaultSize); // Use default size as a hint
    if (!bestPixmap) {
        qWarning() << "EditorSprite::drawTo (QPoint, int...): No suitable pixmap found for default size, drawing fallback placeholder";
        int defaultSize = Sprite::getSizeInPixels(m_defaultSize);
        QRect targetRect(targetPos, QSize(defaultSize, defaultSize));
        drawFallbackPlaceholder(painter, targetRect);
        return;
    }

    QRect sourceRect;
    if (sourceWidth > 0 && sourceHeight > 0) {
        sourceRect.setRect(sourceX, sourceY, sourceWidth, sourceHeight);
    } else {
        // If sourceWidth/Height is invalid, draw the whole pixmap
        sourceRect = bestPixmap->rect();
    }

    painter->drawPixmap(targetPos, *bestPixmap, sourceRect);
}

void EditorSprite::drawTo(QPainter* painter, const QPoint& targetPos, SpriteSize sz) {
    if (!validateDrawingParameters(painter, targetPos)) {
        return;
    }

    // Task 014: Enhanced fallback handling
    if (m_pixmaps.isEmpty()) {
        qWarning() << "EditorSprite::drawTo (QPoint, SpriteSize): No pixmaps loaded, drawing fallback placeholder";
        int targetSize = Sprite::getSizeInPixels(sz);
        QRect targetRect(targetPos, QSize(targetSize, targetSize));
        drawFallbackPlaceholder(painter, targetRect);
        return;
    }

    const QPixmap* pixmapToDraw = getBestFitPixmap(sz);
    if (!pixmapToDraw) {
        qWarning() << "EditorSprite::drawTo (QPoint, SpriteSize): No suitable pixmap found for SpriteSize" << sz << ", drawing fallback placeholder";
        int targetSize = Sprite::getSizeInPixels(sz);
        QRect targetRect(targetPos, QSize(targetSize, targetSize));
        drawFallbackPlaceholder(painter, targetRect);
        return;
    }

    // Check if we need to scale (when scaling cache is disabled and we got a different size)
    if (!m_scalingCacheEnabled && sz == SPRITE_SIZE_64x64 &&
        !m_pixmaps.contains(SPRITE_SIZE_64x64) && m_pixmaps.contains(SPRITE_SIZE_32x32)) {
        // Direct scaling without caching
        int targetSize = Sprite::getSizeInPixels(sz);
        QRect targetRect(targetPos, QSize(targetSize, targetSize));
        painter->drawPixmap(targetRect, *pixmapToDraw);
    } else {
        painter->drawPixmap(targetPos, *pixmapToDraw);
    }
}

void EditorSprite::unload() {
    m_pixmaps.clear();
    m_scaledPixmaps.clear();
}

int EditorSprite::width() const {
    if (m_pixmaps.contains(m_defaultSize)) {
        return m_pixmaps.value(m_defaultSize).width();
    }
    if (!m_pixmaps.isEmpty()) {
        return m_pixmaps.first().width(); // Fallback to first available
    }
    return 0;
}

int EditorSprite::height() const {
    if (m_pixmaps.contains(m_defaultSize)) {
        return m_pixmaps.value(m_defaultSize).height();
    }
    if (!m_pixmaps.isEmpty()) {
        return m_pixmaps.first().height(); // Fallback to first available
    }
    return 0;
}

int EditorSprite::width(SpriteSize size) const {
    if (m_pixmaps.contains(size)) {
        return m_pixmaps.value(size).width();
    }
    // Return expected size even if pixmap doesn't exist
    return Sprite::getSizeInPixels(size);
}

int EditorSprite::height(SpriteSize size) const {
    if (m_pixmaps.contains(size)) {
        return m_pixmaps.value(size).height();
    }
    // Return expected size even if pixmap doesn't exist
    return Sprite::getSizeInPixels(size);
}

bool EditorSprite::isLoaded() const {
    return !m_pixmaps.isEmpty();
}

QPixmap EditorSprite::getPixmap(SpriteSize sz) const {
    return m_pixmaps.value(sz, QPixmap()); // Returns null QPixmap if not found
}

bool EditorSprite::hasPixmap(SpriteSize sz) const {
    return m_pixmaps.contains(sz);
}

// Private helper methods
const QPixmap* EditorSprite::getBestFitPixmap(SpriteSize requestedSize) const {
    // First, try to get exact match
    if (m_pixmaps.contains(requestedSize)) {
        return &m_pixmaps.value(requestedSize);
    }

    // Check if we have a scaled version cached
    if (m_scalingCacheEnabled && m_scaledPixmaps.contains(requestedSize)) {
        return &m_scaledPixmaps.value(requestedSize);
    }

    // Scaling fallback logic (matching wxWidgets behavior)
    if (requestedSize == SPRITE_SIZE_64x64 && m_pixmaps.contains(SPRITE_SIZE_32x32)) {
        // Create scaled version and cache it
        if (m_scalingCacheEnabled) {
            QPixmap scaledPixmap = getScaledPixmap(SPRITE_SIZE_64x64, SPRITE_SIZE_32x32);
            return &m_scaledPixmaps.value(SPRITE_SIZE_64x64); // getScaledPixmap already cached it
        } else {
            // Return the 32x32 version - caller will handle scaling
            return &m_pixmaps.value(SPRITE_SIZE_32x32);
        }
    }

    // General fallback: try default, then 32, then 16, then first available
    if (m_pixmaps.contains(m_defaultSize)) return &m_pixmaps.value(m_defaultSize);
    if (m_pixmaps.contains(SPRITE_SIZE_32x32)) return &m_pixmaps.value(SPRITE_SIZE_32x32);
    if (m_pixmaps.contains(SPRITE_SIZE_16x16)) return &m_pixmaps.value(SPRITE_SIZE_16x16);
    if (!m_pixmaps.isEmpty()) return &m_pixmaps.first();

    return nullptr; // No pixmap available
}

const QPixmap* EditorSprite::getBestFitPixmap(const QSize& targetSize) const {
    // Simple logic: if target is large, prefer larger sprites.
    // This could be made more sophisticated.
    if (targetSize.width() > 32 || targetSize.height() > 32) {
        if (m_pixmaps.contains(SPRITE_SIZE_64x64)) return &m_pixmaps.value(SPRITE_SIZE_64x64);
        if (m_pixmaps.contains(SPRITE_SIZE_32x32)) return &m_pixmaps.value(SPRITE_SIZE_32x32); // Will be scaled up
    }
    if (targetSize.width() > 16 || targetSize.height() > 16) {
        if (m_pixmaps.contains(SPRITE_SIZE_32x32)) return &m_pixmaps.value(SPRITE_SIZE_32x32);
        if (m_pixmaps.contains(SPRITE_SIZE_16x16)) return &m_pixmaps.value(SPRITE_SIZE_16x16); // Will be scaled up
    }
    if (m_pixmaps.contains(SPRITE_SIZE_16x16)) return &m_pixmaps.value(SPRITE_SIZE_16x16);

    // Fallback to default or first available
    if (m_pixmaps.contains(m_defaultSize)) return &m_pixmaps.value(m_defaultSize);
    if (!m_pixmaps.isEmpty()) return &m_pixmaps.first();

    return nullptr;
}

// New methods implementation

void EditorSprite::clearScalingCache() {
    m_scaledPixmaps.clear();
}

EditorSprite* EditorSprite::createFromId(EditorSpriteId spriteId, QObject* parent) {
    if (!isValidSpriteId(spriteId)) {
        qWarning() << "EditorSprite::createFromId: Invalid sprite ID" << spriteId;
        return nullptr;
    }

    // Task 014: Enhanced creation with fallback guarantee
    EditorSprite* sprite = new EditorSprite(spriteId, parent);

    // Ensure the sprite has at least fallback pixmaps
    if (!sprite->isLoaded()) {
        qWarning() << "EditorSprite::createFromId: Created sprite for ID" << spriteId << "but it has no pixmaps, this should not happen with fallback system";
    }

    return sprite;
}

bool EditorSprite::isValidSpriteId(EditorSpriteId spriteId) {
    return spriteId >= EDITOR_SPRITE_SELECTION_MARKER && spriteId < EDITOR_SPRITE_LAST;
}

void EditorSprite::loadFromPaths(const QString& path16x16, const QString& path32x32, const QString& path64x64) {
    if (!path16x16.isEmpty()) {
        QPixmap pm16 = loadPixmapFromPath(path16x16);
        if (!pm16.isNull()) {
            m_pixmaps.insert(SPRITE_SIZE_16x16, pm16);
        } else {
            qWarning() << "EditorSprite: Failed to load 16x16 image from" << path16x16;
        }
    }
    if (!path32x32.isEmpty()) {
        QPixmap pm32 = loadPixmapFromPath(path32x32);
        if (!pm32.isNull()) {
            m_pixmaps.insert(SPRITE_SIZE_32x32, pm32);
        } else {
            qWarning() << "EditorSprite: Failed to load 32x32 image from" << path32x32;
        }
    }
    if (!path64x64.isEmpty()) {
        QPixmap pm64 = loadPixmapFromPath(path64x64);
        if (!pm64.isNull()) {
            m_pixmaps.insert(SPRITE_SIZE_64x64, pm64);
        } else {
            qWarning() << "EditorSprite: Failed to load 64x64 image from" << path64x64;
        }
    }

    // Set default size based on available pixmaps if 32x32 is missing
    if (!m_pixmaps.contains(SPRITE_SIZE_32x32)) {
        if (m_pixmaps.contains(SPRITE_SIZE_16x16)) {
            m_defaultSize = SPRITE_SIZE_16x16;
        } else if (m_pixmaps.contains(SPRITE_SIZE_64x64)) {
            m_defaultSize = SPRITE_SIZE_64x64;
        }
    }
}

QPixmap EditorSprite::loadPixmapFromPath(const QString& path) const {
    if (path.isEmpty()) {
        return QPixmap();
    }

    // Task 014: Enhanced path resolution with fallbacks
    QStringList pathsToTry;
    pathsToTry << path; // Try original path first

    // If path doesn't have extension, try common image extensions
    if (!path.contains('.')) {
        pathsToTry << (path + ".png") << (path + ".bmp") << (path + ".xpm") << (path + ".jpg");
    }

    // If it's not a resource path, try resource variants
    if (!path.startsWith(":/") && !path.startsWith("qrc:")) {
        pathsToTry << (":/images/" + path);
        pathsToTry << (":/icons/" + path);
        pathsToTry << (":/sprites/" + path);

        // Try with extensions in resource paths too
        if (!path.contains('.')) {
            pathsToTry << (":/images/" + path + ".png");
            pathsToTry << (":/icons/" + path + ".png");
            pathsToTry << (":/sprites/" + path + ".png");
        }
    }

    // Try each path until one works
    for (const QString& tryPath : pathsToTry) {
        QPixmap pixmap(tryPath);
        if (!pixmap.isNull()) {
            qDebug() << "EditorSprite: Successfully loaded pixmap from" << tryPath;
            return pixmap;
        }
    }

    qWarning() << "EditorSprite: Failed to load pixmap from any of the attempted paths for" << path;
    return QPixmap();
}

QPixmap EditorSprite::getScaledPixmap(SpriteSize targetSize, SpriteSize sourceSize) const {
    if (!m_scalingCacheEnabled) {
        // Direct scaling without caching
        const QPixmap& sourcePixmap = m_pixmaps.value(sourceSize);
        int targetPixels = Sprite::getSizeInPixels(targetSize);
        return sourcePixmap.scaled(targetPixels, targetPixels, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }

    // Check cache first
    if (m_scaledPixmaps.contains(targetSize)) {
        return m_scaledPixmaps.value(targetSize);
    }

    // Create scaled pixmap
    const QPixmap& sourcePixmap = m_pixmaps.value(sourceSize);
    int targetPixels = Sprite::getSizeInPixels(targetSize);
    QPixmap scaledPixmap = sourcePixmap.scaled(targetPixels, targetPixels, Qt::KeepAspectRatio, Qt::SmoothTransformation);

    // Cache it
    m_scaledPixmaps.insert(targetSize, scaledPixmap);
    return scaledPixmap;
}

// Task 014: Missing validation and fallback methods implementation
bool EditorSprite::validateDrawingParameters(QPainter* painter, const QPoint& targetPos) const {
    if (!painter) {
        qWarning() << "EditorSprite::validateDrawingParameters: Null painter provided";
        return false;
    }

    if (targetPos.x() < -10000 || targetPos.y() < -10000 ||
        targetPos.x() > 10000 || targetPos.y() > 10000) {
        qWarning() << "EditorSprite::validateDrawingParameters: Invalid target position" << targetPos;
        return false;
    }

    return true;
}

QPixmap EditorSprite::createFallbackPixmap(SpriteSize size) const {
    int pixelSize = Sprite::getSizeInPixels(size);
    QPixmap fallback(pixelSize, pixelSize);
    fallback.fill(QColor(255, 0, 255, 128)); // Magenta with transparency

    QPainter painter(&fallback);
    painter.setPen(QPen(Qt::black, 2));
    painter.setBrush(Qt::NoBrush);

    // Draw a border
    painter.drawRect(1, 1, pixelSize - 2, pixelSize - 2);

    // Draw an X pattern
    painter.drawLine(2, 2, pixelSize - 3, pixelSize - 3);
    painter.drawLine(pixelSize - 3, 2, 2, pixelSize - 3);

    // Draw "?" in center for larger sizes
    if (pixelSize >= 32) {
        QFont font = painter.font();
        font.setPixelSize(pixelSize / 3);
        font.setBold(true);
        painter.setFont(font);
        painter.drawText(QRect(0, 0, pixelSize, pixelSize), Qt::AlignCenter, "?");
    }

    painter.end();
    return fallback;
}

void EditorSprite::drawFallbackPlaceholder(QPainter* painter, const QRect& targetRect) const {
    if (!painter) return;

    painter->save();

    // Fill with magenta background
    painter->fillRect(targetRect, QColor(255, 0, 255, 128));

    // Draw border
    painter->setPen(QPen(Qt::black, 2));
    painter->setBrush(Qt::NoBrush);
    painter->drawRect(targetRect);

    // Draw X pattern
    painter->drawLine(targetRect.topLeft(), targetRect.bottomRight());
    painter->drawLine(targetRect.topRight(), targetRect.bottomLeft());

    // Draw "?" for larger rectangles
    if (targetRect.width() >= 32 && targetRect.height() >= 32) {
        QFont font = painter->font();
        font.setPixelSize(qMin(targetRect.width(), targetRect.height()) / 3);
        font.setBold(true);
        painter->setFont(font);
        painter->setPen(Qt::black);
        painter->drawText(targetRect, Qt::AlignCenter, "?");
    }

    painter->restore();
}
