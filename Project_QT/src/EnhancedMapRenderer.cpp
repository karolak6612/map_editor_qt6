#include "EnhancedMapRenderer.h"
#include "LODManager.h"
#include "EnhancedLightingSystem.h"
#include "TransparencyManager.h"
#include "EnhancedDrawingOptions.h"
#include "DrawingOptions.h"
#include "MapView.h"
#include "Map.h"
#include "Tile.h"
#include "Item.h"
#include "GameSprite.h"
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>
#include <QTimer>
#include <QMutexLocker>
#include <QPainter>
#include <QElapsedTimer>
#include <QApplication>
#include <QThread>
#include <qmath.h>
#include <QVariant>
#include <QRect>
#include <QPoint>
#include <QColor>
#include <QPixmap>
#include <algorithm>
#include <exception>

// Static constants for rendering optimization (matching wxwidgets)
const int EnhancedMapRenderer::STATISTICS_UPDATE_INTERVAL = 1000; // 1 second
const int EnhancedMapRenderer::MAX_CACHE_SIZE = 100;
const int EnhancedMapRenderer::FPS_HISTORY_SIZE = 60;
const double EnhancedMapRenderer::CULLING_MARGIN = 2.0; // 2 tiles margin for culling

// EnhancedMapRenderer Implementation
EnhancedMapRenderer::EnhancedMapRenderer(QObject* parent)
    : QObject(parent)
    , lodManager_(nullptr)
    , lightingSystem_(nullptr)
    , transparencyManager_(nullptr)
    , drawingOptions_(nullptr)
    , lastFPSUpdate_(0.0)
    , batchingActive_(false)
    , renderThread_(nullptr)
{
    // Initialize configuration with wxwidgets-compatible defaults
    resetToDefaults();
    
    // Initialize statistics
    resetStatistics();
    
    // Setup performance monitoring timer
    statisticsTimer_ = new QTimer(this);
    statisticsTimer_->setInterval(STATISTICS_UPDATE_INTERVAL);
    connect(statisticsTimer_, &QTimer::timeout, this, &EnhancedMapRenderer::onStatisticsTimer);
    statisticsTimer_->start();
    
    // Initialize performance timer
    performanceTimer_.start();
}

EnhancedMapRenderer::~EnhancedMapRenderer() {
    cleanupRenderCache();
    
    if (renderThread_) {
        renderThread_->quit();
        renderThread_->wait();
        delete renderThread_;
    }
}

void EnhancedMapRenderer::setRenderingConfig(const RenderingConfig& config) {
    RenderingConfig oldConfig = config_;
    config_ = config;
    
    // Clear caches when configuration changes
    cleanupRenderCache();
    
    emit renderingConfigChanged();
}

void EnhancedMapRenderer::resetToDefaults() {
    config_ = RenderingConfig();
    
    // Set wxwidgets-compatible defaults (matching map_drawer.cpp)
    config_.enableLOD = true;
    config_.enableLighting = true;
    config_.enableTransparency = true;
    config_.enableCaching = true;
    config_.enableBatchRendering = true;
    config_.enableCulling = true;
    config_.enableMultithreading = false; // Disabled by default for stability
    
    // Performance settings
    config_.maxFPS = 60.0;
    config_.targetFrameTime = 16.67; // ~60 FPS
    config_.enableVSync = true;
    config_.enablePerformanceMonitoring = true;
    
    // Quality settings
    config_.renderQuality = RenderQuality::HIGH;
    config_.antialiasing = true;
    config_.smoothPixmapTransform = true;
    config_.highQualityAntialiasing = false;
    
    // Optimization settings
    config_.cullingMargin = CULLING_MARGIN;
    config_.maxCacheSize = MAX_CACHE_SIZE;
    config_.enableAsyncRendering = false;
    
    emit renderingConfigChanged();
}

