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
#include "PlaceholderPaletteWidget.h"   
#include "PlaceholderMinimapWidget.h"
#include "PlaceholderPropertiesWidget.h"
#include "AutomagicSettingsDialog.h" // Include for AutomagicSettingsDialog


MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    setWindowTitle(tr("Idler's Map Editor (Qt)"));
    resize(1280, 720);

    setupMenuBar();
    setupToolBars(); 
    setupDockWidgets(); // Call setupDockWidgets
    // setupStatusBar(); 
    
    // restoreLayoutState(); // Call this after all toolbars and docks are created.
    qDebug() << "MainWindow created. Menu, toolbars, and docks setup initiated.";
}

MainWindow::~MainWindow() {
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


QAction* MainWindow::createAction(const QString& text, const QString& objectName, const QString& shortcut, const QString& statusTip, bool checkable, bool checked) {
    QAction *action = new QAction(tr(text.toStdString().c_str()), this);
    action->setObjectName(objectName);
    if (!shortcut.isEmpty()) {
        action->setShortcut(QKeySequence::fromString(tr(shortcut.toStdString().c_str())));
    }
    action->setStatusTip(tr(statusTip.toStdString().c_str()));
    action->setCheckable(checkable);
    action->setChecked(checked);
    action->setIcon(QIcon()); 
    connect(action, &QAction::triggered, this, &MainWindow::onMenuActionTriggered);
    return action;
}

QMenu* MainWindow::createFileMenu() {
    QMenu *fileMenu = new QMenu(tr("&File"), this);
    newAction_ = createAction("&New...", "NEW", "P", "Create a new map.");
    fileMenu->addAction(newAction_);
    openAction_ = createAction("&Open...", "OPEN", "Ctrl+O", "Open another map.");
    fileMenu->addAction(openAction_);
    saveAction_ = createAction("&Save", "SAVE", "Ctrl+S", "Save the current map.");
    fileMenu->addAction(saveAction_);
    fileMenu->addAction(createAction("Save &As...", "SAVE_AS", "Ctrl+Alt+S", "Save the current map as a new file."));
    fileMenu->addAction(createAction("&Generate Map", "GENERATE_MAP", "Ctrl+Shift+G", "Generate a new map."));
    fileMenu->addAction(createAction("&Close", "CLOSE", "Shift+B", "Closes the currently open map."));
    fileMenu->addSeparator();
    QMenu *importMenu = fileMenu->addMenu(tr("&Import"));
    importMenu->addAction(createAction("Import &Map...", "IMPORT_MAP", "", "Import map data from another map file."));
    importMenu->addAction(createAction("Import &Monsters/NPC...", "IMPORT_MONSTERS", "", "Import either a monsters.xml file or a specific monster/NPC."));
    QMenu *exportMenu = fileMenu->addMenu(tr("&Export"));
    exportMenu->addAction(createAction("Export &Minimap...", "EXPORT_MINIMAP", "", "Export minimap to an image file."));
    exportMenu->addAction(createAction("Export &Tilesets...", "EXPORT_TILESETS", "", "Export tilesets to an xml file."));
    QMenu *reloadMenu = fileMenu->addMenu(tr("&Reload"));
    reloadMenu->addAction(createAction("&Reload All Data", "RELOAD_DATA", "F5", "Reloads all data files."));
    fileMenu->addSeparator();
    QMenu *recentFilesMenu = fileMenu->addMenu(tr("Recent &Files"));
    recentFilesMenu->setObjectName(QStringLiteral("RECENT_FILES"));
    QAction* placeholderRecent = recentFilesMenu->addAction(tr("(No recent files)"));
    placeholderRecent->setEnabled(false);
    fileMenu->addAction(createAction("&Preferences", "PREFERENCES", "Ctrl+Shift+V", "Configure the map editor."));
    fileMenu->addSeparator();
    fileMenu->addAction(createAction("E&xit", "EXIT", "", "Close the editor."));
    return fileMenu;
}

QMenu* MainWindow::createEditMenu() {
    QMenu *editMenu = new QMenu(tr("&Edit"), this);
    undoAction_ = createAction("&Undo", "UNDO", "Ctrl+Z", "Undo last action.");
    editMenu->addAction(undoAction_);
    redoAction_ = createAction("&Redo", "REDO", "Ctrl+Shift+Z", "Redo last undid action.");
    editMenu->addAction(redoAction_);
    editMenu->addSeparator();
    editMenu->addAction(createAction("&Replace Items...", "REPLACE_ITEMS", "Ctrl+Shift+F", "Replaces all occurrences of one item with another."));
    editMenu->addAction(createAction("Refresh Items", "REFRESH_ITEMS", "", "Refresh items to fix flags"));
    editMenu->addSeparator();
    QMenu *borderOptionsMenu = editMenu->addMenu(tr("&Border Options"));
    borderOptionsMenu->addAction(createAction("Border &Automagic", "AUTOMAGIC", "A", "Turns on all automatic border functions.", true));
    borderOptionsMenu->addSeparator();
    borderOptionsMenu->addAction(createAction("&Borderize Selection", "BORDERIZE_SELECTION", "Ctrl+B", "Creates automatic borders in the entire selected area."));
    borderOptionsMenu->addAction(createAction("Borderize &Map", "BORDERIZE_MAP", "", "Reborders the entire map."));
    borderOptionsMenu->addAction(createAction("&Randomize Selection", "RANDOMIZE_SELECTION", "", "Randomizes the ground tiles of the selected area."));
    borderOptionsMenu->addAction(createAction("Randomize M&ap", "RANDOMIZE_MAP", "", "Randomizes all tiles of the entire map."));
    QMenu *otherOptionsMenu = editMenu->addMenu(tr("&Other Options"));
    otherOptionsMenu->addAction(createAction("Remove all &Unreachable Tiles...", "MAP_REMOVE_UNREACHABLE_TILES", "", "Removes all tiles that cannot be reached (or seen) by the player from the map."));
    otherOptionsMenu->addAction(createAction("&Clear Invalid Houses", "CLEAR_INVALID_HOUSES", "", "Clears house tiles not belonging to any house."));
    otherOptionsMenu->addAction(createAction("Clear &Modified State", "CLEAR_MODIFIED_STATE", "", "Clears the modified state from all tiles."));
    editMenu->addSeparator();
    cutAction_ = createAction("Cu&t", "CUT", "Ctrl+X", "Cut a part of the map.");
    editMenu->addAction(cutAction_);
    copyAction_ = createAction("&Copy", "COPY", "Ctrl+C", "Copy a part of the map.");
    editMenu->addAction(copyAction_);
    pasteAction_ = createAction("&Paste", "PASTE", "Ctrl+V", "Paste a part of the map.");
    editMenu->addAction(pasteAction_);
    return editMenu;
}

QMenu* MainWindow::createEditorMenu() { /* ... identical to previous ... */ 
    QMenu *editorMenu = new QMenu(tr("Edito&r"), this); 
    editorMenu->addAction(createAction("&New View", "NEW_VIEW", "Ctrl+Shift+N", "Creates a new view of the current map."));
    editorMenu->addAction(createAction("New &Detached View", "NEW_DETACHED_VIEW", "Ctrl+Shift+D", "Creates a new detached view of the current map that can be moved to another monitor."));
    editorMenu->addAction(createAction("Enter &Fullscreen", "TOGGLE_FULLSCREEN", "F11", "Changes between fullscreen mode and windowed mode.", true));
    editorMenu->addAction(createAction("Take &Screenshot", "TAKE_SCREENSHOT", "F10", "Saves the current view to the disk."));
    editorMenu->addSeparator();
    QMenu *zoomMenu = editorMenu->addMenu(tr("&Zoom"));
    zoomMenu->addAction(createAction("Zoom &In", "ZOOM_IN", "Ctrl++", "Increase the zoom."));
    zoomMenu->addAction(createAction("Zoom &Out", "ZOOM_OUT", "Ctrl+-", "Decrease the zoom."));
    zoomMenu->addAction(createAction("Zoom &Normal", "ZOOM_NORMAL", "Ctrl+0", "Normal zoom(100%)."));
    return editorMenu;
}
QMenu* MainWindow::createSearchMenu() { /* ... identical to previous ... */ 
    QMenu* menu = new QMenu(tr("&Search"), this);
    menu->addAction(createAction("&Find Item...", "FIND_ITEM", "Ctrl+F", "Find all instances of an item type the map."));
    menu->addAction(createAction("Find &Creature...", "FIND_CREATURE", "Ctrl+Shift+C", "Find all instances of a creature on the map."));
    menu->addSeparator();
    menu->addAction(createAction("Find &Zones", "SEARCH_ON_MAP_ZONES", "", "Find all zones on map."));
    menu->addAction(createAction("Find &Unique", "SEARCH_ON_MAP_UNIQUE", "L", "Find all items with an unique ID on map."));
    menu->addAction(createAction("Find &Action", "SEARCH_ON_MAP_ACTION", "", "Find all items with an action ID on map."));
    menu->addAction(createAction("Find &Container", "SEARCH_ON_MAP_CONTAINER", "", "Find all containers on map."));
    menu->addAction(createAction("Find &Writeable", "SEARCH_ON_MAP_WRITEABLE", "", "Find all writeable items on map."));
    menu->addSeparator();
    menu->addAction(createAction("Find &Everything", "SEARCH_ON_MAP_EVERYTHING", "", "Find all unique/action/text/container items."));
    return menu;
}
QMenu* MainWindow::createMapMenu() { /* ... identical to previous ... */ 
    QMenu* menu = new QMenu(tr("&Map"), this);
    menu->addAction(createAction("Edit &Towns", "EDIT_TOWNS", "Ctrl+T", "Edit towns."));
    menu->addSeparator();
    menu->addAction(createAction("&Cleanup...", "MAP_CLEANUP", "", "Removes all items that do not exist in the OTB file (red tiles the server can't load)."));
    menu->addAction(createAction("&Properties...", "MAP_PROPERTIES", "Ctrl+P", "Show and change the map properties."));
    menu->addAction(createAction("S&tatistics", "MAP_STATISTICS", "F8", "Show map statistics."));
    return menu;
}
QMenu* MainWindow::createSelectionMenu() { /* ... identical to previous ... */ 
    QMenu* menu = new QMenu(tr("S&election"), this);
    menu->addAction(createAction("&Replace Items on Selection", "REPLACE_ON_SELECTION_ITEMS", "", "Replace items on selected area."));
    menu->addAction(createAction("&Find Item on Selection", "SEARCH_ON_SELECTION_ITEM", "", "Find items on selected area."));
    menu->addAction(createAction("&Remove Item on Selection", "REMOVE_ON_SELECTION_ITEM", "", "Remove item on selected area."));
    menu->addSeparator();
    QMenu *findOnSelectionMenu = menu->addMenu(tr("Find on Selection"));
    findOnSelectionMenu->addAction(createAction("Find &Everything", "SEARCH_ON_SELECTION_EVERYTHING", "", "Find all unique/action/text/container items."));
    findOnSelectionMenu->addSeparator();
    findOnSelectionMenu->addAction(createAction("Find &Zones", "SEARCH_ON_SELECTION_ZONES", "", "Find all zones on selected area."));
    findOnSelectionMenu->addAction(createAction("Find &Unique", "SEARCH_ON_SELECTION_UNIQUE", "", "Find all items with an unique ID on selected area."));
    findOnSelectionMenu->addAction(createAction("Find &Action", "SEARCH_ON_SELECTION_ACTION", "", "Find all items with an action ID on selected area."));
    findOnSelectionMenu->addAction(createAction("Find &Container", "SEARCH_ON_SELECTION_CONTAINER", "", "Find all containers on selected area."));
    findOnSelectionMenu->addAction(createAction("Find &Writeable", "SEARCH_ON_SELECTION_WRITEABLE", "", "Find all writeable items on selected area."));
    menu->addSeparator();
    QMenu *selectionModeMenu = menu->addMenu(tr("Selection &Mode"));
    QActionGroup* selectionModeGroup = new QActionGroup(this);
    selectionModeGroup->setExclusive(true);
    QAction* compensateAction = createAction("&Compensate Selection", "SELECT_MODE_COMPENSATE", "", "Compensate for floor difference when selecting.", true);
    selectionModeMenu->addAction(compensateAction);
    selectionModeGroup->addAction(compensateAction);
    selectionModeMenu->addSeparator();
    QAction* currentFloorAction = createAction("&Current Floor", "SELECT_MODE_CURRENT", "", "Select only current floor.", true);
    selectionModeMenu->addAction(currentFloorAction);
    selectionModeGroup->addAction(currentFloorAction);
    currentFloorAction->setChecked(true); 
    QAction* lowerFloorsAction = createAction("&Lower Floors", "SELECT_MODE_LOWER", "", "Select all lower floors.", true);
    selectionModeMenu->addAction(lowerFloorsAction);
    selectionModeGroup->addAction(lowerFloorsAction);
    QAction* visibleFloorsAction = createAction("&Visible Floors", "SELECT_MODE_VISIBLE", "", "Select only visible floors.", true);
    selectionModeMenu->addAction(visibleFloorsAction);
    selectionModeGroup->addAction(visibleFloorsAction);
    menu->addSeparator();
    menu->addAction(createAction("&Borderize Selection", "BORDERIZE_SELECTION", "Ctrl+B", "Creates automatic borders in the entire selected area."));
    menu->addAction(createAction("&Randomize Selection", "RANDOMIZE_SELECTION", "", "Randomizes the ground tiles of the selected area."));
    return menu;
}

QMenu* MainWindow::createViewMenu() { 
    QMenu *viewMenu = new QMenu(tr("&View"), this);
    // ... (visibility options from previous step, ensure these are not mixed with dock toggles) ...
    viewMenu->addAction(createAction("Show &all Floors", "SHOW_ALL_FLOORS", "Ctrl+W", "If not checked other floors are hidden.", true, true));
    viewMenu->addAction(createAction("Show as &Minimap", "SHOW_AS_MINIMAP", "Shift+E", "Show only the tile minimap colors.", true));
    // ... more view options ...
    viewMenu->addSeparator(); // Separator before dock/panel visibility items if they are in this menu
    return viewMenu;
}
QMenu* MainWindow::createShowMenu() { /* ... identical to previous ... */ 
    QMenu *showMenu = new QMenu(tr("Sho&w"), this); 
    showMenu->addAction(createAction("Show A&nimation", "SHOW_PREVIEW", "N", "Show item animations.", true, true));
    showMenu->addAction(createAction("Show &Light", "SHOW_LIGHTS", "H", "Show lights.", true, true));
    // ... more show options ...
    return showMenu;
}
QMenu* MainWindow::createNavigateMenu() { /* ... identical to previous ... */ 
    QMenu* menu = new QMenu(tr("&Navigate"), this);
    menu->addAction(createAction("Go to &Previous Position", "GOTO_PREVIOUS_POSITION", "P", "Go to the previous screen center position."));
    // ... more navigate options ...
    QMenu *floorMenu = menu->addMenu(tr("&Floor"));
    QActionGroup* floorGroup = new QActionGroup(this);
    floorGroup->setExclusive(true);
    for (int i = 0; i <= 15; ++i) {
        QAction* floorAction = createAction(QString("Floor %1").arg(i), QString("FLOOR_%1").arg(i), "", "", true);
        floorMenu->addAction(floorAction);
        floorGroup->addAction(floorAction);
        if (i == 7) floorAction->setChecked(true); 
    }
    return menu;
}

QMenu* MainWindow::createWindowMenu() { 
    QMenu* menu = new QMenu(tr("&Window"), this);

    // Dock Widget Toggles
    viewPaletteDockAction_ = createAction(tr("Palette Panel"), "VIEW_PALETTE_DOCK", "", "Show or hide the Palette panel.", true, true);
    menu->addAction(viewPaletteDockAction_);
    viewMinimapDockAction_ = createAction(tr("Minimap Panel"), "VIEW_MINIMAP_DOCK", "", "Show or hide the Minimap panel.", true, true);
    menu->addAction(viewMinimapDockAction_);
    viewPropertiesDockAction_ = createAction(tr("Properties Panel"), "VIEW_PROPERTIES_DOCK", "", "Show or hide the Properties panel.", true, true);
    menu->addAction(viewPropertiesDockAction_);
    // Add actions for other dock widgets here if created (ActionList, TileList)

    menu->addSeparator();
    menu->addAction(createAction("&New Palette", "NEW_PALETTE", "", "Creates a new palette.")); // From XML
    menu->addSeparator();

    QMenu *paletteMenu = menu->addMenu(tr("&Palette")); // From XML
    paletteMenu->addAction(createAction("&Terrain", "SELECT_TERRAIN", "T", "Select the Terrain palette."));
    paletteMenu->addAction(createAction("&Doodad", "SELECT_DOODAD", "D", "Select the Doodad palette."));
    // ... more palette selections ...
    menu->addSeparator(); 

    QMenu *toolbarsMenu = menu->addMenu(tr("&Toolbars"));
    toolbarsMenu->addAction(createAction("&Brushes", "VIEW_TOOLBARS_BRUSHES", "", "Show or hide the Brushes toolbar.", true, true));
    toolbarsMenu->addAction(createAction("&Position", "VIEW_TOOLBARS_POSITION", "", "Show or hide the Position toolbar.", true, true));
    toolbarsMenu->addAction(createAction("&Sizes", "VIEW_TOOLBARS_SIZES", "", "Show or hide the Sizes toolbar.", true, true));
    toolbarsMenu->addAction(createAction("&Standard", "VIEW_TOOLBARS_STANDARD", "", "Show or hide the Standard toolbar.", true, true));
    return menu;
}
QMenu* MainWindow::createExperimentalMenu() { /* ... identical to previous ... */ 
    QMenu* menu = new QMenu(tr("E&xperimental"), this);
    menu->addAction(createAction("&Fog in light view", "EXPERIMENTAL_FOG", "", "Apply fog filter to light effect.", true));
    return menu;
}
QMenu* MainWindow::createAboutMenu() { /* ... identical to previous ... */ 
    QMenu* menu = new QMenu(tr("A&bout"), this); 
    menu->addAction(createAction("E&xtensions...", "EXTENSIONS", "F2", ""));
    menu->addAction(createAction("&Goto Website", "GOTO_WEBSITE", "F3", ""));
    menu->addAction(createAction("&About...", "ABOUT", "F1", ""));
    return menu;
}
QMenu* MainWindow::createServerMenu() { /* ... identical to previous ... */ 
    QMenu* menu = new QMenu(tr("Se&rver"), this);
    menu->addAction(createAction("&Host Server", "ID_MENU_SERVER_HOST", "", "Host a new server for collaborative mapping"));
    menu->addAction(createAction("&Connect to Server", "ID_MENU_SERVER_CONNECT", "", "Connect to an existing map server"));
    return menu;
}
QMenu* MainWindow::createIdlerMenu() { /* ... identical to previous ... */ 
    QMenu* menu = new QMenu(tr("&Idler"), this);
    menu->addAction(createAction("&Hotkeys", "SHOW_HOTKEYS", "F6", "Hotkeys"));
    // ... more Idler items ...
    return menu;
}

QToolBar* MainWindow::createStandardToolBar() { /* ... identical to previous, with Zoom/Layer ... */ 
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
QToolBar* MainWindow::createBrushesToolBar() { /* ... identical to previous ... */ 
    QToolBar* tb = new QToolBar(tr("Brushes"), this);
    tb->setObjectName(QStringLiteral("BrushesToolBar"));
    QActionGroup* brushGroup = new QActionGroup(this);
    brushGroup->setExclusive(true);
    QList<QPair<QString, QString>> brushes = {
        {"Ground Brush", "BRUSH_GROUND"}, {"Optional Border Brush", "BRUSH_OPTIONAL_BORDER"},
        {"Eraser", "BRUSH_ERASER"}, {"PZ Brush", "BRUSH_PZ"},
        {"NoPVP Brush", "BRUSH_NOPVP"}, {"Door Brush", "BRUSH_DOOR"}
    };
    for(const auto& brushInfo : brushes){
        QAction* action = createAction(brushInfo.first, brushInfo.second, "", brushInfo.first, true);
        tb->addAction(action);
        brushGroup->addAction(action);
    }
    if(!tb->actions().isEmpty() && tb->actions().first()->isCheckable()){
        tb->actions().first()->setChecked(true);
    }
    return tb;
}
QToolBar* MainWindow::createPositionToolBar() { /* ... identical to previous ... */ 
    QToolBar* tb = new QToolBar(tr("Position"), this);
    tb->setObjectName(QStringLiteral("PositionToolBar"));
    tb->addWidget(new QLabel(tr("X:"), this));
    xCoordSpinBox_ = new QSpinBox(this);
    xCoordSpinBox_->setRange(0, 32767); 
    xCoordSpinBox_->setToolTip(tr("Current X coordinate"));
    connect(xCoordSpinBox_, qOverload<int>(&QSpinBox::valueChanged), this, &MainWindow::onPositionXChanged);
    tb->addWidget(xCoordSpinBox_);
    tb->addWidget(new QLabel(tr("Y:"), this));
    yCoordSpinBox_ = new QSpinBox(this);
    yCoordSpinBox_->setRange(0, 32767); 
    yCoordSpinBox_->setToolTip(tr("Current Y coordinate"));
    connect(yCoordSpinBox_, qOverload<int>(&QSpinBox::valueChanged), this, &MainWindow::onPositionYChanged);
    tb->addWidget(yCoordSpinBox_);
    tb->addWidget(new QLabel(tr("Z:"), this));
    zCoordSpinBox_ = new QSpinBox(this);
    zCoordSpinBox_->setRange(0, 15);    
    zCoordSpinBox_->setToolTip(tr("Current Z coordinate (floor)"));
    connect(zCoordSpinBox_, qOverload<int>(&QSpinBox::valueChanged), this, &MainWindow::onPositionZChanged);
    tb->addWidget(zCoordSpinBox_);
    QPushButton* goButton = new QPushButton(tr("Go"), this);
    goButton->setToolTip(tr("Go to specified X,Y,Z coordinates"));
    connect(goButton, &QPushButton::clicked, this, &MainWindow::onPositionGo);
    tb->addWidget(goButton);
    return tb;
}
QToolBar* MainWindow::createSizesToolBar() { /* ... identical to previous ... */ 
    QToolBar* tb = new QToolBar(tr("Sizes"), this);
    tb->setObjectName(QStringLiteral("SizesToolBar"));
    brushShapeActionGroup_ = new QActionGroup(this);
    brushShapeActionGroup_->setExclusive(true);
    rectangularBrushShapeAction_ = createAction(tr("Rectangular"), "TOGGLE_BRUSH_SHAPE_RECT", "", "Use rectangular brush shape", true);
    rectangularBrushShapeAction_->setChecked(true);
    brushShapeActionGroup_->addAction(rectangularBrushShapeAction_);
    tb->addAction(rectangularBrushShapeAction_);
    circularBrushShapeAction_ = createAction(tr("Circular"), "TOGGLE_BRUSH_SHAPE_CIRC", "", "Use circular brush shape", true);
    brushShapeActionGroup_->addAction(circularBrushShapeAction_);
    tb->addAction(circularBrushShapeAction_);
    tb->addSeparator();
    brushSizeActionGroup_ = new QActionGroup(this);
    brushSizeActionGroup_->setExclusive(true);
    brushSize1Action_ = createAction(tr("Size 1x1"), "SET_BRUSH_SIZE_1", "", "Set brush size to 1x1", true);
    brushSize1Action_->setChecked(true); 
    brushSizeActionGroup_->addAction(brushSize1Action_);
    tb->addAction(brushSize1Action_);
    // ... other size actions ...
    brushSize2Action_ = createAction(tr("Size 3x3"), "SET_BRUSH_SIZE_2", "", "Set brush size to 3x3", true);
    brushSizeActionGroup_->addAction(brushSize2Action_);
    tb->addAction(brushSize2Action_);
    brushSize3Action_ = createAction(tr("Size 5x5"), "SET_BRUSH_SIZE_3", "", "Set brush size to 5x5", true);
    brushSizeActionGroup_->addAction(brushSize3Action_);
    tb->addAction(brushSize3Action_);
    brushSize4Action_ = createAction(tr("Size 7x7"), "SET_BRUSH_SIZE_4", "", "Set brush size to 7x7", true);
    brushSizeActionGroup_->addAction(brushSize4Action_);
    tb->addAction(brushSize4Action_);
    brushSize5Action_ = createAction(tr("Size 9x9"), "SET_BRUSH_SIZE_5", "", "Set brush size to 9x9", true);
    brushSizeActionGroup_->addAction(brushSize5Action_);
    tb->addAction(brushSize5Action_);
    brushSize6Action_ = createAction(tr("Size 11x11"), "SET_BRUSH_SIZE_6", "", "Set brush size to 11x11", true);
    brushSizeActionGroup_->addAction(brushSize6Action_);
    tb->addAction(brushSize6Action_);
    brushSize7Action_ = createAction(tr("Size 13x13"), "SET_BRUSH_SIZE_7", "", "Set brush size to 13x13", true);
    brushSizeActionGroup_->addAction(brushSize7Action_);
    tb->addAction(brushSize7Action_);
    return tb;
}

void MainWindow::onMenuActionTriggered() {
    QAction *action = qobject_cast<QAction*>(sender());
    if (!action) {
        qDebug() << "onMenuActionTriggered called, but sender is not a QAction.";
        return;
    }

    QString actionName = action->objectName();
    qDebug() << "Action triggered: Name =" << actionName << ", Text =" << action->text() 
             << ", Checkable:" << action->isCheckable() << ", Checked:" << action->isChecked();

    if (actionName == QLatin1String("EXIT")) {
        close();
    } else if (actionName == QLatin1String("VIEW_TOOLBARS_STANDARD")) {
        if (standardToolBar_) {
            standardToolBar_->setVisible(!standardToolBar_->isVisible());
            action->setChecked(standardToolBar_->isVisible()); 
        }
    } else if (actionName == QLatin1String("VIEW_TOOLBARS_BRUSHES")) {
        if (brushesToolBar_) {
            brushesToolBar_->setVisible(!brushesToolBar_->isVisible());
            action->setChecked(brushesToolBar_->isVisible());
        }
    } else if (actionName == QLatin1String("VIEW_TOOLBARS_POSITION")) {
        if (positionToolBar_) {
            positionToolBar_->setVisible(!positionToolBar_->isVisible());
            action->setChecked(positionToolBar_->isVisible());
        }
    } else if (actionName == QLatin1String("VIEW_TOOLBARS_SIZES")) {
        if (sizesToolBar_) {
            sizesToolBar_->setVisible(!sizesToolBar_->isVisible());
            action->setChecked(sizesToolBar_->isVisible());
        }
    } else if (actionName == QLatin1String("VIEW_PALETTE_DOCK")) {
        if (paletteDock_) {
            paletteDock_->setVisible(!paletteDock_->isVisible());
            action->setChecked(paletteDock_->isVisible());
        }
    } else if (actionName == QLatin1String("VIEW_MINIMAP_DOCK")) {
        if (minimapDock_) {
            minimapDock_->setVisible(!minimapDock_->isVisible());
            action->setChecked(minimapDock_->isVisible());
        }
    } else if (actionName == QLatin1String("VIEW_PROPERTIES_DOCK")) {
        if (propertiesDock_) {
            propertiesDock_->setVisible(!propertiesDock_->isVisible());
            action->setChecked(propertiesDock_->isVisible());
        }
    }
    // else {
        // qDebug() << "Action" << actionName << "not specifically handled for visibility/direct action yet.";
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
        layerComboBox_->setCurrentIndex(value);
    }
}
void MainWindow::onZoomControlChanged(int value) {
    qDebug() << "Zoom control changed to:" << value << "%";
}
void MainWindow::onLayerControlChanged(int index) {
    qDebug() << "Layer control changed to index:" << index << "Text:" << (layerComboBox_ ? layerComboBox_->itemText(index) : "N/A");
    if(zCoordSpinBox_ && index != zCoordSpinBox_->value()){
        zCoordSpinBox_->setValue(index);
    }
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
