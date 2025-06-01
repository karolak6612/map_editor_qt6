#ifndef DIALOGMANAGER_H
#define DIALOGMANAGER_H

#include <QObject>

// Forward declarations
class MainWindow;
class AutomagicSettingsDialog;
class ReplaceItemsDialog;
class GroundValidationDialog;
class ImportMapDialog;
class ExportMiniMapDialog;
class GotoPositionDialog;
class PreferencesDialog;
class AboutDialog;
class TilesetWindow;
class HelpSystem;

/**
 * @brief Manages all dialog creation and handling for MainWindow
 * 
 * This class extracts dialog management from MainWindow
 * to improve maintainability and comply with M6 file size management.
 */
class DialogManager : public QObject
{
    Q_OBJECT

public:
    explicit DialogManager(MainWindow* mainWindow, QObject* parent = nullptr);
    ~DialogManager() = default;

    // Dialog creation and handling methods
    void showAutomagicSettingsDialog();
    void showReplaceItemsDialog();
    void showGroundValidationDialog();
    void showImportMapDialog();
    void showExportMiniMapDialog();
    void showGotoPositionDialog();

    // Task 78: Advanced dialogs
    void showPreferencesDialog();
    void showAboutDialog();
    void showTilesetWindow();
    void showTilesetWindow(const class Map* map, const class Tile* tile, class Item* item);
    void showHelpSystem();
    void showHelpTopic(const QString& topic);
    void showKeyboardShortcuts();
    void showToolHelp(const QString& toolName);

private:
    // Helper methods for dialog configuration
    void configureAutomagicDialog(AutomagicSettingsDialog* dialog);
    void configureReplaceItemsDialog(ReplaceItemsDialog* dialog);
    void configureGroundValidationDialog(GroundValidationDialog* dialog);
    void configureImportMapDialog(ImportMapDialog* dialog);
    void configureExportMiniMapDialog(ExportMiniMapDialog* dialog);
    void configureGotoPositionDialog(GotoPositionDialog* dialog);

    // Task 78: Advanced dialog configuration
    void configurePreferencesDialog(PreferencesDialog* dialog);
    void configureAboutDialog(AboutDialog* dialog);
    void configureTilesetWindow(TilesetWindow* window);
    void configureHelpSystem(HelpSystem* helpSystem);

    // Result handlers
    void handleAutomagicDialogResult(AutomagicSettingsDialog* dialog, int result);
    void handleReplaceItemsDialogResult(ReplaceItemsDialog* dialog, int result);
    void handleGroundValidationDialogResult(GroundValidationDialog* dialog, int result);
    void handleImportMapDialogResult(ImportMapDialog* dialog, int result);
    void handleExportMiniMapDialogResult(ExportMiniMapDialog* dialog, int result);
    void handleGotoPositionDialogResult(GotoPositionDialog* dialog, int result);

    // Task 78: Advanced dialog result handlers
    void handlePreferencesDialogResult(PreferencesDialog* dialog, int result);
    void handleAboutDialogResult(AboutDialog* dialog, int result);
    void handleTilesetWindowResult(TilesetWindow* window, int result);
    void handleHelpSystemResult(HelpSystem* helpSystem, int result);

    MainWindow* mainWindow_;
};

#endif // DIALOGMANAGER_H
