#include "MainWindow.h"
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QKeySequence>
#include <QDebug> 
#include <QApplication> 
#include <QToolBar>     
#include <QIcon>        
#include <QActionGroup> 
#include <QSpinBox>     
#include <QComboBox>    
#include <QLabel>       
#include <QPushButton>  
#include <QDockWidget> // Added for QDockWidget
#include <QStatusBar>  // Added for QStatusBar
#include "PlaceholderPaletteWidget.h"   
#include "PlaceholderMinimapWidget.h"
#include "PlaceholderPropertiesWidget.h"
#include "AutomagicSettingsDialog.h" // Include for AutomagicSettingsDialog
#include "ClipboardData.h"           // For internal clipboard
#include "Map.h"                     // For Map and MapPos
#include "Selection.h"               // For Selection
#include <QApplication>             // For future QClipboard access
#include <QClipboard>               // For future QClipboard access
#include <QtMath>                   // For qRound
#include <QSettings>                // For saving/restoring state
#include <QByteArray>               // For saving/restoring state
#include <QCloseEvent>              // For closeEvent
// QDebug is already included via QAction or similar Qt headers usually, but explicit include is fine if needed


MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    setWindowTitle(tr("Idler's Map Editor (Qt)"));
    resize(1280, 720);

    internalClipboard_ = new ClipboardData();

    setupMenuBar();
    setupToolBars(); 
    setupDockWidgets(); // Call setupDockWidgets
    setupStatusBar();
    
    restoreToolBarState(); // Restore state after UI is setup
    qDebug() << "MainWindow created. Menu, toolbars, and docks setup initiated. State restored.";
}

MainWindow::~MainWindow() {
    delete internalClipboard_;
    internalClipboard_ = nullptr;
}

void MainWindow::setupMenuBar() {
    menuBar_ = menuBar(); 
    menuBar_->addMenu(createFileMenu());
    menuBar_->addMenu(createEditMenu());
    menuBar_->addMenu(createEditorMenu()); 
    menuBar_->addMenu(createSearchMenu());
    menuBar_->addMenu(createMapMenu());
    menuBar_->addMenu(createSelectionMenu());
    menuBar_->addMenu(createViewMenu());   
    menuBar_->addMenu(createShowMenu());   
    menuBar_->addMenu(createNavigateMenu());
    menuBar_->addMenu(createWindowMenu());
    menuBar_->addMenu(createExperimentalMenu());
    menuBar_->addMenu(createAboutMenu()); 
    menuBar_->addMenu(createServerMenu());
    menuBar_->addMenu(createIdlerMenu());
    qDebug() << "Menu bar setup complete with menus.";
}

void MainWindow::setupToolBars() {
    standardToolBar_ = createStandardToolBar();
    if (standardToolBar_) { 
        addToolBar(Qt::TopToolBarArea, standardToolBar_);
        standardToolBar_->setVisible(true); 
    }

    brushesToolBar_ = createBrushesToolBar();
    if (brushesToolBar_) { 
         addToolBar(Qt::TopToolBarArea, brushesToolBar_);
         brushesToolBar_->setVisible(true); 
    }
    
    positionToolBar_ = createPositionToolBar();
    if (positionToolBar_){
        addToolBar(Qt::TopToolBarArea, positionToolBar_);
        positionToolBar_->setVisible(true); 
    }

    sizesToolBar_ = createSizesToolBar();
    if(sizesToolBar_){
        addToolBar(Qt::TopToolBarArea, sizesToolBar_);
        sizesToolBar_->setVisible(true); 
    }
    qDebug() << "All toolbars setup attempted.";
}

void MainWindow::setupDockWidgets() {
    setDockNestingEnabled(true);

    // Palette Dock
    paletteDock_ = new QDockWidget(tr("Palette"), this);
    paletteDock_->setObjectName(QStringLiteral("PaletteDock"));
    paletteDock_->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    PlaceholderPaletteWidget* paletteContent = new PlaceholderPaletteWidget("Palette", paletteDock_);
    paletteDock_->setWidget(paletteContent);
    addDockWidget(Qt::LeftDockWidgetArea, paletteDock_);
    paletteDock_->setVisible(true); 
    if (viewPaletteDockAction_) { // Ensure action exists (created in createWindowMenu)
        viewPaletteDockAction_->setChecked(paletteDock_->isVisible());
    }

    // Minimap Dock
    minimapDock_ = new QDockWidget(tr("Minimap"), this);
    minimapDock_->setObjectName(QStringLiteral("MinimapDock"));
    minimapDock_->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea | Qt::BottomDockWidgetArea);
    PlaceholderMinimapWidget* minimapContent = new PlaceholderMinimapWidget("Minimap", minimapDock_);
    minimapDock_->setWidget(minimapContent);
    addDockWidget(Qt::RightDockWidgetArea, minimapDock_); 
    minimapDock_->setVisible(true); 
    if (viewMinimapDockAction_) {
        viewMinimapDockAction_->setChecked(minimapDock_->isVisible());
    }

    // Properties Dock
    propertiesDock_ = new QDockWidget(tr("Properties"), this);
    propertiesDock_->setObjectName(QStringLiteral("PropertiesDock"));
    propertiesDock_->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    PlaceholderPropertiesWidget* propertiesContent = new PlaceholderPropertiesWidget("Properties", propertiesDock_);
    propertiesDock_->setWidget(propertiesContent);
    addDockWidget(Qt::RightDockWidgetArea, propertiesDock_); 
    propertiesDock_->setVisible(true); 
    if (viewPropertiesDockAction_) {
        viewPropertiesDockAction_->setChecked(propertiesDock_->isVisible());
    }
    
    // Example of tabbing:
    // tabifyDockWidget(minimapDock_, propertiesDock_);

    qDebug() << "Dock widgets setup.";
}

void MainWindow::setupStatusBar() {
    QStatusBar *sb = statusBar(); // QMainWindow::statusBar() creates one if it doesn't exist
    if (!sb) {
        qWarning("MainWindow: Could not get or create QStatusBar.");
        return;
    }

    // Initialize and add labels as permanent widgets
    // Permanent widgets are typically added from right to left

    currentLayerLabel_ = new QLabel(this);
    currentLayerLabel_->setText(tr("Floor: 7"));
    currentLayerLabel_->setToolTip(tr("Current map floor/layer"));
    sb->addPermanentWidget(currentLayerLabel_);

    zoomLevelLabel_ = new QLabel(this);
    zoomLevelLabel_->setText(tr("Zoom: 100%"));
    zoomLevelLabel_->setToolTip(tr("Current map zoom level"));
    sb->addPermanentWidget(zoomLevelLabel_);

    itemInfoLabel_ = new QLabel(this);
    itemInfoLabel_->setText(tr("Item: None")); // Placeholder
    itemInfoLabel_->setToolTip(tr("Information about the selected item or item under cursor"));
    itemInfoLabel_->setMinimumWidth(200); // Give it some space
    sb->addPermanentWidget(itemInfoLabel_);

    brushInfoLabel_ = new QLabel(this);
    brushInfoLabel_->setText(tr("Brush: None")); // Placeholder
    brushInfoLabel_->setToolTip(tr("Current active brush"));
    sb->addPermanentWidget(brushInfoLabel_);

    // mouseCoordsLabel_ will be a normal message shown with showMessage, or a temporary widget.
    // For persistent coordinate display, it can also be a permanent widget. Let's make it permanent for now.
    mouseCoordsLabel_ = new QLabel(this);
    mouseCoordsLabel_->setText(tr("X: -, Y: -, Z: -"));
    mouseCoordsLabel_->setToolTip(tr("Current map coordinates under mouse cursor"));
    mouseCoordsLabel_->setMinimumWidth(150); // Give it some space
    sb->addPermanentWidget(mouseCoordsLabel_);

    // Main status messages will appear on the left, temporary messages.
    sb->showMessage(tr("Ready"), 2000); // Example temporary message
    qDebug() << "Status bar setup complete.";
}


QAction* MainWindow::createAction(const QString& text, const QString& objectName, const QIcon& icon, const QString& shortcut, const QString& statusTip, bool checkable, bool checked, bool connectToGenericHandler) {
    QAction *action = new QAction(tr(text.toStdString().c_str()), this);
    action->setObjectName(objectName);
    action->setIcon(icon); // Set the icon
    if (!shortcut.isEmpty()) {
        action->setShortcut(QKeySequence::fromString(tr(shortcut.toStdString().c_str())));
    }
    action->setStatusTip(tr(statusTip.toStdString().c_str()));
    action->setCheckable(checkable);
    action->setChecked(checked);
    if (connectToGenericHandler) { // Conditional connection
        connect(action, &QAction::triggered, this, &MainWindow::onMenuActionTriggered);
    }
    return action;
}

