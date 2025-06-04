#include "ResourceManager.h"
#include <QDebug>
#include <QRect>
#include <QDir>
#include <QFileInfo>
#include <QDirIterator>
#include <QImageReader>
#include <QDateTime>
#include <QPainter>
#include <QApplication>
#include <QStandardPaths>
#include <algorithm>

// Initialize static instance to nullptr for singleton
// Note: Proper singleton often involves more complex static initialization,
// but for a single-threaded GUI app, a simple static local in instance() is common.
// Let's use the static local in instance() method for robust initialization order.

ResourceManager& ResourceManager::instance() {
    static ResourceManager instance; // Static local instance, created on first call
    return instance;
}

ResourceManager::ResourceManager() : QObject(nullptr),
    maxCacheItems_(1000),
    maxCacheSizeMB_(100),
    cleanupTimer_(new QTimer(this)) {

    qDebug() << "ResourceManager initialized.";

    // Initialize supported formats
    initializeSupportedFormats();

    // Initialize category base paths
    initializeCategoryPaths();

    // Initialize fallback resources
    initializeFallbackResources();

    // Setup cleanup timer (runs every 5 minutes)
    cleanupTimer_->setInterval(300000); // 5 minutes
    connect(cleanupTimer_, &QTimer::timeout, this, &ResourceManager::performCleanup);
    cleanupTimer_->start();

    qDebug() << "ResourceManager: Initialized with" << supportedFormats_.size() << "supported formats";
}

ResourceManager::~ResourceManager() {
    qDebug() << "ResourceManager destroyed. Cache size was:" << pixmapCache_.size();

    if (cleanupTimer_) {
        cleanupTimer_->stop();
    }

    clearCache(); // Ensure all QPixmaps are cleared
}

QPixmap ResourceManager::getPixmap(const QString& path, ResourceCategory category) {
    return loadPixmapInternal(path, category);
}

QIcon ResourceManager::getIcon(const QString& path, ResourceCategory category) {
    QPixmap pixmap = getPixmap(path, category);
    if (pixmap.isNull()) {
        qWarning() << "ResourceManager::getIcon: Failed to load base pixmap for icon from" << path;
        // Return fallback icon
        QPixmap fallback = getFallbackPixmap(QSize(16, 16));
        return fallback.isNull() ? QIcon() : QIcon(fallback);
    }
    return QIcon(pixmap);
}

// Enhanced sprite sheet functionality
QPixmap ResourceManager::getPixmapFromSheet(const QString& sheetPath, const QRect& rect, ResourceCategory category) {
    QPixmap sheetPixmap = getPixmap(sheetPath, category);
    if (sheetPixmap.isNull()) {
        qWarning() << "ResourceManager::getPixmapFromSheet: Sprite sheet not found at" << sheetPath;
        return getFallbackPixmap(rect.size());
    }

    if (!sheetPixmap.rect().contains(rect)) {
        qWarning() << "ResourceManager::getPixmapFromSheet: Source rectangle" << rect
                   << "is outside the bounds of sprite sheet" << sheetPath
                   << "with size" << sheetPixmap.size();
        return getFallbackPixmap(rect.size());
    }

    return sheetPixmap.copy(rect);
}

QPixmap ResourceManager::getPixmapFromSheet(const QString& sheetPath, int x, int y, int width, int height, ResourceCategory category) {
    return getPixmapFromSheet(sheetPath, QRect(x, y, width, height), category);
}

// Categorized resource access methods
QPixmap ResourceManager::getIconPixmap(const QString& name) {
    QString path = resolveCategorizedPath(name, ResourceCategory::ICONS);
    return getPixmap(path, ResourceCategory::ICONS);
}

QPixmap ResourceManager::getTexturePixmap(const QString& name) {
    QString path = resolveCategorizedPath(name, ResourceCategory::TEXTURES);
    return getPixmap(path, ResourceCategory::TEXTURES);
}

