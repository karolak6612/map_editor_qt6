#include "DialogManager.h"
#include "MainWindow.h"
#include "AutomagicSettingsDialog.h"
#include "ReplaceItemsDialog.h"
#include "GroundValidationDialog.h"
#include "ImportMapDialog.h"
#include "ExportMiniMapDialog.h"
#include "GotoPositionDialog.h"
#include "PreferencesDialog.h"
#include "AboutDialog.h"
#include "TilesetWindow.h"
#include "HelpSystem.h"
#include "Position.h"
#include <QDebug>

DialogManager::DialogManager(MainWindow* mainWindow, QObject* parent)
    : QObject(parent)
    , mainWindow_(mainWindow)
{
    Q_ASSERT(mainWindow_);
}

void DialogManager::showAutomagicSettingsDialog()
{
    qDebug() << "DialogManager: Showing AutomagicSettingsDialog...";
    AutomagicSettingsDialog dialog(mainWindow_);
    
    configureAutomagicDialog(&dialog);
    int result = dialog.exec();
    handleAutomagicDialogResult(&dialog, result);
}

void DialogManager::showReplaceItemsDialog()
{
    qDebug() << "DialogManager: Showing ReplaceItemsDialog...";
    ReplaceItemsDialog dialog(mainWindow_);
    
    configureReplaceItemsDialog(&dialog);
    int result = dialog.exec();
    handleReplaceItemsDialogResult(&dialog, result);
}

void DialogManager::showGroundValidationDialog()
{
    qDebug() << "DialogManager: Showing GroundValidationDialog...";
    GroundValidationDialog dialog(mainWindow_);
    
    configureGroundValidationDialog(&dialog);
    int result = dialog.exec();
    handleGroundValidationDialogResult(&dialog, result);
}

void DialogManager::showImportMapDialog()
{
    qDebug() << "DialogManager: Showing ImportMapDialog...";
    ImportMapDialog dialog(mainWindow_);
    
    configureImportMapDialog(&dialog);
    int result = dialog.exec();
    handleImportMapDialogResult(&dialog, result);
}

void DialogManager::showExportMiniMapDialog()
{
    qDebug() << "DialogManager: Showing ExportMiniMapDialog...";
    ExportMiniMapDialog dialog(mainWindow_);
    
    configureExportMiniMapDialog(&dialog);
    int result = dialog.exec();
    handleExportMiniMapDialogResult(&dialog, result);
}

void DialogManager::showGotoPositionDialog()
{
    qDebug() << "DialogManager: Showing GotoPositionDialog...";
    GotoPositionDialog dialog(mainWindow_);
    
    configureGotoPositionDialog(&dialog);
    int result = dialog.exec();
    handleGotoPositionDialogResult(&dialog, result);
}

// Configuration methods
void DialogManager::configureAutomagicDialog(AutomagicSettingsDialog* dialog)
{
    // Set default values for automagic settings
    dialog->setAutomagicEnabled(true);
    dialog->setAutomagicMode(AutomagicSettingsDialog::AUTOMAGIC_BORDERS);
    dialog->setAutomagicDelay(100);
}

void DialogManager::configureReplaceItemsDialog(ReplaceItemsDialog* dialog)
{
    // Set default values for replace items
    dialog->setFromItemId(0);
    dialog->setToItemId(0);
    dialog->setReplaceInSelection(false);
}

void DialogManager::configureGroundValidationDialog(GroundValidationDialog* dialog)
{
    // Set default values for ground validation
    dialog->setValidationMode(GroundValidationDialog::VALIDATE_ALL);
    dialog->setFixErrors(false);
}

void DialogManager::configureImportMapDialog(ImportMapDialog* dialog)
{
    // Set default values for import map
    dialog->setFilePath("");
    dialog->setHouseOption(ImportMapDialog::HOUSE_IGNORE);
    dialog->setSpawnOption(ImportMapDialog::SPAWN_IGNORE);
    dialog->setXOffset(0);
    dialog->setYOffset(0);
}

void DialogManager::configureExportMiniMapDialog(ExportMiniMapDialog* dialog)
{
    // Set default values for export minimap
    dialog->setFileName("minimap");
    dialog->setFloorRange(7, 7); // Ground floor
    dialog->setExportType(ExportMiniMapDialog::EXPORT_PNG);
    dialog->setExportEmptyFloors(false);
}

void DialogManager::configureGotoPositionDialog(GotoPositionDialog* dialog)
{
    // Set current position as default (placeholder values)
    Position currentPos(1000, 1000, 7); // Example coordinates
    dialog->setPosition(currentPos);
}

