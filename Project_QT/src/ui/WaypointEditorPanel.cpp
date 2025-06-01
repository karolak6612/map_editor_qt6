#include "WaypointEditorPanel.h"
#include "Map.h"
#include "Waypoint.h"
#include "Waypoints.h"
#include "WaypointBrush.h"
#include <QApplication>
#include <QStyle>
#include <QPixmap>
#include <QIcon>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QXmlStreamWriter>
#include <QXmlStreamReader>
#include <QStandardPaths>
#include <QDir>
#include <QMimeData>
#include <QDragEnterEvent>
#include <QDropEvent>

// WaypointEditorPanel implementation
WaypointEditorPanel::WaypointEditorPanel(QWidget* parent)
    : QWidget(parent)
    , map_(nullptr)
    , selectedWaypoint_(nullptr)
    , updatingUI_(false)
    , readOnly_(false) {
    
    setupUI();
    connectSignals();
    
    // Initialize refresh timer
    refreshTimer_ = new QTimer(this);
    refreshTimer_->setSingleShot(true);
    refreshTimer_->setInterval(100); // 100ms delay for batch updates
    connect(refreshTimer_, &QTimer::timeout, this, &WaypointEditorPanel::refreshWaypointList);
    
    // Enable drag and drop for waypoint files
    setAcceptDrops(true);
}

WaypointEditorPanel::~WaypointEditorPanel() {
    disconnectMapSignals();
}

void WaypointEditorPanel::setupUI() {
    mainLayout_ = new QVBoxLayout(this);
    mainLayout_->setContentsMargins(4, 4, 4, 4);
    mainLayout_->setSpacing(4);
    
    // Create main splitter
    mainSplitter_ = new QSplitter(Qt::Vertical, this);
    mainLayout_->addWidget(mainSplitter_);
    
    // Create tab widget
    tabWidget_ = new QTabWidget(this);
    mainSplitter_->addWidget(tabWidget_);
    
    setupWaypointList();
    setupPropertyEditor();
    setupAdvancedTools();
    setupContextMenu();
    
    // Set initial splitter proportions
    mainSplitter_->setStretchFactor(0, 1);
    
    setMinimumSize(300, 400);
}

void WaypointEditorPanel::setupWaypointList() {
    // Waypoint list tab
    listTab_ = new QWidget();
    listLayout_ = new QVBoxLayout(listTab_);
    listLayout_->setContentsMargins(4, 4, 4, 4);
    listLayout_->setSpacing(4);
    
    // Search and filter section
    setupSearchAndFilter();
    
    // Waypoint list group
    listGroup_ = new QGroupBox("Waypoints", listTab_);
    QVBoxLayout* listGroupLayout = new QVBoxLayout(listGroup_);
    
    // Waypoint count label
    waypointCountLabel_ = new QLabel("0 waypoints", listGroup_);
    waypointCountLabel_->setStyleSheet("color: gray; font-size: 10px;");
    listGroupLayout->addWidget(waypointCountLabel_);
    
    // Waypoint list widget
    waypointList_ = new QListWidget(listGroup_);
    waypointList_->setSelectionMode(QAbstractItemView::SingleSelection);
    waypointList_->setContextMenuPolicy(Qt::CustomContextMenu);
    waypointList_->setAlternatingRowColors(true);
    waypointList_->setSortingEnabled(true);
    waypointList_->setDragDropMode(QAbstractItemView::NoDragDrop);
    listGroupLayout->addWidget(waypointList_);
    
    listLayout_->addWidget(listGroup_);
    
    // Control buttons
    setupControlButtons();
    
    tabWidget_->addTab(listTab_, "Waypoints");
}