QMenu* MainWindow::createFileMenu() {
    QMenu *fileMenu = new QMenu(tr("&File"), this);
    newAction_ = createAction("&New...", "NEW", QIcon::fromTheme("document-new"), QKeySequence::New, "Create a new map."); // Corrected: Was "P"
    fileMenu->addAction(newAction_);
    openAction_ = createAction("&Open...", "OPEN", QIcon::fromTheme("document-open"), QKeySequence::Open, "Open another map.");
    fileMenu->addAction(openAction_);
    saveAction_ = createAction("&Save", "SAVE", QIcon::fromTheme("document-save"), QKeySequence::Save, "Save the current map.");
    fileMenu->addAction(saveAction_);
    fileMenu->addAction(createAction("Save &As...", "SAVE_AS", QIcon::fromTheme("document-save-as"), QKeySequence::SaveAs, "Save the current map as a new file."));
    fileMenu->addAction(createAction("&Generate Map", "GENERATE_MAP", QIcon(), QKeySequence("Ctrl+Shift+G"), "Generate a new map."));
    fileMenu->addAction(createAction("&Close", "CLOSE", QIcon::fromTheme("window-close"), QKeySequence("Ctrl+W"), "Closes the currently open map.")); // Changed Shift+B to Ctrl+W for consistency
    fileMenu->addSeparator();
    QMenu *importMenu = fileMenu->addMenu(tr("&Import"));
    importMenu->addAction(createAction("Import &Map...", "IMPORT_MAP", QIcon::fromTheme("document-import"), "", "Import map data from another map file."));
    importMenu->addAction(createAction("Import &Monsters/NPC...", "IMPORT_MONSTERS", QIcon::fromTheme("document-import"), "", "Import either a monsters.xml file or a specific monster/NPC."));
    QMenu *exportMenu = fileMenu->addMenu(tr("&Export"));
    exportMenu->addAction(createAction("Export &Minimap...", "EXPORT_MINIMAP", QIcon::fromTheme("document-export"), "", "Export minimap to an image file."));
    exportMenu->addAction(createAction("Export &Tilesets...", "EXPORT_TILESETS", QIcon::fromTheme("document-export"), "", "Export tilesets to an xml file."));
    QMenu *reloadMenu = fileMenu->addMenu(tr("&Reload"));
    reloadMenu->addAction(createAction("&Reload All Data", "RELOAD_DATA", QIcon::fromTheme("view-refresh"), QKeySequence::Refresh, "Reloads all data files.")); // F5 is Refresh
    fileMenu->addSeparator();
    QMenu *recentFilesMenu = fileMenu->addMenu(tr("Recent &Files"));
    recentFilesMenu->setObjectName(QStringLiteral("RECENT_FILES"));
    QAction* placeholderRecent = recentFilesMenu->addAction(tr("(No recent files)"));
    placeholderRecent->setEnabled(false);
    fileMenu->addAction(createAction("&Preferences", "PREFERENCES", QIcon::fromTheme("preferences-system"), QKeySequence::Preferences, "Configure the map editor.")); // Often Ctrl+, or Ctrl+Shift+P
    fileMenu->addSeparator();
    fileMenu->addAction(createAction("E&xit", "EXIT", QIcon::fromTheme("application-exit"), QKeySequence::Quit, "Close the editor."));
    return fileMenu;
}

QMenu* MainWindow::createEditMenu() {
    QMenu *editMenu = new QMenu(tr("&Edit"), this);
    undoAction_ = createAction("&Undo", "UNDO", QIcon::fromTheme("edit-undo"), QKeySequence::Undo, "Undo last action.");
    editMenu->addAction(undoAction_);
    redoAction_ = createAction("&Redo", "REDO", QIcon::fromTheme("edit-redo"), QKeySequence::Redo, "Redo last undid action.");
    editMenu->addAction(redoAction_);
    editMenu->addSeparator();
    editMenu->addAction(createAction("&Replace Items...", "REPLACE_ITEMS", QIcon::fromTheme("edit-find-replace"), QKeySequence("Ctrl+Shift+F"), "Replaces all occurrences of one item with another.")); // QKeySequence::Replace is specific to find/replace dialog context
    editMenu->addAction(createAction("Refresh Items", "REFRESH_ITEMS", QIcon::fromTheme("view-refresh"), "", "Refresh items to fix flags"));
    editMenu->addSeparator();
    QMenu *borderOptionsMenu = editMenu->addMenu(tr("&Border Options"));
    borderOptionsMenu->addAction(createAction("Border &Automagic", "AUTOMAGIC", QIcon(), QKeySequence("A"), "Turns on all automatic border functions.", true));
    borderOptionsMenu->addSeparator();
    borderOptionsMenu->addAction(createAction("&Borderize Selection", "BORDERIZE_SELECTION", QIcon(), QKeySequence("Ctrl+B"), "Creates automatic borders in the entire selected area."));
    borderOptionsMenu->addAction(createAction("Borderize &Map", "BORDERIZE_MAP", QIcon(), "", "Reborders the entire map."));
    borderOptionsMenu->addAction(createAction("&Randomize Selection", "RANDOMIZE_SELECTION", QIcon(), "", "Randomizes the ground tiles of the selected area."));
    borderOptionsMenu->addAction(createAction("Randomize M&ap", "RANDOMIZE_MAP", QIcon(), "", "Randomizes all tiles of the entire map."));
    QMenu *otherOptionsMenu = editMenu->addMenu(tr("&Other Options"));
    otherOptionsMenu->addAction(createAction("Remove all &Unreachable Tiles...", "MAP_REMOVE_UNREACHABLE_TILES", QIcon(), "", "Removes all tiles that cannot be reached (or seen) by the player from the map."));
    otherOptionsMenu->addAction(createAction("&Clear Invalid Houses", "CLEAR_INVALID_HOUSES", QIcon(), "", "Clears house tiles not belonging to any house."));
    otherOptionsMenu->addAction(createAction("Clear &Modified State", "CLEAR_MODIFIED_STATE", QIcon(), "", "Clears the modified state from all tiles."));
    editMenu->addSeparator();
    cutAction_ = createAction("Cu&t", "CUT", QIcon::fromTheme("edit-cut"), QKeySequence::Cut, "Cut a part of the map.");
    editMenu->addAction(cutAction_);
    copyAction_ = createAction("&Copy", "COPY", QIcon::fromTheme("edit-copy"), QKeySequence::Copy, "Copy a part of the map.");
    editMenu->addAction(copyAction_);
    pasteAction_ = createAction("&Paste", "PASTE", QIcon::fromTheme("edit-paste"), QKeySequence::Paste, "Paste a part of the map.");
    editMenu->addAction(pasteAction_);
    return editMenu;
}

QMenu* MainWindow::createEditorMenu() {
    QMenu *editorMenu = new QMenu(tr("Edito&r"), this);
    editorMenu->addAction(createAction("&New View", "NEW_VIEW", QIcon::fromTheme("window-new"), QKeySequence("Ctrl+Shift+N"), "Creates a new view of the current map.")); // Explicit string to avoid conflict if QKeySequence::New used elsewhere for "New Window"
    editorMenu->addAction(createAction("New &Detached View", "NEW_DETACHED_VIEW", QIcon::fromTheme("window-new"), QKeySequence("Ctrl+Shift+D"), "Creates a new detached view of the current map that can be moved to another monitor."));
    editorMenu->addAction(createAction("Enter &Fullscreen", "TOGGLE_FULLSCREEN", QIcon::fromTheme("view-fullscreen"), QKeySequence::FullScreen, "Changes between fullscreen mode and windowed mode.", true));
    editorMenu->addAction(createAction("Take &Screenshot", "TAKE_SCREENSHOT", QIcon::fromTheme("applets-screenshooter"), QKeySequence("F10"), "Saves the current view to the disk."));
    editorMenu->addSeparator();
    QMenu *zoomMenu = editorMenu->addMenu(tr("&Zoom"));
    zoomMenu->addAction(createAction("Zoom &In", "ZOOM_IN", QIcon::fromTheme("zoom-in"), QKeySequence::ZoomIn, "Increase the zoom."));
    zoomMenu->addAction(createAction("Zoom &Out", "ZOOM_OUT", QIcon::fromTheme("zoom-out"), QKeySequence::ZoomOut, "Decrease the zoom."));
    zoomMenu->addAction(createAction("Zoom &Normal", "ZOOM_NORMAL", QIcon::fromTheme("zoom-original"), QKeySequence("Ctrl+0"), "Normal zoom(100%).")); // Ctrl+0 is standard for actual size
    return editorMenu;
}