QPixmap ResourceManager::getBrushPixmap(const QString& name) {
    QString path = resolveCategorizedPath(name, ResourceCategory::BRUSHES);
    return getPixmap(path, ResourceCategory::BRUSHES);
}

QPixmap ResourceManager::getItemPixmap(const QString& name) {
    QString path = resolveCategorizedPath(name, ResourceCategory::ITEMS);
    return getPixmap(path, ResourceCategory::ITEMS);
}

QPixmap ResourceManager::getBorderPixmap(const QString& name) {
    QString path = resolveCategorizedPath(name, ResourceCategory::BORDERS);
    return getPixmap(path, ResourceCategory::BORDERS);
}

QPixmap ResourceManager::getGroundPixmap(const QString& name) {
    QString path = resolveCategorizedPath(name, ResourceCategory::GROUND);
    return getPixmap(path, ResourceCategory::GROUND);
}

QPixmap ResourceManager::getEffectPixmap(const QString& name) {
    QString path = resolveCategorizedPath(name, ResourceCategory::EFFECTS);
    return getPixmap(path, ResourceCategory::EFFECTS);
}

QPixmap ResourceManager::getCursorPixmap(const QString& name) {
    QString path = resolveCategorizedPath(name, ResourceCategory::CURSORS);
    return getPixmap(path, ResourceCategory::CURSORS);
}

// Fallback and placeholder resources
QPixmap ResourceManager::getFallbackPixmap(const QSize& size) {
    if (!defaultFallbackPixmap_.isNull() && defaultFallbackPixmap_.size() == size) {
        return defaultFallbackPixmap_;
    }
    return createFallbackPixmap(size, ResourceCategory::FALLBACK);
}

QPixmap ResourceManager::getPlaceholderPixmap(ResourceCategory category, const QSize& size) {
    return createFallbackPixmap(size, category);
}

// Core internal loading method
QPixmap ResourceManager::loadPixmapInternal(const QString& path, ResourceCategory category) {
    QMutexLocker locker(&cacheMutex_);

    if (path.isEmpty()) {
        qWarning() << "ResourceManager::loadPixmapInternal: Requested pixmap with empty path.";
        return getFallbackPixmap();
    }

    // Check cache first
    auto it = pixmapCache_.find(path);
    if (it != pixmapCache_.end()) {
        // Update access information
        if (resourceInfo_.contains(path)) {
            resourceInfo_[path].lastAccessed = QDateTime::currentMSecsSinceEpoch();
            resourceInfo_[path].accessCount++;
        }
        return it.value();
    }

    // Load the pixmap
    QPixmap pixmap;
    qint64 startTime = QDateTime::currentMSecsSinceEpoch();

    if (!pixmap.load(path)) {
        qWarning() << "ResourceManager::loadPixmapInternal: Failed to load pixmap from" << path;
        return getFallbackPixmap();
    }

    // Update cache and metadata
    updateResourceInfo(path, pixmap, category);
    pixmapCache_.insert(path, pixmap);

    // Add to category index
    if (!categoryIndex_[category].contains(path)) {
        categoryIndex_[category].append(path);
    }

    // Enforce memory limits
    enforceMemoryLimits();

    qDebug() << "ResourceManager: Loaded pixmap" << path << "in"
             << (QDateTime::currentMSecsSinceEpoch() - startTime) << "ms";

    return pixmap;
}

// Initialization methods
void ResourceManager::initializeSupportedFormats() {
    // Get all supported image formats from Qt
    QList<QByteArray> formats = QImageReader::supportedImageFormats();
    for (const QByteArray& format : formats) {
        supportedFormats_.append(QString::fromLatin1(format).toLower());
    }

    // Ensure common formats are included
    QStringList commonFormats = {"png", "jpg", "jpeg", "bmp", "gif", "xpm", "svg"};
    for (const QString& format : commonFormats) {
        if (!supportedFormats_.contains(format)) {
            supportedFormats_.append(format);
        }
    }
}

