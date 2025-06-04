#ifndef MENUACTIONHANDLER_H
#define MENUACTIONHANDLER_H

#include <QObject>
#include <QAction>
#include "MenuBarActionIDs.h"  // Fixed: Use correct header name

// Forward declarations
class MainWindow;
class BorderSystem;

/**
 * @brief Handles all menu action processing for MainWindow
 * 
 * This class extracts the large onActionTriggered method from MainWindow
 * to improve maintainability and comply with M6 file size management.
 */
class MenuActionHandler : public QObject
{
    Q_OBJECT

public:
    explicit MenuActionHandler(MainWindow* mainWindow, QObject* parent = nullptr);
    ~MenuActionHandler() = default;

    // Main action handler
    void handleAction(MenuBar::ActionID actionId);

    // Set dependencies
    void setBorderSystem(BorderSystem* borderSystem);

private slots:
    // File menu actions
    void handleNewMap();
    void handleOpenMap();
    void handleSaveMap();
    void handleSaveAsMap();
    void handleGenerateMap();
    void handleCloseMap();
    void handleImportMap();
    void handleImportMonsters();
    void handleExportMinimap();
    void handleExportTilesets();
    void handleReloadData();
    void handlePreferences();
    void handleExit();

    // Edit menu actions
    void handleUndo();
    void handleRedo();
    void handleCut();
    void handleCopy();
    void handlePaste();
    void handleReplaceItems();
    void handleAutomagic();
    void handleBorderizeSelection();
    void handleGroundValidation();

    // Navigate menu actions
    void handleGotoPosition();
    void handleGotoPreviousPosition();

    // Floor actions
    void handleFloorAction(int floor);

    // Toolbar actions
    void handlePositionGo();
    void handleBrushShapeAction(MenuBar::ActionID actionId);
    void handleBrushSizeAction(MenuBar::ActionID actionId);
    void handleBrushAction(MenuBar::ActionID actionId);

    // View menu actions
    void handleZoomIn();
    void handleZoomOut();
    void handleZoomNormal();
    void handleToolbarVisibility(MenuBar::ActionID actionId);
    void handleDockVisibility(MenuBar::ActionID actionId);

    // Window menu actions
    void handleNewPalette();
    void handleDestroyPalette();
    void handleNewDockableView();
    void handleCloseDockableViews();
    void handleSavePerspective();
    void handleLoadPerspective();
    void handleResetPerspective();

    // About menu actions
    void handleAbout();
    void handleShowHotkeys();

private:
    MainWindow* mainWindow_;
    BorderSystem* borderSystem_;

    // Helper methods
    QAction* getAction(MenuBar::ActionID actionId) const;
    void showTemporaryMessage(const QString& message, int timeout = 3000);
};

#endif // MENUACTIONHANDLER_H
