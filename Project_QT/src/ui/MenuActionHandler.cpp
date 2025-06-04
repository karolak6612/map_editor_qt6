#include "MenuActionHandler.h"
#include "MainWindow.h"
#include "BorderSystem.h"
#include <QDebug>

MenuActionHandler::MenuActionHandler(MainWindow* mainWindow, QObject* parent)
    : QObject(parent)
    , mainWindow_(mainWindow)
    , borderSystem_(nullptr)
{
    Q_ASSERT(mainWindow_);
}

void MenuActionHandler::setBorderSystem(BorderSystem* borderSystem)
{
    borderSystem_ = borderSystem;
}

void MenuActionHandler::handleAction(MenuBar::ActionID actionId)
{
    qDebug() << "MenuActionHandler: Action triggered:" << static_cast<int>(actionId);

    switch (actionId) {
        // File Menu
        case MenuBar::NEW: handleNewMap(); break;
        case MenuBar::OPEN: handleOpenMap(); break;
        case MenuBar::SAVE: handleSaveMap(); break;
        case MenuBar::SAVE_AS: handleSaveAsMap(); break;
        case MenuBar::GENERATE_MAP: handleGenerateMap(); break;
        case MenuBar::CLOSE: handleCloseMap(); break;
        case MenuBar::IMPORT_MAP: handleImportMap(); break;
        case MenuBar::IMPORT_MONSTERS: handleImportMonsters(); break;
        case MenuBar::EXPORT_MINIMAP: handleExportMinimap(); break;
        case MenuBar::EXPORT_TILESETS: handleExportTilesets(); break;
        case MenuBar::RELOAD_DATA: handleReloadData(); break;
        case MenuBar::PREFERENCES: handlePreferences(); break;
        case MenuBar::EXIT: handleExit(); break;

        // Edit Menu
        case MenuBar::UNDO: handleUndo(); break;
        case MenuBar::REDO: handleRedo(); break;
        case MenuBar::CUT: handleCut(); break;
        case MenuBar::COPY: handleCopy(); break;
        case MenuBar::PASTE: handlePaste(); break;
        case MenuBar::REPLACE_ITEMS: handleReplaceItems(); break;
        case MenuBar::AUTOMAGIC: handleAutomagic(); break;
        case MenuBar::BORDERIZE_SELECTION: handleBorderizeSelection(); break;
        case MenuBar::GROUND_VALIDATION: handleGroundValidation(); break;

        // Navigate Menu
        case MenuBar::GOTO_POSITION: handleGotoPosition(); break;
        case MenuBar::GOTO_PREVIOUS_POSITION: handleGotoPreviousPosition(); break;

        // Floor actions
        case MenuBar::FLOOR_0:
        case MenuBar::FLOOR_1:
        case MenuBar::FLOOR_2:
        case MenuBar::FLOOR_3:
        case MenuBar::FLOOR_4:
        case MenuBar::FLOOR_5:
        case MenuBar::FLOOR_6:
        case MenuBar::FLOOR_7:
        case MenuBar::FLOOR_8:
        case MenuBar::FLOOR_9:
        case MenuBar::FLOOR_10:
        case MenuBar::FLOOR_11:
        case MenuBar::FLOOR_12:
        case MenuBar::FLOOR_13:
        case MenuBar::FLOOR_14:
        case MenuBar::FLOOR_15: {
            int floor = static_cast<int>(actionId) - static_cast<int>(MenuBar::FLOOR_0);
            handleFloorAction(floor);
            break;
        }

        // Toolbar actions
        case MenuBar::TOOLBAR_POSITION_GO: handlePositionGo(); break;
        case MenuBar::TOOLBAR_SIZES_RECTANGULAR:
        case MenuBar::TOOLBAR_SIZES_CIRCULAR:
            handleBrushShapeAction(actionId);
            break;
        case MenuBar::TOOLBAR_SIZES_1:
        case MenuBar::TOOLBAR_SIZES_2:
        case MenuBar::TOOLBAR_SIZES_3:
        case MenuBar::TOOLBAR_SIZES_4:
        case MenuBar::TOOLBAR_SIZES_5:
        case MenuBar::TOOLBAR_SIZES_6:
        case MenuBar::TOOLBAR_SIZES_7:
            handleBrushSizeAction(actionId);
            break;

        // Brush toolbar actions
        case MenuBar::PALETTE_TERRAIN_OPTIONAL_BORDER_TOOL:
        case MenuBar::PALETTE_TERRAIN_ERASER:
        case MenuBar::PALETTE_TERRAIN_PZ_TOOL:
        case MenuBar::PALETTE_TERRAIN_NOPVP_TOOL:
        case MenuBar::PALETTE_TERRAIN_NOLOGOUT_TOOL:
        case MenuBar::PALETTE_TERRAIN_PVPZONE_TOOL:
        case MenuBar::PALETTE_TERRAIN_ZONE_BRUSH:
        case MenuBar::PALETTE_TERRAIN_NORMAL_DOOR:
        case MenuBar::PALETTE_TERRAIN_LOCKED_DOOR:
        case MenuBar::PALETTE_TERRAIN_MAGIC_DOOR:
        case MenuBar::PALETTE_TERRAIN_QUEST_DOOR:
        case MenuBar::PALETTE_TERRAIN_NORMAL_ALT_DOOR:
        case MenuBar::PALETTE_TERRAIN_ARCHWAY_DOOR:
        case MenuBar::PALETTE_TERRAIN_HATCH_DOOR:
        case MenuBar::PALETTE_TERRAIN_WINDOW_DOOR:
            handleBrushAction(actionId);
            break;

        // View Menu - Zoom actions
        case MenuBar::ZOOM_IN: handleZoomIn(); break;
        case MenuBar::ZOOM_OUT: handleZoomOut(); break;
        case MenuBar::ZOOM_NORMAL: handleZoomNormal(); break;

        // View Menu - Toolbar visibility
        case MenuBar::VIEW_TOOLBARS_STANDARD:
        case MenuBar::VIEW_TOOLBARS_BRUSHES:
        case MenuBar::VIEW_TOOLBARS_POSITION:
        case MenuBar::VIEW_TOOLBARS_SIZES:
            handleToolbarVisibility(actionId);
            break;

        // Window Menu - Dock visibility
        case MenuBar::VIEW_PALETTE_DOCK:
        case MenuBar::VIEW_MINIMAP_DOCK:
        case MenuBar::VIEW_PROPERTIES_DOCK:
            handleDockVisibility(actionId);
            break;

        // Window Menu - Palette management
        case MenuBar::NEW_PALETTE: handleNewPalette(); break;
        case MenuBar::DESTROY_PALETTE: handleDestroyPalette(); break;

        // Window Menu - Dockable views
        case MenuBar::NEW_DOCKABLE_VIEW: handleNewDockableView(); break;
        case MenuBar::CLOSE_DOCKABLE_VIEWS: handleCloseDockableViews(); break;

        // Window Menu - Perspective management
        case MenuBar::SAVE_PERSPECTIVE: handleSavePerspective(); break;
        case MenuBar::LOAD_PERSPECTIVE: handleLoadPerspective(); break;
        case MenuBar::RESET_PERSPECTIVE: handleResetPerspective(); break;

        // About Menu
        case MenuBar::ABOUT: handleAbout(); break;
        case MenuBar::SHOW_HOTKEYS: handleShowHotkeys(); break;

        default:
            qDebug() << "MenuActionHandler: Unhandled action:" << static_cast<int>(actionId);
            break;
    }
}

