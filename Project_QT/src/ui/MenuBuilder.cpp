#include "MenuBuilder.h"
#include "MainWindow.h"
#include <QMenuBar>
#include <QDebug>

MenuBuilder::MenuBuilder(MainWindow* mainWindow, QObject *parent)
    : QObject(parent), mainWindow_(mainWindow)
{
}

MenuBuilder::~MenuBuilder()
{
}

QAction* MenuBuilder::createAction(const QString& text, const QString& objectName, 
                                  const QIcon& icon, const QString& shortcut, 
                                  const QString& statusTip, bool checkable, 
                                  bool checked, bool connectToGenericHandler)
{
    QAction *action = new QAction(QObject::tr(text.toStdString().c_str()), mainWindow_);
    action->setObjectName(objectName);
    action->setIcon(icon);
    if (!shortcut.isEmpty()) {
        action->setShortcut(QKeySequence::fromString(QObject::tr(shortcut.toStdString().c_str())));
    }
    action->setStatusTip(QObject::tr(statusTip.toStdString().c_str()));
    action->setCheckable(checkable);
    action->setChecked(checked);
    
    if (connectToGenericHandler && mainWindow_) {
        // Connect to MainWindow's generic handler
        QObject::connect(action, &QAction::triggered, mainWindow_, &MainWindow::onMenuActionTriggered);
    }
    
    return action;
}

QAction* MenuBuilder::createActionWithId(MenuBar::ActionID actionId, const QString& text, 
                                        const QIcon& icon, const QString& shortcut, 
                                        const QString& statusTip, bool checkable, bool checked)
{
    QAction *action = new QAction(QObject::tr(text.toStdString().c_str()), mainWindow_);
    action->setObjectName(QString("ACTION_%1").arg(static_cast<int>(actionId)));
    action->setIcon(icon);
    if (!shortcut.isEmpty()) {
        action->setShortcut(QKeySequence::fromString(QObject::tr(shortcut.toStdString().c_str())));
    }
    action->setStatusTip(QObject::tr(statusTip.toStdString().c_str()));
    action->setCheckable(checkable);
    action->setChecked(checked);

    // Store action in map for easy access
    actions_[actionId] = action;

    // Connect to MainWindow's centralized action handler
    if (mainWindow_) {
        QObject::connect(action, &QAction::triggered, [this, actionId]() {
            mainWindow_->onActionTriggered(actionId);
        });
    }

    return action;
}

QAction* MenuBuilder::getAction(MenuBar::ActionID actionId) const
{
    return actions_.value(actionId, nullptr);
}

QMenu* MenuBuilder::createFileMenu()
{
    QMenu *fileMenu = new QMenu(QObject::tr("&File"), mainWindow_);
    
    // Main file actions
    fileMenu->addAction(createActionWithId(MenuBar::NEW, "&New...", QIcon::fromTheme("document-new"), "Ctrl+N", "Create a new map."));
    fileMenu->addAction(createActionWithId(MenuBar::OPEN, "&Open...", QIcon::fromTheme("document-open"), "Ctrl+O", "Open another map."));
    fileMenu->addAction(createActionWithId(MenuBar::SAVE, "&Save", QIcon::fromTheme("document-save"), "Ctrl+S", "Save the current map."));
    fileMenu->addAction(createActionWithId(MenuBar::SAVE_AS, "Save &As...", QIcon::fromTheme("document-save-as"), "Ctrl+Shift+S", "Save the current map as a new file."));
    fileMenu->addAction(createActionWithId(MenuBar::GENERATE_MAP, "&Generate Map", QIcon(), "Ctrl+Shift+G", "Generate a new map."));
    fileMenu->addAction(createActionWithId(MenuBar::CLOSE, "&Close", QIcon::fromTheme("window-close"), "Ctrl+W", "Closes the currently open map."));
    
    fileMenu->addSeparator();
    
    // Import/Export submenus
    fileMenu->addMenu(createImportSubmenu(fileMenu));
    fileMenu->addMenu(createExportSubmenu(fileMenu));
    
    fileMenu->addSeparator();
    fileMenu->addAction(createActionWithId(MenuBar::RELOAD_DATA, "&Reload Data", QIcon::fromTheme("view-refresh"), "F5", "Reloads all data files."));
    fileMenu->addSeparator();
    
    // Recent files submenu
    fileMenu->addMenu(createRecentFilesSubmenu(fileMenu));
    
    fileMenu->addSeparator();
    fileMenu->addAction(createActionWithId(MenuBar::PREFERENCES, "&Preferences...", QIcon::fromTheme("preferences-system"), "", "Configure the map editor."));
    fileMenu->addSeparator();
    fileMenu->addAction(createActionWithId(MenuBar::EXIT, "E&xit", QIcon::fromTheme("application-exit"), "Ctrl+Q", "Close the editor."));
    
    return fileMenu;
}

