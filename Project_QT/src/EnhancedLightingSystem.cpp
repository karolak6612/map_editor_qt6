#include "EnhancedLightingSystem.h"
#include "DrawingOptions.h"
#include "MapView.h"
#include "Item.h"
#include "GameSprite.h"
#include "Tile.h"
#include "Map.h"
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QTimer>
#include <QMutexLocker>
#include <QPainter>
#include <QRadialGradient>
#include <QLinearGradient>
#include <QApplication>
#include <qmath.h>

// Static constants for lighting calculations (matching wxwidgets)
const double EnhancedLightingSystem::DEFAULT_LIGHT_RADIUS = 5.0;
const double EnhancedLightingSystem::DEFAULT_FALLOFF_FACTOR = 2.0;
const int EnhancedLightingSystem::ANIMATION_UPDATE_INTERVAL = 16; // ~60 FPS
const int EnhancedLightingSystem::STATISTICS_UPDATE_INTERVAL = 1000; // 1 second
const int EnhancedLightingSystem::MAX_CACHE_SIZE = 1000;
const double EnhancedLightingSystem::MIN_LIGHT_INTENSITY = 0.01;

// LightSource Implementation
double LightSource::getEffectiveIntensity(double time) const {
    if (!enabled || intensity == 0) return 0.0;
    
    double baseIntensity = intensity / 255.0; // Convert to 0-1 range
    
    if (dynamic && animationType != LightAnimationType::NONE) {
        switch (animationType) {
            case LightAnimationType::PULSE:
                return baseIntensity * (0.5 + 0.5 * qSin(time * animationSpeed));
            case LightAnimationType::FLICKER:
                return baseIntensity * (0.8 + 0.2 * (qrand() % 100) / 100.0);
            case LightAnimationType::FADE:
                return baseIntensity * qMax(0.0, 1.0 - time * animationSpeed);
            default:
                break;
        }
    }
    
    return baseIntensity;
}

QColor LightSource::getEffectiveColor(double time) const {
    Q_UNUSED(time)
    return rgbColor;
}

void LightSource::updateBoundingRect() {
    double effectiveRadius = radius * (intensity / 255.0);
    boundingRect = QRectF(position.x() - effectiveRadius, position.y() - effectiveRadius,
                         effectiveRadius * 2, effectiveRadius * 2);
}

QString LightSource::toString() const {
    return QString("Light[pos=%1,%2 floor=%3 intensity=%4 color=%5 radius=%6]")
           .arg(position.x()).arg(position.y()).arg(floor)
           .arg(intensity).arg(color).arg(radius);
}

// EnhancedLightingSystem Implementation
EnhancedLightingSystem::EnhancedLightingSystem(QObject* parent)
    : QObject(parent)
    , mapView_(nullptr)
    , lastAnimationTime_(0.0)
{
    // Initialize configuration with wxwidgets-compatible defaults
    resetToDefaults();
    
    // Initialize statistics
    resetStatistics();
    
    // Setup timers
    statisticsTimer_ = new QTimer(this);
    statisticsTimer_->setInterval(STATISTICS_UPDATE_INTERVAL);
    connect(statisticsTimer_, &QTimer::timeout, this, &EnhancedLightingSystem::onStatisticsTimer);
    statisticsTimer_->start();
    
    animationTimer_ = new QTimer(this);
    animationTimer_->setInterval(ANIMATION_UPDATE_INTERVAL);
    connect(animationTimer_, &QTimer::timeout, this, &EnhancedLightingSystem::onAnimationTimer);
    
    if (globalConfig_.enableLightAnimation) {
        animationTimer_->start();
    }
}

EnhancedLightingSystem::~EnhancedLightingSystem() {
    clearLightCache();
}

void EnhancedLightingSystem::setGlobalLightingConfig(const GlobalLightingConfig& config) {
    GlobalLightingConfig oldConfig = globalConfig_;
    globalConfig_ = config;
    
    // Clear caches when configuration changes
    clearLightCache();
    
    // Update animation timer
    if (globalConfig_.enableLightAnimation && !animationTimer_->isActive()) {
        animationTimer_->start();
    } else if (!globalConfig_.enableLightAnimation && animationTimer_->isActive()) {
        animationTimer_->stop();
    }
    
    emit globalLightingChanged();
}

