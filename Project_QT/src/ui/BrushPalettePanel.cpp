#include "BrushPalettePanel.h"
#include "BrushPanel.h"     // For m_brushPanel
#include "Brush.h"          // For Brush* type
#include "SpriteButton.h"   // For SpriteButton
#include <QVBoxLayout>
#include <QDebug>
#include <QVariant>         // For QVariant property
#include <QPixmap>          // For QPixmap placeholder
// QPalette and QColor might not be needed if background coloring is removed/finalized
// #include <QPalette>
// #include <QColor>

BrushPalettePanel::BrushPalettePanel(const QString& name, QWidget *parent) : QWidget(parent) {
    QVBoxLayout *layout = new QVBoxLayout(this); // This will be the main layout
    setLayout(layout); // Set it early

    m_brushPanel = new BrushPanel(this); // Create BrushPanel instance
    layout->addWidget(m_brushPanel);     // Add BrushPanel to the layout

    // The old label_ setup is removed.
    // label_ = new QLabel(QString("Brush Palette Panel: %1").arg(name), this);
    // label_->setAlignment(Qt::AlignCenter);
    // layout->addWidget(label_);

    // Optional: Give it a distinct background color for easy identification
    // setAutoFillBackground(true); // This would apply to BrushPalettePanel itself
    // QPalette pal = palette();
    // pal.setColor(QPalette::Window, QColor(230, 230, 250));
    // setPalette(pal);

    qDebug() << QString("BrushPalettePanel '%1' created, containing BrushPanel.").arg(name);
}

BrushPalettePanel::~BrushPalettePanel() {
    // m_brushPanel is a child of BrushPalettePanel, so it will be deleted automatically.
    // No need to delete label_ as it's removed.
    qDebug() << QString("BrushPalettePanel destroyed.");
}

void BrushPalettePanel::clearBrushDisplay() {
    if (m_brushPanel) {
        m_brushPanel->clearButtons();
        qDebug() << "BrushPalettePanel: clearBrushDisplay called, forwarded to BrushPanel.";
    } else {
        qDebug() << "BrushPalettePanel: clearBrushDisplay called, but m_brushPanel is null.";
    }
}

void BrushPalettePanel::populateBrushes(const QList<Brush*>& brushes) {
    if (!m_brushPanel) {
        qWarning() << "BrushPalettePanel::populateBrushes - m_brushPanel is null!";
        return;
    }
    clearBrushDisplay(); // Clear existing buttons

    qDebug() << "BrushPalettePanel: Populating with" << brushes.count() << "brushes.";

    for (Brush* brush : brushes) {
        if (!brush) {
            qWarning() << "BrushPalettePanel::populateBrushes - encountered a null brush.";
            continue;
        }

        // The SpriteButton should be parented to the widget that has the layout (m_buttonContainerWidget inside BrushPanel)
        // Passing m_brushPanel as parent, addBrushButton's layout->addWidget will handle reparenting to m_buttonContainerWidget.
        SpriteButton* spriteButton = new SpriteButton(QPixmap(), m_brushPanel); // Start with empty pixmap, then set

        // --- Placeholder for Pixmap from ResourceManager ---
        // TODO: Task32 - Replace this with actual ResourceManager integration.
        //       The ResourceManager should provide a QPixmap based on brush properties,
        //       e.g., brush->getLookID(), brush->getItemType(), or a specific preview generation routine.
        // Example of future call:
        // QPixmap brushPixmap = ResourceManager::instance()->getPixmapForBrush(brush);

        qDebug() << "BrushPalettePanel: Populating brush -" << brush->name() << "(ID:" << brush->getLookID() << "). Using placeholder icon.";
        QPixmap iconPlaceholder(32, 32); // Standard size for now

        // Current simple visual differentiation for testing - RETAIN THIS or similar for now
        if (brush->name().contains("Eraser", Qt::CaseInsensitive)) {
            iconPlaceholder.fill(Qt::red);
        } else if (brush->name().contains("Door", Qt::CaseInsensitive)) {
            iconPlaceholder.fill(Qt::blue);
        } else if (brush->name().contains("Zone", Qt::CaseInsensitive)) {
            iconPlaceholder.fill(Qt::yellow);
        } else if (brush->name().contains("Border", Qt::CaseInsensitive)) {
            iconPlaceholder.fill(Qt::magenta);
        } else if (brush->getLookID() != 0) { // Add some variation based on LookID if name doesn't match
            int lookId = brush->getLookID();
            if (lookId % 5 == 0) iconPlaceholder.fill(Qt::cyan);
            else if (lookId % 5 == 1) iconPlaceholder.fill(Qt::darkCyan);
            else if (lookId % 5 == 2) iconPlaceholder.fill(Qt::darkMagenta);
            else if (lookId % 5 == 3) iconPlaceholder.fill(Qt::darkYellow);
            else iconPlaceholder.fill(Qt::darkGreen);
        } else {
            iconPlaceholder.fill(Qt::gray); // Default placeholder for others
        }
        // --- End of Placeholder ---

        spriteButton->setPixmap(iconPlaceholder);
        // Assuming Brush has a name() and getLookID() method as per the task description.
        spriteButton->setToolTip(brush->name() + QString(" (ID: %1)").arg(brush->getLookID()));
        spriteButton->setFixedSize(36,36); // Fixed size for uniformity

        // Store a pointer to the Brush object with the button
        spriteButton->setProperty("brush_ptr", QVariant::fromValue(reinterpret_cast<void*>(brush)));

        connect(spriteButton, &SpriteButton::clicked, this, &BrushPalettePanel::onBrushButtonClicked);

        m_brushPanel->addBrushButton(spriteButton);
    }
}

void BrushPalettePanel::onBrushButtonClicked() {
    SpriteButton* button = qobject_cast<SpriteButton*>(sender());
    if (button) {
        QVariant brushProperty = button->property("brush_ptr");
        if (brushProperty.isValid() && brushProperty.canConvert<void*>()) {
            Brush* brush = reinterpret_cast<Brush*>(brushProperty.value<void*>());
            if (brush) {
                qDebug() << "BrushPalettePanel: Brush button clicked for brush:" << brush->name()
                         << "(ID:" << brush->getLookID() << ")";
                emit brushSelected(brush);
            } else {
                qWarning() << "BrushPalettePanel: Clicked button's brush_ptr property is null after cast.";
            }
        } else {
            qWarning() << "BrushPalettePanel: Clicked button does not have a valid brush_ptr property or cannot convert to void*. Value:" << brushProperty;
        }
    } else {
        qWarning() << "BrushPalettePanel::onBrushButtonClicked - sender is not a SpriteButton.";
    }
}

void BrushPalettePanel::resetPanelState() {
    qDebug() << "BrushPalettePanel: resetPanelState called.";
    clearBrushDisplay(); // Clears all buttons from the m_brushPanel

    // If BrushPalettePanel had its own model or list of brushes it was displaying,
    // that would be cleared here. For example:
    // m_displayedBrushesList.clear();

    // Reset any other internal state of BrushPalettePanel itself.
    // For now, there isn't much complex state directly in BrushPalettePanel
    // beyond what m_brushPanel holds.

    // Example: If there was a selected brush pointer *within BrushPalettePanel itself*:
    // m_selectedBrushInPalettePanel = nullptr;

    // No explicit call to m_brushPanel->resetState() is made here unless BrushPanel
    // develops its own complex state beyond its buttons.
    // clearButtons() in BrushPanel already handles cleaning its main content.
}