QMenu* MainWindow::createSearchMenu() {
    QMenu* menu = new QMenu(tr("&Search"), this);
    menu->addAction(createAction("&Find Item...", "FIND_ITEM", QIcon::fromTheme("edit-find"), QKeySequence::Find, "Find all instances of an item type the map."));
    menu->addAction(createAction("Find &Creature...", "FIND_CREATURE", QIcon::fromTheme("edit-find"), QKeySequence("Ctrl+Shift+C"), "Find all instances of a creature on the map."));
    menu->addSeparator();
    menu->addAction(createAction("Find &Zones", "SEARCH_ON_MAP_ZONES", QIcon::fromTheme("edit-find"), "", "Find all zones on map."));
    menu->addAction(createAction("Find &Unique", "SEARCH_ON_MAP_UNIQUE", QIcon::fromTheme("edit-find"), QKeySequence("L"), "Find all items with an unique ID on map."));
    menu->addAction(createAction("Find &Action", "SEARCH_ON_MAP_ACTION", QIcon::fromTheme("edit-find"), "", "Find all items with an action ID on map."));
    menu->addAction(createAction("Find &Container", "SEARCH_ON_MAP_CONTAINER", QIcon::fromTheme("edit-find"), "", "Find all containers on map."));
    menu->addAction(createAction("Find &Writeable", "SEARCH_ON_MAP_WRITEABLE", QIcon::fromTheme("edit-find"), "", "Find all writeable items on map."));
    menu->addSeparator();
    menu->addAction(createAction("Find &Everything", "SEARCH_ON_MAP_EVERYTHING", QIcon::fromTheme("edit-find"), "", "Find all unique/action/text/container items."));
    return menu;
}

QMenu* MainWindow::createMapMenu() {
    QMenu* menu = new QMenu(tr("&Map"), this);
    menu->addAction(createAction("Edit &Towns", "EDIT_TOWNS", QIcon::fromTheme("applications-office"), QKeySequence("Ctrl+T"), "Edit towns."));
    menu->addSeparator();
    menu->addAction(createAction("&Cleanup...", "MAP_CLEANUP", QIcon::fromTheme("process-stop"), "", "Removes all items that do not exist in the OTB file (red tiles the server can't load)."));
    menu->addAction(createAction("&Properties...", "MAP_PROPERTIES", QIcon::fromTheme("document-properties"), QKeySequence("Ctrl+P"), "Show and change the map properties.")); // No standard QKeySequence::Properties
    menu->addAction(createAction("S&tatistics", "MAP_STATISTICS", QIcon::fromTheme("utilities-log"), QKeySequence("F8"), "Show map statistics."));
    return menu;
}

QMenu* MainWindow::createSelectionMenu() {
    QMenu* menu = new QMenu(tr("S&election"), this);
    menu->addAction(createAction("&Replace Items on Selection", "REPLACE_ON_SELECTION_ITEMS", QIcon::fromTheme("edit-find-replace"), "", "Replace items on selected area."));
    menu->addAction(createAction("&Find Item on Selection", "SEARCH_ON_SELECTION_ITEM", QIcon::fromTheme("edit-find"), "", "Find items on selected area."));
    menu->addAction(createAction("&Remove Item on Selection", "REMOVE_ON_SELECTION_ITEM", QIcon::fromTheme("edit-delete"), "", "Remove item on selected area."));
    menu->addSeparator();
    QMenu *findOnSelectionMenu = menu->addMenu(tr("Find on Selection"));
    findOnSelectionMenu->addAction(createAction("Find &Everything", "SEARCH_ON_SELECTION_EVERYTHING", QIcon::fromTheme("edit-find"), "", "Find all unique/action/text/container items."));
    findOnSelectionMenu->addSeparator();
    findOnSelectionMenu->addAction(createAction("Find &Zones", "SEARCH_ON_SELECTION_ZONES", QIcon::fromTheme("edit-find"), "", "Find all zones on selected area."));
    findOnSelectionMenu->addAction(createAction("Find &Unique", "SEARCH_ON_SELECTION_UNIQUE", QIcon::fromTheme("edit-find"), "", "Find all items with an unique ID on selected area."));
    findOnSelectionMenu->addAction(createAction("Find &Action", "SEARCH_ON_SELECTION_ACTION", QIcon::fromTheme("edit-find"), "", "Find all items with an action ID on selected area."));
    findOnSelectionMenu->addAction(createAction("Find &Container", "SEARCH_ON_SELECTION_CONTAINER", QIcon::fromTheme("edit-find"), "", "Find all containers on selected area."));
    findOnSelectionMenu->addAction(createAction("Find &Writeable", "SEARCH_ON_SELECTION_WRITEABLE", QIcon::fromTheme("edit-find"), "", "Find all writeable items on selected area."));
    menu->addSeparator();
    QMenu *selectionModeMenu = menu->addMenu(tr("Selection &Mode"));
    QActionGroup* selectionModeGroup = new QActionGroup(this);
    selectionModeGroup->setExclusive(true);
    QAction* compensateAction = createAction("&Compensate Selection", "SELECT_MODE_COMPENSATE", QIcon(), "", "Compensate for floor difference when selecting.", true); // No specific icon
    selectionModeMenu->addAction(compensateAction);
    selectionModeGroup->addAction(compensateAction);
    selectionModeMenu->addSeparator();
    QAction* currentFloorAction = createAction("&Current Floor", "SELECT_MODE_CURRENT", QIcon(), "", "Select only current floor.", true);
    selectionModeMenu->addAction(currentFloorAction);
    selectionModeGroup->addAction(currentFloorAction);
    currentFloorAction->setChecked(true); 
    QAction* lowerFloorsAction = createAction("&Lower Floors", "SELECT_MODE_LOWER", QIcon(), "", "Select all lower floors.", true);
    selectionModeMenu->addAction(lowerFloorsAction);
    selectionModeGroup->addAction(lowerFloorsAction);
    QAction* visibleFloorsAction = createAction("&Visible Floors", "SELECT_MODE_VISIBLE", QIcon(), "", "Select only visible floors.", true);
    selectionModeMenu->addAction(visibleFloorsAction);
    selectionModeGroup->addAction(visibleFloorsAction);
    menu->addSeparator();
    menu->addAction(createAction("&Borderize Selection", "BORDERIZE_SELECTION", QIcon(), QKeySequence("Ctrl+B"), "Creates automatic borders in the entire selected area.")); // Re-uses from Edit Menu
    menu->addAction(createAction("&Randomize Selection", "RANDOMIZE_SELECTION", QIcon(), "", "Randomizes the ground tiles of the selected area.")); // Re-uses from Edit Menu
    return menu;
}

QMenu* MainWindow::createViewMenu() { 
    QMenu *viewMenu = new QMenu(tr("&View"), this);
    viewMenu->addAction(createAction("Show &all Floors", "SHOW_ALL_FLOORS", QIcon::fromTheme("visibility-show-all"), QKeySequence("Ctrl+W"), "If not checked other floors are hidden.", true, true)); // Ctrl+W was also Close in File menu, this might be an issue. Keeping as is per current code.
    viewMenu->addAction(createAction("Show as &Minimap", "SHOW_AS_MINIMAP", QIcon::fromTheme("view-preview"), QKeySequence("Shift+E"), "Show only the tile minimap colors.", true));
    viewMenu->addSeparator();
    return viewMenu;
}

QMenu* MainWindow::createShowMenu() {
    QMenu *showMenu = new QMenu(tr("Sho&w"), this); 
    showMenu->addAction(createAction("Show A&nimation", "SHOW_PREVIEW", QIcon::fromTheme("media-playback-start"), QKeySequence("N"), "Show item animations.", true, true));
    showMenu->addAction(createAction("Show &Light", "SHOW_LIGHTS", QIcon::fromTheme("weather-clear-night"), QKeySequence("H"), "Show lights.", true, true));
    return showMenu;
}

