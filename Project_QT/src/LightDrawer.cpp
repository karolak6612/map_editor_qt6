// LightDrawer.cpp - Qt-based light drawing system implementation

#include "LightDrawer.h"
#include "DrawingOptions.h"
#include <QPainter>
#include <QRadialGradient>
#include <QLinearGradient>
#include <QBrush>
#include <QPen>
#include <QDebug>
#include <QtMath>
#include <QApplication>

LightDrawer::LightDrawer(QObject* parent)
    : QObject(parent)
    , globalLightColor_(50, 50, 50, 255)
    , enabled_(true)
    , fogEnabled_(false)
    , lightCacheDirty_(true)
{
    qDebug() << "LightDrawer: Qt-based light drawing system initialized";
}

LightDrawer::~LightDrawer() {
    clear();
    lightTextureCache_.clear();
    qDebug() << "LightDrawer: Destroyed";
}

// Task 76: Main drawing method (replaces OpenGL draw)
void LightDrawer::draw(QPainter* painter, int mapX, int mapY, int endX, int endY, 
                      int scrollX, int scrollY, bool fog, const DrawingOptions& options) {
    if (!painter || !enabled_ || !options.showLights) {
        return;
    }

    painter->save();

    // Calculate view rectangle
    QRectF viewRect(mapX - scrollX, mapY - scrollY, endX - mapX, endY - mapY);

    // Draw global lighting base
    drawGlobalLighting(painter, viewRect, options);

    // Draw individual light effects
    for (const Light& light : lights_) {
        if (isLightVisible(light, viewRect)) {
            drawLightEffect(painter, light, viewRect, options);
        }
    }

    // Draw fog effect if enabled
    if (fog || fogEnabled_) {
        drawFogEffect(painter, viewRect, options);
    }

    painter->restore();
    
    qDebug() << "LightDrawer::draw: Rendered" << lights_.size() << "lights in area"
             << mapX << mapY << endX << endY;
}

// Task 76: Light management
void LightDrawer::setGlobalLightColor(uint8_t color) {
    globalLightColor_ = colorFromEightBit(color);
    lightCacheDirty_ = true;
    qDebug() << "LightDrawer::setGlobalLightColor:" << globalLightColor_;
}

void LightDrawer::addLight(int mapX, int mapY, int mapZ, const SpriteLight& light) {
    // Adjust coordinates for floor offset (from wxwidgets logic)
    if (mapZ <= GroundLayer) {
        mapX -= (GroundLayer - mapZ);
        mapY -= (GroundLayer - mapZ);
    }

    // Validate coordinates
    if (mapX <= 0 || mapX >= 65536 || mapY <= 0 || mapY >= 65536) {
        return;
    }

    uint8_t intensity = qMin(light.intensity, MaxLightIntensity);

    // Check for existing light at same position with same color (optimization from wxwidgets)
    if (!lights_.isEmpty()) {
        Light& previous = lights_.last();
        if (previous.mapX == mapX && previous.mapY == mapY && previous.color == light.color) {
            previous.intensity = qMax(previous.intensity, intensity);
            return;
        }
    }

    lights_.append(Light(static_cast<uint16_t>(mapX), static_cast<uint16_t>(mapY), 
                        light.color, intensity));
    lightCacheDirty_ = true;
    
    qDebug() << "LightDrawer::addLight: Added light at" << mapX << mapY 
             << "color:" << light.color << "intensity:" << intensity;
}

void LightDrawer::clear() {
    lights_.clear();
    lightCacheDirty_ = true;
    qDebug() << "LightDrawer::clear: Cleared all lights";
}

// Task 76: Internal drawing methods
void LightDrawer::drawLightEffect(QPainter* painter, const Light& light, 
                                 const QRectF& viewRect, const DrawingOptions& options) {
    QColor lightColor = colorFromEightBit(light.color);
    
    // Calculate light position in view coordinates
    QPointF lightPos(light.mapX - viewRect.x(), light.mapY - viewRect.y());
    
    // Calculate light radius based on intensity
    qreal radius = (LightRadius * light.intensity) / MaxLightIntensity;
    
    // Create radial gradient for light effect
    QRadialGradient gradient(lightPos, radius);
    
    // Set gradient colors based on intensity
    QColor centerColor = lightColor;
    centerColor.setAlpha(light.intensity);
    
    QColor edgeColor = lightColor;
    edgeColor.setAlpha(0);
    
    gradient.setColorAt(0.0, centerColor);
    gradient.setColorAt(0.7, lightColor);
    gradient.setColorAt(1.0, edgeColor);
    
    // Draw light effect
    painter->setCompositionMode(QPainter::CompositionMode_Plus);
    painter->setBrush(QBrush(gradient));
    painter->setPen(Qt::NoPen);
    
    QRectF lightRect(lightPos.x() - radius, lightPos.y() - radius, 
                     radius * 2, radius * 2);
    painter->drawEllipse(lightRect);
    
    painter->setCompositionMode(QPainter::CompositionMode_SourceOver);
}

