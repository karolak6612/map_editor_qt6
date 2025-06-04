#ifndef QT_SPRITEBUTTON_H
#define QT_SPRITEBUTTON_H

#include <QPushButton>
#include <QPixmap>
#include <QIcon>
#include "Sprite.h"  // FIXED: Include Sprite.h for SpriteSize enum

// Forward declarations
class QPaintEvent;
class QPainter;
class Sprite;
#include "SpriteManager.h" // For SpriteManager::getInstance()

// Enums matching original DCButton functionality
enum SpriteButtonType {
    SPRITE_BTN_NORMAL,
    SPRITE_BTN_TOGGLE
};

// FIXED: Removed SpriteRenderSize enum - using SpriteSize from Sprite.h instead

class SpriteButton : public QPushButton {
    Q_OBJECT

public:
    // Constructors matching original DCButton functionality
    explicit SpriteButton(QWidget *parent = nullptr);
    explicit SpriteButton(QWidget* parent, SpriteButtonType type, SpriteSize size, int spriteId = 0);  // FIXED: Use SpriteSize instead of SpriteRenderSize
    explicit SpriteButton(const QPixmap& pixmap, QWidget *parent = nullptr);
    explicit SpriteButton(const QIcon& icon, const QString& text, QWidget *parent = nullptr);
    ~SpriteButton() override;

    // Sprite management (matching DCButton API)
    void setSprite(int spriteId);
    void setSprite(Sprite* sprite);
    void setOverlay(Sprite* overlay);

    // Pixmap management (Qt-style API)
    void setPixmap(const QPixmap& pixmap);
    QPixmap pixmap() const;

    // Toggle functionality (matching DCButton)
    void setValue(bool value);
    bool getValue() const;

    // Button type and size
    SpriteButtonType buttonType() const { return buttonType_; }
    SpriteSize renderSize() const { return renderSize_; }  // FIXED: Use SpriteSize instead of SpriteRenderSize

    // Size management
    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;

private slots:
    void handleToggle();

private:
    // Core properties
    SpriteButtonType buttonType_;
    SpriteSize renderSize_;  // FIXED: Use SpriteSize instead of SpriteRenderSize
    bool toggleState_;

    // Sprite rendering
    Sprite* sprite_; // FIXED: Uncommented - still needed for direct sprite rendering
    Sprite* overlay_; // Still needed for overlay drawing
    QPixmap currentPixmap_;
    int spriteId_;

    // Initialization
    void init();
    void setupSize();
    void connectSignals();

    // Drawing helpers
    void drawButtonFrame(QPainter& painter, const QRect& rect, bool pressed);
    void drawSprite(QPainter& painter, const QRect& rect);
    void drawOverlay(QPainter& painter, const QRect& rect);
    QSize getSizeForRenderSize(SpriteSize size) const;  // FIXED: Use SpriteSize instead of SpriteRenderSize
};

#endif // QT_SPRITEBUTTON_H