// Result handlers
void DialogManager::handleAutomagicDialogResult(AutomagicSettingsDialog* dialog, int result)
{
    if (result == QDialog::Accepted) {
        qDebug() << "Automagic settings:";
        qDebug() << "  - Enabled:" << dialog->isAutomagicEnabled();
        qDebug() << "  - Mode:" << dialog->getAutomagicMode();
        qDebug() << "  - Delay:" << dialog->getAutomagicDelay();

        // TODO: Apply automagic settings to the map editor
        if (mainWindow_) {
            mainWindow_->showTemporaryStatusMessage(tr("Automagic settings updated"), 2000);
        }
    } else {
        qDebug() << "AutomagicSettingsDialog cancelled";
    }
}

void DialogManager::handleReplaceItemsDialogResult(ReplaceItemsDialog* dialog, int result)
{
    if (result == QDialog::Accepted) {
        qDebug() << "Replace items settings:";
        qDebug() << "  - From item ID:" << dialog->getFromItemId();
        qDebug() << "  - To item ID:" << dialog->getToItemId();
        qDebug() << "  - Replace in selection:" << dialog->shouldReplaceInSelection();

        // TODO: Implement actual item replacement logic
        if (mainWindow_) {
            mainWindow_->showTemporaryStatusMessage(tr("Item replacement completed (stub implementation)"), 3000);
        }
    } else {
        qDebug() << "ReplaceItemsDialog cancelled";
    }
}

void DialogManager::handleGroundValidationDialogResult(GroundValidationDialog* dialog, int result)
{
    if (result == QDialog::Accepted) {
        qDebug() << "Ground validation settings:";
        qDebug() << "  - Validation mode:" << dialog->getValidationMode();
        qDebug() << "  - Fix errors:" << dialog->shouldFixErrors();

        // TODO: Implement actual ground validation logic
        if (mainWindow_) {
            mainWindow_->showTemporaryStatusMessage(tr("Ground validation completed (stub implementation)"), 3000);
        }
    } else {
        qDebug() << "GroundValidationDialog cancelled";
    }
}

void DialogManager::handleImportMapDialogResult(ImportMapDialog* dialog, int result)
{
    if (result == QDialog::Accepted) {
        qDebug() << "Import map settings:";
        qDebug() << "  - File path:" << dialog->getFilePath();
        qDebug() << "  - X offset:" << dialog->getXOffset();
        qDebug() << "  - Y offset:" << dialog->getYOffset();
        qDebug() << "  - House option:" << dialog->getHouseOption();
        qDebug() << "  - Spawn option:" << dialog->getSpawnOption();

        // TODO: Implement actual map import logic when Map system is ready
        if (mainWindow_) {
            mainWindow_->showTemporaryStatusMessage(tr("Map import completed (stub implementation)"), 3000);
        }
    } else {
        qDebug() << "ImportMapDialog cancelled";
    }
}

void DialogManager::handleExportMiniMapDialogResult(ExportMiniMapDialog* dialog, int result)
{
    if (result == QDialog::Accepted) {
        qDebug() << "Export minimap settings:";
        qDebug() << "  - Output directory:" << dialog->getOutputDirectory();
        qDebug() << "  - File name:" << dialog->getFileName();
        qDebug() << "  - Floor range:" << dialog->getStartFloor() << "to" << dialog->getEndFloor();
        qDebug() << "  - Export type:" << dialog->getExportType();
        qDebug() << "  - Export empty floors:" << dialog->shouldExportEmptyFloors();

        // TODO: Implement actual minimap export logic when Map system is ready
        if (mainWindow_) {
            mainWindow_->showTemporaryStatusMessage(tr("Minimap export completed (stub implementation)"), 3000);
        }
    } else {
        qDebug() << "ExportMiniMapDialog cancelled";
    }
}

void DialogManager::handleGotoPositionDialogResult(GotoPositionDialog* dialog, int result)
{
    if (result == QDialog::Accepted) {
        Position targetPos = dialog->getPosition();
        qDebug() << "Goto position:";
        qDebug() << "  - X:" << targetPos.x;
        qDebug() << "  - Y:" << targetPos.y;
        qDebug() << "  - Z:" << targetPos.z;

        // TODO: Implement actual map navigation logic when MapView system is ready
        if (mainWindow_) {
            mainWindow_->showTemporaryStatusMessage(tr("Navigated to position (%1, %2, %3) (stub implementation)")
                                                  .arg(targetPos.x).arg(targetPos.y).arg(targetPos.z), 3000);
        }
    } else {
        qDebug() << "GotoPositionDialog cancelled";
    }
}

