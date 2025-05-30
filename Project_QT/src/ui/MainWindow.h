#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

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
class PlaceholderPaletteWidget;     // Added
class PlaceholderMinimapWidget;     // Added
class PlaceholderPropertiesWidget;  // Added


class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

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
    // void setupStatusBar(); 

    // Helper method for creating actions
    QAction* createAction(const QString& text, const QString& objectName, const QString& shortcut = "", const QString& statusTip = "", bool checkable = false, bool checked = false);

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
};

#endif // MAINWINDOW_H
