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
    // If an icon is set, QPushButton usually handles its drawing.
    // If m_currentPixmap should also be used, or if this constructor implies
    // that the icon itself provides the primary visual, this needs clarification.
    // For now, if an icon is set via this constructor, paintEvent might need to
    // be careful not to overdraw it, or this constructor shouldn't set m_currentPixmap.
    // Let's assume if an icon is provided here, the base QPushButton drawing is mostly sufficient
    // and m_currentPixmap is not set from it unless explicitly desired.
    // If the goal is for THIS class to always custom draw, then perhaps the icon's pixmap
    // for the current state should be extracted and set to m_currentPixmap.
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
    if (!m_currentPixmap.isNull()) {
        // option.text = QString(); // Uncomment if you don't want QPushButton to draw its text
        // option.icon = QIcon();   // Uncomment if you don't want QPushButton to draw its icon
    }

    // Draw the basic button frame/background using the current style
    painter.drawControl(QStyle::CE_PushButtonBevel, option);
    // If you want text drawn by style, it's already in 'option.text'
    // If you want icon drawn by style, it's in 'option.icon'
    // QPushButton::paintEvent(event); // Calling base class paintEvent is another way but can be complex to combine with custom drawing.

    if (!m_currentPixmap.isNull()) {
        QRect pixmapRect = m_currentPixmap.rect();
        // Center the pixmap within the button's contents rect
        QRect contentRect = style()->subElementRect(QStyle::SE_PushButtonContents, &option, this);

        // Adjust for button being down (pressed)
        if (isDown()) {
            contentRect.translate(style()->pixelMetric(QStyle::PM_ButtonShiftHorizontal, &option, this),
                                  style()->pixelMetric(QStyle::PM_ButtonShiftVertical, &option, this));
        }

        pixmapRect.moveCenter(contentRect.center());

        // Ensure pixmap does not exceed contentRect if it's too large (optional, could also scale)
        // if (!contentRect.contains(pixmapRect)) {
        //     pixmapRect = pixmapRect.intersected(contentRect);
        // }

        painter.drawPixmap(pixmapRect.topLeft(), m_currentPixmap);
    } else if (!option.icon.isNull()) {
        // If no custom pixmap, but an icon was set (e.g. via constructor or setIcon),
        // let the style draw it. QStyle::CE_PushButtonLabel includes icon and text.
        // However, CE_PushButtonBevel usually just draws the frame.
        // To draw icon and text as per style if no custom pixmap:
        // painter.drawControl(QStyle::CE_PushButtonLabel, option);
        // This is already handled by default by QPushButton if we don't override paintEvent
        // or call the base paintEvent. Since we *are* overriding, if we want the default
        // icon/text drawing when m_currentPixmap is null, we'd need to replicate it or call base.
        // For now, if m_currentPixmap is null, it will just be a standard button (possibly with text if set).
        // The base QPushButton::paintEvent() would handle text and icon if we called it.
        // Let's rely on text being set on the button itself, and QStyle will draw it
        // as part of CE_PushButtonBevel or if we add CE_PushButtonLabel.
        // The most common case is that if m_currentPixmap is null, this button might just show text.
        // QStyleOptionButton already has text and icon. Let's draw them using CE_PushButtonLabel.
        // This ensures text appears even if no pixmap.
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
