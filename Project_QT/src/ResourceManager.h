#ifndef QT_RESOURCEMANAGER_H
#define QT_RESOURCEMANAGER_H

#include <QObject>
#include <QPixmap>
#include <QIcon>
#include <QString>
#include <QStringList>
#include <QMap>
#include <QHash>
#include <QMutex>
#include <QMutexLocker>
#include <QSize>
#include <QTimer>

// Forward declarations
class QRect;
class QDir;

// Resource categories for organization and management
enum class ResourceCategory {
    ICONS,           // UI icons and toolbar buttons
    TEXTURES,        // Background textures and patterns
    UI_ELEMENTS,     // UI component graphics
    BRUSHES,         // Brush and tool icons
    ITEMS,           // Item representation icons
    BORDERS,         // Border and frame graphics
    GROUND,          // Ground tile representations
    EFFECTS,         // Visual effect graphics
    CURSORS,         // Mouse cursor graphics
    MISC,            // Miscellaneous resources
    FALLBACK         // Fallback/placeholder resources
};

// Resource metadata for enhanced management
struct ResourceInfo {
    QString path;
    ResourceCategory category;
    QSize originalSize;
    qint64 loadTime;
    qint64 lastAccessed;
    int accessCount;
    bool isFromQtResource;

    ResourceInfo() : category(ResourceCategory::MISC), loadTime(0), lastAccessed(0),
                    accessCount(0), isFromQtResource(false) {}
};

class ResourceManager : public QObject {
    Q_OBJECT

public:
    // Singleton access method
    static ResourceManager& instance();

    // Core resource loading methods
    QPixmap getPixmap(const QString& path, ResourceCategory category = ResourceCategory::MISC);
    QIcon getIcon(const QString& path, ResourceCategory category = ResourceCategory::ICONS);

    // Enhanced sprite sheet functionality
    QPixmap getPixmapFromSheet(const QString& sheetPath, const QRect& rect,
                              ResourceCategory category = ResourceCategory::MISC);
    QPixmap getPixmapFromSheet(const QString& sheetPath, int x, int y, int width, int height,
                              ResourceCategory category = ResourceCategory::MISC);

    // Categorized resource access
    QPixmap getIconPixmap(const QString& name);           // Icons category
    QPixmap getTexturePixmap(const QString& name);        // Textures category
    QPixmap getBrushPixmap(const QString& name);          // Brushes category
    QPixmap getItemPixmap(const QString& name);           // Items category
    QPixmap getBorderPixmap(const QString& name);         // Borders category
    QPixmap getGroundPixmap(const QString& name);         // Ground category
    QPixmap getEffectPixmap(const QString& name);         // Effects category
    QPixmap getCursorPixmap(const QString& name);         // Cursors category

    // Fallback and placeholder resources
    QPixmap getFallbackPixmap(const QSize& size = QSize(16, 16));
    QPixmap getPlaceholderPixmap(ResourceCategory category, const QSize& size = QSize(16, 16));

    // Batch loading and preloading
    void preloadPixmap(const QString& path, ResourceCategory category = ResourceCategory::MISC);
    void preloadPixmaps(const QStringList& paths, ResourceCategory category = ResourceCategory::MISC);
    void preloadCategory(ResourceCategory category);
    void preloadFromDirectory(const QString& dirPath, ResourceCategory category);

    // Resource discovery and validation
    QStringList discoverResources(const QString& basePath, const QStringList& patterns = QStringList());
    QStringList getResourcesInCategory(ResourceCategory category) const;
    bool validateResource(const QString& path) const;
    QStringList getSupportedFormats() const;

    // Cache management and statistics
    void clearCache();
    void clearPixmapFromCache(const QString& path);
    void clearCategory(ResourceCategory category);
    bool isPixmapCached(const QString& path) const;
    int cacheSize() const;
    int getCacheSize(ResourceCategory category) const;
    qint64 getCacheMemoryUsage() const;

    // Cache optimization and cleanup
    void optimizeCache();
    void cleanupUnusedResources(int maxUnusedTime = 300000); // 5 minutes default
    void setMaxCacheSize(int maxSizeMB);
    void setMaxCacheItems(int maxItems);

    // Resource information and statistics
    ResourceInfo getResourceInfo(const QString& path) const;
    QStringList getCachedResources() const;
    void printCacheStatistics() const;

private:
    // Private constructor and destructor for singleton pattern
    ResourceManager();
    ~ResourceManager();

    // Disallow copy and assign
    ResourceManager(const ResourceManager&) = delete;
    ResourceManager& operator=(const ResourceManager&) = delete;
    ResourceManager(ResourceManager&&) = delete;
    ResourceManager& operator=(ResourceManager&&) = delete;

    // Core caching system
    mutable QMutex cacheMutex_;                           // Thread safety
    QHash<QString, QPixmap> pixmapCache_;                 // Main pixmap cache
    QHash<QString, ResourceInfo> resourceInfo_;          // Resource metadata
    QHash<ResourceCategory, QStringList> categoryIndex_; // Category organization

    // Cache management
    int maxCacheItems_;
    int maxCacheSizeMB_;
    QTimer* cleanupTimer_;

    // Fallback resources
    QHash<ResourceCategory, QString> fallbackPaths_;
    QPixmap defaultFallbackPixmap_;

    // Resource path mapping for categorized access
    QHash<ResourceCategory, QString> categoryBasePaths_;
    QStringList supportedFormats_;

    // Internal helper methods
    QPixmap loadPixmapInternal(const QString& path, ResourceCategory category);
    QString resolveCategorizedPath(const QString& name, ResourceCategory category) const;
    void updateResourceInfo(const QString& path, const QPixmap& pixmap, ResourceCategory category);
    void initializeFallbackResources();
    void initializeCategoryPaths();
    void initializeSupportedFormats();
    bool isQtResourcePath(const QString& path) const;
    QPixmap createFallbackPixmap(const QSize& size, ResourceCategory category) const;
    void enforceMemoryLimits();

private slots:
    void performCleanup();
};

#endif // QT_RESOURCEMANAGER_H
