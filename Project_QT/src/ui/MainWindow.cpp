#include "MainWindow.h"
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QKeySequence>
#include <QDebug>
#include <QMap>
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
#include <QVBoxLayout> // Added for QVBoxLayout in dockable views
#include "BrushPalettePanel.h"   // Renamed from PlaceholderPaletteWidget.h
#include "PlaceholderMinimapWidget.h"
#include "TilePropertyEditor.h"  // Renamed from PlaceholderPropertiesWidget.h
#include "Tile.h"                // For instantiating Tile in test slot
#include "Item.h"                // For instantiating Item in test slot
#include "ui/ReplaceItemsDialog.h" // For ReplaceItemsDialog
#include "AutomagicSettingsDialog.h" // Include for AutomagicSettingsDialog
// Task7: Basic dialog includes
#include "ui/GroundValidationDialog.h"
#include "ui/ImportMapDialog.h"
#include "ui/ExportMiniMapDialog.h"
#include "ui/GotoPositionDialog.h"
#include "ClipboardData.h"           // For internal clipboard
#include "Map.h"                     // For Map and MapPos
#include "Selection.h"               // For Selection
#include "SettingsManager.h"         // For settings management
#include "BorderSystem.h"            // For border system
#include "MenuActionHandler.h"       // For menu action handling
#include "StatusBarManager.h"        // For status bar management
#include "ToolBarManager.h"          // For toolbar management
#include "DialogManager.h"           // For dialog management
#include "PerspectiveManager.h"      // For perspective management
#include "MapView.h"                // For MapView
#include <QApplication>             // For future QClipboard access
#include <QClipboard>               // For future QClipboard access
#include <QtMath>                   // For qRound
#include <QSettings>                // For saving/restoring state
#include <QByteArray>               // For saving/restoring state
#include <QCloseEvent>              // For closeEvent
#include <QWidget>                  // For QWidget in dockable views
#include <QTabWidget>               // For tab management (Task 62)
#include <QVBoxLayout>              // For layout management (Task 62)
#include <QHBoxLayout>              // For layout management (Task 62)
#include <QSplitter>                // For splitter layout (Task 62)
#include <QTimer>                   // For auto-save timer (Task 62)
#include <QMessageBox>              // For close confirmation (Task 62)
// QDebug is already included via QAction or similar Qt headers usually, but explicit include is fine if needed


MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent),
    paletteCounter_(1),
    currentMapTabIndex_(-1),
    windowStateRestored_(false) {

    setWindowTitle(tr("Idler's Map Editor (Qt)"));
    resize(1280, 720);

    // Task 62: Initialize settings and state management
    settings_ = new QSettings("IdlerMapEditor", "MainWindow", this);
    autoSaveTimer_ = new QTimer(this);
    autoSaveTimer_->setInterval(30000); // Auto-save every 30 seconds
    connect(autoSaveTimer_, &QTimer::timeout, this, &MainWindow::onSaveWindowState);

    // Initialize settings and border system
    settingsManager_ = SettingsManager::getInstance();
    borderSystem_ = BorderSystem::getInstance();

    // Initialize menu action handler
    menuActionHandler_ = new MenuActionHandler(this, this);
    menuActionHandler_->setBorderSystem(borderSystem_);

    // Initialize status bar manager
    statusBarManager_ = new StatusBarManager(statusBar(), this);

    // Initialize toolbar manager
    toolBarManager_ = new ToolBarManager(this, this);

    // Initialize dialog manager
    dialogManager_ = new DialogManager(this, this);

    // Task 77: Initialize brush manager
    brushManager_ = new BrushManager(this);

    // Task 77: Connect BrushManager signals for UI synchronization
    connect(brushManager_, &BrushManager::currentBrushChanged,
            this, &MainWindow::onBrushManagerBrushChanged);
    connect(brushManager_, &BrushManager::actionIdChanged,
            this, &MainWindow::onBrushManagerActionIdChanged);
    connect(brushManager_, &BrushManager::selectedItemChanged,
            this, &MainWindow::onBrushManagerSelectedItemChanged);
    connect(brushManager_, &BrushManager::drawingModeChanged,
            this, &MainWindow::onBrushManagerDrawingModeChanged);

    // Task 77: Connect ToolBarManager signals for UI synchronization
    connect(toolBarManager_, &ToolBarManager::toolbarActionTriggered,
            this, &MainWindow::onToolbarActionTriggered);

    // Initialize perspective manager
    perspectiveManager_ = new PerspectiveManager(this, this);

    // Initialize core map components (placeholders for now)
    map_ = nullptr; // TODO: Initialize with actual map when available
    selection_ = nullptr; // TODO: Initialize with actual selection when available
    internalClipboard_ = new ClipboardData();

    // Initialize MapView (placeholder for now - will be properly initialized when map system is ready)
    mapView_ = nullptr; // TODO: Initialize with actual MapView when BrushManager and Map are available

    setupMenuBar();
    setupToolBars();
    setupCentralWidget(); // Task 62: Setup central widget with tab management
    setupDockWidgets(); // Call setupDockWidgets
    setupStatusBar();

    restoreToolBarState(); // Restore state after UI is setup
    loadPerspective(); // Load dock layout perspective
    onRestoreWindowState(); // Task 62: Restore window state
    autoSaveTimer_->start(); // Task 62: Start auto-save timer
    qDebug() << "MainWindow created. Menu, toolbars, central widget, and docks setup initiated. State restored.";
}

MainWindow::~MainWindow() {
    delete internalClipboard_;
    internalClipboard_ = nullptr;

    delete mapView_;
    mapView_ = nullptr;

    delete menuActionHandler_;
    menuActionHandler_ = nullptr;

    delete statusBarManager_;
    statusBarManager_ = nullptr;

    delete toolBarManager_;
    toolBarManager_ = nullptr;

    delete dialogManager_;
    dialogManager_ = nullptr;

    delete perspectiveManager_;
    perspectiveManager_ = nullptr;
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

    // Add Test Action for TilePropertyEditor to Experimental Menu
    QMenu* experimentalMenu = nullptr;
    QList<QMenu*> menus = menuBar_ ? menuBar_->findChildren<QMenu*>() : QList<QMenu*>();
    for(QMenu* menu : menus){
        if(menu->objectName() == QLatin1String("EXPERIMENTAL_MENU_PLACEHOLDER")){ // Assuming createExperimentalMenu sets an object name
             experimentalMenu = menu;
             break;
        } else if (menu->title() == tr("E&xperimental")) { // Fallback to title
            experimentalMenu = menu;
            break;
        }
    }

    if(!experimentalMenu && menuBar_){ // If still not found, and menuBar exists
        // Try to find it by looking at the last few menus if a specific object name wasn't set.
        // This is a bit fragile. Best to set an objectName in createExperimentalMenu.
        QList<QAction*> menuActions = menuBar_->actions(); // These are QActions that show the menu titles
        for(QAction* menuAction : menuActions) {
            if(menuAction->menu() && menuAction->menu()->title() == tr("E&xperimental")) {
                experimentalMenu = menuAction->menu();
                break;
            }
        }
    }

    if(experimentalMenu){
        experimentalMenu->addSeparator();
        QAction* testTilePropsAction = new QAction(tr("Test Update Tile Properties"), this);
        connect(testTilePropsAction, &QAction::triggered, this, &MainWindow::onTestUpdateTileProperties);
        experimentalMenu->addAction(testTilePropsAction);
    } else {
        qWarning() << "Could not find Experimental menu to add 'Test Update Tile Properties' action. Creating Debug menu.";
        QMenu* debugMenu = menuBar_ ? menuBar_->addMenu(tr("&Debug")) : nullptr;
        if(debugMenu) {
            QAction* testTilePropsAction = new QAction(tr("Test Update Tile Properties"), this);
            connect(testTilePropsAction, &QAction::triggered, this, &MainWindow::onTestUpdateTileProperties);
            debugMenu->addAction(testTilePropsAction);
        } else {
            qWarning() << "Could not add Test Tile Properties action to any menu.";
        }
    }

    qDebug() << "Menu bar setup complete with menus.";
}

