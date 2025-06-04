#include "TransparencyManager.h"
#include "DrawingOptions.h"
#include "MapView.h"
#include "Item.h"
#include "GameSprite.h"
#include "Tile.h"
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>
#include <QTimer>
#include <QMutexLocker>
#include <QPainter>
#include <QGraphicsEffect>
#include <QApplication>
#include <qmath.h>
#include <QVariant>
#include <QRect>
#include <QPoint>
#include <QColor>
#include <QPixmap>

// Static constants for transparency calculations
const double TransparencyManager::MIN_TRANSPARENCY_THRESHOLD = 0.01;
const double TransparencyManager::MAX_TRANSPARENCY_THRESHOLD = 0.99;
const int TransparencyManager::STATISTICS_UPDATE_INTERVAL = 1000; // 1 second
const int TransparencyManager::ANIMATION_UPDATE_INTERVAL = 16; // ~60 FPS

// TransparencyManager Implementation
TransparencyManager::TransparencyManager(QObject* parent)
    : QObject(parent)
    , mapView_(nullptr)
    , batchingEnabled_(false)
{
    // Initialize configuration with wxwidgets-compatible defaults
    resetToDefaults();
    
    // Initialize statistics
    resetStatistics();
    
    // Setup timers
    statisticsTimer_ = new QTimer(this);
    statisticsTimer_->setInterval(STATISTICS_UPDATE_INTERVAL);
    connect(statisticsTimer_, &QTimer::timeout, this, &TransparencyManager::onStatisticsTimer);
    statisticsTimer_->start();
    
    animationTimer_ = new QTimer(this);
    animationTimer_->setInterval(ANIMATION_UPDATE_INTERVAL);
    connect(animationTimer_, &QTimer::timeout, this, &TransparencyManager::onAnimationTimer);
    
    if (config_.enableTransparencyAnimation) {
        animationTimer_->start();
    }
}

TransparencyManager::~TransparencyManager() {
    clearTransparencyCache();
}

void TransparencyManager::setConfiguration(const TransparencyConfig& config) {
    TransparencyConfig oldConfig = config_;
    config_ = config;
    
    // Clear caches when configuration changes
    clearTransparencyCache();
    
    // Update animation timer
    if (config_.enableTransparencyAnimation && !animationTimer_->isActive()) {
        animationTimer_->start();
    } else if (!config_.enableTransparencyAnimation && animationTimer_->isActive()) {
        animationTimer_->stop();
    }
    
    emit transparencyConfigChanged();
}

void TransparencyManager::resetToDefaults() {
    config_ = TransparencyConfig();
    
    // Set wxwidgets-compatible defaults
    config_.enableTransparency = true;
    config_.mode = TransparencyMode::FLOOR_BASED;
    config_.globalTransparencyFactor = 1.0;
    
    // Floor-based transparency (matching wxwidgets transparent_floors)
    config_.enableFloorTransparency = true;
    config_.floorTransparencyFactor = 0.5;  // 50% transparency for other floors
    config_.maxFloorTransparency = 0.9;
    config_.transparentFloorRange = 3;
    config_.fadeUpperFloors = true;
    config_.fadeLowerFloors = false;
    
    // Item-based transparency (matching wxwidgets transparent_items)
    config_.enableItemTransparency = true;
    config_.itemTransparencyFactor = 0.7;   // 30% more transparent
    config_.maxItemTransparency = 0.8;
    config_.transparentItemTypes.clear();
    config_.transparentItemTypes << "decoration" << "effect";
    
    // Performance settings
    config_.enableTransparencyCaching = true;
    config_.enableTransparencyBatching = true;
    config_.enableTransparencyOptimization = true;
    config_.enableTransparencyAnimation = false;
    
    // Animation settings
    config_.animationSpeed = 1.0;
    config_.fadeAnimationDuration = 1000; // 1 second
    config_.pulseAnimationDuration = 2000; // 2 seconds
    
    emit transparencyConfigChanged();
}