QMenu* MenuBuilder::createEditMenu()
{
    QMenu *editMenu = new QMenu(QObject::tr("&Edit"), mainWindow_);
    
    // Undo/Redo
    editMenu->addAction(createActionWithId(MenuBar::UNDO, "&Undo", QIcon::fromTheme("edit-undo"), "Ctrl+Z", "Undo last action."));
    editMenu->addAction(createActionWithId(MenuBar::REDO, "&Redo", QIcon::fromTheme("edit-redo"), "Ctrl+Y", "Redo last undid action."));
    
    editMenu->addSeparator();
    
    // Find and Replace actions
    editMenu->addAction(createActionWithId(MenuBar::FIND_ITEM, "&Find Item...", QIcon::fromTheme("edit-find"), "Ctrl+F", "Find all instances of an item type on the map."));
    editMenu->addAction(createActionWithId(MenuBar::FIND_CREATURE, "Find &Creature...", QIcon::fromTheme("edit-find"), "Ctrl+Shift+C", "Find all instances of a creature on the map."));
    editMenu->addAction(createActionWithId(MenuBar::REPLACE_ITEMS, "&Replace Items...", QIcon::fromTheme("edit-find-replace"), "Ctrl+H", "Replaces all occurrences of one item with another."));
    editMenu->addAction(createActionWithId(MenuBar::REFRESH_ITEMS, "Refresh Items", QIcon::fromTheme("view-refresh"), "", "Refresh items to fix flags"));
    
    editMenu->addSeparator();
    
    // Border and Other Options submenus
    editMenu->addMenu(createBorderOptionsSubmenu(editMenu));
    editMenu->addMenu(createOtherOptionsSubmenu(editMenu));
    
    editMenu->addSeparator();
    
    // Clipboard actions
    editMenu->addAction(createActionWithId(MenuBar::CUT, "Cu&t", QIcon::fromTheme("edit-cut"), "Ctrl+X", "Cut a part of the map."));
    editMenu->addAction(createActionWithId(MenuBar::COPY, "&Copy", QIcon::fromTheme("edit-copy"), "Ctrl+C", "Copy a part of the map."));
    editMenu->addAction(createActionWithId(MenuBar::PASTE, "&Paste", QIcon::fromTheme("edit-paste"), "Ctrl+V", "Paste a part of the map."));
    
    editMenu->addSeparator();
    
    // Replace Items Dialog action
    QAction* replaceItemsAction = new QAction(QObject::tr("Find/Replace Items..."), mainWindow_);
    replaceItemsAction->setObjectName("REPLACE_ITEMS_DIALOG_ACTION");
    replaceItemsAction->setIcon(QIcon::fromTheme("edit-find-replace"));
    replaceItemsAction->setStatusTip(QObject::tr("Open the Find and Replace Items dialog."));
    if (mainWindow_) {
        QObject::connect(replaceItemsAction, &QAction::triggered, mainWindow_, &MainWindow::onShowReplaceItemsDialog);
    }
    editMenu->addAction(replaceItemsAction);
    
    return editMenu;
}

