#ifndef QT_RESOURCEMANAGER_H
#define QT_RESOURCEMANAGER_H

#include <QObject> // QObject for potential signals/slots if ever needed, or just make it non-QObject
#include <QPixmap>
#include <QIcon>
#include <QString>
#include <QMap>
#include <QMutex> // For thread-safe access if needed in the future

// Forward declaration
class QRect;

class ResourceManager {
public:
    // Singleton access method
    static ResourceManager& instance();

    // Get a pixmap from a path (can be Qt resource path e.g. ":/path/to/image.png" or filesystem path)
    QPixmap getPixmap(const QString& path);

    // Get an icon (can also be from resource path or filesystem path)
    QIcon getIcon(const QString& path);

    // Get a specific part of a larger pixmap (sprite sheet functionality)
    // The sheet itself is loaded and cached via getPixmap(sheetPath).
    QPixmap getPixmapFromSheet(const QString& sheetPath, const QRect& rect);

    // Preload a single resource or a list of resources
    void preloadPixmap(const QString& path);
    void preloadPixmaps(const QStringList& paths);

    // Cache management
    void clearCache();
    void clearPixmapFromCache(const QString& path);
    bool isPixmapCached(const QString& path) const;
    int cacheSize() const;

private:
    // Private constructor and destructor for singleton pattern
    ResourceManager();
    ~ResourceManager();

    // Disallow copy and assign
    ResourceManager(const ResourceManager&) = delete;
    ResourceManager& operator=(const ResourceManager&) = delete;
    ResourceManager(ResourceManager&&) = delete;
    ResourceManager& operator=(ResourceManager&&) = delete;

    // Internal cache
    // Using QHash for potentially better performance with string keys if a very large number of resources.
    // QMap is fine for moderate numbers. Let's stick with QMap for ordered iteration if ever needed for debugging.
    QMap<QString, QPixmap> m_pixmapCache;

    // Mutex for thread-safe cache access (optional for now, can be enabled if multi-threading becomes a concern)
    // mutable QMutex m_cacheMutex;
};

#endif // QT_RESOURCEMANAGER_H