QMenu* MainWindow::createNavigateMenu() {
    QMenu* menu = new QMenu(tr("&Navigate"), this);
    menu->addAction(createAction("Go to &Previous Position", "GOTO_PREVIOUS_POSITION", QIcon::fromTheme("go-previous"), QKeySequence("P"), "Go to the previous screen center position.")); // "P" is not standard, but kept as per original.
    QMenu *floorMenu = menu->addMenu(tr("&Floor"));
    QActionGroup* floorGroup = new QActionGroup(this);
    floorGroup->setExclusive(true);
    for (int i = 0; i <= 15; ++i) {
        QAction* floorAction = createAction(QString("Floor %1").arg(i), QString("FLOOR_%1").arg(i), QIcon(), "", "", true); // No individual icons for floors
        floorMenu->addAction(floorAction);
        floorGroup->addAction(floorAction);
        if (i == 7) floorAction->setChecked(true); 
    }
    return menu;
}

QMenu* MainWindow::createWindowMenu() { 
    QMenu* menu = new QMenu(tr("&Window"), this);

    viewPaletteDockAction_ = createAction(tr("Palette Panel"), "VIEW_PALETTE_DOCK", QIcon(), "", "Show or hide the Palette panel.", true, true); // Icon set by dock itself?
    menu->addAction(viewPaletteDockAction_);
    viewMinimapDockAction_ = createAction(tr("Minimap Panel"), "VIEW_MINIMAP_DOCK", QIcon(), "", "Show or hide the Minimap panel.", true, true);
    menu->addAction(viewMinimapDockAction_);
    viewPropertiesDockAction_ = createAction(tr("Properties Panel"), "VIEW_PROPERTIES_DOCK", QIcon(), "", "Show or hide the Properties panel.", true, true);
    menu->addAction(viewPropertiesDockAction_);
    menu->addSeparator();
    menu->addAction(createAction("&New Palette", "NEW_PALETTE", QIcon::fromTheme("document-new"), "", "Creates a new palette."));
    menu->addSeparator();

    QMenu *paletteMenu = menu->addMenu(tr("&Palette"));
    paletteMenu->addAction(createAction("&Terrain", "SELECT_TERRAIN", QIcon(), QKeySequence("T"), "Select the Terrain palette."));
    paletteMenu->addAction(createAction("&Doodad", "SELECT_DOODAD", QIcon(), QKeySequence("D"), "Select the Doodad palette."));
    menu->addSeparator(); 

    QMenu *toolbarsMenu = menu->addMenu(tr("&Toolbars"));
    toolbarsMenu->addAction(createAction("&Brushes", "VIEW_TOOLBARS_BRUSHES", QIcon(), "", "Show or hide the Brushes toolbar.", true, true));
    toolbarsMenu->addAction(createAction("&Position", "VIEW_TOOLBARS_POSITION", QIcon(), "", "Show or hide the Position toolbar.", true, true));
    toolbarsMenu->addAction(createAction("&Sizes", "VIEW_TOOLBARS_SIZES", QIcon(), "", "Show or hide the Sizes toolbar.", true, true));
    toolbarsMenu->addAction(createAction("&Standard", "VIEW_TOOLBARS_STANDARD", QIcon(), "", "Show or hide the Standard toolbar.", true, true));
    return menu;
}

QMenu* MainWindow::createExperimentalMenu() {
    QMenu* menu = new QMenu(tr("E&xperimental"), this);
    menu->addAction(createAction("&Fog in light view", "EXPERIMENTAL_FOG", QIcon(), "", "Apply fog filter to light effect.", true));
    return menu;
}

QMenu* MainWindow::createAboutMenu() {
    QMenu* menu = new QMenu(tr("A&bout"), this); 
    menu->addAction(createAction("E&xtensions...", "EXTENSIONS", QIcon::fromTheme("system-extensions"), QKeySequence("F2"), ""));
    menu->addAction(createAction("&Goto Website", "GOTO_WEBSITE", QIcon::fromTheme("web-browser"), QKeySequence("F3"), ""));
    menu->addAction(createAction("&About...", "ABOUT", QIcon::fromTheme("help-about"), QKeySequence::HelpContents, "")); // F1 for Help
    return menu;
}

QMenu* MainWindow::createServerMenu() {
    QMenu* menu = new QMenu(tr("Se&rver"), this);
    menu->addAction(createAction("&Host Server", "ID_MENU_SERVER_HOST", QIcon::fromTheme("network-server"), "", "Host a new server for collaborative mapping"));
    menu->addAction(createAction("&Connect to Server", "ID_MENU_SERVER_CONNECT", QIcon::fromTheme("network-wired"), "", "Connect to an existing map server"));
    return menu;
}

QMenu* MainWindow::createIdlerMenu() {
    QMenu* menu = new QMenu(tr("&Idler"), this);
    menu->addAction(createAction("&Hotkeys", "SHOW_HOTKEYS", QIcon::fromTheme("help-keyboard-shortcuts"), QKeySequence("F6"), "Hotkeys"));
    return menu;
}

QToolBar* MainWindow::createStandardToolBar() {
    QToolBar* tb = new QToolBar(tr("Standard"), this);
    tb->setObjectName(QStringLiteral("StandardToolBar"));
    if (newAction_) tb->addAction(newAction_);
    if (openAction_) tb->addAction(openAction_);
    if (saveAction_) tb->addAction(saveAction_);
    tb->addSeparator();
    if (undoAction_) tb->addAction(undoAction_);
    if (redoAction_) tb->addAction(redoAction_);
    tb->addSeparator();
    if (cutAction_) tb->addAction(cutAction_);
    if (copyAction_) tb->addAction(copyAction_);
    if (pasteAction_) tb->addAction(pasteAction_);
    tb->addSeparator();
    tb->addWidget(new QLabel(tr("Zoom:"), this));
    zoomSpinBox_ = new QSpinBox(this);
    zoomSpinBox_->setRange(10, 400); 
    zoomSpinBox_->setValue(100);
    zoomSpinBox_->setSuffix(tr("%"));
    zoomSpinBox_->setToolTip(tr("Set map zoom level"));
    connect(zoomSpinBox_, qOverload<int>(&QSpinBox::valueChanged), this, &MainWindow::onZoomControlChanged);
    tb->addWidget(zoomSpinBox_);
    tb->addWidget(new QLabel(tr("Layer:"), this));
    layerComboBox_ = new QComboBox(this);
    for (int i = 0; i <= 15; ++i) { 
        layerComboBox_->addItem(QString(tr("Floor %1")).arg(i), i); 
    }
    layerComboBox_->setCurrentIndex(7); 
    layerComboBox_->setToolTip(tr("Select current map layer/floor"));
    connect(layerComboBox_, qOverload<int>(&QComboBox::currentIndexChanged), this, &MainWindow::onLayerControlChanged);
    tb->addWidget(layerComboBox_);
    return tb;
}

