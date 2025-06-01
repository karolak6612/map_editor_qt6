#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPointF> // Required for QPointF parameter
#include <QMap> // For QMap<ActionID, QAction*>
#include "MenuBarActionIDs.h" // For MenuBar::ActionID enum

// Forward declarations
class QMenuBar;
class QMenu;
class QAction;
class QToolBar;
class QSpinBox;
class QComboBox;
class QLabel;
class QActionGroup;
class QDockWidget;                  // Added
class BrushPalettePanel;     // Renamed from PlaceholderPaletteWidget
class PlaceholderMinimapWidget;     // Added
class TilePropertyEditor;  // Renamed from PlaceholderPropertiesWidget
class QStatusBar;                   // Added for statusBar_ member or statusBar() usage
class QCloseEvent; // Added for closeEvent
class AutomagicSettingsDialog; // Forward declaration
class ClipboardData;           // Forward declaration for clipboard
class Map;                     // Already forward declared in Map.h, but good practice if Map.h isn't fully included here
class Selection;               // Already forward declared in Selection.h, but good practice
class MapPos;                  // Required for updateMouseMapCoordinates if Map.h doesn't bring it transitively
class SettingsManager;         // Forward declaration for settings management
class BorderSystem;            // Forward declaration for border system
class MenuActionHandler;       // Forward declaration for menu action handler
class StatusBarManager;       // Forward declaration for status bar manager
class ToolBarManager;         // Forward declaration for toolbar manager
class DialogManager;          // Forward declaration for dialog manager
class PerspectiveManager;     // Forward declaration for perspective manager
class MapView;                // Forward declaration for map view

// Task 77: Additional forward declarations for UI synchronization
class BrushManager;           // Forward declaration for brush manager
class Brush;                  // Forward declaration for brush
class Item;                   // Forward declaration for item
class MainPalette;            // Forward declaration for main palette

// Task 62: Additional forward declarations for tab management
class QTabWidget;             // For central tab widget
class QSplitter;              // For splitter layout
class QVBoxLayout;            // For layout management
class QHBoxLayout;            // For layout management
class QWidget;                // For central widget
class QTimer;                 // For auto-save timer
class QSettings;              // For settings management


class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

    // Clipboard methods
    void handleCopy();
    void handleCut();
    // Ensure MapPos is known for updateMouseMapCoordinates, include Map.h if not already via other includes
    // Map.h includes MapPos, and MainWindow.h already includes Map.h (forward declaration)
    // For QPointF, <QPointF> is included by QMainWindow or other Qt headers typically.

public slots: // Making them public slots for potential connection from other UI elements or MapView
    void updateMouseMapCoordinates(const QPointF& mapPos, int floor);
    void updateZoomLevel(double zoom);
    void updateCurrentLayer(int layer);
    void showTemporaryStatusMessage(const QString& message, int timeout = 0); // For general messages
    void handlePaste(); // Moved here as it's a slot now in the diff, was public method
    void handleStatusUpdateRequest(const QString& type, const QVariantMap& data); // Enhanced status update handler
    // bool canPaste() const; // canPaste is not a slot, remains public method

public: // Public methods for other status updates if not slots (canPaste kept here)
    bool canPaste() const;
    void updateCurrentBrush(const QString& brushName);
    void updateSelectedItemInfo(const QString& itemInfo);
    void updateSelectedItemInfo(const Item* item); // Overload for Item objects
    void updateStatusBarProgress(const QString& operation, int progress);

    // Task 77: Enhanced UI synchronization methods
    void updateActionId(quint16 actionId, bool enabled);
    void updateDrawingMode(const QString& modeName, const QString& description);
    void updateToolbarButtonStates();
    void updatePaletteSelections();
    void synchronizeUIState();

    // Accessor methods for MenuActionHandler
    QToolBar* getStandardToolBar() const { return standardToolBar_; }
    QToolBar* getBrushesToolBar() const { return brushesToolBar_; }
    QToolBar* getPositionToolBar() const { return positionToolBar_; }
    QToolBar* getSizesToolBar() const { return sizesToolBar_; }
    MapView* getMapView() const;

    // MapView integration methods
    void setMapView(MapView* mapView);
    void connectMapViewToStatusBar();
    QDockWidget* getPaletteDock() const { return paletteDock_; }
    QDockWidget* getMinimapDock() const { return minimapDock_; }
    QDockWidget* getPropertiesDock() const { return propertiesDock_; }
    QAction* getAction(MenuBar::ActionID actionId) const;

    // Task 62: Tab management methods
    int addMapTab(MapView* mapView, const QString& title = "New Map");
    void removeMapTab(int index);
    void setCurrentMapTab(int index);
    int getCurrentMapTabIndex() const;
    MapView* getCurrentMapView() const;
    MapView* getMapViewAt(int index) const;
    int getMapTabCount() const;
    void setMapTabTitle(int index, const QString& title);
    QString getMapTabTitle(int index) const;
    void cycleMapTabs(bool forward = true);

    // Task 62: Central widget setup
    void setupCentralWidget();
    QTabWidget* getMapTabWidget() const { return mapTabWidget_; }

    // Automagic settings methods
    void openAutomagicSettingsDialog();
    bool mainGetAutomagicEnabled() const;
    bool mainGetSameGroundTypeBorderEnabled() const;
    bool mainGetWallsRepelBordersEnabled() const;
    bool mainGetLayerCarpetsEnabled() const;
    bool mainGetBorderizeDeleteEnabled() const;
    bool mainGetCustomBorderEnabled() const;
    int mainGetCustomBorderId() const;
    void mainUpdateAutomagicSettings(bool automagicEnabled, bool sameGround, bool wallsRepel, bool layerCarpets, bool borderizeDelete, bool customBorder, int customBorderId);
    void mainTriggerMapOrUIRefreshForAutomagic();