void TransparencyManager::loadConfiguration(const QString& filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Failed to load transparency configuration from" << filePath;
        return;
    }
    
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    QJsonObject obj = doc.object();
    
    // Load global settings
    config_.enableTransparency = obj.value("enableTransparency").toBool(true);
    config_.mode = static_cast<TransparencyMode>(obj.value("mode").toInt(static_cast<int>(TransparencyMode::FLOOR_BASED)));
    config_.globalTransparencyFactor = obj.value("globalTransparencyFactor").toDouble(1.0);
    
    // Load floor transparency settings
    config_.enableFloorTransparency = obj.value("enableFloorTransparency").toBool(true);
    config_.floorTransparencyFactor = obj.value("floorTransparencyFactor").toDouble(0.5);
    config_.maxFloorTransparency = obj.value("maxFloorTransparency").toDouble(0.9);
    config_.transparentFloorRange = obj.value("transparentFloorRange").toInt(3);
    config_.fadeUpperFloors = obj.value("fadeUpperFloors").toBool(true);
    config_.fadeLowerFloors = obj.value("fadeLowerFloors").toBool(false);
    
    // Load item transparency settings
    config_.enableItemTransparency = obj.value("enableItemTransparency").toBool(true);
    config_.itemTransparencyFactor = obj.value("itemTransparencyFactor").toDouble(0.7);
    config_.maxItemTransparency = obj.value("maxItemTransparency").toDouble(0.8);
    
    // Load performance settings
    config_.enableTransparencyCaching = obj.value("enableTransparencyCaching").toBool(true);
    config_.enableTransparencyBatching = obj.value("enableTransparencyBatching").toBool(true);
    config_.enableTransparencyOptimization = obj.value("enableTransparencyOptimization").toBool(true);
    config_.enableTransparencyAnimation = obj.value("enableTransparencyAnimation").toBool(false);
    
    // Load animation settings
    config_.animationSpeed = obj.value("animationSpeed").toDouble(1.0);
    config_.fadeAnimationDuration = obj.value("fadeAnimationDuration").toInt(1000);
    config_.pulseAnimationDuration = obj.value("pulseAnimationDuration").toInt(2000);
    
    emit transparencyConfigChanged();
}

void TransparencyManager::saveConfiguration(const QString& filePath) const {
    QJsonObject obj;
    
    // Save global settings
    obj["enableTransparency"] = config_.enableTransparency;
    obj["mode"] = static_cast<int>(config_.mode);
    obj["globalTransparencyFactor"] = config_.globalTransparencyFactor;
    
    // Save floor transparency settings
    obj["enableFloorTransparency"] = config_.enableFloorTransparency;
    obj["floorTransparencyFactor"] = config_.floorTransparencyFactor;
    obj["maxFloorTransparency"] = config_.maxFloorTransparency;
    obj["transparentFloorRange"] = config_.transparentFloorRange;
    obj["fadeUpperFloors"] = config_.fadeUpperFloors;
    obj["fadeLowerFloors"] = config_.fadeLowerFloors;
    
    // Save item transparency settings
    obj["enableItemTransparency"] = config_.enableItemTransparency;
    obj["itemTransparencyFactor"] = config_.itemTransparencyFactor;
    obj["maxItemTransparency"] = config_.maxItemTransparency;
    
    // Save performance settings
    obj["enableTransparencyCaching"] = config_.enableTransparencyCaching;
    obj["enableTransparencyBatching"] = config_.enableTransparencyBatching;
    obj["enableTransparencyOptimization"] = config_.enableTransparencyOptimization;
    obj["enableTransparencyAnimation"] = config_.enableTransparencyAnimation;
    
    // Save animation settings
    obj["animationSpeed"] = config_.animationSpeed;
    obj["fadeAnimationDuration"] = config_.fadeAnimationDuration;
    obj["pulseAnimationDuration"] = config_.pulseAnimationDuration;
    
    QJsonDocument doc(obj);
    
    QFile file(filePath);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(doc.toJson());
    } else {
        qWarning() << "Failed to save transparency configuration to" << filePath;
    }
}

void TransparencyManager::setTransparencyMode(TransparencyMode mode) {
    if (mode != config_.mode) {
        TransparencyMode oldMode = config_.mode;
        config_.mode = mode;
        
        // Clear caches when mode changes
        clearTransparencyCache();
        
        emit transparencyModeChanged(mode, oldMode);
        emit transparencyConfigChanged();
    }
}

void TransparencyManager::setGlobalTransparencyFactor(double factor) {
    factor = qBound(0.0, factor, 1.0);
    if (qAbs(factor - config_.globalTransparencyFactor) > 0.001) {
        config_.globalTransparencyFactor = factor;
        clearTransparencyCache();
        emit transparencyConfigChanged();
    }
}

double TransparencyManager::calculateTransparency(const Item* item, const QPoint& position, int floor, int currentFloor) const {
    if (!item || !config_.enableTransparency) {
        return 1.0; // Fully opaque
    }
    
    // Calculate base transparency
    double baseTransparency = calculateBaseTransparency(item, position, floor, currentFloor);
    
    // Apply transparency mode
    double finalTransparency = applyTransparencyMode(baseTransparency, item, position, floor, currentFloor);
    
    // Apply global transparency factor
    finalTransparency *= config_.globalTransparencyFactor;
    
    // Clamp to valid range
    return clampTransparency(finalTransparency);
}

