#ifndef MINIMAPRRENDERER_H
#define MINIMAPRRENDERER_H

#include <QObject>
#include <QPixmap>
#include <QBitmap>
#include <QPainter>
#include <QColor>
#include <QPen>
#include <QBrush>
#include <QRect>
#include <QPoint>
#include <QSize>
#include <QMutex>
#include <QMutexLocker>
#include <QThread>
#include <QTimer>
#include <QElapsedTimer>
#include <QVariantMap>
#include <QCache>
#include <QQueue>
#include <QSet>
#include <functional>

#include "MapPos.h"
#include "ui/MinimapWindow.h"

// Forward declarations
class Map;
class Tile;
class Item;

/**
 * @brief Task 90: Minimap Renderer for optimized tile rendering
 * 
 * High-performance minimap rendering system:
 * - Block-based rendering with intelligent caching
 * - Multi-threaded rendering for large maps
 * - Optimized color lookup and batch drawing
 * - Progressive loading and update strategies
 * - Memory-efficient tile color computation
 * - Integration with Map change notifications
 * - 1:1 compatibility with wxwidgets rendering performance
 */

/**
 * @brief Render Job for threaded rendering
 */
struct MinimapRenderJob {
    int blockX;
    int blockY;
    int floor;
    QRect area;
    int priority;
    bool urgent;
    
    MinimapRenderJob(int bx = 0, int by = 0, int f = 0, const QRect& a = QRect(), 
                    int p = 0, bool u = false)
        : blockX(bx), blockY(by), floor(f), area(a), priority(p), urgent(u) {}
    
    QString getKey() const { return QString("%1_%2_%3").arg(blockX).arg(blockY).arg(floor); }
    
    bool operator<(const MinimapRenderJob& other) const {
        if (urgent != other.urgent) return !urgent; // Urgent jobs first
        return priority < other.priority; // Higher priority first
    }
};

/**
 * @brief Render Statistics for performance monitoring
 */
struct MinimapRenderStats {
    int blocksRendered;
    int tilesProcessed;
    int cacheHits;
    int cacheMisses;
    qint64 totalRenderTime;
    qint64 averageBlockTime;
    double tilesPerSecond;
    int memoryUsage;
    
    MinimapRenderStats() { reset(); }
    
    void reset() {
        blocksRendered = 0;
        tilesProcessed = 0;
        cacheHits = 0;
        cacheMisses = 0;
        totalRenderTime = 0;
        averageBlockTime = 0;
        tilesPerSecond = 0.0;
        memoryUsage = 0;
    }
    
    void update() {
        if (blocksRendered > 0) {
            averageBlockTime = totalRenderTime / blocksRendered;
        }
        if (totalRenderTime > 0) {
            tilesPerSecond = (tilesProcessed * 1000.0) / totalRenderTime;
        }
    }
};

/**
 * @brief Main Minimap Renderer
 */
class MinimapRenderer : public QObject
{
    Q_OBJECT

public:
    explicit MinimapRenderer(QObject* parent = nullptr);
    ~MinimapRenderer() override;

    // Configuration
    void setMap(Map* map);
    Map* getMap() const { return map_; }
    
    void setBlockSize(int size);
    int getBlockSize() const { return blockSize_; }
    
    void setCacheSize(int maxBlocks);
    int getCacheSize() const;
    
    void setThreadCount(int threads);
    int getThreadCount() const;

    // Rendering methods
    QPixmap renderBlock(int blockX, int blockY, int floor);
    QPixmap renderArea(const QRect& area, int floor, double scale = 1.0);
    QPixmap renderFullMap(int floor, const QSize& targetSize);
    
    // Async rendering
    void requestBlockRender(int blockX, int blockY, int floor, int priority = 0);
    void requestAreaRender(const QRect& area, int floor, int priority = 0);
    void cancelPendingRenders();
    
    // Cache management
    bool hasBlockInCache(int blockX, int blockY, int floor) const;
    QPixmap getBlockFromCache(int blockX, int blockY, int floor) const;
    void invalidateBlock(int blockX, int blockY, int floor);
    void invalidateArea(const QRect& area, int floor);
    void clearCache();
    
    // Statistics and monitoring
    MinimapRenderStats getStatistics() const;
    void resetStatistics();
    bool isRenderingInProgress() const;
    int getPendingJobCount() const;

    // Color computation
    static quint8 computeTileColor(const Tile* tile);
    static quint8 computeItemColor(const Item* item);
    static QColor getColorFromIndex(quint8 colorIndex);
    static quint8 findBestColorMatch(const QColor& color);

public slots:
    void onMapChanged();
    void onTileChanged(int x, int y, int z);
    void onAreaChanged(const QRect& area, int floor);
    void startRendering();
    void stopRendering();
    void pauseRendering();
    void resumeRendering();

signals:
    void blockRendered(int blockX, int blockY, int floor, const QPixmap& pixmap);
    void areaRendered(const QRect& area, int floor, const QPixmap& pixmap);
    void renderingProgress(int completed, int total);
    void renderingFinished();
    void statisticsUpdated(const MinimapRenderStats& stats);

private slots:
    void processRenderQueue();
    void onRenderJobCompleted();
    void updateStatistics();

private:
    // Core rendering implementation
    QPixmap renderBlockInternal(int blockX, int blockY, int floor);
    void renderTileToPixmap(QPainter* painter, int x, int y, int floor, const QPoint& offset);
    quint8 getTileColorFast(int x, int y, int floor) const;
    
