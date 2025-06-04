#include "LODManager.h"
#include "DrawingOptions.h"
#include "MapView.h"
#include "Item.h"
#include "ItemManager.h"
#include "GameSprite.h"
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>
#include <QStandardPaths>
#include <QDir>
#include <QApplication>
#include <QMutexLocker>
#include <QTimer>
#include <QPainter>
#include <algorithm>

// Static constants - matching wxwidgets LOD thresholds exactly
const double LODManager::DEFAULT_FULL_DETAIL_MAX_ZOOM = 3.0;
const double LODManager::DEFAULT_MEDIUM_DETAIL_MAX_ZOOM = 7.0;
const double LODManager::DEFAULT_GROUND_ONLY_MAX_ZOOM = 10.0;
const int LODManager::STATISTICS_UPDATE_INTERVAL = 1000; // 1 second
const int LODManager::MAX_CACHE_SIZE = 1000;

// LODManager Implementation
LODManager::LODManager(QObject* parent)
    : QObject(parent)
    , currentLevel_(LODLevel::FULL_DETAIL)
    , mapView_(nullptr)
{
    // Initialize configuration with wxwidgets-compatible defaults
    resetToDefaults();
    
    // Initialize statistics
    resetStatistics();
    
    // Setup statistics timer
    statisticsTimer_ = new QTimer(this);
    statisticsTimer_->setInterval(STATISTICS_UPDATE_INTERVAL);
    connect(statisticsTimer_, &QTimer::timeout, this, &LODManager::onStatisticsTimer);
    statisticsTimer_->start();
}

LODManager::~LODManager() {
    clearCaches();
}

LODLevel LODManager::getLevelForZoom(double zoom) const {
    // Exact 1:1 mapping from wxwidgets lod_manager.h
    if (zoom >= config_.groundOnlyMaxZoom) {
        return LODLevel::GROUND_ONLY;
    }
    if (zoom >= config_.mediumDetailMaxZoom) {
        return LODLevel::MEDIUM_DETAIL;
    }
    return LODLevel::FULL_DETAIL;
}

void LODManager::updateLODLevel(double zoom) {
    LODLevel newLevel = getLevelForZoom(zoom);
    if (newLevel != currentLevel_) {
        LODLevel oldLevel = currentLevel_;
        currentLevel_ = newLevel;
        
        // Update rendering hints for performance
        optimizeRenderingForLOD(currentLevel_);
        
        emit lodLevelChanged(newLevel, oldLevel);
        
        qDebug() << "LOD level changed from" << static_cast<int>(oldLevel) 
                 << "to" << static_cast<int>(newLevel) << "at zoom" << zoom;
    }
}

void LODManager::setLODLevel(LODLevel level) {
    if (level != currentLevel_) {
        LODLevel oldLevel = currentLevel_;
        currentLevel_ = level;
        
        optimizeRenderingForLOD(currentLevel_);
        emit lodLevelChanged(level, oldLevel);
    }
}

void LODManager::setConfiguration(const LODConfiguration& config) {
    config_ = config;
    
    // Clear caches when configuration changes
    clearCaches();
    
    // Update current LOD level based on new configuration
    if (mapView_) {
        updateLODLevel(mapView_->getZoom());
    }
    
    emit configurationChanged();
}

void LODManager::resetToDefaults() {
    config_ = LODConfiguration();
    
    // Set wxwidgets-compatible defaults
    config_.fullDetailMaxZoom = DEFAULT_FULL_DETAIL_MAX_ZOOM;
    config_.mediumDetailMaxZoom = DEFAULT_MEDIUM_DETAIL_MAX_ZOOM;
    config_.groundOnlyMaxZoom = DEFAULT_GROUND_ONLY_MAX_ZOOM;
    
    // Item limits per LOD level (performance optimization)
    config_.maxItemsFullDetail = -1;  // No limit
    config_.maxItemsMediumDetail = 100;
    config_.maxItemsGroundOnly = 1;   // Ground only
    config_.maxItemsMinimal = 0;      // No items
    
    // Sprite detail settings
    config_.useSimplifiedSprites = true;
    config_.skipAnimations = true;
    config_.skipEffects = true;
    config_.skipTransparency = false;
    
    // Performance settings
    config_.enableCaching = true;
    config_.enableBatching = true;
    config_.enableCulling = true;
    
    emit configurationChanged();
}

