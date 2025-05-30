#include "SpriteButton.h"
#include <QStylePainter>
#include <QStyleOptionButton>
#include <QPaintEvent>

SpriteButton::SpriteButton(QWidget *parent)
    : QPushButton(parent) {
    init();
}

SpriteButton::SpriteButton(const QPixmap& pixmap, QWidget *parent)
    : QPushButton(parent), m_currentPixmap(pixmap) {
    init();
}

SpriteButton::SpriteButton(const QIcon& icon, const QString& text, QWidget *parent)
    : QPushButton(icon, text, parent) { // Call base constructor for icon and text
    init();
    // If an icon is set via this constructor, QPushButton usually handles its drawing.
    // If m_currentPixmap should also be used, or if this constructor implies
    // that the icon itself provides the primary visual, this needs clarification.
    // For now, if an icon is set via this constructor, the base QPushButton drawing is mostly sufficient
    // and m_currentPixmap is not set from it unless explicitly desired.
}

SpriteButton::~SpriteButton() {
}

void SpriteButton::init() {
    // Set any common properties for SpriteButton instances here
    // For example, if they should have a specific focus policy:
    // setFocusPolicy(Qt::StrongFocus); // Or Qt::TabFocus, etc.
    // By default, QPushButton is already focusable.
}

void SpriteButton::setPixmap(const QPixmap& pixmap) {
    m_currentPixmap = pixmap;
    update(); // Request a repaint
    updateGeometry(); // Inform layout system that size hint might have changed
}

QPixmap SpriteButton::pixmap() const {
    return m_currentPixmap;
}

QSize SpriteButton::sizeHint() const {
    if (!m_currentPixmap.isNull()) {
        // Add some padding around the pixmap
        return m_currentPixmap.size() + QSize(10, 6); // e.g., 5px padding horizontal, 3px vertical on each side
    }
    return QPushButton::sizeHint(); // Fallback to base class size hint
}

void SpriteButton::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event); // event might not be used if we repaint the whole button

    QStylePainter painter(this);
    QStyleOptionButton option;
    initStyleOption(&option); // Initialize option with current button's state (text, icon, features, state, etc.)

    // If we have a custom pixmap, we might not want the text or icon from the style option
    // to be drawn by the primitive, or we draw them ourselves.
    // For a pure sprite button, clear text and icon from option so base drawing doesn't draw them.
    // However, if text is also desired alongside the pixmap, this needs adjustment.
    // For now, assume m_currentPixmap is the primary content if present.
    // If m_currentPixmap is present, we could clear option.text and option.icon.
    // This task focuses on drawing the sprite; text alongside sprite is a further enhancement.

    // Draw the basic button frame/background using the current style
    painter.drawControl(QStyle::CE_PushButtonBevel, option);

    if (!m_currentPixmap.isNull()) {
        QRect pixmapRect = m_currentPixmap.rect();
        QRect contentRect = style()->subElementRect(QStyle::SE_PushButtonContents, &option, this);

        // Adjust for button being down (pressed)
        if (isDown()) { // or option.state & QStyle::State_Sunken
            contentRect.translate(style()->pixelMetric(QStyle::PM_ButtonShiftHorizontal, &option, this),
                                  style()->pixelMetric(QStyle::PM_ButtonShiftVertical, &option, this));
        }

        pixmapRect.moveCenter(contentRect.center());

        painter.drawPixmap(pixmapRect.topLeft(), m_currentPixmap);
    } else if (!option.icon.isNull() || !option.text.isEmpty()) {
        // If no custom pixmap, but an icon or text was set (e.g. via constructor or setIcon/setText),
        // let the style draw it. CE_PushButtonLabel handles both icon and text.
        painter.drawControl(QStyle::CE_PushButtonLabel, option);
    }

    // Draw focus rectangle if button has focus
    if (option.state & QStyle::State_HasFocus) {
        QStyleOptionFocusRect focusRectOption;
        focusRectOption.QStyleOption::operator=(option);
        focusRectOption.rect = style()->subElementRect(QStyle::SE_PushButtonFocusRect, &option, this);
        painter.drawPrimitive(QStyle::PE_FrameFocusRect, &focusRectOption);
    }
}