void WaypointEditorPanel::setupControlButtons() {
    buttonLayout_ = new QHBoxLayout();
    buttonLayout_->setSpacing(4);
    
    // Primary action buttons
    addButton_ = new QPushButton(QIcon(":/icons/add.png"), "Add", this);
    addButton_->setToolTip("Add new waypoint");
    buttonLayout_->addWidget(addButton_);
    
    removeButton_ = new QPushButton(QIcon(":/icons/remove.png"), "Remove", this);
    removeButton_->setToolTip("Remove selected waypoint");
    removeButton_->setEnabled(false);
    buttonLayout_->addWidget(removeButton_);
    
    editButton_ = new QPushButton(QIcon(":/icons/edit.png"), "Edit", this);
    editButton_->setToolTip("Edit selected waypoint");
    editButton_->setEnabled(false);
    buttonLayout_->addWidget(editButton_);
    
    buttonLayout_->addSpacing(10);
    
    // Navigation buttons
    centerButton_ = new QPushButton(QIcon(":/icons/center.png"), "Center", this);
    centerButton_->setToolTip("Center map on selected waypoint");
    centerButton_->setEnabled(false);
    buttonLayout_->addWidget(centerButton_);
    
    goToButton_ = new QPushButton(QIcon(":/icons/goto.png"), "Go To", this);
    goToButton_->setToolTip("Navigate to selected waypoint");
    goToButton_->setEnabled(false);
    buttonLayout_->addWidget(goToButton_);
    
    buttonLayout_->addStretch();
    
    // Utility buttons
    duplicateButton_ = new QPushButton(QIcon(":/icons/duplicate.png"), "Duplicate", this);
    duplicateButton_->setToolTip("Duplicate selected waypoint");
    duplicateButton_->setEnabled(false);
    buttonLayout_->addWidget(duplicateButton_);
    
    refreshButton_ = new QPushButton(QIcon(":/icons/refresh.png"), "Refresh", this);
    refreshButton_->setToolTip("Refresh waypoint list");
    buttonLayout_->addWidget(refreshButton_);
    
    listLayout_->addLayout(buttonLayout_);
}

void WaypointEditorPanel::setupSearchAndFilter() {
    filterGroup_ = new QGroupBox("Search & Filter", listTab_);
    filterLayout_ = new QGridLayout(filterGroup_);
    filterLayout_->setContentsMargins(4, 4, 4, 4);
    filterLayout_->setSpacing(4);
    
    // Search field
    QLabel* searchLabel = new QLabel("Search:", filterGroup_);
    filterLayout_->addWidget(searchLabel, 0, 0);
    
    searchEdit_ = new QLineEdit(filterGroup_);
    searchEdit_->setPlaceholderText("Enter waypoint name...");
    searchEdit_->setClearButtonEnabled(true);
    filterLayout_->addWidget(searchEdit_, 0, 1);
    
    // Type filter
    QLabel* typeLabel = new QLabel("Type:", filterGroup_);
    filterLayout_->addWidget(typeLabel, 1, 0);
    
    typeFilterCombo_ = new QComboBox(filterGroup_);
    typeFilterCombo_->addItem("All Types", "");
    typeFilterCombo_->addItem("Temple", "temple");
    typeFilterCombo_->addItem("Town", "town");
    typeFilterCombo_->addItem("Depot", "depot");
    typeFilterCombo_->addItem("Custom", "custom");
    filterLayout_->addWidget(typeFilterCombo_, 1, 1);
    
    // Clear filters button
    clearFiltersButton_ = new QPushButton("Clear", filterGroup_);
    clearFiltersButton_->setMaximumWidth(60);
    filterLayout_->addWidget(clearFiltersButton_, 0, 2, 2, 1);
    
    listLayout_->addWidget(filterGroup_);
}