void EnhancedLightingSystem::resetToDefaults() {
    globalConfig_ = GlobalLightingConfig();
    
    // Set wxwidgets-compatible defaults (matching light_drawer.cpp)
    globalConfig_.globalLightColor = QColor(50, 50, 50, 255);  // Exact wxwidgets global_color
    globalConfig_.globalLightIntensity = 140;
    globalConfig_.enableGlobalLighting = true;
    
    // Floor shading (matching wxwidgets behavior)
    globalConfig_.enableFloorShading = true;
    globalConfig_.floorShadingIntensity = 128;
    globalConfig_.floorShadingColor = QColor(0, 0, 0, 128);
    
    // Light calculation (matching wxwidgets algorithm)
    globalConfig_.maxLightRadius = 10.0;  // MaxLightIntensity from wxwidgets
    globalConfig_.lightFalloffFactor = 2.0;
    globalConfig_.enableLightBlending = true;
    globalConfig_.enableLightCaching = true;
    
    // Performance settings
    globalConfig_.maxLightsPerTile = 10;
    globalConfig_.minLightIntensity = 0.01;
    globalConfig_.enableLightCulling = true;
    globalConfig_.enableBatchRendering = true;
    globalConfig_.enableLightAnimation = false;
    
    emit globalLightingChanged();
}

void EnhancedLightingSystem::loadConfiguration(const QString& filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Failed to load lighting configuration from" << filePath;
        return;
    }
    
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    QJsonObject obj = doc.object();
    
    // Load global lighting settings
    QJsonObject globalObj = obj.value("global").toObject();
    globalConfig_.globalLightColor = QColor(globalObj.value("lightColor").toString());
    globalConfig_.globalLightIntensity = globalObj.value("lightIntensity").toInt(140);
    globalConfig_.enableGlobalLighting = globalObj.value("enableGlobalLighting").toBool(true);
    
    // Load floor shading settings
    QJsonObject floorObj = obj.value("floorShading").toObject();
    globalConfig_.enableFloorShading = floorObj.value("enableFloorShading").toBool(true);
    globalConfig_.floorShadingIntensity = floorObj.value("shadingIntensity").toInt(128);
    globalConfig_.floorShadingColor = QColor(floorObj.value("shadingColor").toString());
    
    // Load calculation settings
    QJsonObject calcObj = obj.value("calculation").toObject();
    globalConfig_.maxLightRadius = calcObj.value("maxLightRadius").toDouble(10.0);
    globalConfig_.lightFalloffFactor = calcObj.value("lightFalloffFactor").toDouble(2.0);
    globalConfig_.enableLightBlending = calcObj.value("enableLightBlending").toBool(true);
    globalConfig_.enableLightCaching = calcObj.value("enableLightCaching").toBool(true);
    
    // Load performance settings
    QJsonObject perfObj = obj.value("performance").toObject();
    globalConfig_.maxLightsPerTile = perfObj.value("maxLightsPerTile").toInt(10);
    globalConfig_.minLightIntensity = perfObj.value("minLightIntensity").toDouble(0.01);
    globalConfig_.enableLightCulling = perfObj.value("enableLightCulling").toBool(true);
    globalConfig_.enableBatchRendering = perfObj.value("enableBatchRendering").toBool(true);
    globalConfig_.enableLightAnimation = perfObj.value("enableLightAnimation").toBool(false);
    
    // Load light sources
    QJsonArray lightsArray = obj.value("lightSources").toArray();
    lightSources_.clear();
    for (const QJsonValue& lightValue : lightsArray) {
        QJsonObject lightObj = lightValue.toObject();
        
        LightSource light;
        light.position = QPoint(lightObj.value("x").toInt(), lightObj.value("y").toInt());
        light.floor = lightObj.value("floor").toInt(7);
        light.intensity = lightObj.value("intensity").toInt(0);
        light.color = lightObj.value("color").toInt(215);
        light.rgbColor = QColor(lightObj.value("rgbColor").toString());
        light.radius = lightObj.value("radius").toDouble(1.0);
        light.falloff = lightObj.value("falloff").toDouble(1.0);
        light.enabled = lightObj.value("enabled").toBool(true);
        light.dynamic = lightObj.value("dynamic").toBool(false);
        
        if (light.isValid()) {
            lightSources_.append(light);
        }
    }
    
    // Rebuild spatial index
    rebuildSpatialIndex();
    
    emit globalLightingChanged();
}