void EnhancedMapRenderer::loadConfiguration(const QString& filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Failed to load rendering configuration from" << filePath;
        return;
    }
    
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    QJsonObject obj = doc.object();
    
    // Load feature settings
    QJsonObject featuresObj = obj.value("features").toObject();
    config_.enableLOD = featuresObj.value("enableLOD").toBool(true);
    config_.enableLighting = featuresObj.value("enableLighting").toBool(true);
    config_.enableTransparency = featuresObj.value("enableTransparency").toBool(true);
    config_.enableCaching = featuresObj.value("enableCaching").toBool(true);
    config_.enableBatchRendering = featuresObj.value("enableBatchRendering").toBool(true);
    config_.enableCulling = featuresObj.value("enableCulling").toBool(true);
    config_.enableMultithreading = featuresObj.value("enableMultithreading").toBool(false);
    
    // Load performance settings
    QJsonObject perfObj = obj.value("performance").toObject();
    config_.maxFPS = perfObj.value("maxFPS").toDouble(60.0);
    config_.targetFrameTime = perfObj.value("targetFrameTime").toDouble(16.67);
    config_.enableVSync = perfObj.value("enableVSync").toBool(true);
    config_.enablePerformanceMonitoring = perfObj.value("enablePerformanceMonitoring").toBool(true);
    
    // Load quality settings
    QJsonObject qualityObj = obj.value("quality").toObject();
    config_.renderQuality = static_cast<RenderQuality>(qualityObj.value("renderQuality").toInt(static_cast<int>(RenderQuality::HIGH)));
    config_.antialiasing = qualityObj.value("antialiasing").toBool(true);
    config_.smoothPixmapTransform = qualityObj.value("smoothPixmapTransform").toBool(true);
    config_.highQualityAntialiasing = qualityObj.value("highQualityAntialiasing").toBool(false);
    
    // Load optimization settings
    QJsonObject optObj = obj.value("optimization").toObject();
    config_.cullingMargin = optObj.value("cullingMargin").toDouble(CULLING_MARGIN);
    config_.maxCacheSize = optObj.value("maxCacheSize").toInt(MAX_CACHE_SIZE);
    config_.enableAsyncRendering = optObj.value("enableAsyncRendering").toBool(false);
    
    emit renderingConfigChanged();
}

void EnhancedMapRenderer::saveConfiguration(const QString& filePath) const {
    QJsonObject obj;
    
    // Save feature settings
    QJsonObject featuresObj;
    featuresObj["enableLOD"] = config_.enableLOD;
    featuresObj["enableLighting"] = config_.enableLighting;
    featuresObj["enableTransparency"] = config_.enableTransparency;
    featuresObj["enableCaching"] = config_.enableCaching;
    featuresObj["enableBatchRendering"] = config_.enableBatchRendering;
    featuresObj["enableCulling"] = config_.enableCulling;
    featuresObj["enableMultithreading"] = config_.enableMultithreading;
    obj["features"] = featuresObj;
    
    // Save performance settings
    QJsonObject perfObj;
    perfObj["maxFPS"] = config_.maxFPS;
    perfObj["targetFrameTime"] = config_.targetFrameTime;
    perfObj["enableVSync"] = config_.enableVSync;
    perfObj["enablePerformanceMonitoring"] = config_.enablePerformanceMonitoring;
    obj["performance"] = perfObj;
    
    // Save quality settings
    QJsonObject qualityObj;
    qualityObj["renderQuality"] = static_cast<int>(config_.renderQuality);
    qualityObj["antialiasing"] = config_.antialiasing;
    qualityObj["smoothPixmapTransform"] = config_.smoothPixmapTransform;
    qualityObj["highQualityAntialiasing"] = config_.highQualityAntialiasing;
    obj["quality"] = qualityObj;
    
    // Save optimization settings
    QJsonObject optObj;
    optObj["cullingMargin"] = config_.cullingMargin;
    optObj["maxCacheSize"] = config_.maxCacheSize;
    optObj["enableAsyncRendering"] = config_.enableAsyncRendering;
    obj["optimization"] = optObj;
    
    QJsonDocument doc(obj);
    
    QFile file(filePath);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(doc.toJson());
    } else {
        qWarning() << "Failed to save rendering configuration to" << filePath;
    }
}