double TransparencyManager::calculateFloorTransparency(int floor, int currentFloor) const {
    if (!config_.enableFloorTransparency || floor == currentFloor) {
        return 1.0; // Fully opaque for current floor
    }
    
    int floorDifference = qAbs(floor - currentFloor);
    
    // Check if floor is within transparent range
    if (floorDifference > config_.transparentFloorRange) {
        return 0.0; // Fully transparent (invisible)
    }
    
    // Determine if we should apply transparency
    bool shouldApplyTransparency = false;
    if (floor > currentFloor && config_.fadeUpperFloors) {
        shouldApplyTransparency = true;
    } else if (floor < currentFloor && config_.fadeLowerFloors) {
        shouldApplyTransparency = true;
    }
    
    if (!shouldApplyTransparency) {
        return 1.0; // Fully opaque
    }
    
    // Calculate transparency based on floor difference (matching wxwidgets algorithm)
    double transparency = 1.0 - (floorDifference * config_.floorTransparencyFactor);
    transparency = qMax(transparency, 1.0 - config_.maxFloorTransparency);
    
    return clampTransparency(transparency);
}

double TransparencyManager::calculateItemTransparency(const Item* item) const {
    if (!item || !config_.enableItemTransparency) {
        return 1.0; // Fully opaque
    }
    
    // Ground items are usually not made transparent (matching wxwidgets)
    if (item->isGroundTile()) {
        return 1.0;
    }
    
    // Apply item transparency factor (matching wxwidgets transparent_items)
    double transparency = config_.itemTransparencyFactor;
    
    // Check if item type should be transparent
    QString itemTypeName = item->getTypeName();
    if (config_.transparentItemTypes.contains(itemTypeName)) {
        transparency *= 0.5; // Make specific types more transparent
    }
    
    return clampTransparency(transparency);
}

// Private methods implementation
double TransparencyManager::calculateBaseTransparency(const Item* item, const QPoint& position, int floor, int currentFloor) const {
    Q_UNUSED(position)

    if (!item) return 1.0;

    // Start with full opacity
    double transparency = 1.0;

    // Apply floor-based transparency first
    double floorTransparency = calculateFloorTransparency(floor, currentFloor);
    transparency *= floorTransparency;

    // Apply item-based transparency
    double itemTransparency = calculateItemTransparency(item);
    transparency *= itemTransparency;

    return transparency;
}

double TransparencyManager::applyTransparencyMode(double baseTransparency, const Item* item, const QPoint& position, int floor, int currentFloor) const {
    switch (config_.mode) {
        case TransparencyMode::NONE:
            return 1.0; // No transparency

        case TransparencyMode::FLOOR_BASED:
            // Use floor-based transparency only
            return calculateFloorTransparency(floor, currentFloor);

        case TransparencyMode::ITEM_BASED:
            // Use item-based transparency only
            return calculateItemTransparency(item);

        case TransparencyMode::DISTANCE_BASED:
            // Distance-based transparency (for future use)
            return baseTransparency;

        case TransparencyMode::CUSTOM:
            // Custom transparency rules
            return baseTransparency;
    }

    return baseTransparency;
}

double TransparencyManager::clampTransparency(double transparency) const {
    return qBound(MIN_TRANSPARENCY_THRESHOLD, transparency, MAX_TRANSPARENCY_THRESHOLD);
}

bool TransparencyManager::shouldSkipTransparentItem(double transparency) const {
    return transparency < MIN_TRANSPARENCY_THRESHOLD;
}

bool TransparencyManager::isTransparencySignificant(double transparency) const {
    return qAbs(transparency - 1.0) > MIN_TRANSPARENCY_THRESHOLD;
}

// Rendering methods
void TransparencyManager::renderWithTransparency(QPainter* painter, const QRect& rect, const Item* item, const QPoint& position, int floor, int currentFloor) {
    if (!painter || !item) return;

    double transparency = calculateTransparency(item, position, floor, currentFloor);

    // Skip if item is too transparent to be visible
    if (shouldSkipTransparentItem(transparency)) {
        return;
    }

    // Save current opacity
    qreal oldOpacity = painter->opacity();

    // Apply transparency
    painter->setOpacity(oldOpacity * transparency);

    // Render the item (this would call item->draw() or similar)
    item->draw(painter, rect, DrawingOptions()); // Placeholder - would need proper DrawingOptions

    // Restore opacity
    painter->setOpacity(oldOpacity);

    // Update statistics
    updateStatistics(1, isTransparencySignificant(transparency) ? 1 : 0,
                    isTransparencySignificant(transparency) ? 0 : 1, 0.0, 0.0);
}