    // Threading and job management
    void initializeThreadPool();
    void shutdownThreadPool();
    void scheduleRenderJob(const MinimapRenderJob& job);
    MinimapRenderJob getNextRenderJob();
    bool hasHighPriorityJobs() const;
    
    // Cache optimization
    void optimizeCache();
    void evictLeastRecentlyUsed();
    QString getBlockCacheKey(int blockX, int blockY, int floor) const;
    
    // Performance optimization
    void precomputeColorLookup();
    void optimizeColorPalette();
    void batchTileProcessing(const QList<QPoint>& tiles, int floor, QPainter* painter);
    
    // Memory management
    void cleanupResources();
    int calculateMemoryUsage() const;
    void enforceMemoryLimits();

private:
    // Core components
    Map* map_;
    
    // Rendering configuration
    int blockSize_;
    int maxCacheSize_;
    int threadCount_;
    bool renderingEnabled_;
    bool renderingPaused_;
    
    // Cache system
    mutable QMutex cacheMutex_;
    QCache<QString, QPixmap> blockCache_;
    QSet<QString> pendingBlocks_;
    QQueue<MinimapRenderJob> renderQueue_;
    
    // Threading
    QList<QThread*> renderThreads_;
    QMutex queueMutex_;
    QTimer* processTimer_;
    QTimer* statsTimer_;
    
    // Statistics
    mutable QMutex statsMutex_;
    MinimapRenderStats statistics_;
    QElapsedTimer renderTimer_;
    
    // Color optimization
    QVector<QColor> colorLookup_;
    QVector<QPen> penLookup_;
    bool colorLookupInitialized_;
    
    // Performance settings
    static const int DEFAULT_BLOCK_SIZE = 256;
    static const int DEFAULT_CACHE_SIZE = 100;
    static const int DEFAULT_THREAD_COUNT = 2;
    static const int PROCESS_INTERVAL = 50; // milliseconds
    static const int STATS_INTERVAL = 1000; // milliseconds
    static const int MAX_MEMORY_MB = 100;
};

/**
 * @brief Minimap Render Worker for threaded rendering
 */
class MinimapRenderWorker : public QObject
{
    Q_OBJECT

public:
    explicit MinimapRenderWorker(MinimapRenderer* renderer, QObject* parent = nullptr);
    ~MinimapRenderWorker() override = default;

public slots:
    void processJob(const MinimapRenderJob& job);

signals:
    void jobCompleted(const MinimapRenderJob& job, const QPixmap& result);
    void jobFailed(const MinimapRenderJob& job, const QString& error);

private:
    MinimapRenderer* renderer_;
};

/**
 * @brief Minimap Color Manager for color palette management
 */
class MinimapColorManager : public QObject
{
    Q_OBJECT

public:
    static MinimapColorManager* instance();
    
    // Color palette management
    void initializePalette();
    QColor getColor(quint8 index) const;
    quint8 findClosestColor(const QColor& color) const;
    QPen getPen(quint8 index) const;
    QBrush getBrush(quint8 index) const;
    
    // Palette customization
    void setColor(quint8 index, const QColor& color);
    void loadPalette(const QString& filePath);
    void savePalette(const QString& filePath) const;
    void resetToDefault();
    
    // Color analysis
    QList<quint8> getUsedColors(Map* map, int floor) const;
    QVariantMap getColorStatistics(Map* map, int floor) const;
    void optimizePaletteForMap(Map* map);

signals:
    void paletteChanged();
    void colorChanged(quint8 index, const QColor& color);

private:
    explicit MinimapColorManager(QObject* parent = nullptr);
    ~MinimapColorManager() override = default;

    void initializeDefaultPalette();
    void precomputePensAndBrushes();
    double colorDistance(const QColor& c1, const QColor& c2) const;

private:
    static MinimapColorManager* instance_;
    
    QVector<QColor> palette_;
    QVector<QPen> pens_;
    QVector<QBrush> brushes_;
    mutable QMutex paletteMutex_;
    
    static const int PALETTE_SIZE = 256;
};

/**
 * @brief Minimap Export Manager for saving minimap images
 */
class MinimapExportManager : public QObject
{
    Q_OBJECT

public:
    explicit MinimapExportManager(MinimapRenderer* renderer, QObject* parent = nullptr);
    ~MinimapExportManager() override = default;

    // Export methods
    bool exportFloor(int floor, const QString& filePath, const QSize& size = QSize());
    bool exportArea(const QRect& area, int floor, const QString& filePath, const QSize& size = QSize());
    bool exportAllFloors(const QString& directoryPath, const QSize& size = QSize());
    
    // Export options
    void setExportFormat(const QString& format);
    QString getExportFormat() const { return exportFormat_; }
    
    void setExportQuality(int quality);
    int getExportQuality() const { return exportQuality_; }
    
    void setIncludeWaypoints(bool include);
    bool isIncludeWaypoints() const { return includeWaypoints_; }

signals:
    void exportStarted(const QString& operation);
    void exportProgress(int current, int total);
    void exportCompleted(const QString& filePath);
    void exportFailed(const QString& error);

private:
    void addWaypointsToImage(QPixmap& image, int floor) const;
    QString generateFileName(int floor, const QString& basePath) const;

private:
    MinimapRenderer* renderer_;
    QString exportFormat_;
    int exportQuality_;
    bool includeWaypoints_;
};

#endif // MINIMAPRRENDERER_H