// File menu actions
void MenuActionHandler::handleNewMap()
{
    qDebug() << "New map action triggered";
    // TODO: Implement new map functionality
}

void MenuActionHandler::handleOpenMap()
{
    qDebug() << "Open map action triggered";
    // TODO: Implement open map functionality
}

void MenuActionHandler::handleSaveMap()
{
    qDebug() << "Save map action triggered";
    // TODO: Implement save map functionality
}

void MenuActionHandler::handleSaveAsMap()
{
    qDebug() << "Save As action triggered";
    // TODO: Implement save as functionality
}

void MenuActionHandler::handleGenerateMap()
{
    qDebug() << "Generate map action triggered";
    // TODO: Implement generate map functionality
}

void MenuActionHandler::handleCloseMap()
{
    qDebug() << "Close map action triggered";
    // TODO: Implement close map functionality
}

void MenuActionHandler::handleImportMap()
{
    if (mainWindow_) {
        mainWindow_->onShowImportMapDialog();
    }
}

void MenuActionHandler::handleImportMonsters()
{
    qDebug() << "Import monsters action triggered";
    // TODO: Implement import monsters functionality
}

void MenuActionHandler::handleExportMinimap()
{
    if (mainWindow_) {
        mainWindow_->onShowExportMiniMapDialog();
    }
}

void MenuActionHandler::handleExportTilesets()
{
    qDebug() << "Export tilesets action triggered";
    // TODO: Implement export tilesets functionality
}