QMenu* MenuBuilder::createEditorMenu()
{
    QMenu *editorMenu = new QMenu(QObject::tr("Edito&r"), mainWindow_);
    
    editorMenu->addAction(createAction("&New View", "NEW_VIEW", QIcon::fromTheme("window-new"), "Ctrl+Shift+N", "Creates a new view of the current map."));
    editorMenu->addAction(createAction("New &Detached View", "NEW_DETACHED_VIEW", QIcon::fromTheme("window-new"), "Ctrl+Shift+D", "Creates a new detached view of the current map that can be moved to another monitor."));
    editorMenu->addAction(createAction("Enter &Fullscreen", "TOGGLE_FULLSCREEN", QIcon::fromTheme("view-fullscreen"), "F11", "Changes between fullscreen mode and windowed mode.", true));
    editorMenu->addAction(createAction("Take &Screenshot", "TAKE_SCREENSHOT", QIcon::fromTheme("applets-screenshooter"), "F10", "Saves the current view to the disk."));
    
    editorMenu->addSeparator();
    
    // Zoom submenu
    editorMenu->addMenu(createZoomSubmenu(editorMenu));
    
    return editorMenu;
}

QMenu* MenuBuilder::createSearchMenu()
{
    QMenu* menu = new QMenu(QObject::tr("&Search"), mainWindow_);
    
    menu->addAction(createAction("&Find Item...", "FIND_ITEM", QIcon::fromTheme("edit-find"), "Ctrl+F", "Find all instances of an item type the map."));
    menu->addAction(createAction("Find &Creature...", "FIND_CREATURE", QIcon::fromTheme("edit-find"), "Ctrl+Shift+C", "Find all instances of a creature on the map."));
    
    menu->addSeparator();
    menu->addAction(createAction("Find &Zones", "SEARCH_ON_MAP_ZONES", QIcon::fromTheme("edit-find"), "", "Find all zones on map."));
    menu->addAction(createAction("Find &Unique Items", "SEARCH_ON_MAP_UNIQUE", QIcon::fromTheme("edit-find"), "", "Find all unique items on map."));
    menu->addAction(createAction("Find &Action Items", "SEARCH_ON_MAP_ACTION", QIcon::fromTheme("edit-find"), "", "Find all action items on map."));
    menu->addAction(createAction("Find &Container Items", "SEARCH_ON_MAP_CONTAINER", QIcon::fromTheme("edit-find"), "", "Find all container items on map."));
    menu->addAction(createAction("Find &Text Items", "SEARCH_ON_MAP_TEXT", QIcon::fromTheme("edit-find"), "", "Find all text items on map."));
    menu->addSeparator();
    menu->addAction(createAction("Find &Everything", "SEARCH_ON_MAP_EVERYTHING", QIcon::fromTheme("edit-find"), "", "Find all unique/action/text/container items."));
    
    return menu;
}

QMenu* MenuBuilder::createMapMenu()
{
    QMenu* menu = new QMenu(QObject::tr("&Map"), mainWindow_);
    
    menu->addAction(createAction("Edit &Towns", "EDIT_TOWNS", QIcon::fromTheme("applications-office"), "Ctrl+T", "Edit towns."));
    menu->addSeparator();
    menu->addAction(createAction("&Cleanup...", "MAP_CLEANUP", QIcon::fromTheme("process-stop"), "", "Removes all items that do not exist in the OTB file (red tiles the server can't load)."));
    menu->addAction(createAction("&Properties...", "MAP_PROPERTIES", QIcon::fromTheme("document-properties"), "Ctrl+P", "Show and change the map properties."));
    menu->addAction(createAction("S&tatistics", "MAP_STATISTICS", QIcon::fromTheme("utilities-log"), "F8", "Show map statistics."));
    
    return menu;
}

QMenu* MenuBuilder::createSelectionMenu()
{
    QMenu* menu = new QMenu(QObject::tr("S&election"), mainWindow_);
    
    menu->addAction(createAction("&Replace Items on Selection", "REPLACE_ON_SELECTION_ITEMS", QIcon::fromTheme("edit-find-replace"), "", "Replace items on selected area."));
    menu->addAction(createAction("&Find Item on Selection", "SEARCH_ON_SELECTION_ITEM", QIcon::fromTheme("edit-find"), "", "Find items on selected area."));
    menu->addAction(createAction("&Remove Item on Selection", "REMOVE_ON_SELECTION_ITEM", QIcon::fromTheme("edit-delete"), "", "Remove item on selected area."));
    
    menu->addSeparator();
    menu->addAction(createAction("&Select All", "SELECT_ALL", QIcon::fromTheme("edit-select-all"), "Ctrl+A", "Select all tiles."));
    menu->addAction(createAction("Select &None", "SELECT_NONE", QIcon::fromTheme("edit-select-none"), "Ctrl+D", "Deselect all tiles."));
    menu->addAction(createAction("&Invert Selection", "SELECT_INVERT", QIcon::fromTheme("edit-select-invert"), "Ctrl+I", "Invert current selection."));
    
    menu->addSeparator();
    menu->addAction(createAction("&Borderize Selection", "BORDERIZE_SELECTION", QIcon(), "Ctrl+B", "Creates automatic borders in the entire selected area."));
    menu->addAction(createAction("&Randomize Selection", "RANDOMIZE_SELECTION", QIcon(), "", "Randomizes the ground tiles of the selected area."));
    
    return menu;
}

