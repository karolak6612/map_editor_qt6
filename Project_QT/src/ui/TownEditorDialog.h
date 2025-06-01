#ifndef TOWNEDITORDIALOG_H
#define TOWNEDITORDIALOG_H

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
#include <QScrollArea>
#include <QFrame>
#include <QMenu>
#include <QAction>
#include <QActionGroup>
#include <QTimer>
#include <QUndoStack>
#include <QUndoCommand>

#include "Town.h"
#include "House.h"
#include "Map.h"
#include "MapView.h"

/**
 * @brief Task 86: Town Editor Dialog
 * 
 * Comprehensive town editing dialog with:
 * - Complete property editing (name, temple position, description)
 * - House management and assignment
 * - Visual preview and validation
 * - Statistics and area calculations
 * - Undo/redo support
 * - 1:1 compatibility with wxwidgets town editor
 */

class TownEditorDialog : public QDialog
{
    Q_OBJECT

public:
    explicit TownEditorDialog(Town* town, Map* map, QUndoStack* undoStack, QWidget* parent = nullptr);
    ~TownEditorDialog() override;

    // Dialog management
    Town* getTown() const { return town_; }
    bool hasChanges() const { return hasChanges_; }
    void setReadOnly(bool readOnly);
    bool isReadOnly() const { return readOnly_; }

public slots:
    void accept() override;
    void reject() override;
    void apply();
    void reset();
    void validateTown();

private slots:
    // Property editing slots
    void onNameChanged();
    void onDescriptionChanged();
    void onTemplePositionChanged();
    void onKeywordsChanged();
    
    // House management slots
    void onAddHouse();
    void onRemoveHouse();
    void onHouseSelectionChanged();
    void onHouseDoubleClicked(QListWidgetItem* item);
    void onRefreshHouses();
    
    // Temple position slots
    void onSelectTemplePosition();
    void onCenterOnTemple();
    void onValidateTemplePosition();
    
    // Validation and preview slots
    void onValidationTimer();
    void onPreviewTown();
    void onCenterOnTown();
    void onHighlightTown();
    void onCalculateStatistics();
    
    // Utility slots
    void onTownChanged();
    void onMapChanged();
    void onUndoRedoChanged();

signals:
    void townModified(Town* town);
    void requestMapHighlight(const QList<MapPos>& positions);
    void requestMapCenter(const MapPos& position);

private:
    void setupUI();
    void setupBasicPropertiesTab();
    void setupHouseManagementTab();
    void setupTempleManagementTab();
    void setupStatisticsTab();
    void setupValidationTab();
    void setupButtonBox();
    
    void connectSignals();
    void loadTownData();
    void saveTownData();
    void updateUI();
    void updateValidation();
    void updatePreview();
    void updateStatistics();
    
    // Validation helpers
    bool validateBasicProperties();
    bool validateTemplePosition();
    bool validateHouses();
    void showValidationResults();
    
    // UI update helpers
    void updateHouseList();
    void updateAvailableHousesList();
    void updateTemplePositionDisplay();
    void updateStatisticsDisplay();
    
    // Data management
    void markAsChanged();
    void createUndoCommand(const QString& description);
    
private:
    // Core data
    Town* town_;
    Town* originalTown_; // For reset functionality
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
    QTextEdit* descriptionEdit_;
    QTextEdit* keywordsEdit_;
    QLabel* idLabel_;
    
    // House Management Tab
    QWidget* houseManagementTab_;
    QListWidget* assignedHousesList_;
    QListWidget* availableHousesList_;
    QPushButton* addHouseButton_;
    QPushButton* removeHouseButton_;
    QPushButton* refreshHousesButton_;
    QLabel* houseCountLabel_;
    QLabel* totalHouseTilesLabel_;
    
    // Temple Management Tab
    QWidget* templeManagementTab_;
    QSpinBox* templeXSpinBox_;
    QSpinBox* templeYSpinBox_;
    QSpinBox* templeZSpinBox_;
    QPushButton* selectTempleButton_;
    QPushButton* centerOnTempleButton_;
    QPushButton* validateTempleButton_;
    QLabel* templeStatusLabel_;
    
    // Statistics Tab
    QWidget* statisticsTab_;
    QTextEdit* statisticsDisplay_;
    QPushButton* calculateStatsButton_;
    QProgressBar* calculationProgress_;
    
    // Validation Tab
    QWidget* validationTab_;
    QTextEdit* validationResults_;
    QProgressBar* validationProgress_;
    QPushButton* validateButton_;
    QPushButton* previewButton_;
    QPushButton* centerButton_;
    QPushButton* highlightButton_;
    
    // Button box
    QPushButton* okButton_;
    QPushButton* cancelButton_;
    QPushButton* applyButton_;
    QPushButton* resetButton_;
    
    // Timers and utilities
    QTimer* validationTimer_;
    QTimer* statisticsTimer_;
};

/**
 * @brief Towns Management Dialog
 * 
 * Dialog for managing all towns in the map:
 * - List all towns with filtering and sorting
 * - Create, edit, and delete towns
 * - Import/export town data
 * - Bulk operations and validation
 */
class TownsManagementDialog : public QDialog
{
    Q_OBJECT

public:
    explicit TownsManagementDialog(Map* map, QUndoStack* undoStack, QWidget* parent = nullptr);
    ~TownsManagementDialog() override;

public slots:
    void refreshTownsList();
    void validateAllTowns();
    void exportTowns();
    void importTowns();

private slots:
    // Town management slots
    void onCreateTown();
    void onEditTown();
    void onDeleteTown();
    void onDuplicateTown();
    void onTownSelectionChanged();
    void onTownDoubleClicked(QListWidgetItem* item);
    
    // Filtering and sorting slots
    void onFilterChanged();
    void onSortChanged();
    void onSearchChanged();
    
    // Bulk operations slots
    void onSelectAllTowns();
    void onDeselectAllTowns();
    void onDeleteSelectedTowns();
    void onValidateSelectedTowns();
    
    // Import/Export slots
    void onExportSelected();
    void onImportFromFile();
    void onExportToFile();

signals:
    void townCreated(Town* town);
    void townModified(Town* town);
    void townDeleted(quint32 townId);

private:
    void setupUI();
    void setupTownsList();
    void setupFilterControls();
    void setupButtonBox();
    void connectSignals();
    
    void updateTownsList();
    void updateTownDetails();
    void updateButtonStates();
    
    // Helper methods
    QList<Town*> getSelectedTowns() const;
    void selectTown(Town* town);
    bool confirmDeletion(const QList<Town*>& towns);

private:
    // Core data
    Map* map_;
    QUndoStack* undoStack_;
    
    // UI components
    QSplitter* mainSplitter_;
    
    // Towns list
    QListWidget* townsList_;
    QLineEdit* searchEdit_;
    QComboBox* sortCombo_;
    QComboBox* filterCombo_;
    
    // Town details
    QTextEdit* townDetails_;
    
    // Buttons
    QPushButton* createButton_;
    QPushButton* editButton_;
    QPushButton* deleteButton_;
    QPushButton* duplicateButton_;
    QPushButton* validateButton_;
    QPushButton* exportButton_;
    QPushButton* importButton_;
    QPushButton* selectAllButton_;
    QPushButton* deselectAllButton_;
    
    // Status
    QLabel* statusLabel_;
    QProgressBar* progressBar_;
};

#endif // TOWNEDITORDIALOG_H