void MenuActionHandler::handleReloadData()
{
    qDebug() << "Reload data action triggered";
    // TODO: Implement reload data functionality
}

void MenuActionHandler::handlePreferences()
{
    qDebug() << "Preferences action triggered";
    // TODO: Implement preferences dialog
}

void MenuActionHandler::handleExit()
{
    qDebug() << "Exit action triggered";
    if (mainWindow_) {
        mainWindow_->close();
    }
}

// Edit menu actions
void MenuActionHandler::handleUndo()
{
    qDebug() << "Undo action triggered";
    // TODO: Implement undo functionality
}

void MenuActionHandler::handleRedo()
{
    qDebug() << "Redo action triggered";
    // TODO: Implement redo functionality
}

void MenuActionHandler::handleCut()
{
    if (mainWindow_) {
        mainWindow_->handleCut();
    }
}

void MenuActionHandler::handleCopy()
{
    if (mainWindow_) {
        mainWindow_->handleCopy();
    }
}

void MenuActionHandler::handlePaste()
{
    if (mainWindow_) {
        mainWindow_->handlePaste();
    }
}

void MenuActionHandler::handleReplaceItems()
{
    if (mainWindow_) {
        mainWindow_->onShowReplaceItemsDialog();
    }
}

void MenuActionHandler::handleAutomagic()
{
    if (mainWindow_) {
        mainWindow_->openAutomagicSettingsDialog();
    }
}

void MenuActionHandler::handleBorderizeSelection()
{
    qDebug() << "Borderize selection action triggered";
    if (borderSystem_) {
        borderSystem_->onSelectionBorderize();
        showTemporaryMessage("Borderizing selection...", 2000);
    }
}

void MenuActionHandler::handleGroundValidation()
{
    if (mainWindow_) {
        mainWindow_->onShowGroundValidationDialog();
    }
}

QAction* MenuActionHandler::getAction(MenuBar::ActionID actionId) const
{
    if (mainWindow_) {
        return mainWindow_->getAction(actionId);
    }
    return nullptr;
}

void MenuActionHandler::showTemporaryMessage(const QString& message, int timeout)
{
    if (mainWindow_) {
        mainWindow_->showTemporaryStatusMessage(message, timeout);
    }
}

// Navigate menu actions
void MenuActionHandler::handleGotoPosition()
{
    if (mainWindow_) {
        mainWindow_->onShowGotoPositionDialog();
    }
}

void MenuActionHandler::handleGotoPreviousPosition()
{
    qDebug() << "Go to previous position action triggered";
    // TODO: Implement go to previous position functionality
}

// Floor actions
void MenuActionHandler::handleFloorAction(int floor)
{
    qDebug() << "Floor" << floor << "action triggered";
    if (auto mapView = mainWindow_->getMapView()) {
        mapView->changeFloor(floor);
    }
}

// Toolbar actions
void MenuActionHandler::handlePositionGo()
{
    if (mainWindow_) {
        mainWindow_->onPositionGo();
    }
}

void MenuActionHandler::handleBrushShapeAction(MenuBar::ActionID actionId)
{
    if (actionId == MenuBar::TOOLBAR_SIZES_RECTANGULAR) {
        qDebug() << "Rectangular brush shape selected";
    } else if (actionId == MenuBar::TOOLBAR_SIZES_CIRCULAR) {
        qDebug() << "Circular brush shape selected";
    }

    if (mainWindow_) {
        mainWindow_->onBrushShapeActionTriggered();
    }
}

void MenuActionHandler::handleBrushSizeAction(MenuBar::ActionID actionId)
{
    int size = static_cast<int>(actionId) - static_cast<int>(MenuBar::TOOLBAR_SIZES_1) + 1;
    qDebug() << "Brush size" << size << "selected";

    if (mainWindow_) {
        mainWindow_->onBrushSizeActionTriggered();
    }
}

void MenuActionHandler::handleBrushAction(MenuBar::ActionID actionId)
{
    qDebug() << "Brush action triggered:" << static_cast<int>(actionId);

    if (mainWindow_) {
        mainWindow_->onBrushActionTriggered();
    }
}

// View menu actions
void MenuActionHandler::handleZoomIn()
{
    qDebug() << "Zoom In action triggered";
    if (auto mapView = mainWindow_->getMapView()) {
        mapView->zoomIn();
    }
}

void MenuActionHandler::handleZoomOut()
{
    qDebug() << "Zoom Out action triggered";
    if (auto mapView = mainWindow_->getMapView()) {
        mapView->zoomOut();
    }
}