void LODManager::loadConfiguration(const QString& filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Failed to load LOD configuration from" << filePath;
        return;
    }
    
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    QJsonObject obj = doc.object();
    
    // Load zoom thresholds
    config_.fullDetailMaxZoom = obj.value("fullDetailMaxZoom").toDouble(DEFAULT_FULL_DETAIL_MAX_ZOOM);
    config_.mediumDetailMaxZoom = obj.value("mediumDetailMaxZoom").toDouble(DEFAULT_MEDIUM_DETAIL_MAX_ZOOM);
    config_.groundOnlyMaxZoom = obj.value("groundOnlyMaxZoom").toDouble(DEFAULT_GROUND_ONLY_MAX_ZOOM);
    
    // Load item limits
    config_.maxItemsFullDetail = obj.value("maxItemsFullDetail").toInt(-1);
    config_.maxItemsMediumDetail = obj.value("maxItemsMediumDetail").toInt(100);
    config_.maxItemsGroundOnly = obj.value("maxItemsGroundOnly").toInt(1);
    config_.maxItemsMinimal = obj.value("maxItemsMinimal").toInt(0);
    
    // Load sprite settings
    config_.useSimplifiedSprites = obj.value("useSimplifiedSprites").toBool(true);
    config_.skipAnimations = obj.value("skipAnimations").toBool(true);
    config_.skipEffects = obj.value("skipEffects").toBool(true);
    config_.skipTransparency = obj.value("skipTransparency").toBool(false);
    
    // Load performance settings
    config_.enableCaching = obj.value("enableCaching").toBool(true);
    config_.enableBatching = obj.value("enableBatching").toBool(true);
    config_.enableCulling = obj.value("enableCulling").toBool(true);
    
    emit configurationChanged();
}

void LODManager::saveConfiguration(const QString& filePath) const {
    QJsonObject obj;
    
    // Save zoom thresholds
    obj["fullDetailMaxZoom"] = config_.fullDetailMaxZoom;
    obj["mediumDetailMaxZoom"] = config_.mediumDetailMaxZoom;
    obj["groundOnlyMaxZoom"] = config_.groundOnlyMaxZoom;
    
    // Save item limits
    obj["maxItemsFullDetail"] = config_.maxItemsFullDetail;
    obj["maxItemsMediumDetail"] = config_.maxItemsMediumDetail;
    obj["maxItemsGroundOnly"] = config_.maxItemsGroundOnly;
    obj["maxItemsMinimal"] = config_.maxItemsMinimal;
    
    // Save sprite settings
    obj["useSimplifiedSprites"] = config_.useSimplifiedSprites;
    obj["skipAnimations"] = config_.skipAnimations;
    obj["skipEffects"] = config_.skipEffects;
    obj["skipTransparency"] = config_.skipTransparency;
    
    // Save performance settings
    obj["enableCaching"] = config_.enableCaching;
    obj["enableBatching"] = config_.enableBatching;
    obj["enableCulling"] = config_.enableCulling;
    
    QJsonDocument doc(obj);
    
    QFile file(filePath);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(doc.toJson());
    } else {
        qWarning() << "Failed to save LOD configuration to" << filePath;
    }
}