void MainWindow::setupToolBars() {
    if (toolBarManager_) {
        toolBarManager_->setupToolBars();

        // Get references to toolbars for backward compatibility
        standardToolBar_ = toolBarManager_->getStandardToolBar();
        brushesToolBar_ = toolBarManager_->getBrushesToolBar();
        positionToolBar_ = toolBarManager_->getPositionToolBar();
        sizesToolBar_ = toolBarManager_->getSizesToolBar();

        // Get references to controls for backward compatibility
        zoomSpinBox_ = toolBarManager_->getZoomSpinBox();
        layerComboBox_ = toolBarManager_->getLayerComboBox();
        xCoordSpinBox_ = toolBarManager_->getXCoordSpinBox();
        yCoordSpinBox_ = toolBarManager_->getYCoordSpinBox();
        zCoordSpinBox_ = toolBarManager_->getZCoordSpinBox();

        // Connect toolbar manager signals to MainWindow slots
        connect(toolBarManager_, &ToolBarManager::zoomControlChanged,
                this, &MainWindow::onZoomControlChanged);
        connect(toolBarManager_, &ToolBarManager::layerControlChanged,
                this, &MainWindow::onLayerControlChanged);
        connect(toolBarManager_, &ToolBarManager::positionControlChanged,
                this, &MainWindow::onPositionGo);
        connect(toolBarManager_, &ToolBarManager::brushShapeActionTriggered,
                this, &MainWindow::onBrushShapeActionTriggered);
        connect(toolBarManager_, &ToolBarManager::brushSizeActionTriggered,
                this, &MainWindow::onBrushSizeActionTriggered);
        connect(toolBarManager_, &ToolBarManager::brushActionTriggered,
                this, &MainWindow::onBrushActionTriggered);
    } else {
        qWarning("MainWindow::setupToolBars: ToolBarManager is null!");
    }
}

void MainWindow::setupDockWidgets() {
    setDockNestingEnabled(true);

    // Palette Dock (Primary)
    paletteDock_ = new QDockWidget(tr("Palette"), this);
    paletteDock_->setObjectName(QStringLiteral("PaletteDock"));
    paletteDock_->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    BrushPalettePanel* paletteContent = new BrushPalettePanel("Palette", paletteDock_); // Renamed class
    paletteDock_->setWidget(paletteContent);
    addDockWidget(Qt::LeftDockWidgetArea, paletteDock_);
    paletteDock_->setVisible(true);

    // Task 77: Connect MainPalette signals for UI synchronization
    if (paletteContent) {
        // Connect action ID changes from palette to MainWindow
        connect(paletteContent, &BrushPalettePanel::actionIdChanged,
                this, &MainWindow::onMainPaletteActionIdChanged);

        // Connect brush selection from palette to MainWindow
        connect(paletteContent, &BrushPalettePanel::brushSelected,
                this, &MainWindow::onMainPaletteBrushSelected);
    }

    // Track the primary palette in our list
    paletteDocks_.append(paletteDock_);

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
    TilePropertyEditor* propertiesEditor = new TilePropertyEditor(propertiesDock_); // Renamed class, simplified constructor
    propertiesDock_->setWidget(propertiesEditor);
    addDockWidget(Qt::RightDockWidgetArea, propertiesDock_); 
    propertiesDock_->setVisible(true); 
    if (viewPropertiesDockAction_) {
        viewPropertiesDockAction_->setChecked(propertiesDock_->isVisible());
    }
    
    // Example of tabbing:
    // tabifyDockWidget(minimapDock_, propertiesDock_);

    qDebug() << "Dock widgets setup.";
}

void MainWindow::setupCentralWidget() {
    // Task 62: Setup central widget with tab management (replacing wxAuiNotebook)
    centralWidget_ = new QWidget(this);
    centralLayout_ = new QVBoxLayout(centralWidget_);
    centralLayout_->setContentsMargins(0, 0, 0, 0);
    centralLayout_->setSpacing(0);

    // Create tab widget for multiple map views (replaces wxAuiNotebook)
    mapTabWidget_ = new QTabWidget(centralWidget_);
    mapTabWidget_->setTabsClosable(true);
    mapTabWidget_->setMovable(true);
    mapTabWidget_->setDocumentMode(true);
    mapTabWidget_->setUsesScrollButtons(true);

    // Connect tab widget signals
    connect(mapTabWidget_, &QTabWidget::currentChanged,
            this, &MainWindow::onMapTabChanged);
    connect(mapTabWidget_, &QTabWidget::tabCloseRequested,
            this, &MainWindow::onMapTabCloseRequested);
    connect(mapTabWidget_, &QTabWidget::tabBarClicked,
            [this](int index) { setCurrentMapTab(index); });

    centralLayout_->addWidget(mapTabWidget_);
    setCentralWidget(centralWidget_);

    qDebug() << "Central widget with tab management setup complete.";
}

