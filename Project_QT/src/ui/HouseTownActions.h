#ifndef HOUSETOWNACTIONS_H
#define HOUSETOWNACTIONS_H

#include <QObject>
#include <QAction>
#include <QActionGroup>
#include <QMenu>
#include <QMenuBar>
#include <QToolBar>
#include <QKeySequence>
#include <QIcon>
#include <QString>
#include <QUndoStack>

#include "House.h"
#include "Town.h"
#include "Map.h"
#include "MapView.h"

// Forward declarations
class HouseEditorDialog;
class TownEditorDialog;
class TownsManagementDialog;

/**
 * @brief Task 86: House and Town Management Actions
 * 
 * Complete UI actions system for house and town management:
 * - Menu actions for all house/town operations
 * - Toolbar integration with icons and shortcuts
 * - Context menu support for map interactions
 * - Keyboard shortcuts (1:1 wxwidgets compatibility)
 * - Action state management and validation
 * - Integration with undo/redo system
 */

class HouseTownActions : public QObject
{
    Q_OBJECT

public:
    explicit HouseTownActions(Map* map, MapView* mapView, QUndoStack* undoStack, QObject* parent = nullptr);
    ~HouseTownActions() override;

    // Action access
    QAction* getEditHousesAction() const { return editHousesAction_; }
    QAction* getEditTownsAction() const { return editTownsAction_; }
    QAction* getCreateHouseAction() const { return createHouseAction_; }
    QAction* getCreateTownAction() const { return createTownAction_; }
    QAction* getDeleteHouseAction() const { return deleteHouseAction_; }
    QAction* getDeleteTownAction() const { return deleteTownAction_; }
    QAction* getShowHousesAction() const { return showHousesAction_; }
    QAction* getShowTownsAction() const { return showTownsAction_; }
    QAction* getHouseBrushAction() const { return houseBrushAction_; }
    QAction* getHouseExitBrushAction() const { return houseExitBrushAction_; }
    QAction* getValidateHousesAction() const { return validateHousesAction_; }
    QAction* getCleanHouseItemsAction() const { return cleanHouseItemsAction_; }
    QAction* getClearInvalidHousesAction() const { return clearInvalidHousesAction_; }
    QAction* getImportHousesAction() const { return importHousesAction_; }
    QAction* getExportHousesAction() const { return exportHousesAction_; }
    QAction* getImportTownsAction() const { return importTownsAction_; }
    QAction* getExportTownsAction() const { return exportTownsAction_; }
    QAction* getHouseStatisticsAction() const { return houseStatisticsAction_; }
    QAction* getTownStatisticsAction() const { return townStatisticsAction_; }

    // Menu integration
    void addToMenu(QMenu* editMenu, QMenu* viewMenu, QMenu* toolsMenu);
    void addToMenuBar(QMenuBar* menuBar);
    void addToToolBar(QToolBar* toolBar);
    void addToContextMenu(QMenu* contextMenu, const MapPos& position);

    // Action state management
    void updateActionStates();
    void setMapLoaded(bool loaded);
    void setHouseSelected(House* house);
    void setTownSelected(Town* town);

public slots:
    // House management slots
    void editHouses();
    void createHouse();
    void createHouseAt(const MapPos& position);
    void editHouse(House* house = nullptr);
    void deleteHouse(House* house = nullptr);
    void duplicateHouse(House* house = nullptr);
    void validateHouses();
    void cleanHouseItems();
    void clearInvalidHouses();
    
    // Town management slots
    void editTowns();
    void createTown();
    void createTownAt(const MapPos& position);
    void editTown(Town* town = nullptr);
    void deleteTown(Town* town = nullptr);
    void duplicateTown(Town* town = nullptr);
    void validateTowns();
    
    // View management slots
    void toggleShowHouses(bool show);
    void toggleShowTowns(bool show);
    void centerOnHouse(House* house);
    void centerOnTown(Town* town);
    void highlightHouse(House* house);
    void highlightTown(Town* town);
    
    // Brush management slots
    void activateHouseBrush();
    void activateHouseExitBrush();
    void deactivateBrushes();
    
    // Import/Export slots
    void importHouses();
    void exportHouses();
    void importTowns();
    void exportTowns();
    void importHousesFromFile(const QString& filePath);
    void exportHousesToFile(const QString& filePath);
    void importTownsFromFile(const QString& filePath);
    void exportTownsToFile(const QString& filePath);
    
    // Statistics slots
    void showHouseStatistics();
    void showTownStatistics();
    void showHouseTownStatistics();
    
