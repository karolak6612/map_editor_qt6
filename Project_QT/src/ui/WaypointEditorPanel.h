#ifndef WAYPOINTEDITORPANEL_H
#define WAYPOINTEDITORPANEL_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QListWidget>
#include <QListWidgetItem>
#include <QPushButton>
#include <QToolButton>
#include <QLineEdit>
#include <QTextEdit>
#include <QSpinBox>
#include <QComboBox>
#include <QLabel>
#include <QSplitter>
#include <QTabWidget>
#include <QCheckBox>
#include <QSlider>
#include <QColorDialog>
#include <QMenu>
#include <QAction>
#include <QTimer>
#include <QProgressBar>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QHeaderView>
#include <QFileDialog>
#include <QMessageBox>
#include "../MapPos.h" // For MapPos struct definition

// Forward declarations
class Map;
class Waypoint;
class Waypoints;
class WaypointBrush;

// Task 71: Enhanced waypoint editor panel for full waypoint system functionality
class WaypointEditorPanel : public QWidget {
    Q_OBJECT

public:
    explicit WaypointEditorPanel(QWidget* parent = nullptr);
    ~WaypointEditorPanel() override;

    // Map integration
    void setMap(Map* map);
    Map* getMap() const { return map_; }
    
    // Waypoint selection and management
    Waypoint* getSelectedWaypoint() const;
    void selectWaypoint(const QString& name);
    void selectWaypoint(Waypoint* waypoint);
    void clearSelection();
    
    // UI state management
    void refreshWaypointList();
    void updateButtonStates();
    void setReadOnly(bool readOnly);
    bool isReadOnly() const { return readOnly_; }
    
    // Search and filtering
    void setSearchFilter(const QString& filter);
    void setTypeFilter(const QString& type);
    void clearFilters();
    
    // Import/Export functionality
    bool exportWaypoints(const QString& filePath) const;
    bool importWaypoints(const QString& filePath);

signals:
    // Waypoint management signals
    void waypointSelected(Waypoint* waypoint);
    void waypointDoubleClicked(Waypoint* waypoint);
    void waypointModified(Waypoint* waypoint);  // Task 019: Added missing signal
    void newWaypointRequested();
    void waypointDeleteRequested(Waypoint* waypoint);
    void waypointEditRequested(Waypoint* waypoint);
    
    // Navigation signals
    void centerOnWaypoint(Waypoint* waypoint);
    void goToWaypoint(Waypoint* waypoint);
    void findWaypointRequested(const QString& searchTerm);
    
    // Brush signals
    void waypointBrushRequested(Waypoint* waypoint);
    void waypointToolActivated();

public slots:
    // Map change handlers
    void onMapChanged();
    void onWaypointAdded(Waypoint* waypoint);
    void onWaypointRemoved(const QString& name);
    void onWaypointRemoved(Waypoint* waypoint);
    void onWaypointModified(Waypoint* waypoint);
    void onWaypointsCleared();
    
    // External waypoint operations
    void addWaypoint(Waypoint* waypoint);
    void removeWaypoint(Waypoint* waypoint);
    void updateWaypoint(Waypoint* waypoint);

private slots:
    // UI event handlers
    void onWaypointListSelectionChanged();
    void onWaypointListItemDoubleClicked(QListWidgetItem* item);
    void onWaypointListContextMenu(const QPoint& pos);
    
    // Button handlers
    void onAddWaypointClicked();
    void onRemoveWaypointClicked();
    void onEditWaypointClicked();
    void onDuplicateWaypointClicked();
    void onCenterOnWaypointClicked();
    void onGoToWaypointClicked();
    void onRefreshListClicked();
    
    // Property editor handlers
    void onPropertyChanged();
    void onNameChanged();
    void onTypeChanged();
    void onPositionChanged();
    void onRadiusChanged(int radius);
    void onColorChanged();
    void onScriptChanged();
    
    // Search and filter handlers
    void onSearchTextChanged(const QString& text);
    void onTypeFilterChanged(const QString& type);
    void onClearFiltersClicked();
    
    // Import/Export handlers
    void onExportWaypointsClicked();
    void onImportWaypointsClicked();
    
    // Advanced features
    void onBatchEditClicked();
    void onSortWaypointsClicked();
    void onValidateWaypointsClicked();

private:
    // UI setup methods
    void setupUI();
    void setupWaypointList();
    void setupControlButtons();
    void setupPropertyEditor();
    void setupSearchAndFilter();
    void setupAdvancedTools();
    void setupContextMenu();
    void connectSignals();
    void disconnectMapSignals();
    void connectMapSignals();
    