void WaypointEditorPanel::setupPropertyEditor() {
    // Property editor tab
    propertyTab_ = new QWidget();
    propertyLayout_ = new QVBoxLayout(propertyTab_);
    propertyLayout_->setContentsMargins(4, 4, 4, 4);
    propertyLayout_->setSpacing(4);
    
    // Property group
    propertyGroup_ = new QGroupBox("Waypoint Properties", propertyTab_);
    propertyGridLayout_ = new QGridLayout(propertyGroup_);
    propertyGridLayout_->setContentsMargins(4, 4, 4, 4);
    propertyGridLayout_->setSpacing(4);
    
    int row = 0;
    
    // Name
    propertyGridLayout_->addWidget(new QLabel("Name:"), row, 0);
    nameEdit_ = new QLineEdit(propertyGroup_);
    nameEdit_->setPlaceholderText("Enter waypoint name");
    propertyGridLayout_->addWidget(nameEdit_, row, 1, 1, 2);
    row++;
    
    // Type
    propertyGridLayout_->addWidget(new QLabel("Type:"), row, 0);
    typeCombo_ = new QComboBox(propertyGroup_);
    typeCombo_->setEditable(true);
    typeCombo_->addItems({"temple", "town", "depot", "custom"});
    propertyGridLayout_->addWidget(typeCombo_, row, 1, 1, 2);
    row++;
    
    // Position
    propertyGridLayout_->addWidget(new QLabel("Position:"), row, 0);
    xSpinBox_ = new QSpinBox(propertyGroup_);
    xSpinBox_->setRange(0, 65535);
    xSpinBox_->setPrefix("X: ");
    propertyGridLayout_->addWidget(xSpinBox_, row, 1);
    
    ySpinBox_ = new QSpinBox(propertyGroup_);
    ySpinBox_->setRange(0, 65535);
    ySpinBox_->setPrefix("Y: ");
    propertyGridLayout_->addWidget(ySpinBox_, row, 2);
    row++;
    
    propertyGridLayout_->addWidget(new QLabel("Floor:"), row, 0);
    zSpinBox_ = new QSpinBox(propertyGroup_);
    zSpinBox_->setRange(0, 15);
    zSpinBox_->setPrefix("Z: ");
    propertyGridLayout_->addWidget(zSpinBox_, row, 1);
    
    // Radius
    radiusSpinBox_ = new QSpinBox(propertyGroup_);
    radiusSpinBox_->setRange(1, 100);
    radiusSpinBox_->setPrefix("Radius: ");
    radiusSpinBox_->setSuffix(" tiles");
    radiusSpinBox_->setValue(3);
    propertyGridLayout_->addWidget(radiusSpinBox_, row, 2);
    row++;
    
    // Color
    propertyGridLayout_->addWidget(new QLabel("Color:"), row, 0);
    colorButton_ = new QToolButton(propertyGroup_);
    colorButton_->setMinimumSize(40, 24);
    colorButton_->setStyleSheet("background-color: #FF0000; border: 1px solid gray;");
    colorButton_->setToolTip("Click to change waypoint color");
    propertyGridLayout_->addWidget(colorButton_, row, 1);
    row++;
    
    // Script
    propertyGridLayout_->addWidget(new QLabel("Script:"), row, 0, Qt::AlignTop);
    scriptEdit_ = new QTextEdit(propertyGroup_);
    scriptEdit_->setMaximumHeight(80);
    scriptEdit_->setPlaceholderText("Enter waypoint script (optional)");
    propertyGridLayout_->addWidget(scriptEdit_, row, 1, 1, 2);
    row++;
    
    // Apply button
    applyButton_ = new QPushButton("Apply Changes", propertyGroup_);
    applyButton_->setEnabled(false);
    propertyGridLayout_->addWidget(applyButton_, row, 0, 1, 3);
    
    propertyLayout_->addWidget(propertyGroup_);
    propertyLayout_->addStretch();
    
    tabWidget_->addTab(propertyTab_, "Properties");
}

void WaypointEditorPanel::setupAdvancedTools() {
    // Advanced tools tab
    toolsTab_ = new QWidget();
    toolsLayout_ = new QVBoxLayout(toolsTab_);
    toolsLayout_->setContentsMargins(4, 4, 4, 4);
    toolsLayout_->setSpacing(4);
    
    // Import/Export group
    QGroupBox* importExportGroup = new QGroupBox("Import/Export", toolsTab_);
    QVBoxLayout* importExportLayout = new QVBoxLayout(importExportGroup);
    
    exportButton_ = new QPushButton(QIcon(":/icons/export.png"), "Export Waypoints...", importExportGroup);
    exportButton_->setToolTip("Export waypoints to file");
    importExportLayout->addWidget(exportButton_);
    
    importButton_ = new QPushButton(QIcon(":/icons/import.png"), "Import Waypoints...", importExportGroup);
    importButton_->setToolTip("Import waypoints from file");
    importExportLayout->addWidget(importButton_);
    
    toolsLayout_->addWidget(importExportGroup);
    
    // Batch operations group
    QGroupBox* batchGroup = new QGroupBox("Batch Operations", toolsTab_);
    QVBoxLayout* batchLayout = new QVBoxLayout(batchGroup);
    
    batchEditButton_ = new QPushButton(QIcon(":/icons/batch.png"), "Batch Edit...", batchGroup);
    batchEditButton_->setToolTip("Edit multiple waypoints at once");
    batchLayout->addWidget(batchEditButton_);
    
    sortButton_ = new QPushButton(QIcon(":/icons/sort.png"), "Sort Waypoints", batchGroup);
    sortButton_->setToolTip("Sort waypoints by name");
    batchLayout->addWidget(sortButton_);
    
    validateButton_ = new QPushButton(QIcon(":/icons/validate.png"), "Validate Waypoints", batchGroup);
    validateButton_->setToolTip("Check waypoints for errors");
    batchLayout->addWidget(validateButton_);
    
    toolsLayout_->addWidget(batchGroup);
    
    // Progress bar
    progressBar_ = new QProgressBar(toolsTab_);
    progressBar_->setVisible(false);
    toolsLayout_->addWidget(progressBar_);
    
    toolsLayout_->addStretch();
    
    tabWidget_->addTab(toolsTab_, "Tools");
}