QToolBar* MainWindow::createBrushesToolBar() {
    QToolBar* tb = new QToolBar(tr("Brushes"), this);
    tb->setObjectName(QStringLiteral("BrushesToolBar"));
    QActionGroup* brushGroup = new QActionGroup(this);
    brushGroup->setExclusive(true);

    // Helper lambda to create actions, trying specific icons first, then theme icons.
    auto addBrushAction = [&](const QString& text, const QString& objectName, const QString& iconName, const QString& themeIconName, const QString& toolTip) {
        QIcon icon(QStringLiteral(":/icons/%1.png").arg(iconName));
        if (icon.isNull()) {
            icon = QIcon::fromTheme(themeIconName);
        }
        if (icon.isNull()) { // Fallback if theme icon also not found
            qDebug() << "Icon not found for" << objectName << ", tried" << QStringLiteral(":/icons/%1.png").arg(iconName) << "and theme" << themeIconName;
            icon = QIcon(); // Empty icon
        }
        QAction* action = createAction(text, objectName, icon, "", toolTip, true, false); // connectToGenericHandler = false
        connect(action, &QAction::triggered, this, &MainWindow::onBrushActionTriggered);
        tb->addAction(action);
        brushGroup->addAction(action);
        return action;
    };

    addBrushAction("Optional Border", "BRUSH_OPTIONAL_BORDER", "optional_border_small", "draw-border", "Toggle optional border brush");
    addBrushAction("Eraser", "BRUSH_ERASER", "eraser_small", "edit-clear", "Toggle eraser brush");
    tb->addSeparator();
    addBrushAction("Protected Zone", "BRUSH_PZ", "pz_zone", "security-high", "Toggle Protected Zone brush");
    addBrushAction("No PvP Zone", "BRUSH_NOPVP", "nopvp_zone", "user-block", "Toggle No PvP Zone brush");
    addBrushAction("No Logout Zone", "BRUSH_NOLOGOUT", "nologout_zone", "system-log-out", "Toggle No Logout Zone brush");
    addBrushAction("PvP Zone", "BRUSH_PVP", "pvp_zone", "security-medium", "Toggle PvP Zone brush");
    addBrushAction("Zone Brush", "BRUSH_ZONE", "zone_brush", "draw-polygon", "Toggle generic zone brush");
    tb->addSeparator();
    addBrushAction("Normal Door", "BRUSH_DOOR_NORMAL", "door_normal_small", "object-UNSET", "Toggle normal door brush");
    addBrushAction("Locked Door", "BRUSH_DOOR_LOCKED", "door_locked_small", "object-UNSET", "Toggle locked door brush");
    addBrushAction("Magic Door", "BRUSH_DOOR_MAGIC", "door_magic_small", "object-UNSET", "Toggle magic door brush");
    addBrushAction("Quest Door", "BRUSH_DOOR_QUEST", "door_quest_small", "object-UNSET", "Toggle quest door brush");
    addBrushAction("Normal Door (alt)", "BRUSH_DOOR_NORMAL_ALT", "door_normal_alt_small", "object-UNSET", "Toggle alternative normal door brush");
    addBrushAction("Archway", "BRUSH_DOOR_ARCHWAY", "door_archway_small", "object-UNSET", "Toggle archway brush");
    tb->addSeparator();
    addBrushAction("Hatch Window", "BRUSH_WINDOW_HATCH", "window_hatch_small", "object-UNSET", "Toggle hatch window brush");
    addBrushAction("Window", "BRUSH_WINDOW_NORMAL", "window_normal_small", "object-UNSET", "Toggle normal window brush");

    if (!tb->actions().isEmpty() && tb->actions().first()->isCheckable()) {
        tb->actions().first()->setChecked(true); // Set first brush as active by default
    }
    return tb;
}

QToolBar* MainWindow::createPositionToolBar() {
    QToolBar* tb = new QToolBar(tr("Position"), this);
    tb->setObjectName(QStringLiteral("PositionToolBar"));
    tb->addWidget(new QLabel(tr("X:"), this));
    xCoordSpinBox_ = new QSpinBox(this);
    xCoordSpinBox_->setRange(0, 32767); 
    xCoordSpinBox_->setToolTip(tr("X Coordinate")); // Updated tooltip
    connect(xCoordSpinBox_, qOverload<int>(&QSpinBox::valueChanged), this, &MainWindow::onPositionXChanged);
    tb->addWidget(xCoordSpinBox_);
    tb->addWidget(new QLabel(tr("Y:"), this));
    yCoordSpinBox_ = new QSpinBox(this);
    yCoordSpinBox_->setRange(0, 32767); 
    yCoordSpinBox_->setToolTip(tr("Y Coordinate")); // Updated tooltip
    connect(yCoordSpinBox_, qOverload<int>(&QSpinBox::valueChanged), this, &MainWindow::onPositionYChanged);
    tb->addWidget(yCoordSpinBox_);
    tb->addWidget(new QLabel(tr("Z:"), this));
    zCoordSpinBox_ = new QSpinBox(this);
    zCoordSpinBox_->setRange(0, 15);    
    zCoordSpinBox_->setToolTip(tr("Z Coordinate")); // Updated tooltip
    connect(zCoordSpinBox_, qOverload<int>(&QSpinBox::valueChanged), this, &MainWindow::onPositionZChanged);
    tb->addWidget(zCoordSpinBox_);

    QPushButton* goButton = new QPushButton(this);
    goButton->setToolTip(tr("Go To Position")); // Updated tooltip
    QIcon goIcon = QIcon::fromTheme("go-jump");
    if (goIcon.isNull()) {
        goIcon = QIcon(QStringLiteral(":/icons/position_go.png"));
    }
    if (goIcon.isNull()) {
         qDebug() << "Icon not found for Go button";
    }
    goButton->setIcon(goIcon);
    goButton->setText(tr("Go")); // Keep text for clarity if icon is missing
    connect(goButton, &QPushButton::clicked, this, &MainWindow::onPositionGo);
    tb->addWidget(goButton);
    return tb;
}

QToolBar* MainWindow::createSizesToolBar() {
    QToolBar* tb = new QToolBar(tr("Sizes"), this);
    tb->setObjectName(QStringLiteral("SizesToolBar"));

    brushShapeActionGroup_ = new QActionGroup(this);
    brushShapeActionGroup_->setExclusive(true);

    QIcon rectIcon(QStringLiteral(":/icons/rectangular_brush.png"));
    if (rectIcon.isNull()) rectIcon = QIcon::fromTheme("draw-rectangle");
    rectangularBrushShapeAction_ = createAction(tr("Rectangular"), "TOGGLE_BRUSH_SHAPE_RECT", rectIcon, "", "Use rectangular brush shape", true, false); // connectToGenericHandler = false
    rectangularBrushShapeAction_->setChecked(true);
    connect(rectangularBrushShapeAction_, &QAction::triggered, this, &MainWindow::onBrushShapeActionTriggered);
    brushShapeActionGroup_->addAction(rectangularBrushShapeAction_);
    tb->addAction(rectangularBrushShapeAction_);

    QIcon circIcon(QStringLiteral(":/icons/circular_brush.png"));
    if (circIcon.isNull()) circIcon = QIcon::fromTheme("draw-ellipse");
    circularBrushShapeAction_ = createAction(tr("Circular"), "TOGGLE_BRUSH_SHAPE_CIRC", circIcon, "", "Use circular brush shape", true, false); // connectToGenericHandler = false
    connect(circularBrushShapeAction_, &QAction::triggered, this, &MainWindow::onBrushShapeActionTriggered);
    brushShapeActionGroup_->addAction(circularBrushShapeAction_);
    tb->addAction(circularBrushShapeAction_);

    tb->addSeparator();

    brushSizeActionGroup_ = new QActionGroup(this);
    brushSizeActionGroup_->setExclusive(true);

    auto addSizeAction = [&](int size, const QString& objectNameSuffix, const QString& toolTipSuffix) {
        // Default to rectangular icons, assuming dynamic update later if shape changes
        QIcon icon(QStringLiteral(":/icons/rectangular_%1.png").arg(size));
        if (icon.isNull()) {
            // Fallback or no icon if specific size icons don't exist
            // For now, let's use a generic draw-primitive icon or none
            icon = QIcon::fromTheme("draw-primitive");
             if (icon.isNull()) {
                qDebug() << "Icon not found for brush size" << size;
             }
        }
        // In wx, sizes were 0-6 mapping to 1x1 to 13x13 (actual_size = 2*N+1)
        // Here, N is 1-7 for actions. Let's assume N directly maps to the icon number.
        int displaySize = (size -1) * 2 + 1;
        QAction* action = createAction(tr("Size %1x%1").arg(displaySize), QString("SET_BRUSH_SIZE_%1").arg(objectNameSuffix), icon, "", QString("Set brush size to %1").arg(toolTipSuffix), true, false); // connectToGenericHandler = false
        connect(action, &QAction::triggered, this, &MainWindow::onBrushSizeActionTriggered);
        brushSizeActionGroup_->addAction(action);
        tb->addAction(action);
        return action;
    };

    brushSize1Action_ = addSizeAction(1, "1", "1x1");
    brushSize1Action_->setChecked(true); 
    brushSize2Action_ = addSizeAction(2, "2", "3x3");
    brushSize3Action_ = addSizeAction(3, "3", "5x5");
    brushSize4Action_ = addSizeAction(4, "4", "7x7");
    brushSize5Action_ = addSizeAction(5, "5", "9x9");
    brushSize6Action_ = addSizeAction(6, "6", "11x11");
    brushSize7Action_ = addSizeAction(7, "7", "13x13");
    return tb;
}

// Slots for new toolbar actions
void MainWindow::onBrushActionTriggered() {
    QAction* action = qobject_cast<QAction*>(sender());
    if (action) {
        qDebug() << "Brush selected:" << action->objectName() << "(" << action->text() << ")";
        // Future: Update global state or call map editor's brush selection logic
        // Example: g_mapEditor.setCurrentBrush(action->objectName());
        updateCurrentBrush(action->text()); // Update status bar
    }
}