signals:
    // Task 62: Tab management signals
    void currentMapTabChanged(int index);
    void mapTabAdded(int index);
    void mapTabRemoved(int index);
    void mapTabTitleChanged(int index, const QString& title);
    void activeMapChanged(MapView* mapView);

    // Task 62: Panel communication signals
    void activeBrushChanged(const QString& brushName);
    void activeLayerChanged(int layer);
    void selectionChanged();
    void mapModified(bool modified);

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void onMenuActionTriggered();
    void onActionTriggered(MenuBar::ActionID actionId); // New centralized action handler
    void onPositionGo();
    void onPositionXChanged(int value);
    void onPositionYChanged(int value);
    void onPositionZChanged(int value);
    void onZoomControlChanged(int value);
    void onLayerControlChanged(int index);
    // Slots for toolbar actions
    void onBrushActionTriggered();
    void onBrushShapeActionTriggered();
    void onBrushSizeActionTriggered();
    // Slot for testing TilePropertyEditor
    void onTestUpdateTileProperties();
    void onShowReplaceItemsDialog();

    // Slots for new basic dialogs (Task7)
    void onShowGroundValidationDialog();
    void onShowImportMapDialog();
    void onShowExportMiniMapDialog();
    void onShowGotoPositionDialog();

    // Task 62: Tab management slots
    void onMapTabChanged(int index);
    void onMapTabCloseRequested(int index);
    void onMapTabMoved(int from, int to);
    void onNewMapTab();
    void onCloseCurrentMapTab();
    void onCloseAllMapTabs();
    void onNextMapTab();
    void onPreviousMapTab();

    // Task 62: Panel communication slots
    void onPanelBrushChanged(const QString& brushName);
    void onPanelLayerChanged(int layer);
    void onPanelSelectionChanged();
    void onMapViewModified(bool modified);
    void onDockWidgetVisibilityChanged(bool visible);

    // Task 77: Enhanced UI synchronization slots
    void onBrushManagerBrushChanged(Brush* newBrush, Brush* previousBrush);
    void onBrushManagerActionIdChanged(quint16 actionId, bool enabled);
    void onBrushManagerSelectedItemChanged(Item* item, const QString& itemInfo);
    void onBrushManagerDrawingModeChanged(const QString& modeName, const QString& description);
    void onMainPaletteActionIdChanged(quint16 actionId, bool enabled);
    void onMainPaletteBrushSelected(Brush* brush);
    void onToolbarActionTriggered(const QString& actionName, bool active);

    // Task 62: Window state management slots
    void onSaveWindowState();
    void onRestoreWindowState();
    void onResetWindowLayout();