void WaypointEditorPanel::setupContextMenu() {
    contextMenu_ = new QMenu(this);

    editAction_ = contextMenu_->addAction(QIcon(":/icons/edit.png"), "Edit Waypoint");
    deleteAction_ = contextMenu_->addAction(QIcon(":/icons/remove.png"), "Delete Waypoint");
    duplicateAction_ = contextMenu_->addAction(QIcon(":/icons/duplicate.png"), "Duplicate Waypoint");
    contextMenu_->addSeparator();
    centerAction_ = contextMenu_->addAction(QIcon(":/icons/center.png"), "Center on Waypoint");
    goToAction_ = contextMenu_->addAction(QIcon(":/icons/goto.png"), "Go to Waypoint");
    contextMenu_->addSeparator();
    exportAction_ = contextMenu_->addAction(QIcon(":/icons/export.png"), "Export Waypoint");
}

void WaypointEditorPanel::connectSignals() {
    // Waypoint list signals
    connect(waypointList_, &QListWidget::itemSelectionChanged, this, &WaypointEditorPanel::onWaypointListSelectionChanged);
    connect(waypointList_, &QListWidget::itemDoubleClicked, this, &WaypointEditorPanel::onWaypointListItemDoubleClicked);
    connect(waypointList_, &QListWidget::customContextMenuRequested, this, &WaypointEditorPanel::onWaypointListContextMenu);

    // Button signals
    connect(addButton_, &QPushButton::clicked, this, &WaypointEditorPanel::onAddWaypointClicked);
    connect(removeButton_, &QPushButton::clicked, this, &WaypointEditorPanel::onRemoveWaypointClicked);
    connect(editButton_, &QPushButton::clicked, this, &WaypointEditorPanel::onEditWaypointClicked);
    connect(duplicateButton_, &QPushButton::clicked, this, &WaypointEditorPanel::onDuplicateWaypointClicked);
    connect(centerButton_, &QPushButton::clicked, this, &WaypointEditorPanel::onCenterOnWaypointClicked);
    connect(goToButton_, &QPushButton::clicked, this, &WaypointEditorPanel::onGoToWaypointClicked);
    connect(refreshButton_, &QPushButton::clicked, this, &WaypointEditorPanel::onRefreshListClicked);

    // Property editor signals
    connect(nameEdit_, &QLineEdit::textChanged, this, &WaypointEditorPanel::onNameChanged);
    connect(typeCombo_, &QComboBox::currentTextChanged, this, &WaypointEditorPanel::onTypeChanged);
    connect(xSpinBox_, QOverload<int>::of(&QSpinBox::valueChanged), this, &WaypointEditorPanel::onPositionChanged);
    connect(ySpinBox_, QOverload<int>::of(&QSpinBox::valueChanged), this, &WaypointEditorPanel::onPositionChanged);
    connect(zSpinBox_, QOverload<int>::of(&QSpinBox::valueChanged), this, &WaypointEditorPanel::onPositionChanged);
    connect(radiusSpinBox_, QOverload<int>::of(&QSpinBox::valueChanged), this, &WaypointEditorPanel::onRadiusChanged);
    connect(colorButton_, &QToolButton::clicked, this, &WaypointEditorPanel::onColorChanged);
    connect(scriptEdit_, &QTextEdit::textChanged, this, &WaypointEditorPanel::onScriptChanged);
    connect(applyButton_, &QPushButton::clicked, this, &WaypointEditorPanel::onPropertyChanged);

    // Search and filter signals
    connect(searchEdit_, &QLineEdit::textChanged, this, &WaypointEditorPanel::onSearchTextChanged);
    connect(typeFilterCombo_, &QComboBox::currentTextChanged, this, &WaypointEditorPanel::onTypeFilterChanged);
    connect(clearFiltersButton_, &QPushButton::clicked, this, &WaypointEditorPanel::onClearFiltersClicked);

    // Advanced tools signals
    connect(exportButton_, &QPushButton::clicked, this, &WaypointEditorPanel::onExportWaypointsClicked);
    connect(importButton_, &QPushButton::clicked, this, &WaypointEditorPanel::onImportWaypointsClicked);
    connect(batchEditButton_, &QPushButton::clicked, this, &WaypointEditorPanel::onBatchEditClicked);
    connect(sortButton_, &QPushButton::clicked, this, &WaypointEditorPanel::onSortWaypointsClicked);
    connect(validateButton_, &QPushButton::clicked, this, &WaypointEditorPanel::onValidateWaypointsClicked);

    // Context menu signals
    connect(editAction_, &QAction::triggered, this, &WaypointEditorPanel::onEditWaypointClicked);
    connect(deleteAction_, &QAction::triggered, this, &WaypointEditorPanel::onRemoveWaypointClicked);
    connect(duplicateAction_, &QAction::triggered, this, &WaypointEditorPanel::onDuplicateWaypointClicked);
    connect(centerAction_, &QAction::triggered, this, &WaypointEditorPanel::onCenterOnWaypointClicked);
    connect(goToAction_, &QAction::triggered, this, &WaypointEditorPanel::onGoToWaypointClicked);
    connect(exportAction_, &QAction::triggered, this, &WaypointEditorPanel::onExportWaypointsClicked);
}