void MainWindow::onBrushShapeActionTriggered() {
    QAction* action = qobject_cast<QAction*>(sender());
    if (!action) return;

    QString shapePrefix;
    if (action == rectangularBrushShapeAction_) {
        shapePrefix = "rectangular";
    } else if (action == circularBrushShapeAction_) {
        shapePrefix = "circular";
    } else {
        qDebug() << "Unknown brush shape action triggered.";
        return; // Not one of the known shape actions
    }

    qDebug() << "Brush shape selected:" << shapePrefix;

    // Update icons for all brush size actions
    QList<QAction*> sizeActions = {
        brushSize1Action_, brushSize2Action_, brushSize3Action_, brushSize4Action_,
        brushSize5Action_, brushSize6Action_, brushSize7Action_
    };

    for (int i = 0; i < sizeActions.length(); ++i) {
        if (sizeActions[i]) {
            // Icon names are 1-indexed (e.g., rectangular_1.png)
            QIcon icon(QString(":/icons/%1_%2.png").arg(shapePrefix).arg(i + 1));
            if (icon.isNull()) {
                // Fallback if specific icon (e.g., circular_3.png) is not found
                qDebug() << "Specific icon not found: " << QString(":/icons/%1_%2.png").arg(shapePrefix).arg(i + 1) << "Using fallback.";
                icon = QIcon::fromTheme("draw-primitive");
            }
             if (icon.isNull()) { // Further fallback if theme icon also fails
                qDebug() << "Fallback theme icon 'draw-primitive' also not found for size" << (i+1);
             }
            sizeActions[i]->setIcon(icon);
        }
    }
}

void MainWindow::onBrushSizeActionTriggered() {
    QAction* action = qobject_cast<QAction*>(sender());
    if (action) {
        qDebug() << "Brush size selected:" << action->objectName() << "(" << action->text() << ")";
        // Future: Update global state or call map editor's brush size logic
        // Example: g_mapEditor.setBrushSizeFromAction(action->objectName());
    }
}


void MainWindow::onMenuActionTriggered() {
    QAction *action = qobject_cast<QAction*>(sender());
    if (!action) {
        qDebug() << "onMenuActionTriggered called, but sender is not a QAction.";
        return;
    }

    QString actionName = action->objectName();
    QString actionText = action->text(); // Use text for more user-friendly logging at times
    // bool isCheckable = action->isCheckable(); // Not used in this version
    // bool isChecked = action->isChecked(); // Used for dock/toolbar visibility

    qDebug() << "Action triggered: Name =" << actionName << ", Text =" << actionText << ", Shortcut:" << action->shortcut().toString();

    if (actionName == QLatin1String("EXIT")) {
        close(); // This will trigger closeEvent, which calls saveToolBarState
    } else if (actionName == QLatin1String("VIEW_TOOLBARS_STANDARD")) {
        if (standardToolBar_) {
            bool visible = !standardToolBar_->isVisible();
            standardToolBar_->setVisible(visible);
            action->setChecked(visible);
        }
    } else if (actionName == QLatin1String("VIEW_TOOLBARS_BRUSHES")) {
        if (brushesToolBar_) {
            bool visible = !brushesToolBar_->isVisible();
            brushesToolBar_->setVisible(visible);
            action->setChecked(visible);
        }
    } else if (actionName == QLatin1String("VIEW_TOOLBARS_POSITION")) {
        if (positionToolBar_) {
            bool visible = !positionToolBar_->isVisible();
            positionToolBar_->setVisible(visible);
            action->setChecked(visible);
        }
    } else if (actionName == QLatin1String("VIEW_TOOLBARS_SIZES")) {
        if (sizesToolBar_) {
            bool visible = !sizesToolBar_->isVisible();
            sizesToolBar_->setVisible(visible);
            action->setChecked(visible);
        }
    } else if (actionName == QLatin1String("VIEW_PALETTE_DOCK")) {
        if (paletteDock_) {
            bool visible = !paletteDock_->isVisible();
            paletteDock_->setVisible(visible);
            action->setChecked(visible);
        }
    } else if (actionName == QLatin1String("VIEW_MINIMAP_DOCK")) {
        if (minimapDock_) {
            bool visible = !minimapDock_->isVisible();
            minimapDock_->setVisible(visible);
            action->setChecked(visible);
        }
    } else if (actionName == QLatin1String("VIEW_PROPERTIES_DOCK")) {
        if (propertiesDock_) {
            bool visible = !propertiesDock_->isVisible();
            propertiesDock_->setVisible(visible);
            action->setChecked(visible);
        }
    }
    // Placeholder command handlers for common actions from menubar.xml
    else if (actionName == QLatin1String("NEW")) { qDebug() << "Placeholder: File -> New action triggered."; }
    else if (actionName == QLatin1String("OPEN")) { qDebug() << "Placeholder: File -> Open action triggered."; }
    else if (actionName == QLatin1String("SAVE")) { qDebug() << "Placeholder: File -> Save action triggered."; }
    else if (actionName == QLatin1String("SAVE_AS")) { qDebug() << "Placeholder: File -> Save As action triggered."; }
    else if (actionName == QLatin1String("UNDO")) { qDebug() << "Placeholder: Edit -> Undo action triggered."; }
    else if (actionName == QLatin1String("REDO")) { qDebug() << "Placeholder: Edit -> Redo action triggered."; }
    else if (actionName == QLatin1String("CUT")) { qDebug() << "Placeholder: Edit -> Cut action triggered."; handleCut(); } // Existing call
    else if (actionName == QLatin1String("COPY")) { qDebug() << "Placeholder: Edit -> Copy action triggered."; handleCopy(); } // Existing call
    else if (actionName == QLatin1String("PASTE")) { qDebug() << "Placeholder: Edit -> Paste action triggered."; handlePaste(); } // Existing call
    else if (actionName == QLatin1String("ZOOM_IN")) {
        qDebug() << "Placeholder: Editor -> Zoom In action triggered. (MapView should handle actual zoom via Ctrl++)";
        // TODO: Find MapView instance and call a zoomIn method or simulate key event if MainWindow needs to drive this.
    } else if (actionName == QLatin1String("ZOOM_OUT")) {
        qDebug() << "Placeholder: Editor -> Zoom Out action triggered. (MapView should handle actual zoom via Ctrl+-)";
        // TODO: Find MapView instance and call a zoomOut method or simulate key event.
    } else if (actionName == QLatin1String("ZOOM_NORMAL")) {
        qDebug() << "Placeholder: Editor -> Zoom Normal action triggered.";
        // TODO: Find MapView instance and call a zoomNormal method.
    } else if (actionName.startsWith("FLOOR_")) { // Example for floor actions from Navigate menu
        bool ok;
        int floor = actionName.mid(6).toInt(&ok);
        if (ok) {
            qDebug() << "Placeholder: Navigate -> Floor" << floor << "action triggered. (MapView should handle actual floor change)";
            // TODO: Find MapView instance and call changeFloor(floor)
            // Example: if (mapViewInstance_) mapViewInstance_->changeFloor(floor);
        }
    }
    // else {
        // If no specific handler, you might log it, or it might be a checkable action handled by its own toggle slot.
        // qDebug() << "Action" << actionName << "not specifically handled for direct action in onMenuActionTriggered.";
    // }
}


// ... (other slots remain the same) ...
void MainWindow::onPositionGo() {
    qDebug() << "Position Go clicked: X=" << (xCoordSpinBox_ ? xCoordSpinBox_->value() : -1)
             << "Y=" << (yCoordSpinBox_ ? yCoordSpinBox_->value() : -1) 
             << "Z=" << (zCoordSpinBox_ ? zCoordSpinBox_->value() : -1);
}
void MainWindow::onPositionXChanged(int value) {
    qDebug() << "Position X changed to:" << value;
}
void MainWindow::onPositionYChanged(int value) {
    qDebug() << "Position Y changed to:" << value;
}
void MainWindow::onPositionZChanged(int value) {
    qDebug() << "Position Z changed to:" << value;
    if(layerComboBox_ && value >=0 && value < layerComboBox_->count() && value != layerComboBox_->currentIndex()){
        layerComboBox_->setCurrentIndex(value); // Sync with layer combo box
    }
    updateCurrentLayer(value); // Update status bar
}
void MainWindow::onZoomControlChanged(int value) {
    qDebug() << "Zoom control changed to:" << value << "%";
    updateZoomLevel(static_cast<double>(value) / 100.0); // Update status bar
}
void MainWindow::onLayerControlChanged(int index) {
    qDebug() << "Layer control changed to index:" << index << "Text:" << (layerComboBox_ ? layerComboBox_->itemText(index) : "N/A");
    int floorValue = layerComboBox_ ? layerComboBox_->itemData(index).toInt() : index;
    if(zCoordSpinBox_ && floorValue != zCoordSpinBox_->value()){
        zCoordSpinBox_->setValue(floorValue); // Sync with Z spin box
    }
    updateCurrentLayer(floorValue); // Update status bar
}