bool LODManager::shouldRenderItem(const Item* item, LODLevel level) const {
    if (!item) return false;
    
    // Ground items are always rendered (except in minimal mode)
    if (item->isGroundTile()) {
        return level != LODLevel::MINIMAL;
    }
    
    // Check LOD level restrictions
    switch (level) {
        case LODLevel::FULL_DETAIL:
            return true; // Render everything
            
        case LODLevel::MEDIUM_DETAIL:
            // Skip some item types for performance
            return !shouldSkipItemByType(item, level);
            
        case LODLevel::GROUND_ONLY:
            // Only ground items
            return item->isGroundTile();
            
        case LODLevel::MINIMAL:
            // Nothing rendered
            return false;
    }
    
    return true;
}

bool LODManager::shouldSkipItemByType(const Item* item, LODLevel level) const {
    if (!item) return true;
    
    // Get item properties for type checking
    const ItemProperties& props = ItemManager::getInstance().getItemProperties(item->getServerId());
    
    switch (level) {
        case LODLevel::MEDIUM_DETAIL:
            // Skip decorative items, effects, and small details
            if (props.isDecoration || props.isEffect) return true;
            if (item->getTopOrder() < 5) return true; // Skip low-priority items
            break;
            
        case LODLevel::GROUND_ONLY:
            // Only ground items allowed
            return !item->isGroundTile();
            
        case LODLevel::MINIMAL:
            // Skip everything
            return true;
            
        default:
            break;
    }
    
    return false;
}

bool LODManager::shouldSkipItemByProperties(const Item* item, LODLevel level) const {
    if (!item) return true;
    
    // Additional property-based filtering
    switch (level) {
        case LODLevel::MEDIUM_DETAIL:
            // Skip items that are too small or transparent
            if (item->getTopOrder() < 3) return true;
            break;
            
        case LODLevel::GROUND_ONLY:
            return !item->isGroundTile();
            
        case LODLevel::MINIMAL:
            return true;
            
        default:
            break;
    }
    
    return false;
}

QList<Item*> LODManager::filterItemsByLOD(const QList<Item*>& items, LODLevel level) const {
    QList<Item*> filteredItems;
    
    int maxItems = getMaxItemsForLOD(level);
    int itemCount = 0;
    
    for (Item* item : items) {
        if (shouldRenderItem(item, level)) {
            filteredItems.append(item);
            itemCount++;
            
            // Respect item limits for performance
            if (maxItems > 0 && itemCount >= maxItems) {
                break;
            }
        }
    }
    
    return filteredItems;
}

int LODManager::getMaxItemsForLOD(LODLevel level) const {
    switch (level) {
        case LODLevel::FULL_DETAIL:
            return config_.maxItemsFullDetail;
        case LODLevel::MEDIUM_DETAIL:
            return config_.maxItemsMediumDetail;
        case LODLevel::GROUND_ONLY:
            return config_.maxItemsGroundOnly;
        case LODLevel::MINIMAL:
            return config_.maxItemsMinimal;
    }
    return -1; // No limit
}

QStringList LODManager::getSkippedTypesForLOD(LODLevel level) const {
    QStringList skippedTypes;

    switch (level) {
        case LODLevel::MEDIUM_DETAIL:
            skippedTypes << "decoration" << "effect" << "particle" << "shadow";
            break;
        case LODLevel::GROUND_ONLY:
            skippedTypes << "item" << "decoration" << "effect" << "particle" << "shadow" << "creature";
            break;
        case LODLevel::MINIMAL:
            skippedTypes << "ground" << "item" << "decoration" << "effect" << "particle" << "shadow" << "creature";
            break;
        default:
            break;
    }

    return skippedTypes;
}

bool LODManager::shouldUseSimplifiedSprite(const Item* item, LODLevel level) const {
    if (!item || !config_.useSimplifiedSprites) return false;

    switch (level) {
        case LODLevel::MEDIUM_DETAIL:
        case LODLevel::GROUND_ONLY:
            return true;
        default:
            return false;
    }
}

