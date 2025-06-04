#include "MainWindow.h"
#include "MenuBuilder.h"  // Task 011: Extracted menu building logic
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

    // Task 011: Initialize menu builder for mandate M6 compliance
    menuBuilder_ = new MenuBuilder(this, this);

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

    delete menuBuilder_;
    menuBuilder_ = nullptr;

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

    // Task 011: Use MenuBuilder for mandate M6 compliance
    if (menuBuilder_) {
        menuBar_->addMenu(menuBuilder_->createFileMenu());
        menuBar_->addMenu(menuBuilder_->createEditMenu());
        menuBar_->addMenu(menuBuilder_->createEditorMenu());
        menuBar_->addMenu(menuBuilder_->createSearchMenu());
        menuBar_->addMenu(menuBuilder_->createMapMenu());
        menuBar_->addMenu(menuBuilder_->createSelectionMenu());
        menuBar_->addMenu(menuBuilder_->createViewMenu());
        menuBar_->addMenu(menuBuilder_->createShowMenu());
        menuBar_->addMenu(menuBuilder_->createNavigateMenu());
        menuBar_->addMenu(menuBuilder_->createWindowMenu());
        menuBar_->addMenu(menuBuilder_->createExperimentalMenu());
        menuBar_->addMenu(menuBuilder_->createAboutMenu());
        menuBar_->addMenu(menuBuilder_->createServerMenu());
        menuBar_->addMenu(menuBuilder_->createIdlerMenu());

        // Copy actions from MenuBuilder for backward compatibility
        actions_ = menuBuilder_->getActions();

        // Set up action references for backward compatibility
        newAction_ = menuBuilder_->getAction(MenuBuilder::MenuBar::NEW);
        openAction_ = menuBuilder_->getAction(MenuBuilder::MenuBar::OPEN);
        saveAction_ = menuBuilder_->getAction(MenuBuilder::MenuBar::SAVE);
        saveAsAction_ = menuBuilder_->getAction(MenuBuilder::MenuBar::SAVE_AS);
        undoAction_ = menuBuilder_->getAction(MenuBuilder::MenuBar::UNDO);
        redoAction_ = menuBuilder_->getAction(MenuBuilder::MenuBar::REDO);
        cutAction_ = menuBuilder_->getAction(MenuBuilder::MenuBar::CUT);
        copyAction_ = menuBuilder_->getAction(MenuBuilder::MenuBar::COPY);
        pasteAction_ = menuBuilder_->getAction(MenuBuilder::MenuBar::PASTE);
        viewPaletteDockAction_ = menuBuilder_->getAction(MenuBuilder::MenuBar::VIEW_PALETTE_DOCK);
        viewMinimapDockAction_ = menuBuilder_->getAction(MenuBuilder::MenuBar::VIEW_MINIMAP_DOCK);
        viewPropertiesDockAction_ = menuBuilder_->getAction(MenuBuilder::MenuBar::VIEW_PROPERTIES_DOCK);
    }

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
    // Task 011: Delegate to MenuBuilder for mandate M6 compliance
    if (menuBuilder_) {
        return menuBuilder_->createAction(text, objectName, icon, shortcut, statusTip, checkable, checked, connectToGenericHandler);
    }

    // Fallback implementation if MenuBuilder is not available
    QAction *action = new QAction(tr(text.toStdString().c_str()), this);
    action->setObjectName(objectName);
    action->setIcon(icon);
    if (!shortcut.isEmpty()) {
        action->setShortcut(QKeySequence::fromString(tr(shortcut.toStdString().c_str())));
    }
    action->setStatusTip(tr(statusTip.toStdString().c_str()));
    action->setCheckable(checkable);
    action->setChecked(checked);
    if (connectToGenericHandler) {
        connect(action, &QAction::triggered, this, &MainWindow::onMenuActionTriggered);
    }
    return action;
}

QAction* MainWindow::createActionWithId(MenuBar::ActionID actionId, const QString& text, const QIcon& icon, const QString& shortcut, const QString& statusTip, bool checkable, bool checked) {
    // Task 011: Delegate to MenuBuilder for mandate M6 compliance
    if (menuBuilder_) {
        return menuBuilder_->createActionWithId(static_cast<MenuBuilder::MenuBar::ActionID>(actionId), text, icon, shortcut, statusTip, checkable, checked);
    }

    // Fallback implementation if MenuBuilder is not available
    QAction *action = new QAction(tr(text.toStdString().c_str()), this);
    action->setObjectName(QString("ACTION_%1").arg(static_cast<int>(actionId)));
    action->setIcon(icon);
    if (!shortcut.isEmpty()) {
        action->setShortcut(QKeySequence::fromString(tr(shortcut.toStdString().c_str())));
    }
    action->setStatusTip(tr(statusTip.toStdString().c_str()));
    action->setCheckable(checkable);
    action->setChecked(checked);

    actions_[actionId] = action;

    connect(action, &QAction::triggered, [this, actionId]() {
        onActionTriggered(actionId);
    });

    return action;
}

QAction* MainWindow::getAction(MenuBar::ActionID actionId) const {
    return actions_.value(actionId, nullptr);
}

// createFileMenu moved to MenuBuilder for mandate M6 compliance

// createEditMenu moved to MenuBuilder for mandate M6 compliance

// createEditorMenu moved to MenuBuilder for mandate M6 compliance

// createSearchMenu moved to MenuBuilder for mandate M6 compliance

// createMapMenu moved to MenuBuilder for mandate M6 compliance

// createSelectionMenu moved to MenuBuilder for mandate M6 compliance

// createViewMenu moved to MenuBuilder for mandate M6 compliance

// createShowMenu moved to MenuBuilder for mandate M6 compliance

// createNavigateMenu moved to MenuBuilder for mandate M6 compliance

// createWindowMenu moved to MenuBuilder for mandate M6 compliance

// createExperimentalMenu moved to MenuBuilder for mandate M6 compliance

// createAboutMenu moved to MenuBuilder for mandate M6 compliance

// createServerMenu moved to MenuBuilder for mandate M6 compliance

// createIdlerMenu moved to MenuBuilder for mandate M6 compliance









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
