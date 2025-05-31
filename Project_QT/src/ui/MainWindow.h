#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPointF> // Required for QPointF parameter

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
class PlaceholderPropertiesWidget;  // Added
class QStatusBar;                   // Added for statusBar_ member or statusBar() usage
class QCloseEvent; // Added for closeEvent
class AutomagicSettingsDialog; // Forward declaration
class ClipboardData;           // Forward declaration for clipboard
class Map;                     // Already forward declared in Map.h, but good practice if Map.h isn't fully included here
class Selection;               // Already forward declared in Selection.h, but good practice
class MapPos;                  // Required for updateMouseMapCoordinates if Map.h doesn't bring it transitively


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
    // bool canPaste() const; // canPaste is not a slot, remains public method

public: // Public methods for other status updates if not slots (canPaste kept here)
    bool canPaste() const;
    void updateCurrentBrush(const QString& brushName);
    void updateSelectedItemInfo(const QString& itemInfo);

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

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void onMenuActionTriggered(); 
    void onPositionGo();
    void onPositionXChanged(int value);
    void onPositionYChanged(int value);
    void onPositionZChanged(int value);
    void onZoomControlChanged(int value);   
    void onLayerControlChanged(int index); 

private:
    // Main setup methods
    void setupMenuBar();
    void setupToolBars(); 
    void setupDockWidgets(); // Added
    void setupStatusBar();

    // Helper method for creating actions
    QAction* createAction(const QString& text, const QString& objectName, const QIcon& icon = QIcon(), const QString& shortcut = "", const QString& statusTip = "", bool checkable = false, bool checked = false, bool connectToGenericHandler = true);

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

    // Status Bar Labels
    QLabel* mouseCoordsLabel_ = nullptr;
    QLabel* itemInfoLabel_ = nullptr;
    QLabel* zoomLevelLabel_ = nullptr;
    QLabel* currentLayerLabel_ = nullptr;
    QLabel* brushInfoLabel_ = nullptr;

    // Internal clipboard
    ClipboardData* internalClipboard_ = nullptr;

    // Stubbed helper methods for map/position access (can be private)
// private: // Making them private as they are internal helpers for clipboard ops
    Map* getCurrentMap() const; 
    MapPos getPasteTargetPosition() const;

private:
    void saveToolBarState();
    void restoreToolBarState();
};

#endif // MAINWINDOW_H