void EnhancedLightingSystem::saveConfiguration(const QString& filePath) const {
    QJsonObject obj;
    
    // Save global lighting settings
    QJsonObject globalObj;
    globalObj["lightColor"] = globalConfig_.globalLightColor.name();
    globalObj["lightIntensity"] = globalConfig_.globalLightIntensity;
    globalObj["enableGlobalLighting"] = globalConfig_.enableGlobalLighting;
    obj["global"] = globalObj;
    
    // Save floor shading settings
    QJsonObject floorObj;
    floorObj["enableFloorShading"] = globalConfig_.enableFloorShading;
    floorObj["shadingIntensity"] = globalConfig_.floorShadingIntensity;
    floorObj["shadingColor"] = globalConfig_.floorShadingColor.name();
    obj["floorShading"] = floorObj;
    
    // Save calculation settings
    QJsonObject calcObj;
    calcObj["maxLightRadius"] = globalConfig_.maxLightRadius;
    calcObj["lightFalloffFactor"] = globalConfig_.lightFalloffFactor;
    calcObj["enableLightBlending"] = globalConfig_.enableLightBlending;
    calcObj["enableLightCaching"] = globalConfig_.enableLightCaching;
    obj["calculation"] = calcObj;
    
    // Save performance settings
    QJsonObject perfObj;
    perfObj["maxLightsPerTile"] = globalConfig_.maxLightsPerTile;
    perfObj["minLightIntensity"] = globalConfig_.minLightIntensity;
    perfObj["enableLightCulling"] = globalConfig_.enableLightCulling;
    perfObj["enableBatchRendering"] = globalConfig_.enableBatchRendering;
    perfObj["enableLightAnimation"] = globalConfig_.enableLightAnimation;
    obj["performance"] = perfObj;
    
    // Save light sources
    QJsonArray lightsArray;
    for (const LightSource& light : lightSources_) {
        QJsonObject lightObj;
        lightObj["x"] = light.position.x();
        lightObj["y"] = light.position.y();
        lightObj["floor"] = light.floor;
        lightObj["intensity"] = light.intensity;
        lightObj["color"] = light.color;
        lightObj["rgbColor"] = light.rgbColor.name();
        lightObj["radius"] = light.radius;
        lightObj["falloff"] = light.falloff;
        lightObj["enabled"] = light.enabled;
        lightObj["dynamic"] = light.dynamic;
        lightsArray.append(lightObj);
    }
    obj["lightSources"] = lightsArray;
    
    QJsonDocument doc(obj);
    
    QFile file(filePath);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(doc.toJson());
    } else {
        qWarning() << "Failed to save lighting configuration to" << filePath;
    }
}

void EnhancedLightingSystem::addLightSource(const LightSource& light) {
    if (!light.isValid()) {
        qWarning() << "EnhancedLightingSystem::addLightSource: invalid light source";
        return;
    }
    
    // Check for existing light at same position
    removeLightSource(light.position, light.floor);
    
    // Add new light
    LightSource newLight = light;
    newLight.updateBoundingRect();
    lightSources_.append(newLight);
    
    // Update spatial index
    updateSpatialIndex(newLight);
    
    // Clear affected cache entries
    clearLightCacheForArea(newLight.boundingRect.toRect());
    
    emit lightSourceAdded(newLight);
}