    // Helper methods
    void populateWaypointList();
    void updateWaypointListItem(QListWidgetItem* item, Waypoint* waypoint);
    QListWidgetItem* findWaypointItem(const QString& name) const;
    QListWidgetItem* findWaypointItem(Waypoint* waypoint) const;
    Waypoint* getWaypointFromItem(QListWidgetItem* item) const;
    void updatePropertyEditor(Waypoint* waypoint);
    void clearPropertyEditor();
    void applyPropertyChanges();
    bool validateWaypointData() const;
    
    // Filter and search helpers
    bool matchesFilter(Waypoint* waypoint) const;
    void applyFilters();
    QStringList getAvailableTypes() const;
    
    // Import/Export helpers
    bool exportToXML(const QString& filePath) const;
    bool exportToJSON(const QString& filePath) const;
    bool importFromXML(const QString& filePath);
    bool importFromJSON(const QString& filePath);
    
    // Main UI components
    QVBoxLayout* mainLayout_;
    QSplitter* mainSplitter_;
    QTabWidget* tabWidget_;
    
    // Waypoint list tab
    QWidget* listTab_;
    QVBoxLayout* listLayout_;
    QGroupBox* listGroup_;
    QListWidget* waypointList_;
    QLabel* waypointCountLabel_;
    
    // Control buttons
    QHBoxLayout* buttonLayout_;
    QPushButton* addButton_;
    QPushButton* removeButton_;
    QPushButton* editButton_;
    QPushButton* duplicateButton_;
    QPushButton* centerButton_;
    QPushButton* goToButton_;
    QPushButton* refreshButton_;
    
    // Search and filter
    QGroupBox* filterGroup_;
    QGridLayout* filterLayout_;
    QLineEdit* searchEdit_;
    QComboBox* typeFilterCombo_;
    QPushButton* clearFiltersButton_;
    
    // Property editor tab
    QWidget* propertyTab_;
    QVBoxLayout* propertyLayout_;
    QGroupBox* propertyGroup_;
    QGridLayout* propertyGridLayout_;
    
    // Property controls
    QLineEdit* nameEdit_;
    QComboBox* typeCombo_;
    QSpinBox* xSpinBox_;
    QSpinBox* ySpinBox_;
    QSpinBox* zSpinBox_;
    QSpinBox* radiusSpinBox_;
    QToolButton* colorButton_;
    QTextEdit* scriptEdit_;
    QPushButton* applyButton_;
    
    // Advanced tools tab
    QWidget* toolsTab_;
    QVBoxLayout* toolsLayout_;
    QPushButton* exportButton_;
    QPushButton* importButton_;
    QPushButton* batchEditButton_;
    QPushButton* sortButton_;
    QPushButton* validateButton_;
    QProgressBar* progressBar_;
    
    // Context menu
    QMenu* contextMenu_;
    QAction* editAction_;
    QAction* deleteAction_;
    QAction* duplicateAction_;
    QAction* centerAction_;
    QAction* goToAction_;
    QAction* exportAction_;
    
    // Data and state
    Map* map_;
    Waypoint* selectedWaypoint_;
    bool updatingUI_;
    bool readOnly_;
    QString searchFilter_;
    QString typeFilter_;
    
    // Auto-refresh timer
    QTimer* refreshTimer_;
};

// Task 71: Waypoint creation dialog for new waypoint setup
class WaypointCreationDialog : public QDialog {
    Q_OBJECT

public:
    explicit WaypointCreationDialog(Map* map, const MapPos& position = MapPos(), QWidget* parent = nullptr);
    ~WaypointCreationDialog() override;

    // Get the created waypoint data
    QString getWaypointName() const;
    QString getWaypointType() const;
    MapPos getWaypointPosition() const;
    int getWaypointRadius() const;
    QColor getWaypointColor() const;
    QString getWaypointScript() const;
    
    // Set initial values
    void setWaypointName(const QString& name);
    void setWaypointType(const QString& type);
    void setWaypointPosition(const MapPos& position);

private slots:
    void onNameChanged();
    void onPositionChanged();
    void onColorButtonClicked();
    void onValidateInput();

private:
    void setupUI();
    void connectSignals();
    bool validateInput() const;
    void updatePreview();
    
    Map* map_;
    
    // UI components
    QVBoxLayout* mainLayout_;
    QGridLayout* formLayout_;
    QLineEdit* nameEdit_;
    QComboBox* typeCombo_;
    QSpinBox* xSpinBox_;
    QSpinBox* ySpinBox_;
    QSpinBox* zSpinBox_;
    QSpinBox* radiusSpinBox_;
    QToolButton* colorButton_;
    QTextEdit* scriptEdit_;
    QLabel* previewLabel_;
    QDialogButtonBox* buttonBox_;
    
    // Initial values
    MapPos initialPosition_;
    QColor selectedColor_;
};

#endif // WAYPOINTEDITORPANEL_H
