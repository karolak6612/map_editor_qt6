#ifndef QT_SPRITE_H
#define QT_SPRITE_H

#include <QObject>
#include <QRect>

// Forward declaration
class QPainter;

enum SpriteSize {
    SPRITE_SIZE_16x16,
    SPRITE_SIZE_32x32,
    SPRITE_SIZE_64x64,
    SPRITE_SIZE_COUNT // Keep this for consistency if needed, or remove if not used
};

class Sprite : public QObject {
    Q_OBJECT

public:
    explicit Sprite(QObject *parent = nullptr) : QObject(parent) {}
    virtual ~Sprite() = default;

    // Pure virtual methods to be implemented by derived classes
    virtual void drawTo(QPainter* painter, const QRect& targetScreenRect, const QRect& sourceSpriteRect) = 0;
    // Added animation parameters, similar to original GameSprite::DrawTo, but more generic for an abstract class
    // Derived classes can choose how to interpret these, e.g. for animation frames or sprite sheet portions.
    virtual void drawTo(QPainter* painter, const QPoint& targetPos, int sourceX = 0, int sourceY = 0, int sourceWidth = -1, int sourceHeight = -1) = 0;

    virtual void unload() = 0; // For explicit resource cleanup

    virtual int width() const = 0;  // Width of a single sprite frame/image
    virtual int height() const = 0; // Height of a single sprite frame/image

    // Optional: Add methods for animation control if common to all sprites,
    // though detailed animation might be specific to derived classes like GameSprite.
    // virtual int getFrameCount() const { return 1; } // Example
    // virtual void setCurrentFrame(int frame) {}      // Example

private:
    // Disallow copy and assignment
    Sprite(const Sprite&) = delete;
    Sprite& operator=(const Sprite&) = delete;
};

#endif // QT_SPRITE_H