bool LODManager::shouldSkipAnimation(const Item* item, LODLevel level) const {
    if (!item || !config_.skipAnimations) return false;

    switch (level) {
        case LODLevel::MEDIUM_DETAIL:
        case LODLevel::GROUND_ONLY:
        case LODLevel::MINIMAL:
            return true;
        default:
            return false;
    }
}

bool LODManager::shouldSkipEffects(const Item* item, LODLevel level) const {
    if (!item || !config_.skipEffects) return false;

    switch (level) {
        case LODLevel::MEDIUM_DETAIL:
        case LODLevel::GROUND_ONLY:
        case LODLevel::MINIMAL:
            return true;
        default:
            return false;
    }
}

void LODManager::resetStatistics() {
    QMutexLocker locker(&cacheMutex_);

    statistics_.tilesProcessed = 0;
    statistics_.itemsProcessed = 0;
    statistics_.itemsSkipped = 0;
    statistics_.totalRenderTime = 0.0;
    statistics_.averageRenderTime = 0.0;
    statistics_.cacheHits = 0;
    statistics_.cacheMisses = 0;
    statistics_.memoryUsage = 0;
    statistics_.performanceGain = 0.0;
}

void LODManager::updateStatistics(int tilesProcessed, int itemsProcessed, int itemsSkipped, double renderTime) {
    QMutexLocker locker(&cacheMutex_);

    statistics_.tilesProcessed += tilesProcessed;
    statistics_.itemsProcessed += itemsProcessed;
    statistics_.itemsSkipped += itemsSkipped;
    statistics_.totalRenderTime += renderTime;

    // Calculate averages
    if (statistics_.tilesProcessed > 0) {
        statistics_.averageRenderTime = statistics_.totalRenderTime / statistics_.tilesProcessed;
    }

    // Calculate performance gain
    int totalItems = statistics_.itemsProcessed + statistics_.itemsSkipped;
    if (totalItems > 0) {
        statistics_.performanceGain = (double)statistics_.itemsSkipped / totalItems * 100.0;
    }

    // Update memory usage (approximate)
    statistics_.memoryUsage = simplifiedSpriteCache_.size() * sizeof(GameSprite*) * 2; // Rough estimate

    emit statisticsUpdated(statistics_);
}

void LODManager::applyToDrawingOptions(DrawingOptions& options) const {
    // Apply LOD settings to DrawingOptions for wxwidgets compatibility
    switch (currentLevel_) {
        case LODLevel::FULL_DETAIL:
            options.hideItemsWhenZoomed = false;
            options.showPreview = true;
            options.showEffects = true;
            break;

        case LODLevel::MEDIUM_DETAIL:
            options.hideItemsWhenZoomed = true;
            options.showPreview = false;
            options.showEffects = false;
            break;

        case LODLevel::GROUND_ONLY:
            options.hideItemsWhenZoomed = true;
            options.showItems = false;
            options.showCreatures = false;
            options.showPreview = false;
            options.showEffects = false;
            break;

        case LODLevel::MINIMAL:
            options.showGround = false;
            options.showItems = false;
            options.showCreatures = false;
            options.showPreview = false;
            options.showEffects = false;
            break;
    }
}

void LODManager::updateFromDrawingOptions(const DrawingOptions& options) {
    // Update LOD level based on DrawingOptions zoom
    updateLODLevel(options.zoom);
}

void LODManager::updateFromMapView() {
    if (mapView_) {
        updateLODLevel(mapView_->getZoom());
    }
}

QString LODManager::getDiagnosticInfo() const {
    QString info;
    info += QString("LOD Manager Diagnostics:\n");
    info += QString("Current Level: %1\n").arg(static_cast<int>(currentLevel_));
    info += QString("Full Detail Max Zoom: %1\n").arg(config_.fullDetailMaxZoom);
    info += QString("Medium Detail Max Zoom: %1\n").arg(config_.mediumDetailMaxZoom);
    info += QString("Ground Only Max Zoom: %1\n").arg(config_.groundOnlyMaxZoom);
    info += QString("Cache Size: %1\n").arg(simplifiedSpriteCache_.size());
    info += QString("Items Processed: %1\n").arg(statistics_.itemsProcessed);
    info += QString("Items Skipped: %1\n").arg(statistics_.itemsSkipped);
    info += QString("Performance Gain: %1%\n").arg(statistics_.performanceGain, 0, 'f', 1);

    return info;
}