// Core rendering methods (matching wxwidgets MapDrawer structure)
void EnhancedMapRenderer::renderMap(QPainter* painter, Map* map, const QRect& viewRect, int currentFloor, double zoom, const DrawingOptions& options) {
    if (!painter || !map) {
        qWarning() << "EnhancedMapRenderer::renderMap: null painter or map";
        return;
    }
    
    // Start performance measurement
    QElapsedTimer frameTimer;
    frameTimer.start();
    
    painter->save();
    
    // Apply rendering quality settings
    applyRenderingHints(painter);
    
    // Render background (matching wxwidgets DrawBackground)
    renderBackground(painter, viewRect, options);
    
    // Render map layers (matching wxwidgets DrawMap)
    renderMapLayers(painter, map, viewRect, currentFloor, zoom, options);
    
    // Render lighting effects (matching wxwidgets DrawLight)
    if (config_.enableLighting && lightingSystem_ && options.showLights) {
        lightingSystem_->renderLighting(painter, viewRect, currentFloor, options);
    }
    
    // Render higher floors with transparency (matching wxwidgets DrawHigherFloors)
    if (options.showHigherFloors) {
        renderHigherFloors(painter, map, viewRect, currentFloor, zoom, options);
    }
    
    // Render overlays and effects
    renderOverlays(painter, viewRect, currentFloor, options);
    
    painter->restore();
    
    // Update performance statistics
    double frameTime = frameTimer.nsecsElapsed() / 1000000.0; // Convert to milliseconds
    updatePerformanceStatistics(frameTime);
    
    emit frameRendered(frameTime);
}

void EnhancedMapRenderer::renderTile(QPainter* painter, Tile* tile, const QRect& tileRect, int currentFloor, double zoom, const DrawingOptions& options) {
    if (!painter || !tile) {
        return;
    }
    
    // Check if tile should be rendered based on LOD
    if (config_.enableLOD && lodManager_) {
        LODLevel currentLOD = lodManager_->getLevelForZoom(zoom);
        if (currentLOD == LODLevel::MINIMAL) {
            return; // Skip rendering at minimal LOD
        }
    }
    
    // Apply transparency if enabled
    double tileTransparency = 1.0;
    if (config_.enableTransparency && transparencyManager_) {
        tileTransparency = transparencyManager_->calculateFloorTransparency(tile->z(), currentFloor);
        if (tileTransparency < 0.01) {
            return; // Skip fully transparent tiles
        }
    }
    
    // Save painter state
    painter->save();
    
    // Apply transparency
    if (tileTransparency < 1.0) {
        painter->setOpacity(painter->opacity() * tileTransparency);
    }
    
    // Render tile using existing Tile::draw method
    tile->draw(painter, tileRect, options);
    
    painter->restore();
    
    // Update statistics
    statistics_.tilesRendered++;
}

void EnhancedMapRenderer::renderItem(QPainter* painter, Item* item, const QRect& itemRect, int currentFloor, double zoom, const DrawingOptions& options) {
    if (!painter || !item) {
        return;
    }

    // Check LOD filtering
    if (config_.enableLOD && lodManager_) {
        if (!lodManager_->shouldRenderItem(item, lodManager_->getLevelForZoom(zoom))) {
            return;
        }
    }

    // Calculate item transparency
    double itemTransparency = 1.0;
    if (config_.enableTransparency && transparencyManager_) {
        itemTransparency = transparencyManager_->calculateTransparency(item, itemRect.topLeft(), item->getZ(), currentFloor);
        if (itemTransparency < 0.01) {
            return; // Skip fully transparent items
        }
    }

    // Save painter state
    painter->save();

    // Apply transparency
    if (itemTransparency < 1.0) {
        painter->setOpacity(painter->opacity() * itemTransparency);
    }

    // Render item using existing Item::draw method
    item->draw(painter, itemRect, options);

    painter->restore();

    // Update statistics
    statistics_.itemsRendered++;
}

// Private rendering methods (matching wxwidgets MapDrawer structure)
void EnhancedMapRenderer::renderBackground(QPainter* painter, const QRect& viewRect, const DrawingOptions& options) {
    Q_UNUSED(options)

    // Render background color (matching wxwidgets DrawBackground)
    painter->fillRect(viewRect, QColor(64, 64, 64)); // Dark gray background
}

void EnhancedMapRenderer::renderMapLayers(QPainter* painter, Map* map, const QRect& viewRect, int currentFloor, double zoom, const DrawingOptions& options) {
    if (!map) return;

    // Calculate tile range to render
    int startX = qMax(0, viewRect.left() / 32 - static_cast<int>(config_.cullingMargin));
    int startY = qMax(0, viewRect.top() / 32 - static_cast<int>(config_.cullingMargin));
    int endX = qMin(map->width(), viewRect.right() / 32 + static_cast<int>(config_.cullingMargin) + 1);
    int endY = qMin(map->height(), viewRect.bottom() / 32 + static_cast<int>(config_.cullingMargin) + 1);

    // Render tiles in the visible area
    for (int x = startX; x < endX; ++x) {
        for (int y = startY; y < endY; ++y) {
            Tile* tile = map->getTile(x, y, currentFloor);
            if (tile) {
                QRect tileRect(x * 32, y * 32, 32, 32);
                if (isTileVisible(tile, viewRect, currentFloor)) {
                    renderTile(painter, tile, tileRect, currentFloor, zoom, options);
                }
            }
        }
    }

    // Update statistics
    statistics_.tilesProcessed += (endX - startX) * (endY - startY);
}