// Map integration
void WaypointEditorPanel::setMap(Map* map) {
    if (map_ == map) {
        return;
    }

    disconnectMapSignals();
    map_ = map;
    connectMapSignals();

    refreshWaypointList();
    clearSelection();
    updateButtonStates();
}

void WaypointEditorPanel::disconnectMapSignals() {
    if (map_) {
        disconnect(map_, &Map::waypointAdded, this, &WaypointEditorPanel::onWaypointAdded);
        disconnect(map_, &Map::waypointRemoved, this, QOverload<const QString&>::of(&WaypointEditorPanel::onWaypointRemoved));
        disconnect(map_, &Map::waypointRemoved, this, QOverload<Waypoint*>::of(&WaypointEditorPanel::onWaypointRemoved));
        disconnect(map_, &Map::waypointModified, this, &WaypointEditorPanel::onWaypointModified);
        disconnect(map_, &Map::waypointsCleared, this, &WaypointEditorPanel::onWaypointsCleared);
        disconnect(map_, &Map::waypointsChanged, this, &WaypointEditorPanel::onMapChanged);
    }
}

void WaypointEditorPanel::connectMapSignals() {
    if (map_) {
        connect(map_, &Map::waypointAdded, this, &WaypointEditorPanel::onWaypointAdded);
        connect(map_, &Map::waypointRemoved, this, QOverload<const QString&>::of(&WaypointEditorPanel::onWaypointRemoved));
        connect(map_, &Map::waypointRemoved, this, QOverload<Waypoint*>::of(&WaypointEditorPanel::onWaypointRemoved));
        connect(map_, &Map::waypointModified, this, &WaypointEditorPanel::onWaypointModified);
        connect(map_, &Map::waypointsCleared, this, &WaypointEditorPanel::onWaypointsCleared);
        connect(map_, &Map::waypointsChanged, this, &WaypointEditorPanel::onMapChanged);
    }
}

// Waypoint selection and management
Waypoint* WaypointEditorPanel::getSelectedWaypoint() const {
    return selectedWaypoint_;
}

void WaypointEditorPanel::selectWaypoint(const QString& name) {
    if (!map_) {
        return;
    }

    Waypoint* waypoint = map_->findWaypoint(name);
    selectWaypoint(waypoint);
}

