// LightDrawer.h - Qt-based light drawing system (Task 76: OpenGL replacement)

#ifndef LIGHTDRAWER_H
#define LIGHTDRAWER_H

#include <QObject>
#include <QColor>
#include <QVector>
#include <QRectF>
#include <QPointF>
#include <QPainter>
#include <QPixmap>
#include <QRadialGradient>
#include <QHash>
#include "SpriteLight.h"  // Include SpriteLight definition

// Forward declarations
struct DrawingOptions;

// Task 76: Qt-based light drawing system to replace OpenGL LightDrawer
class LightDrawer : public QObject {
    Q_OBJECT

public:
    struct Light {
        uint16_t mapX = 0;
        uint16_t mapY = 0;
        uint8_t color = 0;
        uint8_t intensity = 0;
        
        Light() = default;
        Light(uint16_t x, uint16_t y, uint8_t c, uint8_t i) 
            : mapX(x), mapY(y), color(c), intensity(i) {}
    };

    explicit LightDrawer(QObject* parent = nullptr);
    virtual ~LightDrawer();

    // Task 76: Main drawing method (replaces OpenGL draw)
    void draw(QPainter* painter, int mapX, int mapY, int endX, int endY, 
              int scrollX, int scrollY, bool fog, const DrawingOptions& options);

    // Task 76: Light management
    void setGlobalLightColor(uint8_t color);
    void addLight(int mapX, int mapY, int mapZ, const SpriteLight& light);
    void clear();

    // Task 76: Configuration
    void setEnabled(bool enabled) { enabled_ = enabled; }
    bool isEnabled() const { return enabled_; }
    
    void setFogEnabled(bool fog) { fogEnabled_ = fog; }
    bool isFogEnabled() const { return fogEnabled_; }

    // Task 76: Light color utilities
    static QColor colorFromEightBit(uint8_t color);
    static uint8_t colorToEightBit(const QColor& color);

private:
    // Task 76: Internal drawing methods
    void drawLightEffect(QPainter* painter, const Light& light, 
                        const QRectF& viewRect, const DrawingOptions& options);
    void drawFogEffect(QPainter* painter, const QRectF& viewRect, 
                      const DrawingOptions& options);
    void drawGlobalLighting(QPainter* painter, const QRectF& viewRect, 
                           const DrawingOptions& options);

    // Task 76: Light texture generation
    QPixmap generateLightTexture(uint8_t intensity, const QColor& color);
    QPixmap generateFogTexture(const QRectF& area);

    // Task 76: Optimization
    void updateLightCache();
    bool isLightVisible(const Light& light, const QRectF& viewRect) const;

    // Task 76: Member variables
    QVector<Light> lights_;
    QColor globalLightColor_;
    bool enabled_;
    bool fogEnabled_;
    
    // Task 76: Caching for performance
    QHash<QString, QPixmap> lightTextureCache_;
    QPixmap fogTexture_;
    bool lightCacheDirty_;
    
    // Task 76: Constants
    static constexpr uint8_t MaxLightIntensity = 255;
    static constexpr int LightRadius = 64; // Base light radius in pixels
    static constexpr int GroundLayer = 7;
};

// SpriteLight definition moved to SpriteLight.h to avoid redefinition

#endif // LIGHTDRAWER_H
