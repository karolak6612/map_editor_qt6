#include "BrushPalettePanel.h"
#include "BrushPanel.h"     // For m_brushPanel
#include "Brush.h"          // For Brush* type
#include "SpriteButton.h"   // For SpriteButton
#include "ResourceManager.h" // Added for ResourceManager
#include <QVBoxLayout>
#include <QDebug>
#include <QVariant>         // For QVariant property
#include <QPixmap>          // For QPixmap placeholder
#include <QDateTime>        // For QDateTime
#include <QTimer>           // For QTimer
// QPalette and QColor might not be needed if background coloring is removed/finalized
// #include <QPalette>
// #include <QColor>

BrushPalettePanel::BrushPalettePanel(const QString& name, QWidget *parent)
    : QWidget(parent)
    , m_brushPanel(nullptr)
    , isVisible_(true)
    , maxCacheSize_(100)
    , selectedBrush_(nullptr)
    , needsRefresh_(false)
{
    QVBoxLayout *layout = new QVBoxLayout(this); // This will be the main layout
    setLayout(layout); // Set it early

    m_brushPanel = new BrushPanel(this); // Create BrushPanel instance
    layout->addWidget(m_brushPanel);     // Add BrushPanel to the layout

    // Task 47: Initialize resource management
    memoryOptimizationTimer_ = new QTimer(this);
    memoryOptimizationTimer_->setSingleShot(false);
    memoryOptimizationTimer_->setInterval(30000); // 30 seconds
    connect(memoryOptimizationTimer_, &QTimer::timeout, this, &BrushPalettePanel::optimizeMemoryUsage);
    memoryOptimizationTimer_->start();

    lastOptimizationTime_ = QDateTime::currentMSecsSinceEpoch();

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
    // Task 47: Clean up resources before destruction
    if (memoryOptimizationTimer_) {
        memoryOptimizationTimer_->stop();
    }

    clearPixmapCache();
    currentBrushes_.clear();
    activePixmaps_.clear();

    qDebug() << "BrushPalettePanel: Destructor - all resources cleaned up";

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

    // Task 47: Store current brushes for state management
    currentBrushes_ = brushes;

    clearBrushDisplay(); // Clear existing buttons

    qDebug() << "BrushPalettePanel: Populating with" << brushes.count() << "brushes using ResourceManager.";

    // Task 47: Preload pixmaps for efficiency
    preloadButtonPixmaps(brushes);

    for (Brush* brush : brushes) {
        if (!brush) {
            qWarning() << "BrushPalettePanel::populateBrushes - encountered a null brush.";
            continue;
        }

        SpriteButton* spriteButton = new SpriteButton(m_brushPanel); // Parent to BrushPanel

        // Task 47: Get Pixmap from cache or ResourceManager
        QPixmap brushPixmap;
        int lookID = brush->getLookID();

        // Check cache first for efficiency
        if (pixmapCache_.contains(lookID)) {
            brushPixmap = pixmapCache_.value(lookID);
            activePixmaps_.insert(lookID);
        } else {
            // Load from ResourceManager and cache
            if (lookID != 0) { // Attempt specific icon if lookID is valid
                QString potentialPath = QString(":/sprites/item_%1.png").arg(lookID);
                brushPixmap = ResourceManager::instance().getPixmap(potentialPath);

                if (brushPixmap.isNull()) {
                    qDebug() << "BrushPalettePanel: Specific brush icon for ID" << lookID
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

            // Cache the pixmap for future use
            if (!brushPixmap.isNull() && pixmapCache_.size() < maxCacheSize_) {
                pixmapCache_.insert(lookID, brushPixmap);
                activePixmaps_.insert(lookID);
            }
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

    // Task 47: Reset resource management state
    selectedBrush_ = nullptr;
    needsRefresh_ = false;
    clearPixmapCache();
}

// Task 47: Enhanced resource management methods
void BrushPalettePanel::clearPixmapCache()
{
    int cacheSize = pixmapCache_.size();
    pixmapCache_.clear();
    activePixmaps_.clear();

    qDebug() << "BrushPalettePanel::clearPixmapCache: Cleared" << cacheSize << "cached pixmaps";
}

void BrushPalettePanel::optimizeMemoryUsage()
{
    qint64 currentTime = QDateTime::currentMSecsSinceEpoch();

    // Only optimize if enough time has passed
    if (currentTime - lastOptimizationTime_ < 30000) { // 30 seconds
        return;
    }

    int initialSize = pixmapCache_.size();

    // Remove pixmaps that are not currently active
    QHash<int, QPixmap>::iterator it = pixmapCache_.begin();
    while (it != pixmapCache_.end()) {
        if (!activePixmaps_.contains(it.key())) {
            it = pixmapCache_.erase(it);
        } else {
            ++it;
        }
    }

    int removedCount = initialSize - pixmapCache_.size();
    lastOptimizationTime_ = currentTime;

    if (removedCount > 0) {
        qDebug() << "BrushPalettePanel::optimizeMemoryUsage: Removed" << removedCount << "unused pixmaps";
        emit memoryOptimized(removedCount);
    }
}

void BrushPalettePanel::onMapClose()
{
    qDebug() << "BrushPalettePanel::onMapClose: Clearing resources for map close";

    // Clear all cached resources when map closes
    clearPixmapCache();

    // Reset panel state
    resetPanelState();

    // Clear current brushes
    currentBrushes_.clear();
    selectedBrush_ = nullptr;
}

void BrushPalettePanel::onTilesetChange()
{
    qDebug() << "BrushPalettePanel::onTilesetChange: Refreshing for tileset change";

    // Clear pixmap cache as tileset change may affect sprites
    clearPixmapCache();

    // Mark for refresh
    needsRefresh_ = true;

    // If we have current brushes, repopulate to refresh pixmaps
    if (!currentBrushes_.isEmpty()) {
        populateBrushes(currentBrushes_);
    }
}

void BrushPalettePanel::refreshButtonStates()
{
    if (!m_brushPanel) return;

    qDebug() << "BrushPalettePanel::refreshButtonStates: Refreshing button states";

    // Get all buttons from the brush panel
    QList<SpriteButton*> buttons = m_brushPanel->findChildren<SpriteButton*>();

    for (SpriteButton* button : buttons) {
        if (!button) continue;

        // Get the brush associated with this button
        QVariant brushProperty = button->property("brush_ptr");
        if (brushProperty.isValid() && brushProperty.canConvert<void*>()) {
            Brush* brush = reinterpret_cast<Brush*>(brushProperty.value<void*>());
            if (brush) {
                // Update button state based on selection
                bool isSelected = (brush == selectedBrush_);
                button->setProperty("selected", isSelected);

                // Update visual state (this would depend on SpriteButton implementation)
                // button->setSelected(isSelected);
            }
        }
    }
}

void BrushPalettePanel::updateButtonVisibility(bool visible)
{
    isVisible_ = visible;

    if (!visible) {
        // When hidden, we can optimize memory by releasing unused pixmaps
        releaseUnusedPixmaps();
    }

    qDebug() << "BrushPalettePanel::updateButtonVisibility:" << visible;
}

void BrushPalettePanel::preloadButtonPixmaps(const QList<Brush*>& brushes)
{
    qDebug() << "BrushPalettePanel::preloadButtonPixmaps: Preloading pixmaps for" << brushes.size() << "brushes";

    int preloadedCount = 0;

    for (Brush* brush : brushes) {
        if (!brush) continue;

        int lookID = brush->getLookID();

        // Skip if already cached
        if (pixmapCache_.contains(lookID)) {
            activePixmaps_.insert(lookID);
            continue;
        }

        // Skip if cache is full
        if (pixmapCache_.size() >= maxCacheSize_) {
            break;
        }

        // Load pixmap
        QPixmap brushPixmap;
        if (lookID != 0) {
            QString potentialPath = QString(":/sprites/item_%1.png").arg(lookID);
            brushPixmap = ResourceManager::instance().getPixmap(potentialPath);
        }

        if (brushPixmap.isNull()) {
            brushPixmap = ResourceManager::instance().getPixmap(":/icons/generic_item_placeholder.png");
        }

        if (!brushPixmap.isNull()) {
            pixmapCache_.insert(lookID, brushPixmap);
            activePixmaps_.insert(lookID);
            preloadedCount++;
        }
    }

    qDebug() << "BrushPalettePanel::preloadButtonPixmaps: Preloaded" << preloadedCount << "pixmaps";
}

void BrushPalettePanel::releaseUnusedPixmaps()
{
    int initialSize = pixmapCache_.size();

    // Clear active pixmaps set to mark all as potentially unused
    activePixmaps_.clear();

    // Trigger optimization to remove unused pixmaps
    optimizeMemoryUsage();

    int releasedCount = initialSize - pixmapCache_.size();
    qDebug() << "BrushPalettePanel::releaseUnusedPixmaps: Released" << releasedCount << "pixmaps";
}