void EnhancedLightingSystem::removeLightSource(const QPoint& position, int floor) {
    for (int i = lightSources_.size() - 1; i >= 0; --i) {
        const LightSource& light = lightSources_[i];
        if (light.position == position && light.floor == floor) {
            // Clear affected cache entries
            clearLightCacheForArea(light.boundingRect.toRect());
            
            // Remove from spatial index
            removeSpatialIndexEntry(light);
            
            // Remove light
            lightSources_.removeAt(i);
            
            emit lightSourceRemoved(position, floor);
            break;
        }
    }
}

void EnhancedLightingSystem::updateLightSource(const LightSource& light) {
    for (int i = 0; i < lightSources_.size(); ++i) {
        LightSource& existingLight = lightSources_[i];
        if (existingLight.position == light.position && existingLight.floor == light.floor) {
            // Clear affected cache entries for old light
            clearLightCacheForArea(existingLight.boundingRect.toRect());

            // Update light
            existingLight = light;
            existingLight.updateBoundingRect();

            // Update spatial index
            updateSpatialIndex(existingLight);

            // Clear affected cache entries for new light
            clearLightCacheForArea(existingLight.boundingRect.toRect());

            emit lightSourceUpdated(existingLight);
            return;
        }
    }

    // Light not found, add it
    addLightSource(light);
}

void EnhancedLightingSystem::clearLightSources() {
    lightSources_.clear();
    spatialIndex_.clear();
    clearLightCache();
}

QList<LightSource> EnhancedLightingSystem::getLightSourcesInArea(const QRect& area, int floor) const {
    QList<LightSource> result;

    for (const LightSource& light : lightSources_) {
        if (light.floor == floor && light.boundingRect.intersects(QRectF(area))) {
            result.append(light);
        }
    }

    return result;
}

LightSource* EnhancedLightingSystem::getLightSourceAt(const QPoint& position, int floor) {
    for (LightSource& light : lightSources_) {
        if (light.position == position && light.floor == floor) {
            return &light;
        }
    }
    return nullptr;
}

// Core lighting calculations (matching wxwidgets algorithm)
double EnhancedLightingSystem::calculateLightIntensityAt(const QPoint& position, int floor) const {
    if (!globalConfig_.enableGlobalLighting) {
        return 1.0; // Full brightness if lighting disabled
    }

    // Start with global light intensity
    double totalIntensity = globalConfig_.globalLightIntensity / 255.0;

    // Get lights affecting this position
    QList<LightSource> affectingLights = getLightsAffectingPosition(position, floor);

    // Calculate contribution from each light
    for (const LightSource& light : affectingLights) {
        double contribution = calculateLightContribution(light, position, floor);
        if (globalConfig_.enableLightBlending) {
            // Additive blending (matching wxwidgets)
            totalIntensity = qMin(1.0, totalIntensity + contribution);
        } else {
            // Maximum blending
            totalIntensity = qMax(totalIntensity, contribution);
        }
    }

    return qBound(0.0, totalIntensity, 1.0);
}

QColor EnhancedLightingSystem::calculateLightColorAt(const QPoint& position, int floor) const {
    if (!globalConfig_.enableGlobalLighting) {
        return Qt::white; // No color modification if lighting disabled
    }

    // Start with global light color
    QColor resultColor = globalConfig_.globalLightColor;
    double totalIntensity = globalConfig_.globalLightIntensity / 255.0;

    // Get lights affecting this position
    QList<LightSource> affectingLights = getLightsAffectingPosition(position, floor);

    // Blend colors from each light
    for (const LightSource& light : affectingLights) {
        double contribution = calculateLightContribution(light, position, floor);
        if (contribution > globalConfig_.minLightIntensity) {
            QColor lightColor = light.getEffectiveColor(lastAnimationTime_);
            resultColor = blendLightColor(resultColor, lightColor, contribution);
            totalIntensity += contribution;
        }
    }

    // Apply total intensity to result color
    resultColor.setAlphaF(qBound(0.0, totalIntensity, 1.0));

    return resultColor;
}

