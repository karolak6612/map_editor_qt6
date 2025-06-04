#include "SpriteButton.h"
#include "Sprite.h"
#include <QStylePainter>
#include <QStyleOptionButton>
#include <QPaintEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QDebug>

SpriteButton::SpriteButton(QWidget *parent)
    : QPushButton(parent),
      buttonType_(SPRITE_BTN_NORMAL),
      renderSize_(SPRITE_SIZE_16x16),
      toggleState_(false),
      sprite_(nullptr),
      overlay_(nullptr),
      spriteId_(0) {
    init();
}

SpriteButton::SpriteButton(QWidget* parent, SpriteButtonType type, SpriteSize size, int spriteId)  // FIXED: Use SpriteSize instead of SpriteRenderSize
    : QPushButton(parent),
      buttonType_(type),
      renderSize_(size),
      toggleState_(false),
      sprite_(nullptr),
      overlay_(nullptr),
      spriteId_(spriteId) {
    init();
    if (spriteId != 0) {
        setSprite(spriteId);
    }
}

SpriteButton::SpriteButton(const QPixmap& pixmap, QWidget *parent)
    : QPushButton(parent),
      buttonType_(SPRITE_BTN_NORMAL),
      renderSize_(SPRITE_SIZE_16x16),
      toggleState_(false),
      sprite_(nullptr),
      overlay_(nullptr),
      currentPixmap_(pixmap),
      spriteId_(0) {
    init();
}

SpriteButton::SpriteButton(const QIcon& icon, const QString& text, QWidget *parent)
    : QPushButton(icon, text, parent),
      buttonType_(SPRITE_BTN_NORMAL),
      renderSize_(SPRITE_SIZE_16x16),
      toggleState_(false),
      sprite_(nullptr),
      overlay_(nullptr),
      spriteId_(0) {
    init();
}

SpriteButton::~SpriteButton() {
    // Note: We don't delete sprite_ or overlay_ as they are managed elsewhere
}

void SpriteButton::init() {
    setupSize();
    connectSignals();

    // Set focus policy to match original DCButton behavior
    setFocusPolicy(Qt::StrongFocus);

    // Disable auto-repeat for consistent behavior
    setAutoRepeat(false);
}

// Sprite management methods
void SpriteButton::setSprite(int spriteId) {
    spriteId_ = spriteId;
    // Load sprite from SpriteManager and convert to QPixmap
    SpriteManager* sm = SpriteManager::getInstance();
    if (sm) {
        QImage image = sm->getSpriteImage(spriteId);
        if (!image.isNull()) {
            currentPixmap_ = QPixmap::fromImage(image);
            sprite_ = nullptr; // Ensure sprite_ is null when using pixmap_
        } else {
            currentPixmap_ = QPixmap(); // Clear pixmap if sprite not found
            qWarning() << "SpriteButton: Could not load sprite image for ID" << spriteId;
        }
    } else {
        qWarning() << "SpriteButton: SpriteManager instance not available.";
        currentPixmap_ = QPixmap();
    }
    update();
    updateGeometry(); // Update size hint if content size changes
}

void SpriteButton::setSprite(Sprite* sprite) {
    sprite_ = sprite;
    spriteId_ = 0; // Clear sprite ID when setting sprite directly
    update();
}

void SpriteButton::setOverlay(Sprite* overlay) {
    overlay_ = overlay;
    update();
}

// Pixmap management methods
void SpriteButton::setPixmap(const QPixmap& pixmap) {
    currentPixmap_ = pixmap;
    sprite_ = nullptr; // Clear sprite when setting pixmap
    spriteId_ = 0;
    update();
    updateGeometry();
}

QPixmap SpriteButton::pixmap() const {
    return currentPixmap_;
}

// Toggle functionality
void SpriteButton::setValue(bool value) {
    if (buttonType_ != SPRITE_BTN_TOGGLE) {
        qWarning() << "SpriteButton::setValue called on non-toggle button";
        return;
    }

    bool oldValue = toggleState_;
    toggleState_ = value;

    if (toggleState_ != oldValue) {
        // TODO: Set selection overlay when settings are available
        // For now, just update the visual state
        update();

        // Emit toggled signal to match QPushButton behavior
        setChecked(value);
    }
}

bool SpriteButton::getValue() const {
    if (buttonType_ != SPRITE_BTN_TOGGLE) {
        qWarning() << "SpriteButton::getValue called on non-toggle button";
        return false;
    }
    return toggleState_;
}

// Size management
QSize SpriteButton::sizeHint() const {
    return getSizeForRenderSize(renderSize_);
}

QSize SpriteButton::minimumSizeHint() const {
    return sizeHint(); // For sprite buttons, minimum size equals preferred size
}

// Helper methods
void SpriteButton::setupSize() {
    QSize size = getSizeForRenderSize(renderSize_);
    setFixedSize(size);
}

void SpriteButton::connectSignals() {
    if (buttonType_ == SPRITE_BTN_TOGGLE) {
        // For toggle buttons, connect to our toggle handler
        connect(this, &QPushButton::clicked, this, &SpriteButton::handleToggle);
    }
    // For normal buttons, the standard clicked signal is sufficient
}

QSize SpriteButton::getSizeForRenderSize(SpriteSize size) const {  // FIXED: Use SpriteSize instead of SpriteRenderSize
    // Sizes matching original DCButton with 2px padding on each side
    switch (size) {
        case SPRITE_SIZE_16x16:
            return QSize(20, 20);
        case SPRITE_SIZE_32x32:
            return QSize(36, 36);
        case SPRITE_SIZE_64x64:
            return QSize(68, 68);
        default:
            return QSize(20, 20);
    }
}