void TransparencyManager::renderSpriteWithTransparency(QPainter* painter, const QRect& rect, GameSprite* sprite, double transparency) {
    if (!painter || !sprite) return;

    // Skip if too transparent
    if (shouldSkipTransparentItem(transparency)) {
        return;
    }

    // Save current opacity
    qreal oldOpacity = painter->opacity();

    // Apply transparency
    painter->setOpacity(oldOpacity * transparency);

    // Render the sprite
    sprite->draw(painter, rect, DrawingOptions()); // Placeholder

    // Restore opacity
    painter->setOpacity(oldOpacity);
}

void TransparencyManager::renderTileWithTransparency(QPainter* painter, const QRect& rect, const Tile* tile, int currentFloor) {
    if (!painter || !tile) return;

    int tileFloor = tile->z();
    double floorTransparency = calculateFloorTransparency(tileFloor, currentFloor);

    // Skip if floor is too transparent
    if (shouldSkipTransparentItem(floorTransparency)) {
        return;
    }

    // Save current opacity
    qreal oldOpacity = painter->opacity();

    // Apply floor transparency
    painter->setOpacity(oldOpacity * floorTransparency);

    // Render all items on the tile
    QList<Item*> items = tile->getItems();
    for (Item* item : items) {
        if (item) {
            double itemTransparency = calculateItemTransparency(item);
            double combinedTransparency = floorTransparency * itemTransparency;

            if (!shouldSkipTransparentItem(combinedTransparency)) {
                painter->setOpacity(oldOpacity * combinedTransparency);
                item->draw(painter, rect, DrawingOptions()); // Placeholder
            }
        }
    }

    // Restore opacity
    painter->setOpacity(oldOpacity);
}

void TransparencyManager::applyTransparencyToPixmap(QPixmap& pixmap, double transparency) {
    if (qAbs(transparency - 1.0) < MIN_TRANSPARENCY_THRESHOLD) {
        return; // No transparency to apply
    }

    // Create a new pixmap with alpha channel
    QPixmap transparentPixmap(pixmap.size());
    transparentPixmap.fill(Qt::transparent);

    QPainter painter(&transparentPixmap);
    painter.setOpacity(transparency);
    painter.drawPixmap(0, 0, pixmap);
    painter.end();

    pixmap = transparentPixmap;
}

void TransparencyManager::applyTransparencyToColor(QColor& color, double transparency) {
    int alpha = static_cast<int>(color.alpha() * transparency);
    color.setAlpha(qBound(0, alpha, 255));
}

// Integration methods
void TransparencyManager::updateFromDrawingOptions(const DrawingOptions& options) {
    // Update configuration based on DrawingOptions (wxwidgets compatibility)
    bool configChanged = false;

    if (config_.enableFloorTransparency != options.transparentFloors) {
        config_.enableFloorTransparency = options.transparentFloors;
        configChanged = true;
    }

    if (config_.enableItemTransparency != options.transparentItems) {
        config_.enableItemTransparency = options.transparentItems;
        configChanged = true;
    }

    // Update floor transparency settings based on DrawingOptions
    if (options.showHigherFloorsTransparent != config_.fadeUpperFloors) {
        config_.fadeUpperFloors = options.showHigherFloorsTransparent;
        configChanged = true;
    }

    if (options.showLowerFloorsTransparent != config_.fadeLowerFloors) {
        config_.fadeLowerFloors = options.showLowerFloorsTransparent;
        configChanged = true;
    }

    if (configChanged) {
        clearTransparencyCache();
        emit transparencyConfigChanged();
    }
}

void TransparencyManager::applyToDrawingOptions(DrawingOptions& options) const {
    // Apply transparency settings to DrawingOptions (wxwidgets compatibility)
    options.transparentFloors = config_.enableFloorTransparency;
    options.transparentItems = config_.enableItemTransparency;
    options.showHigherFloorsTransparent = config_.fadeUpperFloors;
    options.showLowerFloorsTransparent = config_.fadeLowerFloors;
}

bool TransparencyManager::shouldRenderTransparent(const Item* item, const QPoint& position, int floor, int currentFloor) const {
    double transparency = calculateTransparency(item, position, floor, currentFloor);
    return !shouldSkipTransparentItem(transparency);
}

