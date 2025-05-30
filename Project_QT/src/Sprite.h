#ifndef SPRITE_H
#define SPRITE_H

#include <QSize>
#include <QPointF>
#include <QPixmap>
#include <QRectF>
#include <QVariantMap>

// Forward declare
class QPainter;

class Sprite {
public:
    Sprite() = default;
    virtual ~Sprite() = default;

    // Pure virtual methods for the core interface:
    virtual void draw(QPainter* painter, const QPointF& targetPosition, int frame = 0, const QVariantMap& options = QVariantMap()) = 0;
    virtual void drawPartial(QPainter* painter, const QRectF& targetScreenRect, const QRect& sourceSpriteRect, int frame = 0, const QVariantMap& options = QVariantMap()) = 0;
    
    virtual QSize size(int frame = 0) const = 0; // Pixel dimensions of a specific frame
    virtual int frameCount() const = 0;
    virtual int layerCount() const = 0; // If sprites can have layers (e.g., for outfits)
    
    virtual QPixmap getFramePixmap(int frame = 0, int layer = 0, const QVariantMap& options = QVariantMap()) = 0;
    
    virtual void unloadResources() = 0; // To free any loaded pixmaps or other graphical resources

    // Non-copyable and non-movable
    Sprite(const Sprite&) = delete;
    Sprite& operator=(const Sprite&) = delete;
    Sprite(Sprite&&) = delete;
    Sprite& operator=(Sprite&&) = delete;
};

#endif // SPRITE_H