// Rendering methods
void EnhancedLightingSystem::renderLighting(QPainter* painter, const QRect& viewRect, int floor, const DrawingOptions& options) {
    if (!painter || !globalConfig_.enableGlobalLighting || !options.showLights) {
        return;
    }

    painter->save();

    // Render global lighting base
    renderGlobalLighting(painter, viewRect, floor, options);

    // Render floor shading if enabled
    if (globalConfig_.enableFloorShading) {
        renderFloorShading(painter, viewRect, floor, options);
    }

    // Render individual light sources
    renderLightSources(painter, viewRect, floor, options);

    painter->restore();

    // Update statistics
    QList<LightSource> visibleLights = getLightSourcesInArea(viewRect, floor);
    updateStatistics(lightSources_.size(), visibleLights.size(), 0.0, 0.0);
}

void EnhancedLightingSystem::renderGlobalLighting(QPainter* painter, const QRect& viewRect, int floor, const DrawingOptions& options) {
    Q_UNUSED(floor)
    Q_UNUSED(options)

    if (!globalConfig_.enableGlobalLighting) {
        return;
    }

    // Create global lighting overlay
    QColor globalColor = globalConfig_.globalLightColor;
    globalColor.setAlpha(globalConfig_.globalLightIntensity);

    painter->setCompositionMode(QPainter::CompositionMode_Multiply);
    painter->fillRect(viewRect, globalColor);
}

void EnhancedLightingSystem::renderLightSources(QPainter* painter, const QRect& viewRect, int floor, const DrawingOptions& options) {
    // Get visible lights for this floor
    QList<LightSource> visibleLights;
    for (const LightSource& light : lightSources_) {
        if (light.floor == floor && isLightVisible(light, viewRect, floor)) {
            visibleLights.append(light);
        }
    }

    if (visibleLights.isEmpty()) {
        return;
    }

    // Render lights
    if (globalConfig_.enableBatchRendering) {
        batchLightRendering(painter, visibleLights, viewRect, options);
    } else {
        for (const LightSource& light : visibleLights) {
            renderLightEffect(painter, light, viewRect, options);
        }
    }
}

void EnhancedLightingSystem::renderFloorShading(QPainter* painter, const QRect& viewRect, int floor, const DrawingOptions& options) {
    Q_UNUSED(options)

    if (!globalConfig_.enableFloorShading) {
        return;
    }

    // Apply floor-based shading (darker for lower floors)
    int floorDifference = qAbs(floor - 7); // Assuming ground floor is 7
    if (floorDifference > 0) {
        double shadingFactor = qMin(1.0, floorDifference * 0.1); // 10% darker per floor
        QColor shadingColor = globalConfig_.floorShadingColor;
        shadingColor.setAlphaF(shadingFactor * (globalConfig_.floorShadingIntensity / 255.0));

        painter->setCompositionMode(QPainter::CompositionMode_Multiply);
        painter->fillRect(viewRect, shadingColor);
    }
}

void EnhancedLightingSystem::renderLightEffect(QPainter* painter, const LightSource& light, const QRect& viewRect, const DrawingOptions& options) {
    Q_UNUSED(options)

    if (!light.isValid()) {
        return;
    }

    double effectiveIntensity = light.getEffectiveIntensity(lastAnimationTime_);
    if (effectiveIntensity < globalConfig_.minLightIntensity) {
        return;
    }

    QColor lightColor = light.getEffectiveColor(lastAnimationTime_);
    lightColor.setAlphaF(effectiveIntensity);

    // Calculate light radius in screen coordinates
    double screenRadius = light.radius * 32; // Assuming 32 pixels per tile

    // Create radial gradient for light effect
    QRadialGradient gradient(light.position * 32, screenRadius);
    gradient.setColorAt(0.0, lightColor);
    gradient.setColorAt(1.0, QColor(lightColor.red(), lightColor.green(), lightColor.blue(), 0));

    // Render light effect
    painter->save();
    painter->setCompositionMode(QPainter::CompositionMode_Screen);
    painter->setBrush(QBrush(gradient));
    painter->setPen(Qt::NoPen);
    painter->drawEllipse(QPointF(light.position * 32), screenRadius, screenRadius);
    painter->restore();
}