// Statistics and monitoring
void TransparencyManager::resetStatistics() {
    QMutexLocker locker(&statisticsMutex_);

    statistics_.itemsProcessed = 0;
    statistics_.transparentItems = 0;
    statistics_.opaqueItems = 0;
    statistics_.totalCalculationTime = 0.0;
    statistics_.totalRenderTime = 0.0;
    statistics_.averageCalculationTime = 0.0;
    statistics_.averageRenderTime = 0.0;
    statistics_.cacheHits = 0;
    statistics_.cacheMisses = 0;
    statistics_.memoryUsage = 0;
}

void TransparencyManager::updateStatistics(int itemsProcessed, int transparentItems, int opaqueItems, double calculationTime, double renderTime) {
    QMutexLocker locker(&statisticsMutex_);

    statistics_.itemsProcessed += itemsProcessed;
    statistics_.transparentItems += transparentItems;
    statistics_.opaqueItems += opaqueItems;
    statistics_.totalCalculationTime += calculationTime;
    statistics_.totalRenderTime += renderTime;

    // Calculate averages
    if (statistics_.itemsProcessed > 0) {
        statistics_.averageCalculationTime = statistics_.totalCalculationTime / statistics_.itemsProcessed;
        statistics_.averageRenderTime = statistics_.totalRenderTime / statistics_.itemsProcessed;
    }

    // Update memory usage (approximate)
    statistics_.memoryUsage = transparencyCache_.size() * sizeof(double);

    emit transparencyStatisticsUpdated(statistics_);
}

QString TransparencyManager::getTransparencyDiagnosticInfo() const {
    QString info;
    info += QString("Transparency Manager Diagnostics:\n");
    info += QString("Mode: %1\n").arg(static_cast<int>(config_.mode));
    info += QString("Global Factor: %1\n").arg(config_.globalTransparencyFactor);
    info += QString("Floor Transparency: %1\n").arg(config_.enableFloorTransparency ? "Enabled" : "Disabled");
    info += QString("Item Transparency: %1\n").arg(config_.enableItemTransparency ? "Enabled" : "Disabled");
    info += QString("Cache Size: %1\n").arg(transparencyCache_.size());
    info += QString("Items Processed: %1\n").arg(statistics_.itemsProcessed);
    info += QString("Transparent Items: %1\n").arg(statistics_.transparentItems);
    info += QString("Opaque Items: %1\n").arg(statistics_.opaqueItems);

    return info;
}

QVariantMap TransparencyManager::getTransparencyDebugInfo() const {
    QVariantMap debug;

    debug["mode"] = static_cast<int>(config_.mode);
    debug["globalTransparencyFactor"] = config_.globalTransparencyFactor;
    debug["enableFloorTransparency"] = config_.enableFloorTransparency;
    debug["enableItemTransparency"] = config_.enableItemTransparency;
    debug["cacheSize"] = transparencyCache_.size();
    debug["itemsProcessed"] = statistics_.itemsProcessed;
    debug["transparentItems"] = statistics_.transparentItems;
    debug["opaqueItems"] = statistics_.opaqueItems;
    debug["memoryUsage"] = statistics_.memoryUsage;

    return debug;
}

// Public slots
void TransparencyManager::onMapChanged() {
    clearTransparencyCache();
}

void TransparencyManager::onDrawingOptionsChanged() {
    // This would be called when DrawingOptions change
    // Implementation depends on how DrawingOptions are managed
}

void TransparencyManager::onCurrentFloorChanged(int newFloor) {
    Q_UNUSED(newFloor)
    clearTransparencyCache();
}

void TransparencyManager::onAnimationTimer() {
    if (config_.enableTransparencyAnimation) {
        double deltaTime = ANIMATION_UPDATE_INTERVAL / 1000.0; // Convert to seconds
        updateTransparencyAnimations(deltaTime);
        emit transparencyAnimationUpdated(deltaTime);
    }
}

void TransparencyManager::updateTransparencySystem() {
    clearTransparencyCache();
    emit transparencyConfigChanged();
}

// Private slots
void TransparencyManager::onStatisticsTimer() {
    emit transparencyStatisticsUpdated(statistics_);
}

// Animation and effects
void TransparencyManager::updateTransparencyAnimations(double deltaTime) {
    Q_UNUSED(deltaTime)
    // Placeholder for transparency animations
    // This would update any animated transparency effects
}

// Cache management
void TransparencyManager::clearTransparencyCache() {
    QMutexLocker locker(&cacheMutex_);
    transparencyCache_.clear();
}