// Task 78: Advanced dialog implementations
void DialogManager::showPreferencesDialog()
{
    qDebug() << "DialogManager: Showing PreferencesDialog...";
    PreferencesDialog dialog(mainWindow_);

    configurePreferencesDialog(&dialog);
    int result = dialog.exec();
    handlePreferencesDialogResult(&dialog, result);
}

void DialogManager::showAboutDialog()
{
    qDebug() << "DialogManager: Showing AboutDialog...";
    AboutDialog dialog(mainWindow_);

    configureAboutDialog(&dialog);
    int result = dialog.exec();
    handleAboutDialogResult(&dialog, result);
}

void DialogManager::showTilesetWindow()
{
    qDebug() << "DialogManager: Showing TilesetWindow...";
    TilesetWindow* window = new TilesetWindow(mainWindow_);

    configureTilesetWindow(window);
    window->show();
    window->raise();
    window->activateWindow();
}

void DialogManager::showTilesetWindow(const Map* map, const Tile* tile, Item* item)
{
    qDebug() << "DialogManager: Showing TilesetWindow with context...";
    TilesetWindow* window = new TilesetWindow(mainWindow_, map, tile, item);

    configureTilesetWindow(window);
    window->show();
    window->raise();
    window->activateWindow();
}

void DialogManager::showHelpSystem()
{
    qDebug() << "DialogManager: Showing HelpSystem...";
    HelpSystem::showHelp();
}

void DialogManager::showHelpTopic(const QString& topic)
{
    qDebug() << "DialogManager: Showing help topic:" << topic;
    HelpSystem::showHelp(topic);
}

void DialogManager::showKeyboardShortcuts()
{
    qDebug() << "DialogManager: Showing keyboard shortcuts...";
    HelpSystem::showKeyboardShortcuts();
}

void DialogManager::showToolHelp(const QString& toolName)
{
    qDebug() << "DialogManager: Showing tool help for:" << toolName;
    HelpSystem::showToolHelp(toolName);
}

// Task 78: Advanced dialog configuration methods
void DialogManager::configurePreferencesDialog(PreferencesDialog* dialog)
{
    if (!dialog) return;

    // Load current settings and configure dialog
    dialog->loadSettings();

    qDebug() << "PreferencesDialog configured";
}

void DialogManager::configureAboutDialog(AboutDialog* dialog)
{
    if (!dialog) return;

    // About dialog doesn't need special configuration
    qDebug() << "AboutDialog configured";
}

void DialogManager::configureTilesetWindow(TilesetWindow* window)
{
    if (!window) return;

    // Configure tileset window with current data
    window->refreshTilesets();
    window->refreshItems();

    qDebug() << "TilesetWindow configured";
}

void DialogManager::configureHelpSystem(HelpSystem* helpSystem)
{
    if (!helpSystem) return;

    // Configure help system
    helpSystem->loadHelpContent();

    qDebug() << "HelpSystem configured";
}

// Task 78: Advanced dialog result handlers
void DialogManager::handlePreferencesDialogResult(PreferencesDialog* dialog, int result)
{
    if (result == QDialog::Accepted) {
        qDebug() << "Preferences saved and applied";

        if (mainWindow_) {
            mainWindow_->showTemporaryStatusMessage(tr("Preferences saved successfully"), 3000);
        }
    } else {
        qDebug() << "PreferencesDialog cancelled";
    }
}

void DialogManager::handleAboutDialogResult(AboutDialog* dialog, int result)
{
    Q_UNUSED(dialog)
    Q_UNUSED(result)
    qDebug() << "AboutDialog closed";
}

void DialogManager::handleTilesetWindowResult(TilesetWindow* window, int result)
{
    if (result == QDialog::Accepted) {
        qDebug() << "Tileset changes saved";

        if (mainWindow_) {
            mainWindow_->showTemporaryStatusMessage(tr("Tileset changes saved successfully"), 3000);
        }
    } else {
        qDebug() << "TilesetWindow cancelled";
    }

    // Clean up window
    if (window) {
        window->deleteLater();
    }
}

void DialogManager::handleHelpSystemResult(HelpSystem* helpSystem, int result)
{
    Q_UNUSED(helpSystem)
    Q_UNUSED(result)
    qDebug() << "HelpSystem closed";
}