QMenu* MenuBuilder::createViewMenu()
{
    QMenu *viewMenu = new QMenu(QObject::tr("&View"), mainWindow_);

    viewMenu->addAction(createAction("Show &all Floors", "SHOW_ALL_FLOORS", QIcon::fromTheme("visibility-show-all"), "Ctrl+W", "If not checked other floors are hidden.", true, true));
    viewMenu->addAction(createAction("Show as &Minimap", "SHOW_AS_MINIMAP", QIcon::fromTheme("view-preview"), "Shift+E", "Show only the tile minimap colors.", true));

    viewMenu->addSeparator();

    return viewMenu;
}

QMenu* MenuBuilder::createShowMenu()
{
    QMenu *showMenu = new QMenu(QObject::tr("Sho&w"), mainWindow_);

    showMenu->addAction(createAction("Show A&nimation", "SHOW_PREVIEW", QIcon::fromTheme("media-playback-start"), "N", "Show item animations.", true, true));
    showMenu->addAction(createAction("Show &Light", "SHOW_LIGHTS", QIcon::fromTheme("weather-clear-night"), "H", "Show lights.", true, true));

    return showMenu;
}

QMenu* MenuBuilder::createNavigateMenu()
{
    QMenu* menu = new QMenu(QObject::tr("&Navigate"), mainWindow_);

    menu->addAction(createActionWithId(MenuBar::GOTO_PREVIOUS_POSITION, "Go to &Previous Position", QIcon::fromTheme("go-previous"), "P", "Go to the previous screen center position."));
    menu->addAction(createActionWithId(MenuBar::GOTO_POSITION, "&Go to Position...", QIcon::fromTheme("go-jump"), "Ctrl+G", "Navigate to a specific map position"));

    menu->addSeparator();

    // Floor submenu
    menu->addMenu(createFloorSubmenu(menu));

    return menu;
}

QMenu* MenuBuilder::createWindowMenu()
{
    QMenu* menu = new QMenu(QObject::tr("&Window"), mainWindow_);

    // Dock visibility actions
    menu->addAction(createActionWithId(MenuBar::VIEW_PALETTE_DOCK, "Palette Panel", QIcon(), "", "Show or hide the Palette panel", true, true));
    menu->addAction(createActionWithId(MenuBar::VIEW_MINIMAP_DOCK, "Minimap Panel", QIcon(), "", "Show or hide the Minimap panel", true, true));
    menu->addAction(createActionWithId(MenuBar::VIEW_PROPERTIES_DOCK, "Properties Panel", QIcon(), "", "Show or hide the Properties panel", true, true));

    menu->addSeparator();

    // Palette management
    QAction* newPaletteAction = createAction("&New Palette", "NEW_PALETTE", QIcon::fromTheme("document-new"), "", "Create a new palette window");
    menu->addAction(newPaletteAction);

    menu->addSeparator();

    // Perspective submenu
    menu->addMenu(createPerspectiveSubmenu(menu));

    return menu;
}

QMenu* MenuBuilder::createExperimentalMenu()
{
    QMenu* menu = new QMenu(QObject::tr("E&xperimental"), mainWindow_);
    menu->setObjectName("EXPERIMENTAL_MENU_PLACEHOLDER"); // For MainWindow to find it

    menu->addAction(createAction("&Fog in light view", "EXPERIMENTAL_FOG", QIcon(), "", "Apply fog filter to light effect.", true));

    return menu;
}

