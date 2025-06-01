#ifndef HOUSEEDITORDIALOG_H
#define HOUSEEDITORDIALOG_H

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QFormLayout>
#include <QTabWidget>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QComboBox>
#include <QCheckBox>
#include <QTextEdit>
#include <QListWidget>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QPushButton>
#include <QToolButton>
#include <QButtonGroup>
#include <QSplitter>
#include <QProgressBar>
#include <QDateTimeEdit>
#include <QSlider>
#include <QScrollArea>
#include <QFrame>
#include <QMenu>
#include <QAction>
#include <QActionGroup>
#include <QTimer>
#include <QUndoStack>
#include <QUndoCommand>

#include "House.h"
#include "Town.h"
#include "Map.h"
#include "MapView.h"

/**
 * @brief Task 86: House Editor Dialog
 * 
 * Comprehensive house editing dialog with:
 * - Complete property editing (name, owner, rent, town, etc.)
 * - Tile management with visual selection
 * - Door and bed management
 * - Access control (guests, subowners)
 * - Visual preview and validation
 * - Undo/redo support
 * - 1:1 compatibility with wxwidgets house editor
 */

class HouseEditorDialog : public QDialog
{
    Q_OBJECT

public:
    explicit HouseEditorDialog(House* house, Map* map, QUndoStack* undoStack, QWidget* parent = nullptr);
    ~HouseEditorDialog() override;

    // Dialog management
    House* getHouse() const { return house_; }
    bool hasChanges() const { return hasChanges_; }
    void setReadOnly(bool readOnly);
    bool isReadOnly() const { return readOnly_; }

public slots:
    void accept() override;
    void reject() override;
    void apply();
    void reset();
    void validateHouse();

private slots:
    // Property editing slots
    void onNameChanged();
    void onOwnerChanged();
    void onRentChanged();
    void onTownChanged();
    void onGuildHallChanged();
    void onDescriptionChanged();
    void onPaidUntilChanged();
    
    // Tile management slots
    void onAddTile();
    void onRemoveTile();
    void onClearTiles();
    void onSelectTilesOnMap();
    void onTileSelectionChanged();
    void onTileDoubleClicked(QListWidgetItem* item);
    
    // Door management slots
    void onAddDoor();
    void onEditDoor();
    void onRemoveDoor();
    void onDoorSelectionChanged();
    void onDoorDoubleClicked(QTreeWidgetItem* item, int column);
    
    // Bed management slots
    void onAddBed();
    void onEditBed();
    void onRemoveBed();
    void onBedSelectionChanged();
    void onBedDoubleClicked(QTreeWidgetItem* item, int column);
    
    // Access management slots
    void onAddGuest();
    void onRemoveGuest();
    void onAddSubowner();
    void onRemoveSubowner();
    void onAccessSelectionChanged();
    
    // Validation and preview slots
    void onValidationTimer();
    void onPreviewHouse();
    void onCenterOnHouse();
    void onHighlightHouse();
    
    // Utility slots
    void onHouseChanged();
    void onMapChanged();
    void onUndoRedoChanged();

signals:
    void houseModified(House* house);
    void requestMapHighlight(const QList<MapPos>& positions);
    void requestMapCenter(const MapPos& position);

private:
    void setupUI();
    void setupBasicPropertiesTab();
    void setupTileManagementTab();
    void setupDoorManagementTab();
    void setupBedManagementTab();
    void setupAccessManagementTab();
    void setupValidationTab();
    void setupButtonBox();
    
    void connectSignals();
    void loadHouseData();
    void saveHouseData();
    void updateUI();
    void updateValidation();
    void updatePreview();
    void updateStatistics();
    
    // Validation helpers
    bool validateBasicProperties();
    bool validateTiles();
    bool validateDoors();
    bool validateBeds();
    bool validateAccess();
    void showValidationResults();
    
    // UI update helpers
    void updateTileList();
    void updateDoorTree();
    void updateBedTree();
    void updateAccessLists();
    void updateTownCombo();
    void updateStatisticsDisplay();
    
    // Data management
    void markAsChanged();
    void createUndoCommand(const QString& description);
    
private:
    // Core data
    House* house_;
    House* originalHouse_; // For reset functionality
    Map* map_;
    QUndoStack* undoStack_;
    
    // State management
    bool hasChanges_;
    bool readOnly_;
    bool isLoading_;
    