void ResourceManager::initializeCategoryPaths() {
    // Set up base paths for each category
    categoryBasePaths_[ResourceCategory::ICONS] = ":/icons/";
    categoryBasePaths_[ResourceCategory::TEXTURES] = ":/textures/";
    categoryBasePaths_[ResourceCategory::UI_ELEMENTS] = ":/ui/";
    categoryBasePaths_[ResourceCategory::BRUSHES] = ":/brushes/";
    categoryBasePaths_[ResourceCategory::ITEMS] = ":/items/";
    categoryBasePaths_[ResourceCategory::BORDERS] = ":/borders/";
    categoryBasePaths_[ResourceCategory::GROUND] = ":/ground/";
    categoryBasePaths_[ResourceCategory::EFFECTS] = ":/effects/";
    categoryBasePaths_[ResourceCategory::CURSORS] = ":/cursors/";
    categoryBasePaths_[ResourceCategory::MISC] = ":/misc/";
    categoryBasePaths_[ResourceCategory::FALLBACK] = ":/fallback/";
}

void ResourceManager::initializeFallbackResources() {
    // Create a default fallback pixmap
    defaultFallbackPixmap_ = createFallbackPixmap(QSize(16, 16), ResourceCategory::FALLBACK);

    // Set up fallback paths for each category
    fallbackPaths_[ResourceCategory::ICONS] = ":/icons/default_icon.png";
    fallbackPaths_[ResourceCategory::TEXTURES] = ":/textures/default_texture.png";
    fallbackPaths_[ResourceCategory::UI_ELEMENTS] = ":/ui/default_ui.png";
    fallbackPaths_[ResourceCategory::BRUSHES] = ":/brushes/default_brush.png";
    fallbackPaths_[ResourceCategory::ITEMS] = ":/items/default_item.png";
    fallbackPaths_[ResourceCategory::BORDERS] = ":/borders/default_border.png";
    fallbackPaths_[ResourceCategory::GROUND] = ":/ground/default_ground.png";
    fallbackPaths_[ResourceCategory::EFFECTS] = ":/effects/default_effect.png";
    fallbackPaths_[ResourceCategory::CURSORS] = ":/cursors/default_cursor.png";
    fallbackPaths_[ResourceCategory::MISC] = ":/misc/default_misc.png";
}

// Helper methods
QString ResourceManager::resolveCategorizedPath(const QString& name, ResourceCategory category) const {
    // If name is already a full path, return it
    if (name.startsWith(":/") || name.startsWith("/") || name.contains(":\\")) {
        return name;
    }

    // Build path from category base path
    QString basePath = categoryBasePaths_.value(category, ":/");
    if (!basePath.endsWith("/")) {
        basePath += "/";
    }

    // Try with different extensions if no extension provided
    if (!name.contains(".")) {
        for (const QString& ext : {"png", "jpg", "bmp", "xpm"}) {
            QString fullPath = basePath + name + "." + ext;
            if (QFileInfo::exists(fullPath) || fullPath.startsWith(":/")) {
                return fullPath;
            }
        }
    }

    return basePath + name;
}

void ResourceManager::updateResourceInfo(const QString& path, const QPixmap& pixmap, ResourceCategory category) {
    ResourceInfo& info = resourceInfo_[path];
    info.path = path;
    info.category = category;
    info.originalSize = pixmap.size();
    info.loadTime = QDateTime::currentMSecsSinceEpoch();
    info.lastAccessed = info.loadTime;
    info.accessCount = 1;
    info.isFromQtResource = isQtResourcePath(path);
}

bool ResourceManager::isQtResourcePath(const QString& path) const {
    return path.startsWith(":/");
}