void EnhancedMapRenderer::renderHigherFloors(QPainter* painter, Map* map, const QRect& viewRect, int currentFloor, double zoom, const DrawingOptions& options) {
    if (!map || !options.showHigherFloors) return;

    // Render floors above current floor with transparency
    for (int floor = currentFloor + 1; floor <= 15; ++floor) { // Assuming max 16 floors (0-15)
        // Calculate transparency for higher floors
        double floorTransparency = 0.3; // 70% transparent for higher floors

        painter->save();
        painter->setOpacity(painter->opacity() * floorTransparency);

        // Calculate tile range
        int startX = qMax(0, viewRect.left() / 32 - static_cast<int>(config_.cullingMargin));
        int startY = qMax(0, viewRect.top() / 32 - static_cast<int>(config_.cullingMargin));
        int endX = qMin(map->width(), viewRect.right() / 32 + static_cast<int>(config_.cullingMargin) + 1);
        int endY = qMin(map->height(), viewRect.bottom() / 32 + static_cast<int>(config_.cullingMargin) + 1);

        // Render tiles on this floor
        for (int x = startX; x < endX; ++x) {
            for (int y = startY; y < endY; ++y) {
                Tile* tile = map->getTile(x, y, floor);
                if (tile) {
                    QRect tileRect(x * 32, y * 32, 32, 32);
                    if (isTileVisible(tile, viewRect, floor)) {
                        tile->draw(painter, tileRect, options);
                    }
                }
            }
        }

        painter->restore();
    }
}

void EnhancedMapRenderer::renderOverlays(QPainter* painter, const QRect& viewRect, int currentFloor, const DrawingOptions& options) {
    Q_UNUSED(painter)
    Q_UNUSED(viewRect)
    Q_UNUSED(currentFloor)
    Q_UNUSED(options)

    // Placeholder for overlay rendering (grid, selection, etc.)
    // This would integrate with existing overlay systems
}

void EnhancedMapRenderer::applyRenderingHints(QPainter* painter) {
    // Apply quality settings based on configuration
    QPainter::RenderHints hints = QPainter::TextAntialiasing;

    if (config_.antialiasing) {
        hints |= QPainter::Antialiasing;
    }

    if (config_.smoothPixmapTransform) {
        hints |= QPainter::SmoothPixmapTransform;
    }

    if (config_.highQualityAntialiasing) {
        hints |= QPainter::HighQualityAntialiasing;
    }

    painter->setRenderHints(hints);

    // Apply composition mode based on quality
    switch (config_.renderQuality) {
        case RenderQuality::LOW:
            painter->setCompositionMode(QPainter::CompositionMode_SourceOver);
            break;
        case RenderQuality::MEDIUM:
            painter->setCompositionMode(QPainter::CompositionMode_SourceOver);
            break;
        case RenderQuality::HIGH:
            painter->setCompositionMode(QPainter::CompositionMode_SourceOver);
            break;
        case RenderQuality::ULTRA:
            painter->setCompositionMode(QPainter::CompositionMode_SourceOver);
            break;
    }
}

// Optimization methods
QList<Tile*> EnhancedMapRenderer::cullTiles(Map* map, const QRect& viewRect, int floor) const {
    QList<Tile*> visibleTiles;

    if (!map || !config_.enableCulling) {
        return visibleTiles;
    }

    // Calculate tile range with culling margin
    int startX = qMax(0, viewRect.left() / 32 - static_cast<int>(config_.cullingMargin));
    int startY = qMax(0, viewRect.top() / 32 - static_cast<int>(config_.cullingMargin));
    int endX = qMin(map->width(), viewRect.right() / 32 + static_cast<int>(config_.cullingMargin) + 1);
    int endY = qMin(map->height(), viewRect.bottom() / 32 + static_cast<int>(config_.cullingMargin) + 1);

    // Collect visible tiles
    for (int x = startX; x < endX; ++x) {
        for (int y = startY; y < endY; ++y) {
            Tile* tile = map->getTile(x, y, floor);
            if (tile && isTileVisible(tile, viewRect, floor)) {
                visibleTiles.append(tile);
            }
        }
    }

    return visibleTiles;
}