    // Context menu slots
    void showHouseContextMenu(const QPoint& position, House* house);
    void showTownContextMenu(const QPoint& position, Town* town);
    void showMapContextMenu(const QPoint& position, const MapPos& mapPos);

signals:
    void houseCreated(House* house);
    void houseModified(House* house);
    void houseDeleted(quint32 houseId);
    void townCreated(Town* town);
    void townModified(Town* town);
    void townDeleted(quint32 townId);
    void requestMapHighlight(const QList<MapPos>& positions);
    void requestMapCenter(const MapPos& position);
    void brushActivated(const QString& brushName);
    void brushDeactivated();

private slots:
    void onMapChanged();
    void onHouseChanged();
    void onTownChanged();
    void onSelectionChanged();

private:
    void createActions();
    void createHouseActions();
    void createTownActions();
    void createViewActions();
    void createBrushActions();
    void createImportExportActions();
    void createStatisticsActions();
    void connectSignals();
    
    void setupActionProperties();
    void setupIcons();
    void setupShortcuts();
    void setupTooltips();
    
    // Helper methods
    House* getCurrentHouse() const;
    Town* getCurrentTown() const;
    House* getHouseAt(const MapPos& position) const;
    Town* getTownAt(const MapPos& position) const;
    bool confirmHouseDeletion(House* house) const;
    bool confirmTownDeletion(Town* town) const;
    void showOperationResult(const QString& operation, bool success, const QString& details = QString());

private:
    // Core components
    Map* map_;
    MapView* mapView_;
    QUndoStack* undoStack_;
    
    // State tracking
    bool mapLoaded_;
    House* selectedHouse_;
    Town* selectedTown_;
    
    // House management actions
    QAction* editHousesAction_;
    QAction* createHouseAction_;
    QAction* editHouseAction_;
    QAction* deleteHouseAction_;
    QAction* duplicateHouseAction_;
    QAction* validateHousesAction_;
    QAction* cleanHouseItemsAction_;
    QAction* clearInvalidHousesAction_;
    
    // Town management actions
    QAction* editTownsAction_;
    QAction* createTownAction_;
    QAction* editTownAction_;
    QAction* deleteTownAction_;
    QAction* duplicateTownAction_;
    QAction* validateTownsAction_;
    
    // View actions
    QAction* showHousesAction_;
    QAction* showTownsAction_;
    QAction* centerOnHouseAction_;
    QAction* centerOnTownAction_;
    QAction* highlightHouseAction_;
    QAction* highlightTownAction_;
    
    // Brush actions
    QAction* houseBrushAction_;
    QAction* houseExitBrushAction_;
    
    // Import/Export actions
    QAction* importHousesAction_;
    QAction* exportHousesAction_;
    QAction* importTownsAction_;
    QAction* exportTownsAction_;
    
    // Statistics actions
    QAction* houseStatisticsAction_;
    QAction* townStatisticsAction_;
    QAction* houseTownStatisticsAction_;
    
    // Action groups
    QActionGroup* brushActionGroup_;
    QActionGroup* viewActionGroup_;
    
    // Dialogs (cached for performance)
    HouseEditorDialog* houseEditorDialog_;
    TownEditorDialog* townEditorDialog_;
    TownsManagementDialog* townsManagementDialog_;
};

/**
 * @brief House and Town Context Menu Provider
 */
class HouseTownContextMenu : public QObject
{
    Q_OBJECT

public:
    explicit HouseTownContextMenu(HouseTownActions* actions, QObject* parent = nullptr);
    ~HouseTownContextMenu() override = default;

    // Context menu creation
    QMenu* createHouseContextMenu(House* house, QWidget* parent = nullptr);
    QMenu* createTownContextMenu(Town* town, QWidget* parent = nullptr);
    QMenu* createMapContextMenu(const MapPos& position, QWidget* parent = nullptr);

private:
    HouseTownActions* actions_;
};

/**
 * @brief House and Town Keyboard Shortcuts Manager
 */
class HouseTownShortcuts : public QObject
{
    Q_OBJECT

public:
    explicit HouseTownShortcuts(HouseTownActions* actions, QWidget* parent = nullptr);
    ~HouseTownShortcuts() override = default;

    // Shortcut management
    void setupShortcuts();
    void updateShortcuts();
    void enableShortcuts(bool enabled);

private:
    HouseTownActions* actions_;
    QWidget* parentWidget_;
};

#endif // HOUSETOWNACTIONS_H
