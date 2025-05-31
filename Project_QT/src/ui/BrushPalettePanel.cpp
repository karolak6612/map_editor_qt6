#include "BrushPalettePanel.h"
#include "BrushPanel.h"     // For m_brushPanel
#include "Brush.h"          // For Brush* type
#include "SpriteButton.h"   // For SpriteButton
#include "ResourceManager.h" // Added for ResourceManager
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

    qDebug() << "BrushPalettePanel: Populating with" << brushes.count() << "brushes using ResourceManager.";

    for (Brush* brush : brushes) {
        if (!brush) {
            qWarning() << "BrushPalettePanel::populateBrushes - encountered a null brush.";
            continue;
        }

        SpriteButton* spriteButton = new SpriteButton(m_brushPanel); // Parent to BrushPanel

        // --- Get Pixmap from ResourceManager ---
        // TODO: The logic for deriving the correct sprite path or sheet coordinates
        //       from 'brush' (e.g., using brush->getLookID() to consult ItemManager
        //       for sprite details) needs full implementation.
        //       For now, we use a placeholder path convention.

        QPixmap brushPixmap;
        if (brush->getLookID() != 0) { // Attempt specific icon if lookID is valid
            QString potentialPath = QString(":/sprites/item_%1.png").arg(brush->getLookID());
            brushPixmap = ResourceManager::instance().getPixmap(potentialPath);

            if (brushPixmap.isNull()) {
                qDebug() << "BrushPalettePanel: Specific brush icon for ID" << brush->getLookID()
                         << "at path" << potentialPath << "not found via ResourceManager. Trying generic placeholder.";
            }
        }

        if (brushPixmap.isNull()) { // If specific icon failed or lookID was 0
            brushPixmap = ResourceManager::instance().getPixmap(":/icons/generic_item_placeholder.png");
        }

        if (brushPixmap.isNull()) { // If generic placeholder also failed
            qWarning() << "BrushPalettePanel: Generic item placeholder ':/icons/generic_item_placeholder.png' not found via ResourceManager. Using fallback color fill for brush:" << brush->name();
            brushPixmap = QPixmap(32, 32); // Default size

            // Fallback visual differentiation (can be simpler than before)
            if (brush->name().contains("Eraser", Qt::CaseInsensitive)) brushPixmap.fill(Qt::red);
            else if (brush->name().contains("Door", Qt::CaseInsensitive)) brushPixmap.fill(Qt::blue);
            else brushPixmap.fill(Qt::darkGray); // General fallback color
        }
        // --- End of Get Pixmap ---

        spriteButton->setPixmap(brushPixmap);
        spriteButton->setToolTip(brush->name() + QString(" (ID: %1)").arg(brush->getLookID()));
        spriteButton->setFixedSize(36, 36);

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