QList<Item*> EnhancedMapRenderer::cullItems(const QList<Item*>& items, const QRect& viewRect, double zoom) const {
    QList<Item*> visibleItems;

    if (!config_.enableCulling) {
        return items;
    }

    for (Item* item : items) {
        if (item) {
            // Calculate item screen position
            QRect itemRect(item->getX() * 32, item->getY() * 32, 32, 32);

            if (isItemVisible(item, itemRect, viewRect)) {
                // Additional LOD-based culling
                if (config_.enableLOD && lodManager_) {
                    if (lodManager_->shouldRenderItem(item, lodManager_->getLevelForZoom(zoom))) {
                        visibleItems.append(item);
                    }
                } else {
                    visibleItems.append(item);
                }
            }
        }
    }

    return visibleItems;
}

bool EnhancedMapRenderer::isTileVisible(Tile* tile, const QRect& viewRect, int floor) const {
    if (!tile) return false;

    // Calculate tile screen position
    QRect tileRect(tile->x() * 32, tile->y() * 32, 32, 32);

    // Check if tile intersects with view rectangle
    return viewRect.intersects(tileRect);
}

bool EnhancedMapRenderer::isItemVisible(Item* item, const QRect& itemRect, const QRect& viewRect) const {
    if (!item) return false;

    // Check if item intersects with view rectangle
    return viewRect.intersects(itemRect);
}

// Batch rendering methods
void EnhancedMapRenderer::beginBatchRendering() {
    batchingActive_ = true;
    batchItems_.clear();
}

void EnhancedMapRenderer::addToBatch(const QRect& rect, Item* item, int floor) {
    if (!batchingActive_ || !item) return;

    BatchItem batchItem;
    batchItem.rect = rect;
    batchItem.item = item;
    batchItem.floor = floor;

    batchItems_.append(batchItem);
}

void EnhancedMapRenderer::renderBatch(QPainter* painter, double zoom) {
    if (!batchingActive_ || !painter || batchItems_.isEmpty()) return;

    // Sort batch items for optimal rendering order
    std::sort(batchItems_.begin(), batchItems_.end(), [](const BatchItem& a, const BatchItem& b) {
        // Sort by floor first, then by Y position, then X position
        if (a.floor != b.floor) return a.floor < b.floor;
        if (a.rect.y() != b.rect.y()) return a.rect.y() < b.rect.y();
        return a.rect.x() < b.rect.x();
    });

    // Render all items in batch
    DrawingOptions defaultOptions; // Use default options for batch rendering
    for (const BatchItem& batchItem : batchItems_) {
        renderItem(painter, batchItem.item, batchItem.rect, batchItem.floor, zoom, defaultOptions);
    }

    // Update statistics
    statistics_.itemsProcessed += batchItems_.size();
}

void EnhancedMapRenderer::endBatchRendering() {
    batchingActive_ = false;
    batchItems_.clear();
}

// Cache management
QString EnhancedMapRenderer::getRenderCacheKey(const QRect& area, int floor) const {
    return QString("%1_%2_%3_%4_%5").arg(area.x()).arg(area.y()).arg(area.width()).arg(area.height()).arg(floor);
}

void EnhancedMapRenderer::invalidateRenderCache(const QRect& area, int floor) {
    QMutexLocker locker(&cacheMutex_);

    QString cacheKey = getRenderCacheKey(area, floor);
    renderCache_.remove(cacheKey);
}

void EnhancedMapRenderer::cleanupRenderCache() {
    QMutexLocker locker(&cacheMutex_);

    // Remove excess cache entries if over limit
    while (renderCache_.size() > config_.maxCacheSize) {
        auto it = renderCache_.begin();
        renderCache_.erase(it);
    }
}

// Statistics and monitoring
void EnhancedMapRenderer::resetStatistics() {
    QMutexLocker locker(&cacheMutex_);

    statistics_.framesRendered = 0;
    statistics_.tilesRendered = 0;
    statistics_.itemsRendered = 0;
    statistics_.tilesProcessed = 0;
    statistics_.itemsProcessed = 0;
    statistics_.totalRenderTime = 0.0;
    statistics_.averageRenderTime = 0.0;
    statistics_.currentFPS = 0.0;
    statistics_.averageFPS = 0.0;
    statistics_.memoryUsage = 0;
    statistics_.cacheHitRate = 0.0;

    frameTimeHistory_.clear();
}