private:
    // Main setup methods
    void setupMenuBar();
    void setupToolBars();
    void setupDockWidgets(); // Added
    void setupStatusBar();



    // Helper methods for creating actions
    QAction* createAction(const QString& text, const QString& objectName, const QIcon& icon = QIcon(), const QString& shortcut = "", const QString& statusTip = "", bool checkable = false, bool checked = false, bool connectToGenericHandler = true);
    QAction* createActionWithId(MenuBar::ActionID actionId, const QString& text, const QIcon& icon = QIcon(), const QString& shortcut = "", const QString& statusTip = "", bool checkable = false, bool checked = false);

    // Helper method to get action by ID
    QAction* getAction(MenuBar::ActionID actionId) const;

    // Toolbar state management methods
    void updateToolbarStates();
    void updateBrushToolbarStates();
    void updateStandardToolbarStates();

    // Dock widget management methods
    void createNewPalette();
    void destroyCurrentPalette();
    void createDockableMapView();
    void closeDockableViews();

    // Perspective management methods
    void savePerspective();
    void loadPerspective();
    void resetPerspective();

    // Menu creation helpers
    QMenu* createFileMenu();
    QMenu* createEditMenu();
    QMenu* createEditorMenu(); 
    QMenu* createSearchMenu(); 
    QMenu* createMapMenu();
    QMenu* createSelectionMenu();
    QMenu* createViewMenu();   
    QMenu* createShowMenu();   
    QMenu* createNavigateMenu();
    QMenu* createWindowMenu();
    QMenu* createExperimentalMenu();
    QMenu* createAboutMenu(); 
    QMenu* createServerMenu(); 
    QMenu* createIdlerMenu();  

    // Toolbar creation helpers
    QToolBar* createStandardToolBar();   
    QToolBar* createBrushesToolBar();    
    QToolBar* createPositionToolBar();   
    QToolBar* createSizesToolBar();      

    // UI Member Variables
    QMenuBar *menuBar_; 

    // Toolbar Members
    QToolBar* standardToolBar_;   
    QToolBar* brushesToolBar_;    
    QToolBar* positionToolBar_;   
    QToolBar* sizesToolBar_;      

    // Dock Widget Members
    QDockWidget* paletteDock_;      // Added
    QDockWidget* minimapDock_;      // Added
    QDockWidget* propertiesDock_;   // Added

    // Common QAction members
    QAction* newAction_;
    QAction* openAction_;
    QAction* saveAction_;
    QAction* undoAction_;
    QAction* redoAction_;
    QAction* cutAction_;
    QAction* copyAction_;
    QAction* pasteAction_;

    // Position Toolbar Controls
    QSpinBox* xCoordSpinBox_;
    QSpinBox* yCoordSpinBox_;
    QSpinBox* zCoordSpinBox_;

    // Sizes Toolbar Controls
    QAction* rectangularBrushShapeAction_;
    QAction* circularBrushShapeAction_;
    QActionGroup* brushShapeActionGroup_; 
    QAction* brushSize1Action_;
    QAction* brushSize2Action_;
    QAction* brushSize3Action_;
    QAction* brushSize4Action_;
    QAction* brushSize5Action_;
    QAction* brushSize6Action_;
    QAction* brushSize7Action_;
    QActionGroup* brushSizeActionGroup_; 

    // Standard Toolbar Controls
    QSpinBox* zoomSpinBox_;     
    QComboBox* layerComboBox_;  

    // Dock Widget Toggle Actions (for menu)
    QAction* viewPaletteDockAction_;    // Added
    QAction* viewMinimapDockAction_;    // Added
    QAction* viewPropertiesDockAction_; // Added



    // Internal clipboard
    ClipboardData* internalClipboard_ = nullptr;

    // Action management
    QMap<MenuBar::ActionID, QAction*> actions_; // Map of ActionID to QAction for easy access

    // Multiple palette management
    QList<QDockWidget*> paletteDocks_; // List of all palette dock widgets
    int paletteCounter_; // Counter for naming new palettes

    // Dockable view management
    QList<QDockWidget*> dockableViews_; // List of additional dockable map views

    // Settings and border system management
    SettingsManager* settingsManager_;
    BorderSystem* borderSystem_;

    // Core map components
    Map* map_;
    Selection* selection_;
    ClipboardData* internalClipboard_;
    MapView* mapView_;

    // Task 77: Brush management
    BrushManager* brushManager_;

    // Refactored managers for better organization
    MenuActionHandler* menuActionHandler_;
    StatusBarManager* statusBarManager_;
    ToolBarManager* toolBarManager_;
    DialogManager* dialogManager_;
    PerspectiveManager* perspectiveManager_;

    // Task 62: Central widget and tab management
    QWidget* centralWidget_;
    QTabWidget* mapTabWidget_;
    QVBoxLayout* centralLayout_;
    QList<MapView*> mapViews_;
    int currentMapTabIndex_;

    // Task 62: Window state management
    QSettings* settings_;
    QTimer* autoSaveTimer_;
    bool windowStateRestored_;
    QString lastSavedPerspective_;

    // Stubbed helper methods for map/position access (can be private)
// private: // Making them private as they are internal helpers for clipboard ops
    Map* getCurrentMap() const;
    MapPos getPasteTargetPosition() const;

private:
    void saveToolBarState();
    void restoreToolBarState();
};

#endif // MAINWINDOW_H