QVariantMap LODManager::getDebugInfo() const {
    QVariantMap debug;

    debug["currentLevel"] = static_cast<int>(currentLevel_);
    debug["fullDetailMaxZoom"] = config_.fullDetailMaxZoom;
    debug["mediumDetailMaxZoom"] = config_.mediumDetailMaxZoom;
    debug["groundOnlyMaxZoom"] = config_.groundOnlyMaxZoom;
    debug["cacheSize"] = simplifiedSpriteCache_.size();
    debug["tilesProcessed"] = statistics_.tilesProcessed;
    debug["itemsProcessed"] = statistics_.itemsProcessed;
    debug["itemsSkipped"] = statistics_.itemsSkipped;
    debug["performanceGain"] = statistics_.performanceGain;
    debug["memoryUsage"] = statistics_.memoryUsage;

    return debug;
}

void LODManager::dumpConfiguration() const {
    qDebug() << "=== LOD Manager Configuration ===";
    qDebug() << "Full Detail Max Zoom:" << config_.fullDetailMaxZoom;
    qDebug() << "Medium Detail Max Zoom:" << config_.mediumDetailMaxZoom;
    qDebug() << "Ground Only Max Zoom:" << config_.groundOnlyMaxZoom;
    qDebug() << "Max Items Full Detail:" << config_.maxItemsFullDetail;
    qDebug() << "Max Items Medium Detail:" << config_.maxItemsMediumDetail;
    qDebug() << "Max Items Ground Only:" << config_.maxItemsGroundOnly;
    qDebug() << "Use Simplified Sprites:" << config_.useSimplifiedSprites;
    qDebug() << "Skip Animations:" << config_.skipAnimations;
    qDebug() << "Skip Effects:" << config_.skipEffects;
    qDebug() << "Enable Caching:" << config_.enableCaching;
    qDebug() << "Enable Batching:" << config_.enableBatching;
    qDebug() << "Enable Culling:" << config_.enableCulling;
    qDebug() << "================================";
}

// Public slots
void LODManager::onZoomChanged(double newZoom) {
    updateLODLevel(newZoom);
}

void LODManager::onMapViewChanged() {
    updateFromMapView();
}

void LODManager::onDrawingOptionsChanged() {
    // This would be called when DrawingOptions change
    // Implementation depends on how DrawingOptions are managed
}

void LODManager::updateStatisticsTimer() {
    // Manual statistics update trigger
    emit statisticsUpdated(statistics_);
}

// Private slots
void LODManager::onStatisticsTimer() {
    // Periodic statistics update
    emit statisticsUpdated(statistics_);
}

// Private methods
LODLevel LODManager::calculateLODLevel(double zoom) const {
    return getLevelForZoom(zoom);
}

bool LODManager::isItemTypeInSkipList(const QString& typeName, LODLevel level) const {
    QStringList skipList = getSkippedTypesForLOD(level);
    return skipList.contains(typeName);
}

GameSprite* LODManager::createSimplifiedSprite(const GameSprite* originalSprite, LODLevel level) const {
    if (!originalSprite || !config_.useSimplifiedSprites) {
        return nullptr;
    }

    // Create a simplified version of the sprite
    // This is a placeholder implementation - actual sprite simplification
    // would depend on the GameSprite implementation
    GameSprite* simplified = new GameSprite(*originalSprite);

    // Apply simplifications based on LOD level
    switch (level) {
        case LODLevel::MEDIUM_DETAIL:
            // Reduce detail but keep recognizable
            break;
        case LODLevel::GROUND_ONLY:
            // Minimal detail
            break;
        default:
            break;
    }

    return simplified;
}