void EnhancedMapRenderer::updatePerformanceStatistics(double frameTime) {
    QMutexLocker locker(&cacheMutex_);

    statistics_.framesRendered++;
    statistics_.totalRenderTime += frameTime;

    // Calculate averages
    if (statistics_.framesRendered > 0) {
        statistics_.averageRenderTime = statistics_.totalRenderTime / statistics_.framesRendered;
    }

    // Update frame time history for FPS calculation
    frameTimeHistory_.append(frameTime);
    if (frameTimeHistory_.size() > FPS_HISTORY_SIZE) {
        frameTimeHistory_.removeFirst();
    }

    // Calculate FPS
    if (!frameTimeHistory_.isEmpty()) {
        double totalTime = 0.0;
        for (double time : frameTimeHistory_) {
            totalTime += time;
        }
        double averageFrameTime = totalTime / frameTimeHistory_.size();
        statistics_.currentFPS = (averageFrameTime > 0.0) ? 1000.0 / averageFrameTime : 0.0;
        statistics_.averageFPS = statistics_.currentFPS; // Simplified for now
    }

    // Update memory usage (approximate)
    statistics_.memoryUsage = renderCache_.size() * 1024; // Rough estimate

    // Calculate cache hit rate (placeholder)
    statistics_.cacheHitRate = 0.0; // Would be calculated based on actual cache usage

    emit statisticsUpdated(statistics_);
}

QString EnhancedMapRenderer::getRenderingDiagnosticInfo() const {
    QString info;
    info += QString("Enhanced Map Renderer Diagnostics:\n");
    info += QString("LOD Enabled: %1\n").arg(config_.enableLOD ? "Yes" : "No");
    info += QString("Lighting Enabled: %1\n").arg(config_.enableLighting ? "Yes" : "No");
    info += QString("Transparency Enabled: %1\n").arg(config_.enableTransparency ? "Yes" : "No");
    info += QString("Caching Enabled: %1\n").arg(config_.enableCaching ? "Yes" : "No");
    info += QString("Batch Rendering: %1\n").arg(config_.enableBatchRendering ? "Yes" : "No");
    info += QString("Culling Enabled: %1\n").arg(config_.enableCulling ? "Yes" : "No");
    info += QString("Render Quality: %1\n").arg(static_cast<int>(config_.renderQuality));
    info += QString("Cache Size: %1\n").arg(renderCache_.size());
    info += QString("Frames Rendered: %1\n").arg(statistics_.framesRendered);
    info += QString("Current FPS: %1\n").arg(statistics_.currentFPS, 0, 'f', 1);
    info += QString("Average Render Time: %1 ms\n").arg(statistics_.averageRenderTime, 0, 'f', 2);

    return info;
}

QVariantMap EnhancedMapRenderer::getRenderingDebugInfo() const {
    QVariantMap debug;

    debug["enableLOD"] = config_.enableLOD;
    debug["enableLighting"] = config_.enableLighting;
    debug["enableTransparency"] = config_.enableTransparency;
    debug["enableCaching"] = config_.enableCaching;
    debug["enableBatchRendering"] = config_.enableBatchRendering;
    debug["enableCulling"] = config_.enableCulling;
    debug["renderQuality"] = static_cast<int>(config_.renderQuality);
    debug["cacheSize"] = renderCache_.size();
    debug["framesRendered"] = statistics_.framesRendered;
    debug["tilesRendered"] = statistics_.tilesRendered;
    debug["itemsRendered"] = statistics_.itemsRendered;
    debug["currentFPS"] = statistics_.currentFPS;
    debug["averageRenderTime"] = statistics_.averageRenderTime;
    debug["memoryUsage"] = statistics_.memoryUsage;

    return debug;
}

// Public slots
void EnhancedMapRenderer::onMapChanged() {
    cleanupRenderCache();
    resetStatistics();
}

void EnhancedMapRenderer::onDrawingOptionsChanged() {
    // This would be called when DrawingOptions change
    // Implementation depends on how DrawingOptions are managed
    cleanupRenderCache();
}