void MenuActionHandler::handleZoomNormal()
{
    qDebug() << "Zoom Normal action triggered";
    if (auto mapView = mainWindow_->getMapView()) {
        mapView->resetZoom();
    }
}

void MenuActionHandler::handleToolbarVisibility(MenuBar::ActionID actionId)
{
    if (!mainWindow_) return;

    switch (actionId) {
        case MenuBar::VIEW_TOOLBARS_STANDARD:
            if (auto toolbar = mainWindow_->getStandardToolBar()) {
                bool visible = !toolbar->isVisible();
                toolbar->setVisible(visible);
                if (auto action = getAction(actionId)) {
                    action->setChecked(visible);
                }
                qDebug() << "Standard toolbar visibility:" << visible;
            }
            break;
        case MenuBar::VIEW_TOOLBARS_BRUSHES:
            if (auto toolbar = mainWindow_->getBrushesToolBar()) {
                bool visible = !toolbar->isVisible();
                toolbar->setVisible(visible);
                if (auto action = getAction(actionId)) {
                    action->setChecked(visible);
                }
                qDebug() << "Brushes toolbar visibility:" << visible;
            }
            break;
        case MenuBar::VIEW_TOOLBARS_POSITION:
            if (auto toolbar = mainWindow_->getPositionToolBar()) {
                bool visible = !toolbar->isVisible();
                toolbar->setVisible(visible);
                if (auto action = getAction(actionId)) {
                    action->setChecked(visible);
                }
                qDebug() << "Position toolbar visibility:" << visible;
            }
            break;
        case MenuBar::VIEW_TOOLBARS_SIZES:
            if (auto toolbar = mainWindow_->getSizesToolBar()) {
                bool visible = !toolbar->isVisible();
                toolbar->setVisible(visible);
                if (auto action = getAction(actionId)) {
                    action->setChecked(visible);
                }
                qDebug() << "Sizes toolbar visibility:" << visible;
            }
            break;
        default:
            break;
    }
}

void MenuActionHandler::handleDockVisibility(MenuBar::ActionID actionId)
{
    if (!mainWindow_) return;

    switch (actionId) {
        case MenuBar::VIEW_PALETTE_DOCK:
            if (auto dock = mainWindow_->getPaletteDock()) {
                bool visible = !dock->isVisible();
                dock->setVisible(visible);
                if (auto action = getAction(actionId)) {
                    action->setChecked(visible);
                }
                qDebug() << "Palette dock visibility:" << visible;
            }
            break;
        case MenuBar::VIEW_MINIMAP_DOCK:
            if (auto dock = mainWindow_->getMinimapDock()) {
                bool visible = !dock->isVisible();
                dock->setVisible(visible);
                if (auto action = getAction(actionId)) {
                    action->setChecked(visible);
                }
                qDebug() << "Minimap dock visibility:" << visible;
            }
            break;
        case MenuBar::VIEW_PROPERTIES_DOCK:
            if (auto dock = mainWindow_->getPropertiesDock()) {
                bool visible = !dock->isVisible();
                dock->setVisible(visible);
                if (auto action = getAction(actionId)) {
                    action->setChecked(visible);
                }
                qDebug() << "Properties dock visibility:" << visible;
            }
            break;
        default:
            break;
    }
}

// Window menu actions
void MenuActionHandler::handleNewPalette()
{
    if (mainWindow_) {
        mainWindow_->createNewPalette();
    }
}

void MenuActionHandler::handleDestroyPalette()
{
    if (mainWindow_) {
        mainWindow_->destroyCurrentPalette();
    }
}

void MenuActionHandler::handleNewDockableView()
{
    if (mainWindow_) {
        mainWindow_->createDockableMapView();
    }
}

void MenuActionHandler::handleCloseDockableViews()
{
    if (mainWindow_) {
        mainWindow_->closeDockableViews();
    }
}

void MenuActionHandler::handleSavePerspective()
{
    if (mainWindow_) {
        mainWindow_->savePerspective();
    }
}

void MenuActionHandler::handleLoadPerspective()
{
    if (mainWindow_) {
        mainWindow_->loadPerspective();
    }
}

void MenuActionHandler::handleResetPerspective()
{
    if (mainWindow_) {
        mainWindow_->resetPerspective();
    }
}

// About menu actions
void MenuActionHandler::handleAbout()
{
    qDebug() << "About action triggered";
    // TODO: Implement about dialog
}

void MenuActionHandler::handleShowHotkeys()
{
    qDebug() << "Show hotkeys action triggered";
    // TODO: Implement hotkeys dialog
}


