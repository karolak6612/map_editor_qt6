#ifndef MENUBUILDER_H
#define MENUBUILDER_H

#include <QObject>
#include <QMenu>
#include <QAction>
#include <QIcon>
#include <QKeySequence>
#include <QMap>

// Forward declarations
class MainWindow;

/**
 * @brief MenuBuilder - Helper class for building MainWindow menus
 * 
 * This class extracts all menu creation logic from MainWindow.cpp to comply with mandate M6.
 * It handles creation of all menu items, action setup, and menu organization.
 * 
 * Task 011: Refactor large source files - Extract menu building logic from MainWindow.cpp
 */

#include "../MenuBarActionIDs.h" // Use primary definition from MenuBarActionIDs.h

class MenuBuilder : public QObject
{
    Q_OBJECT

public:

    explicit MenuBuilder(MainWindow* mainWindow, QObject *parent = nullptr);
    ~MenuBuilder() override;

    // Main menu creation methods
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

    // Action creation utilities
    QAction* createAction(const QString& text, const QString& objectName, 
                         const QIcon& icon = QIcon(), const QString& shortcut = QString(), 
                         const QString& statusTip = QString(), bool checkable = false, 
                         bool checked = false, bool connectToGenericHandler = true);
    
    QAction* createActionWithId(MenuBar::ActionID actionId, const QString& text, 
                               const QIcon& icon = QIcon(), const QString& shortcut = QString(), 
                               const QString& statusTip = QString(), bool checkable = false, 
                               bool checked = false);

    // Action retrieval
    QAction* getAction(MenuBar::ActionID actionId) const;

    // Get action map for MainWindow compatibility
    const QMap<MenuBar::ActionID, QAction*>& getActions() const { return actions_; }

private:
    MainWindow* mainWindow_;
    QMap<MenuBar::ActionID, QAction*> actions_;

    // Helper methods for submenu creation
    QMenu* createImportSubmenu(QMenu* parent);
    QMenu* createExportSubmenu(QMenu* parent);
    QMenu* createBorderOptionsSubmenu(QMenu* parent);
    QMenu* createOtherOptionsSubmenu(QMenu* parent);
    QMenu* createZoomSubmenu(QMenu* parent);
    QMenu* createFloorSubmenu(QMenu* parent);
    QMenu* createPerspectiveSubmenu(QMenu* parent);
    QMenu* createRecentFilesSubmenu(QMenu* parent);
};

#endif // MENUBUILDER_H