void WaypointEditorPanel::selectWaypoint(Waypoint* waypoint) {
    if (selectedWaypoint_ == waypoint) {
        return;
    }

    selectedWaypoint_ = waypoint;

    // Update list selection
    if (waypoint) {
        QListWidgetItem* item = findWaypointItem(waypoint);
        if (item) {
            waypointList_->setCurrentItem(item);
        }
    } else {
        waypointList_->clearSelection();
    }

    updatePropertyEditor(waypoint);
    updateButtonStates();

    emit waypointSelected(waypoint);
}

void WaypointEditorPanel::clearSelection() {
    selectWaypoint(nullptr);
}

// UI state management
void WaypointEditorPanel::refreshWaypointList() {
    if (updatingUI_) {
        return;
    }

    updatingUI_ = true;

    QString selectedName;
    if (selectedWaypoint_) {
        selectedName = selectedWaypoint_->name();
    }

    waypointList_->clear();
    selectedWaypoint_ = nullptr;

    populateWaypointList();
    applyFilters();

    // Restore selection
    if (!selectedName.isEmpty()) {
        selectWaypoint(selectedName);
    }

    updateButtonStates();
    updatingUI_ = false;
}

void WaypointEditorPanel::updateButtonStates() {
    bool hasSelection = selectedWaypoint_ != nullptr;
    bool hasMap = map_ != nullptr;
    bool hasWaypoints = hasMap && map_->getWaypointCount() > 0;

    // Selection-dependent buttons
    removeButton_->setEnabled(hasSelection && !readOnly_);
    editButton_->setEnabled(hasSelection && !readOnly_);
    duplicateButton_->setEnabled(hasSelection && !readOnly_);
    centerButton_->setEnabled(hasSelection);
    goToButton_->setEnabled(hasSelection);

    // Map-dependent buttons
    addButton_->setEnabled(hasMap && !readOnly_);
    refreshButton_->setEnabled(hasMap);

    // Tools buttons
    exportButton_->setEnabled(hasWaypoints);
    importButton_->setEnabled(hasMap && !readOnly_);
    batchEditButton_->setEnabled(hasWaypoints && !readOnly_);
    sortButton_->setEnabled(hasWaypoints && !readOnly_);
    validateButton_->setEnabled(hasWaypoints);

    // Property editor
    bool enableProperties = hasSelection && !readOnly_;
    nameEdit_->setEnabled(enableProperties);
    typeCombo_->setEnabled(enableProperties);
    xSpinBox_->setEnabled(enableProperties);
    ySpinBox_->setEnabled(enableProperties);
    zSpinBox_->setEnabled(enableProperties);
    radiusSpinBox_->setEnabled(enableProperties);
    colorButton_->setEnabled(enableProperties);
    scriptEdit_->setEnabled(enableProperties);
    applyButton_->setEnabled(enableProperties);

    // Context menu actions
    if (editAction_) editAction_->setEnabled(hasSelection && !readOnly_);
    if (deleteAction_) deleteAction_->setEnabled(hasSelection && !readOnly_);
    if (duplicateAction_) duplicateAction_->setEnabled(hasSelection && !readOnly_);
    if (centerAction_) centerAction_->setEnabled(hasSelection);
    if (goToAction_) goToAction_->setEnabled(hasSelection);
    if (exportAction_) exportAction_->setEnabled(hasWaypoints);
}

void WaypointEditorPanel::setReadOnly(bool readOnly) {
    readOnly_ = readOnly;
    updateButtonStates();
}

// Helper methods
void WaypointEditorPanel::populateWaypointList() {
    if (!map_) {
        waypointCountLabel_->setText("0 waypoints");
        return;
    }

    QList<Waypoint*> waypoints = map_->getWaypoints();

    for (Waypoint* waypoint : waypoints) {
        if (waypoint) {
            QListWidgetItem* item = new QListWidgetItem();
            updateWaypointListItem(item, waypoint);
            waypointList_->addItem(item);
        }
    }

    waypointCountLabel_->setText(QString("%1 waypoint%2").arg(waypoints.size()).arg(waypoints.size() == 1 ? "" : "s"));
}