QPixmap ResourceManager::createFallbackPixmap(const QSize& size, ResourceCategory category) const {
    QPixmap pixmap(size);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing, false);

    // Choose color based on category
    QColor color;
    switch (category) {
        case ResourceCategory::ICONS: color = QColor(100, 100, 255); break;
        case ResourceCategory::TEXTURES: color = QColor(150, 150, 150); break;
        case ResourceCategory::BRUSHES: color = QColor(255, 100, 100); break;
        case ResourceCategory::ITEMS: color = QColor(100, 255, 100); break;
        case ResourceCategory::BORDERS: color = QColor(255, 255, 100); break;
        case ResourceCategory::GROUND: color = QColor(139, 69, 19); break;
        case ResourceCategory::EFFECTS: color = QColor(255, 100, 255); break;
        case ResourceCategory::CURSORS: color = QColor(255, 255, 255); break;
        default: color = QColor(128, 128, 128); break;
    }

    // Draw a simple pattern
    painter.fillRect(pixmap.rect(), color);
    painter.setPen(Qt::black);
    painter.drawRect(0, 0, size.width()-1, size.height()-1);
    painter.drawLine(0, 0, size.width()-1, size.height()-1);
    painter.drawLine(0, size.height()-1, size.width()-1, 0);

    return pixmap;
}

void ResourceManager::enforceMemoryLimits() {
    // Remove least recently used items if cache is too large
    if (pixmapCache_.size() > maxCacheItems_) {
        // Find least recently used items
        QList<QPair<qint64, QString>> accessTimes;
        for (auto it = resourceInfo_.begin(); it != resourceInfo_.end(); ++it) {
            accessTimes.append(std::make_pair(it.value().lastAccessed, it.key()));
        }

        // Sort by access time (oldest first)
        std::sort(accessTimes.begin(), accessTimes.end());

        // Remove oldest items
        int itemsToRemove = pixmapCache_.size() - maxCacheItems_ + 100; // Remove extra for efficiency
        for (int i = 0; i < itemsToRemove && i < accessTimes.size(); ++i) {
            const QString& path = accessTimes[i].second;
            pixmapCache_.remove(path);
            resourceInfo_.remove(path);

            // Remove from category index
            for (auto& categoryList : categoryIndex_) {
                categoryList.removeAll(path);
            }
        }

        qDebug() << "ResourceManager: Enforced memory limits, removed" << itemsToRemove << "items";
    }
}

// Basic cache management methods
void ResourceManager::clearCache() {
    QMutexLocker locker(&cacheMutex_);
    pixmapCache_.clear();
    resourceInfo_.clear();
    categoryIndex_.clear();
    qDebug() << "ResourceManager: Cache cleared.";
}

bool ResourceManager::isPixmapCached(const QString& path) const {
    QMutexLocker locker(&cacheMutex_);
    return pixmapCache_.contains(path);
}

int ResourceManager::cacheSize() const {
    QMutexLocker locker(&cacheMutex_);
    return pixmapCache_.size();
}

QStringList ResourceManager::getSupportedFormats() const {
    return supportedFormats_;
}

// Cleanup slot
void ResourceManager::performCleanup() {
    QMutexLocker locker(&cacheMutex_);
    cleanupUnusedResources();
    enforceMemoryLimits();
}

void ResourceManager::cleanupUnusedResources(int maxUnusedTime) {
    qint64 currentTime = QDateTime::currentMSecsSinceEpoch();
    QStringList toRemove;

    for (auto it = resourceInfo_.begin(); it != resourceInfo_.end(); ++it) {
        if (currentTime - it.value().lastAccessed > maxUnusedTime) {
            toRemove.append(it.key());
        }
    }

    for (const QString& path : toRemove) {
        pixmapCache_.remove(path);
        resourceInfo_.remove(path);

        // Remove from category index
        for (auto& categoryList : categoryIndex_) {
            categoryList.removeAll(path);
        }
    }

    if (!toRemove.isEmpty()) {
        qDebug() << "ResourceManager: Cleaned up" << toRemove.size() << "unused resources";
    }
}