// Automagic Settings Placeholder Implementations

void MainWindow::openAutomagicSettingsDialog() {
    AutomagicSettingsDialog dialog(this);

    // Load current settings into the dialog (using placeholder getters from MainWindow)
    dialog.setSettings(
        mainGetAutomagicEnabled(),
        mainGetSameGroundTypeBorderEnabled(),
        mainGetWallsRepelBordersEnabled(),
        mainGetLayerCarpetsEnabled(),
        mainGetBorderizeDeleteEnabled(),
        mainGetCustomBorderEnabled(),
        mainGetCustomBorderId()
    );

    if (dialog.exec() == QDialog::Accepted) {
        // Apply new settings from the dialog (using placeholder updater in MainWindow)
        mainUpdateAutomagicSettings(
            dialog.isAutomagicEnabled(),
            dialog.isSameGroundTypeBorderEnabled(),
            dialog.isWallsRepelBordersEnabled(),
            dialog.isLayerCarpetsEnabled(),
            dialog.isBorderizeDeleteEnabled(),
            dialog.isCustomBorderEnabled(),
            dialog.getCustomBorderId()
        );
    }
}

bool MainWindow::mainGetAutomagicEnabled() const { qDebug("MainWindow::mainGetAutomagicEnabled (stub)"); return false; }
bool MainWindow::mainGetSameGroundTypeBorderEnabled() const { qDebug("MainWindow::mainGetSameGroundTypeBorderEnabled (stub)"); return true; } // Default true based on common use
bool MainWindow::mainGetWallsRepelBordersEnabled() const { qDebug("MainWindow::mainGetWallsRepelBordersEnabled (stub)"); return true; } // Default true
bool MainWindow::mainGetLayerCarpetsEnabled() const { qDebug("MainWindow::mainGetLayerCarpetsEnabled (stub)"); return true; } // Default true
bool MainWindow::mainGetBorderizeDeleteEnabled() const { qDebug("MainWindow::mainGetBorderizeDeleteEnabled (stub)"); return false; }
bool MainWindow::mainGetCustomBorderEnabled() const { qDebug("MainWindow::mainGetCustomBorderEnabled (stub)"); return false; }
int MainWindow::mainGetCustomBorderId() const { qDebug("MainWindow::mainGetCustomBorderId (stub)"); return 1; }

void MainWindow::mainUpdateAutomagicSettings(bool automagicEnabled, bool sameGround, bool wallsRepel, bool layerCarpets, bool borderizeDelete, bool customBorder, int customBorderId) {
    qDebug() << "MainWindow::mainUpdateAutomagicSettings (stub) called with values:";
    qDebug() << "  Automagic:" << automagicEnabled;
    qDebug() << "  Same Ground:" << sameGround;
    qDebug() << "  Walls Repel:" << wallsRepel;
    qDebug() << "  Layer Carpets:" << layerCarpets;
    qDebug() << "  Borderize Delete:" << borderizeDelete;
    qDebug() << "  Custom Border:" << customBorder;
    qDebug() << "  Custom Border ID:" << customBorderId;
    
    // In a real scenario, this would update some internal state or call a settings manager
    // And then potentially trigger a refresh
    mainTriggerMapOrUIRefreshForAutomagic();
}

void MainWindow::mainTriggerMapOrUIRefreshForAutomagic() {
    qDebug() << "MainWindow::mainTriggerMapOrUIRefreshForAutomagic (stub) called.";
    // This would eventually call methods on MapView or other relevant UI components
}

// --- Clipboard Operation Handlers (Stubs) ---

void MainWindow::handleCopy() {
    Map* currentMap = getCurrentMap();
    Selection* currentSelection = currentMap ? currentMap->getSelection() : nullptr;

    if (currentMap && currentSelection && !currentSelection->isEmpty()) {
        if (internalClipboard_) {
            internalClipboard_->populateFromSelection(currentSelection->getSelectedTiles(), *currentMap);
            qDebug() << "MainWindow::handleCopy: Data copied to internal clipboard." << internalClipboard_->getTilesData().count() << "tiles.";
            
            // Future: Serialize and put on QClipboard
            // QByteArray jsonData = internalClipboard_->serializeToJson();
            // QApplication::clipboard()->setText(QString::fromUtf8(jsonData)); 
        } else {
            qWarning() << "MainWindow::handleCopy: internalClipboard_ is null.";
        }
    } else {
        qDebug() << "MainWindow::handleCopy: No map or selection, or selection empty.";
    }
}

void MainWindow::handleCut() {
    Map* currentMap = getCurrentMap();
    Selection* currentSelection = currentMap ? currentMap->getSelection() : nullptr;

    if (currentMap && currentSelection && !currentSelection->isEmpty()) {
        if (internalClipboard_) {
            internalClipboard_->populateFromSelection(currentSelection->getSelectedTiles(), *currentMap);
            qDebug() << "MainWindow::handleCut: Data copied to internal clipboard." << internalClipboard_->getTilesData().count() << "tiles.";

            // Future: Serialize and put on QClipboard (as in handleCopy)

            // Future: Delete the selected content from the map (this would involve creating an Action)
            qDebug() << "MainWindow::handleCut: Deletion of original selection from map is deferred.";
            // Example: editor->deleteSelectionAction(); currentSelection->clear();
        } else {
            qWarning() << "MainWindow::handleCut: internalClipboard_ is null.";
        }
    } else {
        qDebug() << "MainWindow::handleCut: No map or selection, or selection empty.";
    }
}

void MainWindow::handlePaste() {
    Map* currentMap = getCurrentMap();
    MapPos pasteTargetPosition = getPasteTargetPosition(); 

    if (currentMap && internalClipboard_ && !internalClipboard_->isEmpty()) {
        // Future: Get data from QClipboard if it's empty or newer
        // QByteArray clipboardJsonData = QApplication::clipboard()->text().toUtf8();
        // if (!clipboardJsonData.isEmpty()) { internalClipboard_->deserializeFromJson(clipboardJsonData); }
            
        qDebug() << "MainWindow::handlePaste: Pasting" << internalClipboard_->getTilesData().count() << "tiles from internal clipboard to map at (" << pasteTargetPosition.x << "," << pasteTargetPosition.y << "," << pasteTargetPosition.z << ").";
        // Future: Create a PasteAction using internalClipboard_->getTilesData() and pasteTargetPosition
        // Example: editor->pasteAction(internalClipboard_, pasteTargetPosition);
    } else {
        qDebug() << "MainWindow::handlePaste: No map or internal clipboard is empty/null.";
    }
}

bool MainWindow::canPaste() const {
    // Future: Check QClipboard as well
    return internalClipboard_ && !internalClipboard_->isEmpty();
}

// --- Stubbed Helper Methods for Clipboard ---

Map* MainWindow::getCurrentMap() const { 
    qDebug("MainWindow::getCurrentMap (stub) - returning nullptr for now."); 
    // In a real app, this would return a pointer to the currently active Map object.
    // For example: return currentMapDocument_ ? currentMapDocument_->getMap() : nullptr;
    return nullptr; 
}

MapPos MainWindow::getPasteTargetPosition() const { 
    qDebug("MainWindow::getPasteTargetPosition (stub) - returning (0,0,0) for now."); 
    // This should return the current cursor position on the map, or a designated paste target.
    return MapPos(0,0,0); 
}

// --- Status Bar Update Method Implementations ---

void MainWindow::updateMouseMapCoordinates(const QPointF& mapPos, int floor) {
    if (mouseCoordsLabel_) {
        // Round mapPos coordinates for display if they are very precise
        mouseCoordsLabel_->setText(QString("X: %1, Y: %2, Z: %3")
                                     .arg(qRound(mapPos.x()))
                                     .arg(qRound(mapPos.y()))
                                     .arg(floor));
    }
}