// Private helper methods
QList<LightSource> EnhancedLightingSystem::getLightsAffectingPosition(const QPoint& position, int floor) const {
    QList<LightSource> result;

    for (const LightSource& light : lightSources_) {
        if (light.floor == floor && light.isValid()) {
            // Calculate distance
            double dx = position.x() - light.position.x();
            double dy = position.y() - light.position.y();
            double distance = qSqrt(dx * dx + dy * dy);

            // Check if position is within light radius
            if (distance <= light.radius) {
                result.append(light);
            }
        }
    }

    return result;
}

double EnhancedLightingSystem::calculateLightContribution(const LightSource& light, const QPoint& position, int floor) const {
    if (light.floor != floor || !light.isValid()) {
        return 0.0;
    }

    // Calculate distance (matching wxwidgets algorithm)
    double dx = position.x() - light.position.x();
    double dy = position.y() - light.position.y();
    double distance = qSqrt(dx * dx + dy * dy);

    // Check maximum range (matching wxwidgets MaxLightIntensity)
    if (distance > globalConfig_.maxLightRadius) {
        return 0.0;
    }

    // Calculate intensity using wxwidgets algorithm
    // intensity = (-distance + light.intensity) * 0.2f
    double baseIntensity = light.getEffectiveIntensity(lastAnimationTime_);
    double intensity = (-distance + light.intensity) * 0.2;

    if (intensity < globalConfig_.minLightIntensity) {
        return 0.0;
    }

    // Apply falloff
    intensity *= baseIntensity;
    intensity /= qPow(distance + 1.0, globalConfig_.lightFalloffFactor);

    return qBound(0.0, intensity, 1.0);
}

QColor EnhancedLightingSystem::blendLightColor(const QColor& baseColor, const QColor& lightColor, double intensity) const {
    if (intensity <= 0.0) {
        return baseColor;
    }

    // Additive color blending
    int red = qMin(255, baseColor.red() + static_cast<int>(lightColor.red() * intensity));
    int green = qMin(255, baseColor.green() + static_cast<int>(lightColor.green() * intensity));
    int blue = qMin(255, baseColor.blue() + static_cast<int>(lightColor.blue() * intensity));

    return QColor(red, green, blue, baseColor.alpha());
}

bool EnhancedLightingSystem::isLightVisible(const LightSource& light, const QRect& viewRect, int floor) const {
    if (light.floor != floor || !light.isValid()) {
        return false;
    }

    // Check if light's bounding rect intersects with view rect
    return light.boundingRect.intersects(QRectF(viewRect));
}

// Utility methods
QColor EnhancedLightingSystem::colorFromEightBit(quint8 colorIndex) const {
    // Convert 8-bit color index to RGB (matching wxwidgets colorFromEightBit)
    // This is a simplified color palette - actual implementation would use the game's color palette
    static const QColor palette[256] = {
        QColor(0, 0, 0),       // 0: Black
        QColor(255, 255, 255), // 1: White
        QColor(255, 0, 0),     // 2: Red
        QColor(0, 255, 0),     // 3: Green
        QColor(0, 0, 255),     // 4: Blue
        QColor(255, 255, 0),   // 5: Yellow
        QColor(255, 0, 255),   // 6: Magenta
        QColor(0, 255, 255),   // 7: Cyan
        // ... more colors would be defined here
    };

    if (colorIndex < 256) {
        return palette[colorIndex];
    }

    // Default to white for invalid indices
    return QColor(215, 215, 215); // Default light color from wxwidgets
}

quint8 EnhancedLightingSystem::colorToEightBit(const QColor& color) const {
    // Convert RGB color to 8-bit color index (reverse of colorFromEightBit)
    // This is a simplified implementation
    if (color == Qt::black) return 0;
    if (color == Qt::white) return 1;
    if (color == Qt::red) return 2;
    if (color == Qt::green) return 3;
    if (color == Qt::blue) return 4;
    if (color == Qt::yellow) return 5;
    if (color == Qt::magenta) return 6;
    if (color == Qt::cyan) return 7;

    // Default to light gray
    return 215;
}