void LODManager::cacheSimplifiedSprite(const GameSprite* original, GameSprite* simplified, LODLevel level) const {
    if (!original || !simplified || !config_.enableCaching) {
        return;
    }

    QMutexLocker locker(&cacheMutex_);

    // Check cache size limit
    if (simplifiedSpriteCache_.size() >= MAX_CACHE_SIZE) {
        // Remove oldest entries (simple FIFO for now)
        auto it = simplifiedSpriteCache_.begin();
        if (it != simplifiedSpriteCache_.end()) {
            // Delete cached sprites
            for (auto& levelMap : it.value()) {
                delete levelMap;
            }
            simplifiedSpriteCache_.erase(it);
        }
    }

    simplifiedSpriteCache_[original][level] = simplified;
}

GameSprite* LODManager::getCachedSimplifiedSprite(const GameSprite* original, LODLevel level) const {
    if (!original || !config_.enableCaching) {
        return nullptr;
    }

    QMutexLocker locker(&cacheMutex_);

    auto spriteIt = simplifiedSpriteCache_.find(original);
    if (spriteIt != simplifiedSpriteCache_.end()) {
        auto levelIt = spriteIt.value().find(level);
        if (levelIt != spriteIt.value().end()) {
            // Cache hit
            const_cast<LODManager*>(this)->statistics_.cacheHits++;
            return levelIt.value();
        }
    }

    // Cache miss
    const_cast<LODManager*>(this)->statistics_.cacheMisses++;
    return nullptr;
}

void LODManager::optimizeRenderingForLOD(LODLevel level) {
    // Apply rendering optimizations based on LOD level
    switch (level) {
        case LODLevel::FULL_DETAIL:
            // No optimizations needed
            break;

        case LODLevel::MEDIUM_DETAIL:
            // Enable some optimizations
            updateRenderingHints(level);
            break;

        case LODLevel::GROUND_ONLY:
        case LODLevel::MINIMAL:
            // Maximum optimizations
            updateRenderingHints(level);
            break;
    }
}

void LODManager::updateRenderingHints(LODLevel level) {
    // Update rendering hints for Qt painter
    // This would integrate with the rendering system
    Q_UNUSED(level)

    // Placeholder for rendering hint updates
    // Actual implementation would depend on the rendering pipeline
}

void LODManager::clearCaches() {
    QMutexLocker locker(&cacheMutex_);

    // Delete all cached sprites
    for (auto& spriteMap : simplifiedSpriteCache_) {
        for (auto& sprite : spriteMap) {
            delete sprite;
        }
    }

    simplifiedSpriteCache_.clear();
    itemTypeRenderCache_.clear();
    skipListCache_.clear();
}

// LODRenderer Implementation
LODRenderer::LODRenderer(LODManager* lodManager, QObject* parent)
    : QObject(parent)
    , lodManager_(lodManager)
    , batchingEnabled_(false)
{
    if (!lodManager_) {
        qWarning() << "LODRenderer created with null LODManager";
    }
}

LODRenderer::~LODRenderer() {
    endBatchRendering();
}

void LODRenderer::renderTileWithLOD(QPainter* painter, const QRect& tileRect, const QList<Item*>& items, const DrawingOptions& options) {
    if (!painter || !lodManager_) {
        return;
    }

    // Get current LOD level
    LODLevel currentLevel = lodManager_->getCurrentLODLevel();

    // Filter items based on LOD level
    QList<Item*> filteredItems = lodManager_->filterItemsByLOD(items, currentLevel);

    // Render filtered items
    for (Item* item : filteredItems) {
        if (item) {
            renderItemWithLOD(painter, tileRect, item, currentLevel, options);
        }
    }

    // Update statistics
    lodManager_->updateStatistics(1, filteredItems.size(), items.size() - filteredItems.size(), 0.0);
}