void WaypointEditorPanel::updateWaypointListItem(QListWidgetItem* item, Waypoint* waypoint) {
    if (!item || !waypoint) {
        return;
    }

    // Set display text
    QString displayText = waypoint->name();
    if (!waypoint->type().isEmpty()) {
        displayText += QString(" (%1)").arg(waypoint->type());
    }

    MapPos pos = waypoint->position();
    displayText += QString(" [%1, %2, %3]").arg(pos.x).arg(pos.y).arg(pos.z);

    item->setText(displayText);

    // Set tooltip
    QString tooltip = QString("Name: %1\nType: %2\nPosition: %3, %4, %5\nRadius: %6")
                     .arg(waypoint->name())
                     .arg(waypoint->type().isEmpty() ? "None" : waypoint->type())
                     .arg(pos.x).arg(pos.y).arg(pos.z)
                     .arg(waypoint->radius());

    if (!waypoint->script().isEmpty()) {
        tooltip += QString("\nScript: %1").arg(waypoint->script().left(50) + (waypoint->script().length() > 50 ? "..." : ""));
    }

    item->setToolTip(tooltip);

    // Store waypoint pointer
    item->setData(Qt::UserRole, QVariant::fromValue(waypoint));

    // Set icon based on type
    QIcon icon;
    QString type = waypoint->type().toLower();
    if (type == "temple") {
        icon = QIcon(":/icons/waypoint_temple.png");
    } else if (type == "town") {
        icon = QIcon(":/icons/waypoint_town.png");
    } else if (type == "depot") {
        icon = QIcon(":/icons/waypoint_depot.png");
    } else {
        icon = QIcon(":/icons/waypoint_custom.png");
    }
    item->setIcon(icon);
}

QListWidgetItem* WaypointEditorPanel::findWaypointItem(const QString& name) const {
    for (int i = 0; i < waypointList_->count(); ++i) {
        QListWidgetItem* item = waypointList_->item(i);
        if (item) {
            Waypoint* waypoint = getWaypointFromItem(item);
            if (waypoint && waypoint->name() == name) {
                return item;
            }
        }
    }
    return nullptr;
}

QListWidgetItem* WaypointEditorPanel::findWaypointItem(Waypoint* waypoint) const {
    if (!waypoint) {
        return nullptr;
    }

    for (int i = 0; i < waypointList_->count(); ++i) {
        QListWidgetItem* item = waypointList_->item(i);
        if (item && getWaypointFromItem(item) == waypoint) {
            return item;
        }
    }
    return nullptr;
}

Waypoint* WaypointEditorPanel::getWaypointFromItem(QListWidgetItem* item) const {
    if (!item) {
        return nullptr;
    }

    return item->data(Qt::UserRole).value<Waypoint*>();
}

void WaypointEditorPanel::updatePropertyEditor(Waypoint* waypoint) {
    if (updatingUI_) {
        return;
    }

    updatingUI_ = true;

    if (waypoint) {
        nameEdit_->setText(waypoint->name());
        typeCombo_->setCurrentText(waypoint->type());

        MapPos pos = waypoint->position();
        xSpinBox_->setValue(pos.x);
        ySpinBox_->setValue(pos.y);
        zSpinBox_->setValue(pos.z);

        radiusSpinBox_->setValue(waypoint->radius());

        // Update color button
        QColor color = waypoint->color();
        if (color.isValid()) {
            colorButton_->setStyleSheet(QString("background-color: %1; border: 1px solid gray;").arg(color.name()));
        } else {
            colorButton_->setStyleSheet("background-color: #FF0000; border: 1px solid gray;");
        }

        scriptEdit_->setPlainText(waypoint->script());
    } else {
        clearPropertyEditor();
    }

    applyButton_->setEnabled(false);
    updatingUI_ = false;
}

void WaypointEditorPanel::clearPropertyEditor() {
    nameEdit_->clear();
    typeCombo_->setCurrentIndex(0);
    xSpinBox_->setValue(0);
    ySpinBox_->setValue(0);
    zSpinBox_->setValue(0);
    radiusSpinBox_->setValue(3);
    colorButton_->setStyleSheet("background-color: #FF0000; border: 1px solid gray;");
    scriptEdit_->clear();
}