QString EnhancedLightingSystem::getLightingDiagnosticInfo() const {
    QString info;
    info += QString("Enhanced Lighting System Diagnostics:\n");
    info += QString("Global Lighting: %1\n").arg(globalConfig_.enableGlobalLighting ? "Enabled" : "Disabled");
    info += QString("Global Light Color: %1\n").arg(globalConfig_.globalLightColor.name());
    info += QString("Global Light Intensity: %1\n").arg(globalConfig_.globalLightIntensity);
    info += QString("Floor Shading: %1\n").arg(globalConfig_.enableFloorShading ? "Enabled" : "Disabled");
    info += QString("Light Sources: %1\n").arg(lightSources_.size());
    info += QString("Cache Size: %1\n").arg(lightColorCache_.size());
    info += QString("Lights Processed: %1\n").arg(statistics_.lightsProcessed);
    info += QString("Lights Rendered: %1\n").arg(statistics_.lightsRendered);

    return info;
}

QVariantMap EnhancedLightingSystem::getLightingDebugInfo() const {
    QVariantMap debug;

    debug["enableGlobalLighting"] = globalConfig_.enableGlobalLighting;
    debug["globalLightColor"] = globalConfig_.globalLightColor.name();
    debug["globalLightIntensity"] = globalConfig_.globalLightIntensity;
    debug["enableFloorShading"] = globalConfig_.enableFloorShading;
    debug["lightSourceCount"] = lightSources_.size();
    debug["cacheSize"] = lightColorCache_.size();
    debug["lightsProcessed"] = statistics_.lightsProcessed;
    debug["lightsRendered"] = statistics_.lightsRendered;
    debug["memoryUsage"] = statistics_.memoryUsage;

    return debug;
}

// Public slots
void EnhancedLightingSystem::onMapChanged() {
    clearLightCache();
}

void EnhancedLightingSystem::onDrawingOptionsChanged() {
    // This would be called when DrawingOptions change
    // Implementation depends on how DrawingOptions are managed
}

void EnhancedLightingSystem::onAnimationTimer() {
    if (globalConfig_.enableLightAnimation) {
        double deltaTime = ANIMATION_UPDATE_INTERVAL / 1000.0; // Convert to seconds
        lastAnimationTime_ += deltaTime;
        emit lightAnimationUpdated(deltaTime);
    }
}

void EnhancedLightingSystem::updateLightingSystem() {
    clearLightCache();
    rebuildSpatialIndex();
    emit globalLightingChanged();
}

// Private slots
void EnhancedLightingSystem::onStatisticsTimer() {
    emit lightingStatisticsUpdated(statistics_);
}

// Cache management
void EnhancedLightingSystem::clearLightCache() {
    QMutexLocker locker(&cacheMutex_);
    lightColorCache_.clear();
    lightIntensityCache_.clear();
}

void EnhancedLightingSystem::clearLightCacheForArea(const QRect& area) {
    QMutexLocker locker(&cacheMutex_);

    // Remove cache entries that might be affected by changes in this area
    auto colorIt = lightColorCache_.begin();
    while (colorIt != lightColorCache_.end()) {
        // Parse position from cache key (simplified)
        if (colorIt.key().contains(QString("%1_%2").arg(area.x()).arg(area.y()))) {
            colorIt = lightColorCache_.erase(colorIt);
        } else {
            ++colorIt;
        }
    }

    auto intensityIt = lightIntensityCache_.begin();
    while (intensityIt != lightIntensityCache_.end()) {
        // Parse position from cache key (simplified)
        if (intensityIt.key().contains(QString("%1_%2").arg(area.x()).arg(area.y()))) {
            intensityIt = lightIntensityCache_.erase(intensityIt);
        } else {
            ++intensityIt;
        }
    }
}