void MainWindow::updateZoomLevel(double zoom) {
    if (zoomLevelLabel_) {
        zoomLevelLabel_->setText(QString("Zoom: %1%").arg(static_cast<int>(zoom * 100)));
    }
}

void MainWindow::updateCurrentLayer(int layer) {
    if (currentLayerLabel_) {
        currentLayerLabel_->setText(QString("Floor: %1").arg(layer));
    }
    // This slot might also be connected to the layerComboBox_ valueChanged if needed
    // and zCoordSpinBox_ valueChanged.
}

void MainWindow::updateCurrentBrush(const QString& brushName) {
    if (brushInfoLabel_) {
        brushInfoLabel_->setText(QString("Brush: %1").arg(brushName.isEmpty() ? "None" : brushName));
    }
}

void MainWindow::updateSelectedItemInfo(const QString& itemInfo) {
    if (itemInfoLabel_) {
        itemInfoLabel_->setText(itemInfo.isEmpty() ? "Item: None" : itemInfo);
    }
}

void MainWindow::showTemporaryStatusMessage(const QString& message, int timeout) {
    statusBar()->showMessage(message, timeout);
}

// State Saving and Restoring
void MainWindow::closeEvent(QCloseEvent *event) {
    saveToolBarState();
    QMainWindow::closeEvent(event); // Important to call base class
}

void MainWindow::saveToolBarState() {
    qDebug() << "Saving MainWindow state (toolbars, docks)...";
    QSettings settings("IdlersMapEditor", "MainWindow"); // Using specific names

    settings.setValue("mainWindowGeometry", saveGeometry());
    settings.setValue("mainWindowState", saveState());

    if (standardToolBar_) settings.setValue("standardToolBarVisible", standardToolBar_->isVisible());
    if (brushesToolBar_) settings.setValue("brushesToolBarVisible", brushesToolBar_->isVisible());
    if (positionToolBar_) settings.setValue("positionToolBarVisible", positionToolBar_->isVisible());
    if (sizesToolBar_) settings.setValue("sizesToolBarVisible", sizesToolBar_->isVisible());

    // Save control values
    if (zoomSpinBox_) settings.setValue("zoomLevel", zoomSpinBox_->value());
    if (layerComboBox_) settings.setValue("currentLayer", layerComboBox_->currentIndex());
    if (xCoordSpinBox_) settings.setValue("posX", xCoordSpinBox_->value());
    if (yCoordSpinBox_) settings.setValue("posY", yCoordSpinBox_->value());
    if (zCoordSpinBox_) settings.setValue("posZ", zCoordSpinBox_->value());

    // Placeholder: Save selected brush/shape/size
    QString currentBrushName = "";
    if (brushGroup && brushGroup->checkedAction()) { // Assuming brushGroup is accessible, might need to be member or passed
         currentBrushName = brushGroup->checkedAction()->objectName();
    }
    settings.setValue("selectedBrush", currentBrushName);
    qDebug() << "Saving selected brush (objectName):" << currentBrushName;


    if (brushShapeActionGroup_ && brushShapeActionGroup_->checkedAction()) {
        settings.setValue("selectedBrushShape", brushShapeActionGroup_->checkedAction()->objectName());
        qDebug() << "Saving selected brush shape:" << brushShapeActionGroup_->checkedAction()->objectName();
    }
    if (brushSizeActionGroup_ && brushSizeActionGroup_->checkedAction()) {
        settings.setValue("selectedBrushSize", brushSizeActionGroup_->checkedAction()->objectName());
        qDebug() << "Saving selected brush size:" << brushSizeActionGroup_->checkedAction()->objectName();
    }
}

void MainWindow::restoreToolBarState() {
    qDebug() << "Attempting to restore MainWindow state (toolbars, docks)...";
    QSettings settings("IdlersMapEditor", "MainWindow"); // Using specific names

    QByteArray geometry = settings.value("mainWindowGeometry").toByteArray();
    QByteArray state = settings.value("mainWindowState").toByteArray();

    if (!geometry.isEmpty()) {
        restoreGeometry(geometry);
    }
    if (!state.isEmpty()) {
        if (!restoreState(state)) {
            qWarning() << "Failed to restore MainWindow state.";
        }
    } else {
        qDebug() << "No saved state found for MainWindow, using default layout.";
    }

    // Restore individual toolbar visibility AFTER restoreState, as restoreState might affect them.
    // The menu actions for these toolbars should also be updated.
    if (standardToolBar_) {
        bool visible = settings.value("standardToolBarVisible", true).toBool();
        standardToolBar_->setVisible(visible);
        // Assuming viewStandardToolBarAction_ exists and is the menu action for this
        // if (viewStandardToolBarAction_) viewStandardToolBarAction_->setChecked(visible);
        // This connection is handled in onMenuActionTriggered and when menu is created.
        // For direct restoration, ensure menu items reflect this.
        // This might be better handled by iterating menu actions or specific connect.
    }
    if (brushesToolBar_) {
        brushesToolBar_->setVisible(settings.value("brushesToolBarVisible", true).toBool());
    }
    if (positionToolBar_) {
        positionToolBar_->setVisible(settings.value("positionToolBarVisible", true).toBool());
    }
    if (sizesToolBar_) {
        sizesToolBar_->setVisible(settings.value("sizesToolBarVisible", true).toBool());
    }

    // Restore control values
    if (zoomSpinBox_) zoomSpinBox_->setValue(settings.value("zoomLevel", 100).toInt());
    if (layerComboBox_) layerComboBox_->setCurrentIndex(settings.value("currentLayer", 7).toInt());
    if (xCoordSpinBox_) xCoordSpinBox_->setValue(settings.value("posX", 0).toInt());
    if (yCoordSpinBox_) yCoordSpinBox_->setValue(settings.value("posY", 0).toInt());
    if (zCoordSpinBox_) zCoordSpinBox_->setValue(settings.value("posZ", 7).toInt()); // Default to 7 to match combobox

    // Placeholder: Restore selected brush/shape/size
    QString selectedBrushObjectName = settings.value("selectedBrush", "").toString();
    if (!selectedBrushObjectName.isEmpty() && brushGroup) { // Assuming brushGroup is member
        QList<QAction*> brushActions = brushGroup->actions();
        for (QAction* action : brushActions) {
            if (action->objectName() == selectedBrushObjectName) {
                action->setChecked(true);
                qDebug() << "Restored selected brush to:" << selectedBrushObjectName;
                updateCurrentBrush(action->text()); // Update status bar
                break;
            }
        }
    }

    QString selectedShapeObjectName = settings.value("selectedBrushShape", "TOGGLE_BRUSH_SHAPE_RECT").toString();
    if (brushShapeActionGroup_) {
         QList<QAction*> shapeActions = brushShapeActionGroup_->actions();
        for (QAction* action : shapeActions) {
            if (action->objectName() == selectedShapeObjectName) {
                action->setChecked(true);
                qDebug() << "Restored selected brush shape to:" << selectedShapeObjectName;
                break;
            }
        }
    }

    QString selectedSizeObjectName = settings.value("selectedBrushSize", "SET_BRUSH_SIZE_1").toString();
    if (brushSizeActionGroup_) {
        QList<QAction*> sizeActions = brushSizeActionGroup_->actions();
        for (QAction* action : sizeActions) {
            if (action->objectName() == selectedSizeObjectName) {
                action->setChecked(true);
                qDebug() << "Restored selected brush size to:" << selectedSizeObjectName;
                break;
            }
        }
    }

    // Update dock widget action check states after restoreState()
    // restoreState() should handle dock visibility, these lines ensure menu items are synced.
    if (viewPaletteDockAction_ && paletteDock_) viewPaletteDockAction_->setChecked(paletteDock_->isVisible());
    if (viewMinimapDockAction_ && minimapDock_) viewMinimapDockAction_->setChecked(minimapDock_->isVisible());
    if (viewPropertiesDockAction_ && propertiesDock_) viewPropertiesDockAction_->setChecked(propertiesDock_->isVisible());

    // Similarly for toolbars - ensure menu items reflect actual visibility
    // This requires access to the menu actions for toolbar visibility.
    // Example: if (viewStandardToolBarAction_) viewStandardToolBarAction_->setChecked(standardToolBar_->isVisible());
    // This is generally better handled by connecting the toolbar's visibilityChanged signal to the action's setChecked slot.
    // For now, this explicit update after restore is a simpler placeholder.
    // The creation of these menu items already sets their initial checked state.
    // The onMenuActionTriggered handles user changes. This is for programmatically restored state.
}