QMenu* MenuBuilder::createAboutMenu()
{
    QMenu* menu = new QMenu(QObject::tr("A&bout"), mainWindow_);

    menu->addAction(createActionWithId(MenuBar::EXTENSIONS, "E&xtensions...", QIcon::fromTheme("system-extensions"), "F2", "Manage editor extensions"));
    menu->addAction(createActionWithId(MenuBar::GOTO_WEBSITE, "&Goto Website", QIcon::fromTheme("web-browser"), "F3", "Visit the project website"));
    menu->addAction(createActionWithId(MenuBar::SHOW_HOTKEYS, "&Hotkeys", QIcon::fromTheme("help-keyboard-shortcuts"), "F6", "Show keyboard shortcuts"));

    menu->addSeparator();
    menu->addAction(createActionWithId(MenuBar::ABOUT, "&About...", QIcon::fromTheme("help-about"), "F1", "About this application"));

    return menu;
}

QMenu* MenuBuilder::createServerMenu()
{
    QMenu* menu = new QMenu(QObject::tr("Se&rver"), mainWindow_);

    menu->addAction(createAction("&Host Server", "ID_MENU_SERVER_HOST", QIcon::fromTheme("network-server"), "", "Host a new server for collaborative mapping"));
    menu->addAction(createAction("&Connect to Server", "ID_MENU_SERVER_CONNECT", QIcon::fromTheme("network-wired"), "", "Connect to an existing map server"));

    return menu;
}

QMenu* MenuBuilder::createIdlerMenu()
{
    QMenu* menu = new QMenu(QObject::tr("&Idler"), mainWindow_);

    menu->addAction(createAction("&Hotkeys", "SHOW_HOTKEYS", QIcon::fromTheme("help-keyboard-shortcuts"), "F6", "Hotkeys"));

    return menu;
}

// Helper methods for submenu creation
QMenu* MenuBuilder::createImportSubmenu(QMenu* parent)
{
    QMenu *importMenu = parent->addMenu(QObject::tr("&Import"));

    importMenu->addAction(createActionWithId(MenuBar::IMPORT_MAP, "Import &Map...", QIcon::fromTheme("document-import"), "", "Import map data from another map file"));
    importMenu->addAction(createActionWithId(MenuBar::IMPORT_MONSTERS, "Import &Monsters/NPC...", QIcon::fromTheme("document-import"), "", "Import either a monsters.xml file or a specific monster/NPC."));
    importMenu->addAction(createActionWithId(MenuBar::IMPORT_MINIMAP, "Import M&inimap...", QIcon::fromTheme("document-import"), "", "Import minimap data from an image file."));

    return importMenu;
}

QMenu* MenuBuilder::createExportSubmenu(QMenu* parent)
{
    QMenu *exportMenu = parent->addMenu(QObject::tr("&Export"));

    exportMenu->addAction(createActionWithId(MenuBar::EXPORT_MINIMAP, "Export &Minimap...", QIcon::fromTheme("document-export"), "", "Export minimap to an image file"));
    exportMenu->addAction(createActionWithId(MenuBar::EXPORT_TILESETS, "Export &Tilesets...", QIcon::fromTheme("document-export"), "", "Export tilesets to an xml file."));

    return exportMenu;
}

QMenu* MenuBuilder::createBorderOptionsSubmenu(QMenu* parent)
{
    QMenu *borderOptionsMenu = parent->addMenu(QObject::tr("&Border Options"));

    borderOptionsMenu->addAction(createAction("Border &Automagic", "AUTOMAGIC", QIcon(), "A", "Turns on all automatic border functions.", true));
    borderOptionsMenu->addSeparator();
    borderOptionsMenu->addAction(createAction("&Borderize Selection", "BORDERIZE_SELECTION", QIcon(), "Ctrl+B", "Creates automatic borders in the entire selected area."));
    borderOptionsMenu->addAction(createAction("Borderize &Map", "BORDERIZE_MAP", QIcon(), "", "Reborders the entire map."));
    borderOptionsMenu->addAction(createAction("&Randomize Selection", "RANDOMIZE_SELECTION", QIcon(), "", "Randomizes the ground tiles of the selected area."));
    borderOptionsMenu->addAction(createAction("Randomize M&ap", "RANDOMIZE_MAP", QIcon(), "", "Randomizes all tiles of the entire map."));

    return borderOptionsMenu;
}