// Optimization methods
void EnhancedLightingSystem::cullInvisibleLights(const QRect& viewRect, int floor) {
    Q_UNUSED(viewRect)
    Q_UNUSED(floor)
    // Placeholder for light culling optimization
}

void EnhancedLightingSystem::batchLightRendering(QPainter* painter, const QList<LightSource>& lights, const QRect& viewRect, const DrawingOptions& options) {
    // Group lights by similar properties for batch rendering
    QHash<QString, QList<LightSource>> lightGroups;

    for (const LightSource& light : lights) {
        QString groupKey = QString("%1_%2_%3").arg(light.color).arg(light.radius).arg(light.intensity);
        lightGroups[groupKey].append(light);
    }

    // Render each group
    for (auto it = lightGroups.begin(); it != lightGroups.end(); ++it) {
        const QList<LightSource>& group = it.value();
        for (const LightSource& light : group) {
            renderLightEffect(painter, light, viewRect, options);
        }
    }
}

void EnhancedLightingSystem::optimizeLightCalculation(const QRect& area, int floor) {
    Q_UNUSED(area)
    Q_UNUSED(floor)
    // Placeholder for light calculation optimization
}

void EnhancedLightingSystem::updateSpatialIndex(const LightSource& light) {
    // Simple spatial indexing by position
    spatialIndex_[light.position].append(const_cast<LightSource*>(&light));
}

void EnhancedLightingSystem::removeSpatialIndexEntry(const LightSource& light) {
    auto it = spatialIndex_.find(light.position);
    if (it != spatialIndex_.end()) {
        it.value().removeAll(const_cast<LightSource*>(&light));
        if (it.value().isEmpty()) {
            spatialIndex_.erase(it);
        }
    }
}

void EnhancedLightingSystem::rebuildSpatialIndex() {
    spatialIndex_.clear();
    for (const LightSource& light : lightSources_) {
        updateSpatialIndex(light);
    }
}

// Statistics and monitoring
void EnhancedLightingSystem::resetStatistics() {
    QMutexLocker locker(&cacheMutex_);

    statistics_.lightsProcessed = 0;
    statistics_.lightsRendered = 0;
    statistics_.totalCalculationTime = 0.0;
    statistics_.totalRenderTime = 0.0;
    statistics_.averageCalculationTime = 0.0;
    statistics_.averageRenderTime = 0.0;
    statistics_.cacheHits = 0;
    statistics_.cacheMisses = 0;
    statistics_.memoryUsage = 0;
}

void EnhancedLightingSystem::updateStatistics(int lightsProcessed, int lightsRendered, double calculationTime, double renderTime) {
    QMutexLocker locker(&cacheMutex_);

    statistics_.lightsProcessed += lightsProcessed;
    statistics_.lightsRendered += lightsRendered;
    statistics_.totalCalculationTime += calculationTime;
    statistics_.totalRenderTime += renderTime;

    // Calculate averages
    if (statistics_.lightsProcessed > 0) {
        statistics_.averageCalculationTime = statistics_.totalCalculationTime / statistics_.lightsProcessed;
        statistics_.averageRenderTime = statistics_.totalRenderTime / statistics_.lightsProcessed;
    }

    // Update memory usage (approximate)
    statistics_.memoryUsage = lightSources_.size() * sizeof(LightSource) +
                             lightColorCache_.size() * sizeof(QColor) +
                             lightIntensityCache_.size() * sizeof(double);

    emit lightingStatisticsUpdated(statistics_);
}

// Integration methods
void EnhancedLightingSystem::updateFromDrawingOptions(const DrawingOptions& options) {
    bool configChanged = false;

    if (globalConfig_.enableGlobalLighting != options.showLights) {
        globalConfig_.enableGlobalLighting = options.showLights;
        configChanged = true;
    }

    if (configChanged) {
        clearLightCache();
        emit globalLightingChanged();
    }
}

void EnhancedLightingSystem::applyToDrawingOptions(DrawingOptions& options) const {
    options.showLights = globalConfig_.enableGlobalLighting;
}
