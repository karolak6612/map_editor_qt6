#include "ResourceManager.h"
#include <QDebug>
#include <QRect> // Already forward-declared, but good to include for QRect usage.

// Initialize static instance to nullptr for singleton
// Note: Proper singleton often involves more complex static initialization,
// but for a single-threaded GUI app, a simple static local in instance() is common.
// Let's use the static local in instance() method for robust initialization order.

ResourceManager& ResourceManager::instance() {
    static ResourceManager instance; // Static local instance, created on first call
    return instance;
}

ResourceManager::ResourceManager() {
    qDebug() << "ResourceManager initialized.";
    // Initialize any members here if needed, e.g., default placeholder pixmaps
}

ResourceManager::~ResourceManager() {
    qDebug() << "ResourceManager destroyed. Cache size was:" << m_pixmapCache.size();
    clearCache(); // Ensure all QPixmaps are cleared
}

QPixmap ResourceManager::getPixmap(const QString& path) {
    // QMutexLocker locker(&m_cacheMutex); // Uncomment if thread-safety is enabled

    if (path.isEmpty()) {
        qWarning() << "ResourceManager::getPixmap: Requested pixmap with empty path.";
        return QPixmap(); // Return a null QPixmap
    }

    auto it = m_pixmapCache.find(path);
    if (it != m_pixmapCache.end()) {
        // qDebug() << "ResourceManager: Pixmap found in cache:" << path;
        return it.value(); // Return cached pixmap
    }

    // qDebug() << "ResourceManager: Pixmap not in cache, loading:" << path;
    QPixmap pixmap;
    if (!pixmap.load(path)) {
        qWarning() << "ResourceManager::getPixmap: Failed to load pixmap from" << path;
        // Optionally, return a default 'not found' pixmap:
        // if (m_pixmapCache.contains(":/path/to/default_not_found_image.png")) {
        //     return m_pixmapCache.value(":/path/to/default_not_found_image.png");
        // }
        return QPixmap(); // Return a null QPixmap
    }

    m_pixmapCache.insert(path, pixmap);
    return pixmap;
}

QIcon ResourceManager::getIcon(const QString& path) {
    // Icons can also be cached if they are frequently reconstructed from pixmaps,
    // but QIcon itself is relatively lightweight if it just points to a pixmap.
    // For simplicity, we'll load the pixmap and create an icon from it.
    // If complex icons with multiple states are needed, this might need more advanced caching.

    QPixmap pixmap = getPixmap(path); // Utilizes the pixmap cache
    if (pixmap.isNull()) {
        qWarning() << "ResourceManager::getIcon: Failed to load base pixmap for icon from" << path;
        return QIcon(); // Return a null QIcon
    }
    return QIcon(pixmap);
}

QPixmap ResourceManager::getPixmapFromSheet(const QString& sheetPath, const QRect& rect) {
    QPixmap sheetPixmap = getPixmap(sheetPath); // Load/get the full sheet (cached)
    if (sheetPixmap.isNull()) {
        qWarning() << "ResourceManager::getPixmapFromSheet: Sprite sheet not found at" << sheetPath;
        return QPixmap(); // Return null pixmap
    }

    if (!sheetPixmap.rect().contains(rect)) {
        qWarning() << "ResourceManager::getPixmapFromSheet: Source rectangle" << rect
                   << "is outside the bounds of sprite sheet" << sheetPath
                   << "with size" << sheetPixmap.size();
        return QPixmap(); // Return null pixmap
    }

    return sheetPixmap.copy(rect);
}

void ResourceManager::preloadPixmap(const QString& path) {
    if (!isPixmapCached(path)) {
        getPixmap(path); // This will load and cache it
    }
}

void ResourceManager::preloadPixmaps(const QStringList& paths) {
    for (const QString& path : paths) {
        preloadPixmap(path);
    }
}

void ResourceManager::clearCache() {
    // QMutexLocker locker(&m_cacheMutex); // Uncomment if thread-safety is enabled
    m_pixmapCache.clear();
    qDebug() << "ResourceManager: Cache cleared.";
}

void ResourceManager::clearPixmapFromCache(const QString& path) {
    // QMutexLocker locker(&m_cacheMutex); // Uncomment if thread-safety is enabled
    if (m_pixmapCache.remove(path)) {
        // qDebug() << "ResourceManager: Pixmap removed from cache:" << path;
    }
}

bool ResourceManager::isPixmapCached(const QString& path) const {
    // QMutexLocker locker(&m_cacheMutex); // Uncomment if thread-safety is enabled
    return m_pixmapCache.contains(path);
}

int ResourceManager::cacheSize() const {
    // QMutexLocker locker(&m_cacheMutex); // Uncomment if thread-safety is enabled
    return m_pixmapCache.size();
}