    // UI components
    QTabWidget* tabWidget_;
    
    // Basic Properties Tab
    QWidget* basicPropertiesTab_;
    QLineEdit* nameEdit_;
    QLineEdit* ownerEdit_;
    QSpinBox* rentSpinBox_;
    QComboBox* townCombo_;
    QCheckBox* guildHallCheckBox_;
    QTextEdit* descriptionEdit_;
    QDateTimeEdit* paidUntilEdit_;
    
    // Tile Management Tab
    QWidget* tileManagementTab_;
    QListWidget* tileList_;
    QPushButton* addTileButton_;
    QPushButton* removeTileButton_;
    QPushButton* clearTilesButton_;
    QPushButton* selectTilesButton_;
    QLabel* tileCountLabel_;
    QLabel* tileAreaLabel_;
    
    // Door Management Tab
    QWidget* doorManagementTab_;
    QTreeWidget* doorTree_;
    QPushButton* addDoorButton_;
    QPushButton* editDoorButton_;
    QPushButton* removeDoorButton_;
    QLabel* doorCountLabel_;
    
    // Bed Management Tab
    QWidget* bedManagementTab_;
    QTreeWidget* bedTree_;
    QPushButton* addBedButton_;
    QPushButton* editBedButton_;
    QPushButton* removeBedButton_;
    QLabel* bedCountLabel_;
    
    // Access Management Tab
    QWidget* accessManagementTab_;
    QListWidget* guestList_;
    QListWidget* subownerList_;
    QLineEdit* guestEdit_;
    QLineEdit* subownerEdit_;
    QPushButton* addGuestButton_;
    QPushButton* removeGuestButton_;
    QPushButton* addSubownerButton_;
    QPushButton* removeSubownerButton_;
    
    // Validation Tab
    QWidget* validationTab_;
    QTextEdit* validationResults_;
    QProgressBar* validationProgress_;
    QPushButton* validateButton_;
    QPushButton* previewButton_;
    QPushButton* centerButton_;
    QPushButton* highlightButton_;
    
    // Statistics display
    QLabel* statisticsLabel_;
    QTextEdit* statisticsDisplay_;
    
    // Button box
    QPushButton* okButton_;
    QPushButton* cancelButton_;
    QPushButton* applyButton_;
    QPushButton* resetButton_;
    
    // Timers and utilities
    QTimer* validationTimer_;
    QTimer* previewTimer_;
};

/**
 * @brief House Door Editor Dialog
 */
class HouseDoorEditorDialog : public QDialog
{
    Q_OBJECT

public:
    explicit HouseDoorEditorDialog(const HouseDoor& door, QWidget* parent = nullptr);
    ~HouseDoorEditorDialog() override = default;

    HouseDoor getDoor() const;
    void setDoor(const HouseDoor& door);

private slots:
    void onPositionChanged();
    void onDoorIdChanged();
    void onNameChanged();
    void onLockedChanged();
    void onAccessListChanged();

private:
    void setupUI();
    void loadDoorData();
    void saveDoorData();
    void validateDoor();

private:
    HouseDoor door_;
    
    // UI components
    QSpinBox* xSpinBox_;
    QSpinBox* ySpinBox_;
    QSpinBox* zSpinBox_;
    QSpinBox* doorIdSpinBox_;
    QLineEdit* nameEdit_;
    QCheckBox* lockedCheckBox_;
    QTextEdit* accessListEdit_;
};

/**
 * @brief House Bed Editor Dialog
 */
class HouseBedEditorDialog : public QDialog
{
    Q_OBJECT

public:
    explicit HouseBedEditorDialog(const HouseBed& bed, QWidget* parent = nullptr);
    ~HouseBedEditorDialog() override = default;

    HouseBed getBed() const;
    void setBed(const HouseBed& bed);

private slots:
    void onPositionChanged();
    void onBedIdChanged();
    void onOwnerNameChanged();
    void onLastUsedChanged();

private:
    void setupUI();
    void loadBedData();
    void saveBedData();
    void validateBed();

private:
    HouseBed bed_;
    
    // UI components
    QSpinBox* xSpinBox_;
    QSpinBox* ySpinBox_;
    QSpinBox* zSpinBox_;
    QSpinBox* bedIdSpinBox_;
    QLineEdit* ownerNameEdit_;
    QDateTimeEdit* lastUsedEdit_;
};

#endif // HOUSEEDITORDIALOG_H