void LightDrawer::drawFogEffect(QPainter* painter, const QRectF& viewRect, 
                               const DrawingOptions& options) {
    if (!fogEnabled_ && !options.experimentalFog) {
        return;
    }

    // Create fog gradient
    QLinearGradient fogGradient(viewRect.topLeft(), viewRect.bottomRight());
    
    QColor fogColor(100, 100, 120, 80);
    QColor clearColor(100, 100, 120, 20);
    
    fogGradient.setColorAt(0.0, clearColor);
    fogGradient.setColorAt(0.5, fogColor);
    fogGradient.setColorAt(1.0, clearColor);
    
    // Draw fog overlay
    painter->setCompositionMode(QPainter::CompositionMode_Multiply);
    painter->fillRect(viewRect, QBrush(fogGradient));
    painter->setCompositionMode(QPainter::CompositionMode_SourceOver);
}

void LightDrawer::drawGlobalLighting(QPainter* painter, const QRectF& viewRect, 
                                    const DrawingOptions& options) {
    // Apply global lighting tint
    QColor tintColor = globalLightColor_;
    tintColor.setAlpha(100);
    
    painter->setCompositionMode(QPainter::CompositionMode_Multiply);
    painter->fillRect(viewRect, tintColor);
    painter->setCompositionMode(QPainter::CompositionMode_SourceOver);
}

// Task 76: Light color utilities
QColor LightDrawer::colorFromEightBit(uint8_t color) {
    // Convert 8-bit color to RGB (from wxwidgets logic)
    int r = (color & 0x07) * 36;  // 3 bits for red
    int g = ((color >> 3) & 0x07) * 36;  // 3 bits for green  
    int b = ((color >> 6) & 0x03) * 85;  // 2 bits for blue
    
    return QColor(r, g, b, 255);
}

uint8_t LightDrawer::colorToEightBit(const QColor& color) {
    // Convert RGB to 8-bit color
    uint8_t r = (color.red() / 36) & 0x07;
    uint8_t g = ((color.green() / 36) & 0x07) << 3;
    uint8_t b = ((color.blue() / 85) & 0x03) << 6;
    
    return r | g | b;
}

// Task 76: Optimization methods
void LightDrawer::updateLightCache() {
    if (!lightCacheDirty_) {
        return;
    }
    
    // Clear old cache
    lightTextureCache_.clear();
    
    // Pre-generate common light textures
    for (uint8_t intensity = 32; intensity <= MaxLightIntensity; intensity += 32) {
        for (uint8_t color = 0; color < 256; color += 16) {
            QColor lightColor = colorFromEightBit(color);
            QString key = QString("%1_%2").arg(intensity).arg(color);
            lightTextureCache_[key] = generateLightTexture(intensity, lightColor);
        }
    }
    
    lightCacheDirty_ = false;
    qDebug() << "LightDrawer::updateLightCache: Generated" << lightTextureCache_.size() << "textures";
}

bool LightDrawer::isLightVisible(const Light& light, const QRectF& viewRect) const {
    // Check if light is within view bounds with some margin for light radius
    qreal margin = LightRadius;
    QRectF expandedRect = viewRect.adjusted(-margin, -margin, margin, margin);
    
    return expandedRect.contains(light.mapX, light.mapY);
}

QPixmap LightDrawer::generateLightTexture(uint8_t intensity, const QColor& color) {
    int size = LightRadius * 2;
    QPixmap texture(size, size);
    texture.fill(Qt::transparent);
    
    QPainter texturePainter(&texture);
    texturePainter.setRenderHint(QPainter::Antialiasing);
    
    QRadialGradient gradient(size / 2, size / 2, LightRadius);
    
    QColor centerColor = color;
    centerColor.setAlpha(intensity);
    
    QColor edgeColor = color;
    edgeColor.setAlpha(0);
    
    gradient.setColorAt(0.0, centerColor);
    gradient.setColorAt(1.0, edgeColor);
    
    texturePainter.setBrush(QBrush(gradient));
    texturePainter.setPen(Qt::NoPen);
    texturePainter.drawEllipse(0, 0, size, size);
    
    return texture;
}

QPixmap LightDrawer::generateFogTexture(const QRectF& area) {
    QPixmap texture(area.size().toSize());
    texture.fill(Qt::transparent);
    
    QPainter texturePainter(&texture);
    texturePainter.setRenderHint(QPainter::Antialiasing);
    
    QLinearGradient gradient(0, 0, texture.width(), texture.height());
    gradient.setColorAt(0.0, QColor(100, 100, 120, 20));
    gradient.setColorAt(0.5, QColor(100, 100, 120, 80));
    gradient.setColorAt(1.0, QColor(100, 100, 120, 20));
    
    texturePainter.fillRect(texture.rect(), QBrush(gradient));
    
    return texture;
}