void LODRenderer::renderItemWithLOD(QPainter* painter, const QRect& itemRect, Item* item, LODLevel level, const DrawingOptions& options) {
    if (!painter || !item || !lodManager_) {
        return;
    }

    // Check if item should be rendered at this LOD level
    if (!lodManager_->shouldRenderItem(item, level)) {
        return;
    }

    // Get item sprite
    GameSprite* sprite = item->getSprite();
    if (!sprite) {
        return;
    }

    // Use simplified sprite if appropriate
    if (lodManager_->shouldUseSimplifiedSprite(item, level)) {
        GameSprite* simplifiedSprite = lodManager_->getCachedSimplifiedSprite(sprite, level);
        if (!simplifiedSprite) {
            simplifiedSprite = lodManager_->createSimplifiedSprite(sprite, level);
            if (simplifiedSprite) {
                lodManager_->cacheSimplifiedSprite(sprite, simplifiedSprite, level);
            }
        }
        if (simplifiedSprite) {
            sprite = simplifiedSprite;
        }
    }

    // Render sprite with LOD considerations
    renderSpriteWithLOD(painter, itemRect, sprite, level, options);
}

void LODRenderer::renderSpriteWithLOD(QPainter* painter, const QRect& spriteRect, GameSprite* sprite, LODLevel level, const DrawingOptions& options) {
    if (!painter || !sprite) {
        return;
    }

    // Apply LOD-specific rendering settings
    QPainter::RenderHints oldHints = painter->renderHints();

    switch (level) {
        case LODLevel::FULL_DETAIL:
            // High quality rendering
            painter->setRenderHint(QPainter::Antialiasing, true);
            painter->setRenderHint(QPainter::SmoothPixmapTransform, true);
            break;

        case LODLevel::MEDIUM_DETAIL:
            // Balanced quality/performance
            painter->setRenderHint(QPainter::Antialiasing, false);
            painter->setRenderHint(QPainter::SmoothPixmapTransform, false);
            break;

        case LODLevel::GROUND_ONLY:
        case LODLevel::MINIMAL:
            // Performance optimized
            painter->setRenderHint(QPainter::Antialiasing, false);
            painter->setRenderHint(QPainter::SmoothPixmapTransform, false);
            break;
    }

    // Render the sprite
    // This is a placeholder - actual sprite rendering would depend on GameSprite implementation
    sprite->draw(painter, spriteRect, options);

    // Restore original render hints
    painter->setRenderHints(oldHints);
}

void LODRenderer::beginBatchRendering(LODLevel level) {
    batchingEnabled_ = true;
    currentBatchLevel_ = level;
    batchItems_.clear();
}

void LODRenderer::addToBatch(const QRect& rect, Item* item) {
    if (!batchingEnabled_ || !item) {
        return;
    }

    BatchItem batchItem;
    batchItem.rect = rect;
    batchItem.item = item;
    batchItems_.append(batchItem);
}

void LODRenderer::renderBatch(QPainter* painter, const DrawingOptions& options) {
    if (!batchingEnabled_ || !painter || batchItems_.isEmpty()) {
        return;
    }

    // Sort batch items for optimal rendering order
    std::sort(batchItems_.begin(), batchItems_.end(), [](const BatchItem& a, const BatchItem& b) {
        // Sort by Y position first, then X position
        if (a.rect.y() != b.rect.y()) {
            return a.rect.y() < b.rect.y();
        }
        return a.rect.x() < b.rect.x();
    });

    // Render all items in batch
    for (const BatchItem& batchItem : batchItems_) {
        renderItemWithLOD(painter, batchItem.rect, batchItem.item, currentBatchLevel_, options);
    }

    // Update statistics
    if (lodManager_) {
        lodManager_->updateStatistics(0, batchItems_.size(), 0, 0.0);
    }
}

void LODRenderer::endBatchRendering() {
    batchingEnabled_ = false;
    batchItems_.clear();
}
