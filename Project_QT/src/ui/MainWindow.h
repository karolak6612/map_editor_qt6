#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

// Forward declarations
class QMenuBar;
class QMenu; // Added for helper return types
class QAction;
// class MapView; // Example

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    void onMenuActionTriggered(); 

private:
    void setupMenuBar();

    // Helper methods for creating top-level menus
    QMenu* createFileMenu();
    QMenu* createEditMenu();
    QMenu* createEditorMenu(); // Renamed from createViewMenu based on XML structure "Editor" (for view related like zoom, new view)
    QMenu* createSearchMenu(); // Added based on XML
    QMenu* createMapMenu();
    QMenu* createSelectionMenu();
    QMenu* createViewMenu();   // This is for visibility options like "Show all Floors", "Show Grid", etc. (XML "View")
    QMenu* createShowMenu();   // This is for "Show Animation", "Show Light", etc. (XML "Show")
    QMenu* createNavigateMenu();
    QMenu* createWindowMenu();
    QMenu* createExperimentalMenu();
    QMenu* createAboutMenu(); // For "About..." etc. (XML "About")
    QMenu* createServerMenu(); // Added based on XML
    QMenu* createIdlerMenu();  // Added based on XML

    // Placeholder for actions that might need to be accessed later (e.g., for enabling/disabling)
    // File Menu Actions
    // QAction *newAction_;
    // QAction *openAction_;
    // QAction *saveAction_;
    // QAction *saveAsAction_;
    // QAction *generateMapAction_;
    // QAction *closeMapAction_;
    // QAction *exitAction_;
    // QAction *preferencesAction_;

    // Edit Menu Actions
    // QAction *undoAction_;
    // QAction *redoAction_;
    // QAction *cutAction_;
    // QAction *copyAction_;
    // QAction *pasteAction_;
    
    // ... Add more QAction members as needed if they require dynamic updates ...

    QMenuBar *menuBar_; 
};

#endif // MAINWINDOW_H