// Event handlers
void SpriteButton::handleToggle() {
    if (buttonType_ == SPRITE_BTN_TOGGLE) {
        setValue(!getValue());
    }
}

void SpriteButton::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        // Set focus when clicked (matching original DCButton behavior)
        setFocus();
    }

    // Call base implementation to handle the click
    QPushButton::mousePressEvent(event);
}

// Custom paint event matching original DCButton appearance
void SpriteButton::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, false); // Pixel-perfect drawing like original

    QRect rect = this->rect();

    // Draw the 3D button frame (matching original DCButton style)
    bool pressed = (buttonType_ == SPRITE_BTN_TOGGLE && toggleState_) || isDown();
    drawButtonFrame(painter, rect, pressed);

    // Draw the sprite or pixmap content
    drawSprite(painter, rect);

    // Draw overlay if present (for toggle buttons)
    if (overlay_ && buttonType_ == SPRITE_BTN_TOGGLE && toggleState_) {
        drawOverlay(painter, rect);
    }
}

// Drawing helper methods
void SpriteButton::drawButtonFrame(QPainter& painter, const QRect& rect, bool pressed) {
    // Colors matching original DCButton
    QColor highlight(255, 255, 255);      // White highlight
    QColor darkHighlight(212, 208, 200);  // Light gray
    QColor lightShadow(128, 128, 128);    // Medium gray
    QColor shadow(64, 64, 64);            // Dark gray

    // Fill background with black (matching original)
    painter.fillRect(rect, Qt::black);

    if (pressed) {
        // Pressed/toggle state - inset appearance
        painter.setPen(shadow);
        painter.drawLine(rect.left(), rect.top(), rect.right() - 1, rect.top());
        painter.drawLine(rect.left(), rect.top() + 1, rect.left(), rect.bottom() - 1);

        painter.setPen(lightShadow);
        painter.drawLine(rect.left() + 1, rect.top() + 1, rect.right() - 2, rect.top() + 1);
        painter.drawLine(rect.left() + 1, rect.top() + 2, rect.left() + 1, rect.bottom() - 2);

        painter.setPen(darkHighlight);
        painter.drawLine(rect.right() - 2, rect.top() + 1, rect.right() - 2, rect.bottom() - 2);
        painter.drawLine(rect.left() + 1, rect.bottom() - 2, rect.right() - 1, rect.bottom() - 2);

        painter.setPen(highlight);
        painter.drawLine(rect.right() - 1, rect.top(), rect.right() - 1, rect.bottom() - 1);
        painter.drawLine(rect.left(), rect.bottom() - 1, rect.right(), rect.bottom() - 1);
    } else {
        // Normal state - raised appearance
        painter.setPen(highlight);
        painter.drawLine(rect.left(), rect.top(), rect.right() - 1, rect.top());
        painter.drawLine(rect.left(), rect.top() + 1, rect.left(), rect.bottom() - 1);

        painter.setPen(darkHighlight);
        painter.drawLine(rect.left() + 1, rect.top() + 1, rect.right() - 2, rect.top() + 1);
        painter.drawLine(rect.left() + 1, rect.top() + 2, rect.left() + 1, rect.bottom() - 2);

        painter.setPen(lightShadow);
        painter.drawLine(rect.right() - 2, rect.top() + 1, rect.right() - 2, rect.bottom() - 2);
        painter.drawLine(rect.left() + 1, rect.bottom() - 2, rect.right() - 1, rect.bottom() - 2);

        painter.setPen(shadow);
        painter.drawLine(rect.right() - 1, rect.top(), rect.right() - 1, rect.bottom() - 1);
        painter.drawLine(rect.left(), rect.bottom() - 1, rect.right(), rect.bottom() - 1);
    }
}

void SpriteButton::drawSprite(QPainter& painter, const QRect& rect) {
    // Content area with 2px padding (matching original DCButton)
    QRect contentRect = rect.adjusted(2, 2, -2, -2);

    if (!currentPixmap_.isNull()) {
        // Draw pixmap scaled to fit content area, maintaining aspect ratio
        QPixmap scaledPixmap = currentPixmap_.scaled(contentRect.size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        QRect pixmapRect = scaledPixmap.rect();
        pixmapRect.moveCenter(contentRect.center());
        painter.drawPixmap(pixmapRect.topLeft(), scaledPixmap);
    } else if (sprite_) {
        // If a raw Sprite* is set (e.g., for custom drawing logic), use its drawTo method
        // This path is less common for simple buttons, but kept for flexibility.
        // The Sprite::drawTo method should handle its own scaling/positioning.
        sprite_->drawTo(&painter, contentRect.topLeft(), renderSize_);
    } else {
        // Draw a placeholder if no pixmap or sprite is available
        painter.fillRect(contentRect, QColor(100, 100, 100, 128));
        painter.setPen(Qt::white);
        painter.drawText(contentRect, Qt::AlignCenter, QString("N/A"));
    }
}

void SpriteButton::drawOverlay(QPainter& painter, const QRect& rect) {
    if (!overlay_) return;

    // Content area with 2px padding (matching original DCButton)
    QRect contentRect = rect.adjusted(2, 2, -2, -2);

    // TODO: Implement overlay sprite drawing when Sprite interface is available
    // For now, draw a selection indicator
    painter.setPen(QPen(Qt::yellow, 2));
    painter.drawRect(contentRect);
}