void MainWindow::setupStatusBar() {
    if (statusBarManager_) {
        statusBarManager_->setupStatusBar();
    } else {
        qWarning("MainWindow::setupStatusBar: StatusBarManager is null!");
    }
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

QAction* MainWindow::createActionWithId(MenuBar::ActionID actionId, const QString& text, const QIcon& icon, const QString& shortcut, const QString& statusTip, bool checkable, bool checked) {
    QAction *action = new QAction(tr(text.toStdString().c_str()), this);
    action->setObjectName(QString("ACTION_%1").arg(static_cast<int>(actionId))); // Set object name based on ID
    action->setIcon(icon);
    if (!shortcut.isEmpty()) {
        action->setShortcut(QKeySequence::fromString(tr(shortcut.toStdString().c_str())));
    }
    action->setStatusTip(tr(statusTip.toStdString().c_str()));
    action->setCheckable(checkable);
    action->setChecked(checked);

    // Store action in map for easy access
    actions_[actionId] = action;

    // Connect to centralized action handler
    connect(action, &QAction::triggered, [this, actionId]() {
        onActionTriggered(actionId);
    });

    return action;
}

QAction* MainWindow::getAction(MenuBar::ActionID actionId) const {
    return actions_.value(actionId, nullptr);
}

QMenu* MainWindow::createFileMenu() {
    QMenu *fileMenu = new QMenu(tr("&File"), this);
    // Use ActionID system for main file actions
    newAction_ = createActionWithId(MenuBar::NEW, "&New...", QIcon::fromTheme("document-new"), "Ctrl+N", "Create a new map.");
    fileMenu->addAction(newAction_);
    openAction_ = createActionWithId(MenuBar::OPEN, "&Open...", QIcon::fromTheme("document-open"), "Ctrl+O", "Open another map.");
    fileMenu->addAction(openAction_);
    saveAction_ = createActionWithId(MenuBar::SAVE, "&Save", QIcon::fromTheme("document-save"), "Ctrl+S", "Save the current map.");
    fileMenu->addAction(saveAction_);
    saveAsAction_ = createActionWithId(MenuBar::SAVE_AS, "Save &As...", QIcon::fromTheme("document-save-as"), "Ctrl+Shift+S", "Save the current map as a new file.");
    fileMenu->addAction(saveAsAction_);
    fileMenu->addAction(createActionWithId(MenuBar::GENERATE_MAP, "&Generate Map", QIcon(), "Ctrl+Shift+G", "Generate a new map."));
    fileMenu->addAction(createActionWithId(MenuBar::CLOSE, "&Close", QIcon::fromTheme("window-close"), "Ctrl+W", "Closes the currently open map."));
    fileMenu->addSeparator();
    QMenu *importMenu = fileMenu->addMenu(tr("&Import"));

    // Use ActionID system for import/export actions
    importMenu->addAction(createActionWithId(MenuBar::IMPORT_MAP, "Import &Map...", QIcon::fromTheme("document-import"), "", "Import map data from another map file"));
    importMenu->addAction(createActionWithId(MenuBar::IMPORT_MONSTERS, "Import &Monsters/NPC...", QIcon::fromTheme("document-import"), "", "Import either a monsters.xml file or a specific monster/NPC."));
    importMenu->addAction(createActionWithId(MenuBar::IMPORT_MINIMAP, "Import M&inimap...", QIcon::fromTheme("document-import"), "", "Import minimap data from an image file."));

    QMenu *exportMenu = fileMenu->addMenu(tr("&Export"));
    exportMenu->addAction(createActionWithId(MenuBar::EXPORT_MINIMAP, "Export &Minimap...", QIcon::fromTheme("document-export"), "", "Export minimap to an image file"));
    exportMenu->addAction(createActionWithId(MenuBar::EXPORT_TILESETS, "Export &Tilesets...", QIcon::fromTheme("document-export"), "", "Export tilesets to an xml file."));
    fileMenu->addSeparator();
    fileMenu->addAction(createActionWithId(MenuBar::RELOAD_DATA, "&Reload Data", QIcon::fromTheme("view-refresh"), "F5", "Reloads all data files."));
    fileMenu->addSeparator();

    // Recent files submenu (placeholder for now)
    QMenu *recentFilesMenu = fileMenu->addMenu(tr("Recent &Files"));
    recentFilesMenu->setObjectName(QStringLiteral("RECENT_FILES"));
    QAction* placeholderRecent = recentFilesMenu->addAction(tr("(No recent files)"));
    placeholderRecent->setEnabled(false);

    fileMenu->addSeparator();
    fileMenu->addAction(createActionWithId(MenuBar::PREFERENCES, "&Preferences...", QIcon::fromTheme("preferences-system"), "", "Configure the map editor."));
    fileMenu->addSeparator();
    fileMenu->addAction(createActionWithId(MenuBar::EXIT, "E&xit", QIcon::fromTheme("application-exit"), "Ctrl+Q", "Close the editor."));
    return fileMenu;
}

QMenu* MainWindow::createEditMenu() {
    QMenu *editMenu = new QMenu(tr("&Edit"), this);
    // Use ActionID system for main edit actions
    undoAction_ = createActionWithId(MenuBar::UNDO, "&Undo", QIcon::fromTheme("edit-undo"), "Ctrl+Z", "Undo last action.");
    editMenu->addAction(undoAction_);
    redoAction_ = createActionWithId(MenuBar::REDO, "&Redo", QIcon::fromTheme("edit-redo"), "Ctrl+Y", "Redo last undid action.");
    editMenu->addAction(redoAction_);
    editMenu->addSeparator();
    // Find and Replace actions (matching wxWidgets structure)
    editMenu->addAction(createActionWithId(MenuBar::FIND_ITEM, "&Find Item...", QIcon::fromTheme("edit-find"), "Ctrl+F", "Find all instances of an item type on the map."));
    editMenu->addAction(createActionWithId(MenuBar::FIND_CREATURE, "Find &Creature...", QIcon::fromTheme("edit-find"), "Ctrl+Shift+C", "Find all instances of a creature on the map."));
    editMenu->addAction(createActionWithId(MenuBar::REPLACE_ITEMS, "&Replace Items...", QIcon::fromTheme("edit-find-replace"), "Ctrl+H", "Replaces all occurrences of one item with another."));
    editMenu->addAction(createActionWithId(MenuBar::REFRESH_ITEMS, "Refresh Items", QIcon::fromTheme("view-refresh"), "", "Refresh items to fix flags"));
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
    otherOptionsMenu->addSeparator();

    // Task7: Add Ground Validation dialog to Other Options menu
    QAction* groundValidationAction = new QAction(tr("&Ground Validation..."), this);
    groundValidationAction->setObjectName("GROUND_VALIDATION_ACTION");
    groundValidationAction->setStatusTip(tr("Validate and fix ground tile issues"));
    connect(groundValidationAction, &QAction::triggered, this, &MainWindow::onShowGroundValidationDialog);
    otherOptionsMenu->addAction(groundValidationAction);
    editMenu->addSeparator();
    // Use ActionID system for clipboard actions
    cutAction_ = createActionWithId(MenuBar::CUT, "Cu&t", QIcon::fromTheme("edit-cut"), "Ctrl+X", "Cut a part of the map.");
    editMenu->addAction(cutAction_);
    copyAction_ = createActionWithId(MenuBar::COPY, "&Copy", QIcon::fromTheme("edit-copy"), "Ctrl+C", "Copy a part of the map.");
    editMenu->addAction(copyAction_);
    pasteAction_ = createActionWithId(MenuBar::PASTE, "&Paste", QIcon::fromTheme("edit-paste"), "Ctrl+V", "Paste a part of the map.");
    editMenu->addAction(pasteAction_);
    editMenu->addSeparator();

    QAction* replaceItemsAction = new QAction(tr("Find/Replace Items..."), this);
    replaceItemsAction->setObjectName("REPLACE_ITEMS_DIALOG_ACTION");
    replaceItemsAction->setIcon(QIcon::fromTheme("edit-find-replace")); // Standard icon
    replaceItemsAction->setStatusTip(tr("Open the Find and Replace Items dialog."));
    // replaceItemsAction->setShortcut(QKeySequence(tr("Ctrl+Shift+R"))); // Optional shortcut
    connect(replaceItemsAction, &QAction::triggered, this, &MainWindow::onShowReplaceItemsDialog);
    editMenu->addAction(replaceItemsAction);

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
    QAction* zoomInAction = createActionWithId(MenuBar::ZOOM_IN, "Zoom &In", QIcon::fromTheme("zoom-in"), "Ctrl+=", "Increase the zoom.");
    zoomMenu->addAction(zoomInAction);
    QAction* zoomOutAction = createActionWithId(MenuBar::ZOOM_OUT, "Zoom &Out", QIcon::fromTheme("zoom-out"), "Ctrl+-", "Decrease the zoom.");
    zoomMenu->addAction(zoomOutAction);
    QAction* zoomNormalAction = createActionWithId(MenuBar::ZOOM_NORMAL, "Zoom &Normal", QIcon::fromTheme("zoom-original"), "Ctrl+0", "Normal zoom(100%).");
    zoomMenu->addAction(zoomNormalAction);
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
    // Use ActionID system for navigation actions
    menu->addAction(createActionWithId(MenuBar::GOTO_PREVIOUS_POSITION, "Go to &Previous Position", QIcon::fromTheme("go-previous"), "P", "Go to the previous screen center position."));
    menu->addAction(createActionWithId(MenuBar::GOTO_POSITION, "&Go to Position...", QIcon::fromTheme("go-jump"), "Ctrl+G", "Navigate to a specific map position"));
    menu->addSeparator();

    // Floor submenu with ActionID system
    QMenu *floorMenu = menu->addMenu(tr("&Floor"));
    QActionGroup* floorGroup = new QActionGroup(this);
    floorGroup->setExclusive(true);
    for (int i = 0; i <= 15; ++i) {
        MenuBar::ActionID floorActionId = static_cast<MenuBar::ActionID>(static_cast<int>(MenuBar::FLOOR_0) + i);
        QString shortcut = (i <= 9) ? QString::number(i) : ""; // Shortcuts 0-9 for floors 0-9
        QAction* floorAction = createActionWithId(floorActionId, QString("Floor %1").arg(i), QIcon(), shortcut, QString("Switch to floor %1").arg(i), true);
        floorMenu->addAction(floorAction);
        floorGroup->addAction(floorAction);
        if (i == 7) floorAction->setChecked(true); // Ground floor default
    }
    return menu;
}

QMenu* MainWindow::createWindowMenu() { 
    QMenu* menu = new QMenu(tr("&Window"), this);

    viewPaletteDockAction_ = createActionWithId(MenuBar::VIEW_PALETTE_DOCK, "Palette Panel", QIcon(), "", "Show or hide the Palette panel", true, true);
    menu->addAction(viewPaletteDockAction_);
    viewMinimapDockAction_ = createActionWithId(MenuBar::VIEW_MINIMAP_DOCK, "Minimap Panel", QIcon(), "", "Show or hide the Minimap panel", true, true);
    menu->addAction(viewMinimapDockAction_);
    viewPropertiesDockAction_ = createActionWithId(MenuBar::VIEW_PROPERTIES_DOCK, "Properties Panel", QIcon(), "", "Show or hide the Properties panel", true, true);
    menu->addAction(viewPropertiesDockAction_);
    menu->addSeparator();
    menu->addAction(createActionWithId(MenuBar::NEW_PALETTE, "&New Palette", QIcon::fromTheme("document-new"), "Ctrl+Shift+P", "Creates a new palette."));
    menu->addAction(createActionWithId(MenuBar::DESTROY_PALETTE, "&Destroy Palette", QIcon::fromTheme("window-close"), "", "Destroy the current palette window"));
    menu->addSeparator();

    // Dockable views
    menu->addAction(createActionWithId(MenuBar::NEW_DOCKABLE_VIEW, "New &Dockable View", QIcon::fromTheme("view-split-left-right"), "", "Create a new dockable map view"));
    menu->addAction(createActionWithId(MenuBar::CLOSE_DOCKABLE_VIEWS, "&Close Dockable Views", QIcon::fromTheme("window-close"), "", "Close all dockable map views"));
    menu->addSeparator();

    QMenu *paletteMenu = menu->addMenu(tr("&Palette"));
    paletteMenu->addAction(createAction("&Terrain", "SELECT_TERRAIN", QIcon(), QKeySequence("T"), "Select the Terrain palette."));
    paletteMenu->addAction(createAction("&Doodad", "SELECT_DOODAD", QIcon(), QKeySequence("D"), "Select the Doodad palette."));
    menu->addSeparator(); 

    QMenu *toolbarsMenu = menu->addMenu(tr("&Toolbars"));
    toolbarsMenu->addAction(createActionWithId(MenuBar::VIEW_TOOLBARS_BRUSHES, "&Brushes", QIcon(), "", "Show or hide the Brushes toolbar", true, true));
    toolbarsMenu->addAction(createActionWithId(MenuBar::VIEW_TOOLBARS_POSITION, "&Position", QIcon(), "", "Show or hide the Position toolbar", true, true));
    toolbarsMenu->addAction(createActionWithId(MenuBar::VIEW_TOOLBARS_SIZES, "&Sizes", QIcon(), "", "Show or hide the Sizes toolbar", true, true));
    toolbarsMenu->addAction(createActionWithId(MenuBar::VIEW_TOOLBARS_STANDARD, "&Standard", QIcon(), "", "Show or hide the Standard toolbar", true, true));
    menu->addSeparator();

    // Perspective management
    QMenu* perspectiveMenu = menu->addMenu(tr("&Perspective"));
    perspectiveMenu->addAction(createActionWithId(MenuBar::SAVE_PERSPECTIVE, "&Save Perspective", QIcon::fromTheme("document-save"), "", "Save the current layout perspective"));
    perspectiveMenu->addAction(createActionWithId(MenuBar::LOAD_PERSPECTIVE, "&Load Perspective", QIcon::fromTheme("document-open"), "", "Load the saved layout perspective"));
    perspectiveMenu->addAction(createActionWithId(MenuBar::RESET_PERSPECTIVE, "&Reset Perspective", QIcon::fromTheme("view-restore"), "", "Reset layout to default perspective"));

    return menu;
}

QMenu* MainWindow::createExperimentalMenu() {
    QMenu* menu = new QMenu(tr("E&xperimental"), this);
    menu->addAction(createAction("&Fog in light view", "EXPERIMENTAL_FOG", QIcon(), "", "Apply fog filter to light effect.", true));
    return menu;
}

QMenu* MainWindow::createAboutMenu() {
    QMenu* menu = new QMenu(tr("A&bout"), this);
    // Use ActionID system for About menu
    menu->addAction(createActionWithId(MenuBar::EXTENSIONS, "E&xtensions...", QIcon::fromTheme("system-extensions"), "F2", "Manage editor extensions"));
    menu->addAction(createActionWithId(MenuBar::GOTO_WEBSITE, "&Goto Website", QIcon::fromTheme("web-browser"), "F3", "Visit the project website"));
    menu->addAction(createActionWithId(MenuBar::SHOW_HOTKEYS, "&Hotkeys", QIcon::fromTheme("help-keyboard-shortcuts"), "F6", "Show keyboard shortcuts"));
    menu->addSeparator();
    menu->addAction(createActionWithId(MenuBar::ABOUT, "&About...", QIcon::fromTheme("help-about"), "F1", "About this application"));
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
    if (dialogManager_) {
        dialogManager_->showAutomagicSettingsDialog();
    }
}

bool MainWindow::mainGetAutomagicEnabled() const {
    return settingsManager_->isAutomagicEnabled();
}

bool MainWindow::mainGetSameGroundTypeBorderEnabled() const {
    return settingsManager_->isSameGroundTypeBorderEnabled();
}

bool MainWindow::mainGetWallsRepelBordersEnabled() const {
    return settingsManager_->isWallsRepelBordersEnabled();
}

bool MainWindow::mainGetLayerCarpetsEnabled() const {
    return settingsManager_->isLayerCarpetsEnabled();
}

bool MainWindow::mainGetBorderizeDeleteEnabled() const {
    return settingsManager_->isBorderizeDeleteEnabled();
}

bool MainWindow::mainGetCustomBorderEnabled() const {
    return settingsManager_->isCustomBorderEnabled();
}

int MainWindow::mainGetCustomBorderId() const {
    return settingsManager_->getCustomBorderId();
}

void MainWindow::mainUpdateAutomagicSettings(bool automagicEnabled, bool sameGround, bool wallsRepel, bool layerCarpets, bool borderizeDelete, bool customBorder, int customBorderId) {
    qDebug() << "MainWindow::mainUpdateAutomagicSettings called with values:";
    qDebug() << "  Automagic:" << automagicEnabled;
    qDebug() << "  Same Ground:" << sameGround;
    qDebug() << "  Walls Repel:" << wallsRepel;
    qDebug() << "  Layer Carpets:" << layerCarpets;
    qDebug() << "  Borderize Delete:" << borderizeDelete;
    qDebug() << "  Custom Border:" << customBorder;
    qDebug() << "  Custom Border ID:" << customBorderId;

    // Update settings through SettingsManager
    settingsManager_->setAutomagicEnabled(automagicEnabled);
    settingsManager_->setSameGroundTypeBorderEnabled(sameGround);
    settingsManager_->setWallsRepelBordersEnabled(wallsRepel);
    settingsManager_->setLayerCarpetsEnabled(layerCarpets);
    settingsManager_->setBorderizeDeleteEnabled(borderizeDelete);
    settingsManager_->setCustomBorderEnabled(customBorder);
    settingsManager_->setCustomBorderId(customBorderId);

    // Save settings to disk
    settingsManager_->saveSettings();

    // Update status bar
    QString statusMessage = automagicEnabled ? "Automagic enabled." : "Automagic disabled.";
    showTemporaryStatusMessage(statusMessage, 3000);

    // Trigger refresh
    mainTriggerMapOrUIRefreshForAutomagic();
}

void MainWindow::mainTriggerMapOrUIRefreshForAutomagic() {
    qDebug() << "MainWindow::mainTriggerMapOrUIRefreshForAutomagic called.";

    // Update border system with new settings
    borderSystem_->updateFromSettings();

    // TODO: Trigger map view refresh when MapView is implemented
    // TODO: Update any UI elements that depend on automagic settings
    // TODO: Refresh palette if needed

    qDebug() << "Automagic settings refresh completed.";
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
    if (statusBarManager_) {
        statusBarManager_->updateMouseMapCoordinates(mapPos, floor);
    }
}

void MainWindow::updateZoomLevel(double zoom) {
    if (statusBarManager_) {
        statusBarManager_->updateZoomLevel(zoom);
    }
}

void MainWindow::updateCurrentLayer(int layer) {
    if (statusBarManager_) {
        statusBarManager_->updateCurrentLayer(layer);
    }
    // This slot might also be connected to the layerComboBox_ valueChanged if needed
    // and zCoordSpinBox_ valueChanged.
}

void MainWindow::updateCurrentBrush(const QString& brushName) {
    if (statusBarManager_) {
        statusBarManager_->updateCurrentBrush(brushName);
    }
}

// --- Toolbar State Management Methods ---

void MainWindow::updateToolbarStates() {
    if (toolBarManager_) {
        toolBarManager_->updateToolbarStates();
    }
}

void MainWindow::updateStandardToolbarStates() {
    if (toolBarManager_) {
        toolBarManager_->updateStandardToolbarStates();
    }
}

void MainWindow::updateBrushToolbarStates() {
    if (toolBarManager_) {
        toolBarManager_->updateBrushToolbarStates();
    }
}

// --- Dock Widget Management Methods ---

void MainWindow::createNewPalette() {
    if (perspectiveManager_) {
        perspectiveManager_->createNewPalette();
    }
}

void MainWindow::destroyCurrentPalette() {
    if (perspectiveManager_) {
        perspectiveManager_->destroyCurrentPalette();
    }
}

void MainWindow::createDockableMapView() {
    if (perspectiveManager_) {
        perspectiveManager_->createDockableMapView();
    }
}

void MainWindow::closeDockableViews() {
    if (perspectiveManager_) {
        perspectiveManager_->closeDockableViews();
    }
}

// --- Perspective Management Methods ---

void MainWindow::savePerspective() {
    if (perspectiveManager_) {
        perspectiveManager_->savePerspective();
    }
}

void MainWindow::loadPerspective() {
    if (perspectiveManager_) {
        perspectiveManager_->loadPerspective();
    }
}

void MainWindow::resetPerspective() {
    if (perspectiveManager_) {
        perspectiveManager_->resetPerspective();
    }
}

void MainWindow::updateSelectedItemInfo(const QString& itemInfo) {
    if (statusBarManager_) {
        statusBarManager_->updateSelectedItemInfo(itemInfo);
    }
}

void MainWindow::updateSelectedItemInfo(const Item* item) {
    if (statusBarManager_) {
        statusBarManager_->updateSelectedItemInfo(item);
    }
}

void MainWindow::updateStatusBarProgress(const QString& operation, int progress) {
    if (statusBarManager_) {
        statusBarManager_->updateProgress(operation, progress);
    }
}

void MainWindow::handleStatusUpdateRequest(const QString& type, const QVariantMap& data) {
    if (statusBarManager_) {
        statusBarManager_->handleStatusUpdateRequest(type, data);
    }
}

void MainWindow::showTemporaryStatusMessage(const QString& message, int timeout) {
    if (statusBarManager_) {
        statusBarManager_->showTemporaryMessage(message, timeout);
    }
}

// State Saving and Restoring
void MainWindow::closeEvent(QCloseEvent *event) {
    // Task 62: Enhanced close event handling with tab management

    // Check for unsaved changes in all open maps
    bool hasUnsavedChanges = false;
    QStringList unsavedMaps;

    for (int i = 0; i < getMapTabCount(); ++i) {
        MapView* mapView = getMapViewAt(i);
        if (mapView) {
            // TODO: Check if map has unsaved changes
            // For now, assume no unsaved changes
            bool mapModified = false; // mapView->isModified();
            if (mapModified) {
                hasUnsavedChanges = true;
                unsavedMaps.append(getMapTabTitle(i));
            }
        }
    }

    // If there are unsaved changes, ask user what to do
    if (hasUnsavedChanges) {
        QString message;
        if (unsavedMaps.size() == 1) {
            message = tr("The map '%1' has unsaved changes. Do you want to save before closing?")
                     .arg(unsavedMaps.first());
        } else {
            message = tr("The following maps have unsaved changes:\n%1\n\nDo you want to save them before closing?")
                     .arg(unsavedMaps.join("\n"));
        }

        QMessageBox::StandardButton reply = QMessageBox::question(
            this,
            tr("Close Application"),
            message,
            QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel,
            QMessageBox::Save
        );

        switch (reply) {
            case QMessageBox::Save:
                // TODO: Implement save all functionality
                qDebug() << "Save all requested before closing application";
                break;
            case QMessageBox::Discard:
                // Continue with closing
                break;
            case QMessageBox::Cancel:
                event->ignore();
                return;
            default:
                event->ignore();
                return;
        }
    }

    // Stop auto-save timer
    if (autoSaveTimer_) {
        autoSaveTimer_->stop();
    }

    // Save window state
    onSaveWindowState();

    // Save toolbar and dock states (existing functionality)
    saveToolBarState();
    savePerspective(); // Save dock layout perspective

    // Close all map tabs
    while (getMapTabCount() > 0) {
        removeMapTab(0);
    }

    // Accept the close event
    event->accept();

    qDebug() << "Application closing - all states saved and tabs closed";
}

void MainWindow::saveToolBarState() {
    qDebug() << "Saving MainWindow state (toolbars, docks)...";
    QSettings settings("IdlersMapEditor", "MainWindow"); // Using specific names

    settings.setValue("mainWindowGeometry", saveGeometry());
    settings.setValue("mainWindowState", saveState());

    // Task 46: Use ToolBarManager to save toolbar state
    if (toolBarManager_) {
        toolBarManager_->saveToolBarState();
    }

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

void MainWindow::onTestUpdateTileProperties() {
    // Create a couple of static dummy Tile objects for testing persistence across calls
    static Tile testTile1(100, 200, 7); // Parentless for this static test case
    static bool tile1Initialized = false;
    if (!tile1Initialized) {
        testTile1.setHouseId(123);
        testTile1.setPZ(true);
        testTile1.addZoneId(10);
        testTile1.addZoneId(15);

        Item* dummyGround = new Item(357); // Example ground item ID
        // dummyGround->setParent(&testTile1); // This would make Tile a QObject, which it is now.
        testTile1.setGround(dummyGround); // Tile takes ownership if designed so. Current Tile destructor deletes ground_.

        // testTile1.setMapFlag(TileMapFlags::Modified, true); // Assuming TileMapFlags is the enum type
        // testTile1.setMapFlag(TileMapFlags::Selected, true);
        testTile1.setModified(true); // Use existing setters if available
        testTile1.setSelected(true); // Use existing setters if available


        testTile1.setStateFlag(Tile::TileStateFlag::HasTable, true); // Example state flag
        tile1Initialized = true;
        qDebug() << "Initialized Test Tile 1";
    }

    static Tile testTile2(55, 65, 6);
    static bool tile2Initialized = false;
    if (!tile2Initialized) {
        testTile2.setNoPVP(true);
        testTile2.addZoneId(99);
        // No ground for this one to test itemCount variation
        testTile2.setStateFlag(Tile::TileStateFlag::HasCarpet, true);
        tile2Initialized = true;
        qDebug() << "Initialized Test Tile 2";
    }

    // Alternate between testTile1, testTile2, and nullptr to test different states
    static int testState = 0;
    Tile* tileToDisplay = nullptr;
    switch (testState) {
        case 0: tileToDisplay = &testTile1; qDebug() << "Testing with Tile 1"; break;
        case 1: tileToDisplay = &testTile2; qDebug() << "Testing with Tile 2"; break;
        case 2: tileToDisplay = nullptr;    qDebug() << "Testing with nullptr Tile"; break;
    }
    testState = (testState + 1) % 3;

    if (propertiesDock_) {
        TilePropertyEditor* propertiesEditor = qobject_cast<TilePropertyEditor*>(propertiesDock_->widget());
        if (propertiesEditor) {
            qDebug() << "MainWindow: Calling displayTileProperties.";
            propertiesEditor->displayTileProperties(tileToDisplay);
        } else {
            qWarning() << "MainWindow: Properties dock widget is not a TilePropertyEditor instance.";
        }
    } else {
        qWarning() << "MainWindow: propertiesDock_ is null.";
    }
}

void MainWindow::onShowReplaceItemsDialog() {
    if (dialogManager_) {
        dialogManager_->showReplaceItemsDialog();
    }
}

// Task7: Basic dialog implementations
void MainWindow::onShowGroundValidationDialog() {
    if (dialogManager_) {
        dialogManager_->showGroundValidationDialog();
    }
}

void MainWindow::onShowImportMapDialog() {
    if (dialogManager_) {
        dialogManager_->showImportMapDialog();
    }
}

void MainWindow::onShowExportMiniMapDialog() {
    if (dialogManager_) {
        dialogManager_->showExportMiniMapDialog();
    }
}

void MainWindow::onShowGotoPositionDialog() {
    if (dialogManager_) {
        dialogManager_->showGotoPositionDialog();
    }
}

// Centralized action handler for MenuBar::ActionID - now delegates to MenuActionHandler
void MainWindow::onActionTriggered(MenuBar::ActionID actionId) {
    if (menuActionHandler_) {
        menuActionHandler_->handleAction(actionId);
    } else {
        qWarning() << "MainWindow::onActionTriggered: MenuActionHandler is null!";
    }
}

// Accessor method for MapView
MapView* MainWindow::getMapView() const {
    return mapView_;
}

// MapView integration methods
void MainWindow::setMapView(MapView* mapView) {
    if (mapView_ != mapView) {
        mapView_ = mapView;
        connectMapViewToStatusBar();
    }
}

void MainWindow::connectMapViewToStatusBar() {
    if (mapView_ && statusBarManager_) {
        // Connect MapView status update signal to MainWindow status update handler
        connect(mapView_, &MapView::statusUpdateRequested,
                this, &MainWindow::handleStatusUpdateRequest);

        qDebug() << "MapView connected to status bar for status updates";
    }
}

// Task 62: Tab management implementation
int MainWindow::addMapTab(MapView* mapView, const QString& title) {
    if (!mapView || !mapTabWidget_) {
        qWarning() << "MainWindow::addMapTab: Invalid mapView or mapTabWidget";
        return -1;
    }

    // Add to tab widget
    int index = mapTabWidget_->addTab(mapView, title);

    // Add to our list
    mapViews_.append(mapView);

    // Connect MapView signals for panel communication
    connect(mapView, &MapView::statusUpdateRequested,
            this, &MainWindow::handleStatusUpdateRequest);

    // Set as current tab
    mapTabWidget_->setCurrentIndex(index);
    currentMapTabIndex_ = index;

    // Emit signals
    emit mapTabAdded(index);
    emit activeMapChanged(mapView);

    qDebug() << "Added map tab at index" << index << "with title:" << title;
    return index;
}

void MainWindow::removeMapTab(int index) {
    if (!mapTabWidget_ || index < 0 || index >= mapTabWidget_->count()) {
        qWarning() << "MainWindow::removeMapTab: Invalid index" << index;
        return;
    }

    // Get the MapView before removing
    MapView* mapView = getMapViewAt(index);

    // Remove from tab widget
    mapTabWidget_->removeTab(index);

    // Remove from our list
    if (index < mapViews_.size()) {
        mapViews_.removeAt(index);
    }

    // Update current index
    if (mapTabWidget_->count() == 0) {
        currentMapTabIndex_ = -1;
        emit activeMapChanged(nullptr);
    } else {
        currentMapTabIndex_ = mapTabWidget_->currentIndex();
        emit activeMapChanged(getCurrentMapView());
    }

    // Emit signal
    emit mapTabRemoved(index);

    // Clean up MapView if needed
    if (mapView) {
        mapView->disconnect(this);
    }

    qDebug() << "Removed map tab at index" << index;
}

void MainWindow::setCurrentMapTab(int index) {
    if (!mapTabWidget_ || index < 0 || index >= mapTabWidget_->count()) {
        qWarning() << "MainWindow::setCurrentMapTab: Invalid index" << index;
        return;
    }

    mapTabWidget_->setCurrentIndex(index);
    currentMapTabIndex_ = index;

    MapView* mapView = getCurrentMapView();
    emit activeMapChanged(mapView);

    qDebug() << "Set current map tab to index" << index;
}

int MainWindow::getCurrentMapTabIndex() const {
    return mapTabWidget_ ? mapTabWidget_->currentIndex() : -1;
}

MapView* MainWindow::getCurrentMapView() const {
    int index = getCurrentMapTabIndex();
    return getMapViewAt(index);
}

MapView* MainWindow::getMapViewAt(int index) const {
    if (!mapTabWidget_ || index < 0 || index >= mapTabWidget_->count()) {
        return nullptr;
    }

    QWidget* widget = mapTabWidget_->widget(index);
    return qobject_cast<MapView*>(widget);
}

int MainWindow::getMapTabCount() const {
    return mapTabWidget_ ? mapTabWidget_->count() : 0;
}

void MainWindow::setMapTabTitle(int index, const QString& title) {
    if (!mapTabWidget_ || index < 0 || index >= mapTabWidget_->count()) {
        qWarning() << "MainWindow::setMapTabTitle: Invalid index" << index;
        return;
    }

    mapTabWidget_->setTabText(index, title);
    emit mapTabTitleChanged(index, title);

    qDebug() << "Set map tab title at index" << index << "to:" << title;
}

QString MainWindow::getMapTabTitle(int index) const {
    if (!mapTabWidget_ || index < 0 || index >= mapTabWidget_->count()) {
        return QString();
    }

    return mapTabWidget_->tabText(index);
}

void MainWindow::cycleMapTabs(bool forward) {
    if (!mapTabWidget_ || mapTabWidget_->count() <= 1) {
        return;
    }

    int currentIndex = mapTabWidget_->currentIndex();
    int newIndex;

    if (forward) {
        newIndex = (currentIndex + 1) % mapTabWidget_->count();
    } else {
        newIndex = (currentIndex - 1 + mapTabWidget_->count()) % mapTabWidget_->count();
    }

    setCurrentMapTab(newIndex);
    qDebug() << "Cycled map tabs from" << currentIndex << "to" << newIndex;
}

// Task 62: Tab management slots implementation
void MainWindow::onMapTabChanged(int index) {
    if (index == currentMapTabIndex_) {
        return; // No change
    }

    currentMapTabIndex_ = index;
    MapView* mapView = getCurrentMapView();

    // Update UI elements based on new active tab/map
    if (mapView) {
        // Update status bar with current map info
        if (statusBarManager_) {
            statusBarManager_->updateMapInfo(mapView);
        }

        // Update palettes and other panels
        emit activeMapChanged(mapView);

        // Update layer controls and other UI elements
        if (toolBarManager_) {
            toolBarManager_->updateForMapView(mapView);
        }
    }

    emit currentMapTabChanged(index);
    qDebug() << "Map tab changed to index" << index;
}

void MainWindow::onMapTabCloseRequested(int index) {
    if (index < 0 || index >= getMapTabCount()) {
        return;
    }

    MapView* mapView = getMapViewAt(index);
    if (!mapView) {
        removeMapTab(index);
        return;
    }

    // Check if map has unsaved changes
    bool hasUnsavedChanges = false; // TODO: Implement map modification checking

    if (hasUnsavedChanges) {
        QMessageBox::StandardButton reply = QMessageBox::question(
            this,
            tr("Close Map"),
            tr("The map '%1' has unsaved changes. Do you want to save before closing?")
                .arg(getMapTabTitle(index)),
            QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel,
            QMessageBox::Save
        );

        switch (reply) {
            case QMessageBox::Save:
                // TODO: Implement save functionality
                qDebug() << "Save requested before closing tab" << index;
                break;
            case QMessageBox::Discard:
                // Continue with closing
                break;
            case QMessageBox::Cancel:
                return; // Don't close
            default:
                return;
        }
    }

    removeMapTab(index);
}

void MainWindow::onMapTabMoved(int from, int to) {
    // Update our internal list to match the tab widget order
    if (from >= 0 && from < mapViews_.size() && to >= 0 && to < mapViews_.size()) {
        mapViews_.move(from, to);
        qDebug() << "Map tab moved from" << from << "to" << to;
    }
}

void MainWindow::onNewMapTab() {
    // TODO: Create new map and MapView
    qDebug() << "New map tab requested - placeholder implementation";

    // For now, create a placeholder tab
    QWidget* placeholder = new QWidget();
    placeholder->setStyleSheet("background-color: #f0f0f0;");

    int index = mapTabWidget_->addTab(placeholder, tr("New Map %1").arg(getMapTabCount() + 1));
    mapTabWidget_->setCurrentIndex(index);

    emit mapTabAdded(index);
}

void MainWindow::onCloseCurrentMapTab() {
    int currentIndex = getCurrentMapTabIndex();
    if (currentIndex >= 0) {
        onMapTabCloseRequested(currentIndex);
    }
}

void MainWindow::onCloseAllMapTabs() {
    while (getMapTabCount() > 0) {
        onMapTabCloseRequested(0);
    }
}

void MainWindow::onNextMapTab() {
    cycleMapTabs(true);
}

void MainWindow::onPreviousMapTab() {
    cycleMapTabs(false);
}

// Task 62: Panel communication slots
void MainWindow::onPanelBrushChanged(const QString& brushName) {
    emit activeBrushChanged(brushName);

    // Update current MapView if needed
    MapView* currentMapView = getCurrentMapView();
    if (currentMapView) {
        // TODO: Update MapView brush selection
        qDebug() << "Panel brush changed to:" << brushName;
    }
}

void MainWindow::onPanelLayerChanged(int layer) {
    emit activeLayerChanged(layer);

    // Update current MapView if needed
    MapView* currentMapView = getCurrentMapView();
    if (currentMapView) {
        // TODO: Update MapView layer
        qDebug() << "Panel layer changed to:" << layer;
    }
}

void MainWindow::onPanelSelectionChanged() {
    emit selectionChanged();

    // Update UI elements based on selection
    MapView* currentMapView = getCurrentMapView();
    if (currentMapView) {
        // TODO: Update selection-dependent UI elements
        qDebug() << "Panel selection changed";
    }
}

void MainWindow::onMapViewModified(bool modified) {
    emit mapModified(modified);

    // Update tab title to show modification state
    int currentIndex = getCurrentMapTabIndex();
    if (currentIndex >= 0) {
        QString title = getMapTabTitle(currentIndex);
        if (modified && !title.endsWith("*")) {
            setMapTabTitle(currentIndex, title + "*");
        } else if (!modified && title.endsWith("*")) {
            title.chop(1);
            setMapTabTitle(currentIndex, title);
        }
    }
}

void MainWindow::onDockWidgetVisibilityChanged(bool visible) {
    // Update menu actions to reflect dock widget visibility
    QDockWidget* dock = qobject_cast<QDockWidget*>(sender());
    if (!dock) return;

    if (dock == paletteDock_ && viewPaletteDockAction_) {
        viewPaletteDockAction_->setChecked(visible);
    } else if (dock == minimapDock_ && viewMinimapDockAction_) {
        viewMinimapDockAction_->setChecked(visible);
    } else if (dock == propertiesDock_ && viewPropertiesDockAction_) {
        viewPropertiesDockAction_->setChecked(visible);
    }

    qDebug() << "Dock widget visibility changed:" << dock->objectName() << visible;
}

// Task 62: Window state management slots
void MainWindow::onSaveWindowState() {
    if (!settings_) return;

    // Save window geometry and state
    settings_->setValue("geometry", saveGeometry());
    settings_->setValue("windowState", saveState());

    // Save dock widget states
    settings_->setValue("paletteDockVisible", paletteDock_ ? paletteDock_->isVisible() : false);
    settings_->setValue("minimapDockVisible", minimapDock_ ? minimapDock_->isVisible() : false);
    settings_->setValue("propertiesDockVisible", propertiesDock_ ? propertiesDock_->isVisible() : false);

    // Save tab information
    settings_->setValue("currentMapTabIndex", getCurrentMapTabIndex());
    settings_->setValue("mapTabCount", getMapTabCount());

    // Save tab titles
    settings_->beginWriteArray("mapTabs");
    for (int i = 0; i < getMapTabCount(); ++i) {
        settings_->setArrayIndex(i);
        settings_->setValue("title", getMapTabTitle(i));
    }
    settings_->endArray();

    settings_->sync();
    qDebug() << "Window state saved";
}

void MainWindow::onRestoreWindowState() {
    if (!settings_) return;

    // Restore window geometry and state
    QByteArray geometry = settings_->value("geometry").toByteArray();
    if (!geometry.isEmpty()) {
        restoreGeometry(geometry);
    }

    QByteArray windowState = settings_->value("windowState").toByteArray();
    if (!windowState.isEmpty()) {
        restoreState(windowState);
    }

    // Restore dock widget visibility
    if (paletteDock_) {
        bool visible = settings_->value("paletteDockVisible", true).toBool();
        paletteDock_->setVisible(visible);
        if (viewPaletteDockAction_) {
            viewPaletteDockAction_->setChecked(visible);
        }
    }

    if (minimapDock_) {
        bool visible = settings_->value("minimapDockVisible", true).toBool();
        minimapDock_->setVisible(visible);
        if (viewMinimapDockAction_) {
            viewMinimapDockAction_->setChecked(visible);
        }
    }

    if (propertiesDock_) {
        bool visible = settings_->value("propertiesDockVisible", true).toBool();
        propertiesDock_->setVisible(visible);
        if (viewPropertiesDockAction_) {
            viewPropertiesDockAction_->setChecked(visible);
        }
    }

    windowStateRestored_ = true;
    qDebug() << "Window state restored";
}

void MainWindow::onResetWindowLayout() {
    // Reset to default layout
    if (paletteDock_) {
        addDockWidget(Qt::LeftDockWidgetArea, paletteDock_);
        paletteDock_->setVisible(true);
    }

    if (minimapDock_) {
        addDockWidget(Qt::RightDockWidgetArea, minimapDock_);
        minimapDock_->setVisible(true);
    }

    if (propertiesDock_) {
        addDockWidget(Qt::RightDockWidgetArea, propertiesDock_);
        propertiesDock_->setVisible(true);
    }

    // Reset window size
    resize(1280, 720);

    // Update menu actions
    if (viewPaletteDockAction_) viewPaletteDockAction_->setChecked(true);
    if (viewMinimapDockAction_) viewMinimapDockAction_->setChecked(true);
    if (viewPropertiesDockAction_) viewPropertiesDockAction_->setChecked(true);

    qDebug() << "Window layout reset to defaults";
}

// Task 77: Enhanced UI synchronization slot implementations
void MainWindow::onBrushManagerBrushChanged(Brush* newBrush, Brush* previousBrush) {
    Q_UNUSED(previousBrush)

    if (newBrush) {
        updateCurrentBrush(newBrush->getName());

        // Update drawing mode based on brush type
        QString modeName = QString("Brush: %1").arg(newBrush->getName());
        QString description = QString("Drawing with %1 brush").arg(newBrush->getName());
        updateDrawingMode(modeName, description);

        qDebug() << "MainWindow: Brush changed to" << newBrush->getName();
    } else {
        updateCurrentBrush("None");
        updateDrawingMode("None", "No brush selected");
        qDebug() << "MainWindow: Brush cleared";
    }

    // Update toolbar button states
    updateToolbarButtonStates();

    // Update palette selections
    updatePaletteSelections();
}

void MainWindow::onBrushManagerActionIdChanged(quint16 actionId, bool enabled) {
    updateActionId(actionId, enabled);
    qDebug() << "MainWindow: Action ID changed to" << actionId << "enabled:" << enabled;
}

void MainWindow::onBrushManagerSelectedItemChanged(Item* item, const QString& itemInfo) {
    Q_UNUSED(item)
    updateSelectedItemInfo(itemInfo);
    qDebug() << "MainWindow: Selected item changed to" << itemInfo;
}

void MainWindow::onBrushManagerDrawingModeChanged(const QString& modeName, const QString& description) {
    updateDrawingMode(modeName, description);
    qDebug() << "MainWindow: Drawing mode changed to" << modeName;
}

void MainWindow::onMainPaletteActionIdChanged(quint16 actionId, bool enabled) {
    // Forward to BrushManager to maintain single source of truth
    if (brushManager_) {
        brushManager_->setActionId(actionId);
        brushManager_->setActionIdEnabled(enabled);
    }
    qDebug() << "MainWindow: Palette action ID changed to" << actionId << "enabled:" << enabled;
}

void MainWindow::onMainPaletteBrushSelected(Brush* brush) {
    // Forward to BrushManager to maintain single source of truth
    if (brushManager_) {
        brushManager_->setCurrentBrush(brush);
    }
    qDebug() << "MainWindow: Palette brush selected:" << (brush ? brush->getName() : "None");
}

void MainWindow::onToolbarActionTriggered(const QString& actionName, bool active) {
    // Update toolbar state display
    if (statusBarManager_) {
        statusBarManager_->updateToolbarState(actionName, active);
    }
    qDebug() << "MainWindow: Toolbar action triggered:" << actionName << "active:" << active;
}

// Task 77: Enhanced UI synchronization method implementations
void MainWindow::updateActionId(quint16 actionId, bool enabled) {
    if (statusBarManager_) {
        statusBarManager_->updateActionId(actionId, enabled);
    }
}

void MainWindow::updateDrawingMode(const QString& modeName, const QString& description) {
    if (statusBarManager_) {
        statusBarManager_->updateDrawingMode(modeName, description);
    }
}

void MainWindow::updateToolbarButtonStates() {
    if (toolBarManager_) {
        toolBarManager_->updateBrushToolbarStates();
    }

    // Update brush-related toolbar buttons based on current brush
    if (brushManager_) {
        Brush* currentBrush = brushManager_->getCurrentBrush();

        // Update brush size actions
        int brushSize = currentBrush ? currentBrush->getSize() : 1;
        if (brushSizeActionGroup_) {
            QList<QAction*> sizeActions = brushSizeActionGroup_->actions();
            for (int i = 0; i < sizeActions.size(); ++i) {
                if (i + 1 == brushSize) {
                    sizeActions[i]->setChecked(true);
                    break;
                }
            }
        }

        // Update brush shape actions
        if (currentBrush && brushShapeActionGroup_) {
            // This would need to be implemented based on actual brush shape properties
            // For now, just ensure one is selected
            if (!brushShapeActionGroup_->checkedAction()) {
                rectangularBrushShapeAction_->setChecked(true);
            }
        }
    }
}

void MainWindow::updatePaletteSelections() {
    // Update palette selections to reflect current brush
    // This would need to be implemented based on actual palette structure
    // For now, just log the update
    qDebug() << "MainWindow: Updating palette selections";
}

void MainWindow::synchronizeUIState() {
    // Perform complete UI synchronization
    if (brushManager_) {
        // Update brush information
        Brush* currentBrush = brushManager_->getCurrentBrush();
        if (currentBrush) {
            updateCurrentBrush(currentBrush->getName());
        } else {
            updateCurrentBrush("None");
        }

        // Update action ID
        updateActionId(brushManager_->getActionId(), brushManager_->isActionIdEnabled());

        // Update drawing mode
        updateDrawingMode(brushManager_->getCurrentDrawingMode(),
                         brushManager_->getCurrentDrawingModeDescription());

        // Update selected item
        updateSelectedItemInfo(brushManager_->getSelectedItemInfo());
    }

    // Update toolbar states
    updateToolbarButtonStates();

    // Update palette selections
    updatePaletteSelections();

    qDebug() << "MainWindow: UI state synchronized";
}
