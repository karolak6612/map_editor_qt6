#ifndef WAYPOINTPALETTEPANEL_H
#define WAYPOINTPALETTEPANEL_H

#include <QWidget>
#include <QListWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QGroupBox>
#include <QSplitter>
#include <QTextEdit>
#include <QSpinBox>
#include <QComboBox>
#include <QColorDialog>
#include <QToolButton>

// Forward declarations
class Map;
class Waypoint;
class EditWaypointDialog;

/**
 * @brief WaypointPalettePanel - UI panel for waypoint management
 * 
 * This panel provides a list view of waypoints and controls for adding,
 * removing, and editing waypoints. It replaces the wxwidgets WaypointPalettePanel.
 */
class WaypointPalettePanel : public QWidget {
    Q_OBJECT

public:
    explicit WaypointPalettePanel(QWidget* parent = nullptr);
    ~WaypointPalettePanel() override;

    // Map integration
    void setMap(Map* map);
    Map* getMap() const { return map_; }
    
    // Waypoint selection
    Waypoint* getSelectedWaypoint() const;
    void selectWaypoint(const QString& name);
    void selectWaypoint(Waypoint* waypoint);
    
    // UI state
    void refreshWaypointList();
    void updateWaypointDetails(Waypoint* waypoint);
    void clearSelection();
    
    // Panel state
    bool isEnabled() const;
    void setEnabled(bool enabled);

signals:
    // Waypoint interaction signals
    void waypointSelected(Waypoint* waypoint);
    void waypointDoubleClicked(Waypoint* waypoint);
    void waypointEditRequested(Waypoint* waypoint);
    void waypointDeleteRequested(Waypoint* waypoint);
    void newWaypointRequested();
    
    // Map navigation signals
    void centerOnWaypoint(Waypoint* waypoint);
    void highlightWaypoint(Waypoint* waypoint);

public slots:
    // Map change handlers
    void onMapChanged();
    void onWaypointAdded(Waypoint* waypoint);
    void onWaypointRemoved(const QString& name);
    void onWaypointModified(Waypoint* waypoint);
    void onWaypointsCleared();

private slots:
    // UI event handlers
    void onWaypointListSelectionChanged();
    void onWaypointListItemDoubleClicked(QListWidgetItem* item);
    void onWaypointListContextMenu(const QPoint& pos);
    void onAddWaypointClicked();
    void onRemoveWaypointClicked();
    void onEditWaypointClicked();
    void onCenterOnWaypointClicked();
    void onRefreshListClicked();
    
    // Quick edit handlers
    void onQuickNameChanged();
    void onQuickTypeChanged();
    void onQuickRadiusChanged(int radius);
    void onQuickColorClicked();
    void onQuickScriptChanged();

private:
    // UI setup
    void setupUI();
    void setupWaypointList();
    void setupControlButtons();
    void setupQuickEditPanel();
    void setupContextMenu();
    
    // Helper methods
    void connectMapSignals();
    void disconnectMapSignals();
    void updateButtonStates();
    void populateWaypointList();
    QListWidgetItem* findWaypointItem(const QString& name) const;
    QListWidgetItem* findWaypointItem(Waypoint* waypoint) const;
    Waypoint* getWaypointFromItem(QListWidgetItem* item) const;
    void showEditDialog(Waypoint* waypoint);
    void updateQuickEditPanel(Waypoint* waypoint);
    void clearQuickEditPanel();
    void applyQuickEdit(Waypoint* waypoint);
    
    // Member variables
    Map* map_;
    Waypoint* selectedWaypoint_;
    bool updatingUI_;
    
    // Main UI components
    QVBoxLayout* mainLayout_;
    QSplitter* splitter_;
    
    // Waypoint list section
    QGroupBox* listGroup_;
    QVBoxLayout* listLayout_;
    QListWidget* waypointList_;
    QLabel* waypointCountLabel_;
    
    // Control buttons
    QHBoxLayout* buttonLayout_;
    QPushButton* addButton_;
    QPushButton* removeButton_;
    QPushButton* editButton_;
    QPushButton* centerButton_;
    QPushButton* refreshButton_;
    
    // Quick edit panel
    QGroupBox* quickEditGroup_;
    QVBoxLayout* quickEditLayout_;
    QLineEdit* quickNameEdit_;
    QComboBox* quickTypeCombo_;
    QSpinBox* quickRadiusSpin_;
    QToolButton* quickColorButton_;
    QTextEdit* quickScriptEdit_;
    QPushButton* applyQuickEditButton_;
    
    // Context menu
    QMenu* contextMenu_;
    QAction* editAction_;
    QAction* deleteAction_;
    QAction* centerAction_;
    QAction* duplicateAction_;
    QAction* exportAction_;
    
    // Edit dialog
    EditWaypointDialog* editDialog_;
};

/**
 * @brief EditWaypointDialog - Dialog for editing waypoint properties
 * 
 * This dialog provides comprehensive editing capabilities for waypoint
 * properties including name, position, type, script, radius, color, and icon.
 */
class EditWaypointDialog : public QDialog {
    Q_OBJECT

public:
    explicit EditWaypointDialog(QWidget* parent = nullptr);
    ~EditWaypointDialog() override;

    // Waypoint editing
    void setWaypoint(Waypoint* waypoint);
    Waypoint* getWaypoint() const { return waypoint_; }
    
    // Dialog modes
    enum EditMode {
        CreateMode,    // Creating new waypoint
        EditMode,      // Editing existing waypoint
        ViewMode       // View-only mode
    };
    
    void setEditMode(EditMode mode);
    EditMode getEditMode() const { return editMode_; }
    
    // Validation
    bool validateInput();
    QString getValidationError() const;

public slots:
    void accept() override;
    void reject() override;

private slots:
    // UI event handlers
    void onNameChanged();
    void onPositionChanged();
    void onTypeChanged();
    void onRadiusChanged(int radius);
    void onColorClicked();
    void onIconChanged();
    void onScriptChanged();
    void onResetClicked();
    void onPreviewClicked();

private:
    // UI setup
    void setupUI();
    void setupBasicProperties();
    void setupAdvancedProperties();
    void setupButtons();
    
    // Helper methods
    void loadWaypointData();
    void saveWaypointData();
    void resetToOriginal();
    void updatePreview();
    void updateButtonStates();
    
    // Member variables
    Waypoint* waypoint_;
    Waypoint* originalWaypoint_; // Backup for reset
    EditMode editMode_;
    bool dataChanged_;
    
    // UI components
    QVBoxLayout* mainLayout_;
    QTabWidget* tabWidget_;
    
    // Basic properties tab
    QWidget* basicTab_;
    QLineEdit* nameEdit_;
    QSpinBox* posXSpin_;
    QSpinBox* posYSpin_;
    QSpinBox* posZSpin_;
    QComboBox* typeCombo_;
    QSpinBox* radiusSpin_;
    QToolButton* colorButton_;
    QComboBox* iconCombo_;
    
    // Advanced properties tab
    QWidget* advancedTab_;
    QTextEdit* scriptEdit_;
    QLineEdit* descriptionEdit_;
    QCheckBox* visibleCheck_;
    QCheckBox* selectableCheck_;
    
    // Button box
    QDialogButtonBox* buttonBox_;
    QPushButton* resetButton_;
    QPushButton* previewButton_;
    
    // Validation
    QString validationError_;
};

#endif // WAYPOINTPALETTEPANEL_H
