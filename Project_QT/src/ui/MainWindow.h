#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

// Forward declarations
class QMenuBar;
class QMenu;
class QAction;
class QToolBar; 
class QSpinBox;    // Added
class QComboBox;   // Added
class QLabel;      // Added
class QActionGroup;// Added

// class MapView; 

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
    void onZoomControlChanged(int value);   // Added
    void onLayerControlChanged(int index); // Added

private:
    // Main setup methods
    void setupMenuBar();
    void setupToolBars(); 
    // void setupDockWidgets(); 
    // void setupStatusBar(); 

    // Helper method for creating actions (used by menu and toolbar setup)
    QAction* createAction(const QString& text, const QString& objectName, const QString& shortcut = "", const QString& statusTip = "", bool checkable = false, bool checked = false);

    // Helper methods for creating top-level menus
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

    // Helper methods for creating toolbars
    QToolBar* createStandardToolBar();   
    QToolBar* createBrushesToolBar();    
    QToolBar* createPositionToolBar();   
    QToolBar* createSizesToolBar();      

    // UI Member Variables
    QMenuBar *menuBar_; 

    // Toolbar Member Variables
    QToolBar* standardToolBar_;   
    QToolBar* brushesToolBar_;    
    QToolBar* positionToolBar_;   
    QToolBar* sizesToolBar_;      

    // Common QAction members (shared between menu and toolbars)
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

    // Standard Toolbar Controls (Zoom and Layer)
    QSpinBox* zoomSpinBox_;     // Added
    QComboBox* layerComboBox_;  // Added
};

#endif // MAINWINDOW_H