QMenu* MenuBuilder::createOtherOptionsSubmenu(QMenu* parent)
{
    QMenu *otherOptionsMenu = parent->addMenu(QObject::tr("&Other Options"));

    otherOptionsMenu->addAction(createAction("Remove all &Unreachable Tiles...", "MAP_REMOVE_UNREACHABLE_TILES", QIcon(), "", "Removes all tiles that cannot be reached (or seen) by the player from the map."));
    otherOptionsMenu->addAction(createAction("&Clear Invalid Houses", "CLEAR_INVALID_HOUSES", QIcon(), "", "Clears house tiles not belonging to any house."));
    otherOptionsMenu->addAction(createAction("Clear &Modified State", "CLEAR_MODIFIED_STATE", QIcon(), "", "Clears the modified state from all tiles."));
    otherOptionsMenu->addSeparator();

    // Ground Validation dialog
    QAction* groundValidationAction = new QAction(QObject::tr("&Ground Validation..."), mainWindow_);
    groundValidationAction->setObjectName("GROUND_VALIDATION_ACTION");
    groundValidationAction->setStatusTip(QObject::tr("Validate and fix ground tile issues"));
    if (mainWindow_) {
        QObject::connect(groundValidationAction, &QAction::triggered, mainWindow_, &MainWindow::onShowGroundValidationDialog);
    }
    otherOptionsMenu->addAction(groundValidationAction);

    return otherOptionsMenu;
}

QMenu* MenuBuilder::createZoomSubmenu(QMenu* parent)
{
    QMenu *zoomMenu = parent->addMenu(QObject::tr("&Zoom"));

    zoomMenu->addAction(createActionWithId(MenuBar::ZOOM_IN, "Zoom &In", QIcon::fromTheme("zoom-in"), "Ctrl+=", "Increase the zoom."));
    zoomMenu->addAction(createActionWithId(MenuBar::ZOOM_OUT, "Zoom &Out", QIcon::fromTheme("zoom-out"), "Ctrl+-", "Decrease the zoom."));
    zoomMenu->addAction(createActionWithId(MenuBar::ZOOM_NORMAL, "Zoom &Normal", QIcon::fromTheme("zoom-original"), "Ctrl+0", "Normal zoom(100%)."));

    return zoomMenu;
}

QMenu* MenuBuilder::createFloorSubmenu(QMenu* parent)
{
    QMenu *floorMenu = parent->addMenu(QObject::tr("&Floor"));

    // Create floor actions (0-15)
    for (int i = 0; i <= 15; ++i) {
        QAction* floorAction = createAction(QString("Floor %1").arg(i), QString("FLOOR_%1").arg(i), QIcon(), QString("F%1").arg(i + 1), QString("Go to floor %1").arg(i), true);
        floorMenu->addAction(floorAction);
        if (i == 7) floorAction->setChecked(true); // Ground floor default
    }

    return floorMenu;
}

QMenu* MenuBuilder::createPerspectiveSubmenu(QMenu* parent)
{
    QMenu *perspectiveMenu = parent->addMenu(QObject::tr("&Perspective"));

    perspectiveMenu->addAction(createActionWithId(MenuBar::SAVE_PERSPECTIVE, "&Save Perspective", QIcon::fromTheme("document-save"), "", "Save current layout as perspective"));
    perspectiveMenu->addAction(createActionWithId(MenuBar::LOAD_PERSPECTIVE, "&Load Perspective", QIcon::fromTheme("document-open"), "", "Load saved perspective layout"));
    perspectiveMenu->addAction(createActionWithId(MenuBar::RESET_PERSPECTIVE, "&Reset Perspective", QIcon::fromTheme("view-restore"), "", "Reset layout to default perspective"));

    return perspectiveMenu;
}

QMenu* MenuBuilder::createRecentFilesSubmenu(QMenu* parent)
{
    QMenu *recentFilesMenu = parent->addMenu(QObject::tr("Recent &Files"));
    recentFilesMenu->setObjectName(QStringLiteral("RECENT_FILES"));

    QAction* placeholderRecent = recentFilesMenu->addAction(QObject::tr("(No recent files)"));
    placeholderRecent->setEnabled(false);

    return recentFilesMenu;
}