void EnhancedMapRenderer::onZoomChanged(double newZoom) {
    Q_UNUSED(newZoom)
    // Update LOD level if LOD manager is available
    if (config_.enableLOD && lodManager_) {
        lodManager_->updateLODLevel(newZoom);
    }

    // Clear cache as zoom affects rendering
    cleanupRenderCache();
}

void EnhancedMapRenderer::onFloorChanged(int newFloor) {
    Q_UNUSED(newFloor)
    // Clear cache as floor affects rendering
    cleanupRenderCache();
}

void EnhancedMapRenderer::updateRenderingSystem() {
    cleanupRenderCache();
    resetStatistics();
    emit renderingConfigChanged();
}

// Private slots
void EnhancedMapRenderer::onStatisticsTimer() {
    emit statisticsUpdated(statistics_);
}

// Integration methods
void EnhancedMapRenderer::setLODManager(LODManager* lodManager) {
    lodManager_ = lodManager;

    if (lodManager_) {
        // Connect LOD manager signals
        connect(lodManager_, &LODManager::lodLevelChanged, this, [this](LODLevel newLevel, LODLevel oldLevel) {
            Q_UNUSED(newLevel)
            Q_UNUSED(oldLevel)
            cleanupRenderCache();
        });
    }
}

void EnhancedMapRenderer::setLightingSystem(EnhancedLightingSystem* lightingSystem) {
    lightingSystem_ = lightingSystem;

    if (lightingSystem_) {
        // Connect lighting system signals
        connect(lightingSystem_, &EnhancedLightingSystem::globalLightingChanged, this, [this]() {
            cleanupRenderCache();
        });

        connect(lightingSystem_, &EnhancedLightingSystem::lightSourceAdded, this, [this](const LightSource& light) {
            Q_UNUSED(light)
            cleanupRenderCache();
        });

        connect(lightingSystem_, &EnhancedLightingSystem::lightSourceRemoved, this, [this](const QPoint& position, int floor) {
            Q_UNUSED(position)
            Q_UNUSED(floor)
            cleanupRenderCache();
        });
    }
}

void EnhancedMapRenderer::setTransparencyManager(TransparencyManager* transparencyManager) {
    transparencyManager_ = transparencyManager;

    if (transparencyManager_) {
        // Connect transparency manager signals
        connect(transparencyManager_, &TransparencyManager::transparencyConfigChanged, this, [this]() {
            cleanupRenderCache();
        });

        connect(transparencyManager_, &TransparencyManager::transparencyModeChanged, this, [this](TransparencyMode newMode, TransparencyMode oldMode) {
            Q_UNUSED(newMode)
            Q_UNUSED(oldMode)
            cleanupRenderCache();
        });
    }
}

void EnhancedMapRenderer::setDrawingOptions(EnhancedDrawingOptions* drawingOptions) {
    drawingOptions_ = drawingOptions;

    if (drawingOptions_) {
        // Connect drawing options signals if available
        // This would depend on the EnhancedDrawingOptions implementation
    }
}

// AsyncRenderWorker Implementation
AsyncRenderWorker::AsyncRenderWorker(EnhancedMapRenderer* renderer, QObject* parent)
    : QObject(parent)
    , renderer_(renderer)
{
    if (!renderer_) {
        qWarning() << "AsyncRenderWorker created with null renderer";
    }
}

AsyncRenderWorker::~AsyncRenderWorker() {
    // Cleanup if needed
}

void AsyncRenderWorker::renderMapAsync(Map* map, const QRect& viewRect, int currentFloor, double zoom) {
    if (!renderer_ || !map) {
        emit renderingError("Invalid renderer or map");
        return;
    }

    try {
        // Create a pixmap for off-screen rendering
        QPixmap result(viewRect.size());
        result.fill(Qt::transparent);

        QPainter painter(&result);

        // Start timing
        QElapsedTimer timer;
        timer.start();

        // Render map to pixmap
        DrawingOptions defaultOptions; // Use default options for async rendering
        renderer_->renderMap(&painter, map, viewRect, currentFloor, zoom, defaultOptions);

        painter.end();

        double renderTime = timer.nsecsElapsed() / 1000000.0; // Convert to milliseconds

        emit renderingCompleted(result, renderTime);

    } catch (const std::exception& e) {
        emit renderingError(QString("Rendering exception: %1").arg(e.what()));
    } catch (...) {
        emit renderingError("Unknown rendering error");
    }
}
