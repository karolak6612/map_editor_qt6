#include "WaypointPalettePanel.h"
#include "../Map.h"
#include "../Waypoint.h"
#include "../Waypoints.h"
#include <QApplication>
#include <QMessageBox>
#include <QMenu>
#include <QContextMenuEvent>
#include <QHeaderView>
#include <QSplitter>
#include <QTabWidget>
#include <QDialogButtonBox>
#include <QCheckBox>
#include <QDebug>

// WaypointPalettePanel implementation
WaypointPalettePanel::WaypointPalettePanel(QWidget* parent)
    : QWidget(parent),
      map_(nullptr),
      selectedWaypoint_(nullptr),
      updatingUI_(false),
      editDialog_(nullptr) {
    
    setupUI();
    updateButtonStates();
}

WaypointPalettePanel::~WaypointPalettePanel() {
    if (map_) {
        disconnectMapSignals();
    }
}

void WaypointPalettePanel::setupUI() {
    mainLayout_ = new QVBoxLayout(this);
    mainLayout_->setContentsMargins(4, 4, 4, 4);
    mainLayout_->setSpacing(4);
    
    // Create splitter for list and quick edit
    splitter_ = new QSplitter(Qt::Vertical, this);
    mainLayout_->addWidget(splitter_);
    
    setupWaypointList();
    setupControlButtons();
    setupQuickEditPanel();
    setupContextMenu();
    
    // Set splitter proportions
    splitter_->setStretchFactor(0, 3); // List gets more space
    splitter_->setStretchFactor(1, 1); // Quick edit gets less space
}

void WaypointPalettePanel::setupWaypointList() {
    // Waypoint list group
    listGroup_ = new QGroupBox("Waypoints", this);
    listLayout_ = new QVBoxLayout(listGroup_);
    
    // Waypoint count label
    waypointCountLabel_ = new QLabel("0 waypoints", listGroup_);
    waypointCountLabel_->setStyleSheet("color: gray; font-size: 10px;");
    listLayout_->addWidget(waypointCountLabel_);
    
    // Waypoint list widget
    waypointList_ = new QListWidget(listGroup_);
    waypointList_->setSelectionMode(QAbstractItemView::SingleSelection);
    waypointList_->setContextMenuPolicy(Qt::CustomContextMenu);
    waypointList_->setAlternatingRowColors(true);
    waypointList_->setSortingEnabled(true);
    
    // Connect list signals
    connect(waypointList_, &QListWidget::itemSelectionChanged,
            this, &WaypointPalettePanel::onWaypointListSelectionChanged);
    connect(waypointList_, &QListWidget::itemDoubleClicked,
            this, &WaypointPalettePanel::onWaypointListItemDoubleClicked);
    connect(waypointList_, &QListWidget::customContextMenuRequested,
            this, &WaypointPalettePanel::onWaypointListContextMenu);
    
    listLayout_->addWidget(waypointList_);
    splitter_->addWidget(listGroup_);
}

void WaypointPalettePanel::setupControlButtons() {
    // Control buttons layout
    buttonLayout_ = new QHBoxLayout();
    
    addButton_ = new QPushButton("Add", this);
    addButton_->setToolTip("Add new waypoint");
    connect(addButton_, &QPushButton::clicked,
            this, &WaypointPalettePanel::onAddWaypointClicked);
    
    removeButton_ = new QPushButton("Remove", this);
    removeButton_->setToolTip("Remove selected waypoint");
    connect(removeButton_, &QPushButton::clicked,
            this, &WaypointPalettePanel::onRemoveWaypointClicked);
    
    editButton_ = new QPushButton("Edit", this);
    editButton_->setToolTip("Edit selected waypoint");
    connect(editButton_, &QPushButton::clicked,
            this, &WaypointPalettePanel::onEditWaypointClicked);
    
    centerButton_ = new QPushButton("Center", this);
    centerButton_->setToolTip("Center map on selected waypoint");
    connect(centerButton_, &QPushButton::clicked,
            this, &WaypointPalettePanel::onCenterOnWaypointClicked);
    
    refreshButton_ = new QPushButton("Refresh", this);
    refreshButton_->setToolTip("Refresh waypoint list");
    connect(refreshButton_, &QPushButton::clicked,
            this, &WaypointPalettePanel::onRefreshListClicked);
    
    buttonLayout_->addWidget(addButton_);
    buttonLayout_->addWidget(removeButton_);
    buttonLayout_->addWidget(editButton_);
    buttonLayout_->addWidget(centerButton_);
    buttonLayout_->addStretch();
    buttonLayout_->addWidget(refreshButton_);
    
    listLayout_->addLayout(buttonLayout_);
}

void WaypointPalettePanel::setupQuickEditPanel() {
    // Quick edit group
    quickEditGroup_ = new QGroupBox("Quick Edit", this);
    quickEditLayout_ = new QVBoxLayout(quickEditGroup_);
    
    // Name
    QHBoxLayout* nameLayout = new QHBoxLayout();
    nameLayout->addWidget(new QLabel("Name:"));
    quickNameEdit_ = new QLineEdit();
    quickNameEdit_->setPlaceholderText("Waypoint name");
    connect(quickNameEdit_, &QLineEdit::textChanged,
            this, &WaypointPalettePanel::onQuickNameChanged);
    nameLayout->addWidget(quickNameEdit_);
    quickEditLayout_->addLayout(nameLayout);
    
    // Type and Radius
    QHBoxLayout* typeRadiusLayout = new QHBoxLayout();
    typeRadiusLayout->addWidget(new QLabel("Type:"));
    quickTypeCombo_ = new QComboBox();
    quickTypeCombo_->addItems({"generic", "npc_path", "quest_marker", "spawn_point", "treasure", "portal"});
    quickTypeCombo_->setEditable(true);
    connect(quickTypeCombo_, QOverload<const QString&>::of(&QComboBox::currentTextChanged),
            this, &WaypointPalettePanel::onQuickTypeChanged);
    typeRadiusLayout->addWidget(quickTypeCombo_);
    
    typeRadiusLayout->addWidget(new QLabel("Radius:"));
    quickRadiusSpin_ = new QSpinBox();
    quickRadiusSpin_->setRange(0, 100);
    quickRadiusSpin_->setValue(1);
    connect(quickRadiusSpin_, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &WaypointPalettePanel::onQuickRadiusChanged);
    typeRadiusLayout->addWidget(quickRadiusSpin_);
    
    quickColorButton_ = new QToolButton();
    quickColorButton_->setText("Color");
    quickColorButton_->setStyleSheet("background-color: blue;");
    connect(quickColorButton_, &QToolButton::clicked,
            this, &WaypointPalettePanel::onQuickColorClicked);
    typeRadiusLayout->addWidget(quickColorButton_);
    
    quickEditLayout_->addLayout(typeRadiusLayout);
    
    // Script/Description
    QLabel* scriptLabel = new QLabel("Script/Description:");
    quickEditLayout_->addWidget(scriptLabel);
    quickScriptEdit_ = new QTextEdit();
    quickScriptEdit_->setMaximumHeight(60);
    quickScriptEdit_->setPlaceholderText("Script or description text");
    connect(quickScriptEdit_, &QTextEdit::textChanged,
            this, &WaypointPalettePanel::onQuickScriptChanged);
    quickEditLayout_->addWidget(quickScriptEdit_);
    
    // Apply button
    applyQuickEditButton_ = new QPushButton("Apply Changes");
    applyQuickEditButton_->setEnabled(false);
    quickEditLayout_->addWidget(applyQuickEditButton_);
    
    splitter_->addWidget(quickEditGroup_);
    clearQuickEditPanel();
}

void WaypointPalettePanel::setupContextMenu() {
    contextMenu_ = new QMenu(this);
    
    editAction_ = contextMenu_->addAction("Edit Waypoint");
    connect(editAction_, &QAction::triggered,
            this, &WaypointPalettePanel::onEditWaypointClicked);
    
    deleteAction_ = contextMenu_->addAction("Delete Waypoint");
    connect(deleteAction_, &QAction::triggered,
            this, &WaypointPalettePanel::onRemoveWaypointClicked);
    
    contextMenu_->addSeparator();
    
    centerAction_ = contextMenu_->addAction("Center on Map");
    connect(centerAction_, &QAction::triggered,
            this, &WaypointPalettePanel::onCenterOnWaypointClicked);
    
    duplicateAction_ = contextMenu_->addAction("Duplicate Waypoint");
    // TODO: Implement duplicate functionality
    
    contextMenu_->addSeparator();
    
    exportAction_ = contextMenu_->addAction("Export Waypoint");
    // TODO: Implement export functionality
}

void WaypointPalettePanel::setMap(Map* map) {
    if (map_ == map) {
        return;
    }
    
    // Disconnect from old map
    if (map_) {
        disconnectMapSignals();
    }
    
    map_ = map;
    selectedWaypoint_ = nullptr;
    
    // Connect to new map
    if (map_) {
        connectMapSignals();
    }
    
    refreshWaypointList();
    updateButtonStates();
}

void WaypointPalettePanel::connectMapSignals() {
    if (!map_ || !map_->getWaypoints()) {
        return;
    }
    
    Waypoints* waypoints = map_->getWaypoints();
    connect(waypoints, &Waypoints::waypointAdded,
            this, &WaypointPalettePanel::onWaypointAdded);
    connect(waypoints, &Waypoints::waypointRemoved,
            this, &WaypointPalettePanel::onWaypointRemoved);
    connect(waypoints, &Waypoints::waypointsCleared,
            this, &WaypointPalettePanel::onWaypointsCleared);
    
    connect(map_, &Map::mapChanged,
            this, &WaypointPalettePanel::onMapChanged);
}

void WaypointPalettePanel::disconnectMapSignals() {
    if (!map_) {
        return;
    }
    
    map_->disconnect(this);
    
    if (map_->getWaypoints()) {
        map_->getWaypoints()->disconnect(this);
    }
}

void WaypointPalettePanel::refreshWaypointList() {
    updatingUI_ = true;
    
    waypointList_->clear();
    
    if (!map_ || !map_->getWaypoints()) {
        waypointCountLabel_->setText("0 waypoints");
        updatingUI_ = false;
        return;
    }
    
    Waypoints* waypoints = map_->getWaypoints();
    QList<Waypoint*> waypointList = waypoints->getAllWaypoints();
    
    for (Waypoint* waypoint : waypointList) {
        if (!waypoint) continue;
        
        QListWidgetItem* item = new QListWidgetItem(waypoint->name());
        item->setData(Qt::UserRole, QVariant::fromValue(waypoint));
        
        // Set item color based on waypoint color
        QColor waypointColor = waypoint->color();
        if (waypointColor.isValid()) {
            item->setForeground(QBrush(waypointColor));
        }
        
        // Set tooltip with waypoint info
        QString tooltip = QString("Name: %1\nPosition: %2, %3, %4\nType: %5\nRadius: %6")
                         .arg(waypoint->name())
                         .arg(waypoint->position().x)
                         .arg(waypoint->position().y)
                         .arg(waypoint->position().z)
                         .arg(waypoint->type())
                         .arg(waypoint->radius());
        item->setToolTip(tooltip);
        
        waypointList_->addItem(item);
    }
    
    waypointCountLabel_->setText(QString("%1 waypoints").arg(waypointList.size()));
    updatingUI_ = false;
}

Waypoint* WaypointPalettePanel::getSelectedWaypoint() const {
    QListWidgetItem* item = waypointList_->currentItem();
    return getWaypointFromItem(item);
}

void WaypointPalettePanel::selectWaypoint(const QString& name) {
    QListWidgetItem* item = findWaypointItem(name);
    if (item) {
        waypointList_->setCurrentItem(item);
    }
}

void WaypointPalettePanel::selectWaypoint(Waypoint* waypoint) {
    if (!waypoint) {
        waypointList_->clearSelection();
        return;
    }
    
    selectWaypoint(waypoint->name());
}

void WaypointPalettePanel::updateButtonStates() {
    bool hasMap = (map_ != nullptr);
    bool hasSelection = (getSelectedWaypoint() != nullptr);
    
    addButton_->setEnabled(hasMap);
    removeButton_->setEnabled(hasMap && hasSelection);
    editButton_->setEnabled(hasMap && hasSelection);
    centerButton_->setEnabled(hasMap && hasSelection);
    refreshButton_->setEnabled(hasMap);
    
    waypointList_->setEnabled(hasMap);
    quickEditGroup_->setEnabled(hasMap && hasSelection);
}

QListWidgetItem* WaypointPalettePanel::findWaypointItem(const QString& name) const {
    for (int i = 0; i < waypointList_->count(); ++i) {
        QListWidgetItem* item = waypointList_->item(i);
        if (item && item->text() == name) {
            return item;
        }
    }
    return nullptr;
}

QListWidgetItem* WaypointPalettePanel::findWaypointItem(Waypoint* waypoint) const {
    if (!waypoint) return nullptr;
    
    for (int i = 0; i < waypointList_->count(); ++i) {
        QListWidgetItem* item = waypointList_->item(i);
        if (getWaypointFromItem(item) == waypoint) {
            return item;
        }
    }
    return nullptr;
}

Waypoint* WaypointPalettePanel::getWaypointFromItem(QListWidgetItem* item) const {
    if (!item) return nullptr;
    
    QVariant data = item->data(Qt::UserRole);
    return data.value<Waypoint*>();
}

// Slot implementations
void WaypointPalettePanel::onWaypointListSelectionChanged() {
    if (updatingUI_) return;
    
    selectedWaypoint_ = getSelectedWaypoint();
    updateButtonStates();
    updateQuickEditPanel(selectedWaypoint_);
    
    emit waypointSelected(selectedWaypoint_);
}

void WaypointPalettePanel::onWaypointListItemDoubleClicked(QListWidgetItem* item) {
    Waypoint* waypoint = getWaypointFromItem(item);
    if (waypoint) {
        emit waypointDoubleClicked(waypoint);
        emit centerOnWaypoint(waypoint);
    }
}

void WaypointPalettePanel::onWaypointListContextMenu(const QPoint& pos) {
    QListWidgetItem* item = waypointList_->itemAt(pos);
    if (!item) return;
    
    selectedWaypoint_ = getWaypointFromItem(item);
    waypointList_->setCurrentItem(item);
    
    contextMenu_->exec(waypointList_->mapToGlobal(pos));
}

void WaypointPalettePanel::onAddWaypointClicked() {
    emit newWaypointRequested();
}

void WaypointPalettePanel::onRemoveWaypointClicked() {
    Waypoint* waypoint = getSelectedWaypoint();
    if (!waypoint) return;
    
    int result = QMessageBox::question(
        this,
        "Remove Waypoint",
        QString("Are you sure you want to remove waypoint '%1'?").arg(waypoint->name()),
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No
    );
    
    if (result == QMessageBox::Yes) {
        emit waypointDeleteRequested(waypoint);
    }
}

void WaypointPalettePanel::onEditWaypointClicked() {
    Waypoint* waypoint = getSelectedWaypoint();
    if (waypoint) {
        showEditDialog(waypoint);
    }
}

void WaypointPalettePanel::onCenterOnWaypointClicked() {
    Waypoint* waypoint = getSelectedWaypoint();
    if (waypoint) {
        emit centerOnWaypoint(waypoint);
    }
}

void WaypointPalettePanel::onRefreshListClicked() {
    refreshWaypointList();
}

// Map change handlers
void WaypointPalettePanel::onMapChanged() {
    refreshWaypointList();
}

void WaypointPalettePanel::onWaypointAdded(Waypoint* waypoint) {
    Q_UNUSED(waypoint);
    refreshWaypointList();
}

void WaypointPalettePanel::onWaypointRemoved(const QString& name) {
    Q_UNUSED(name);
    refreshWaypointList();
}

void WaypointPalettePanel::onWaypointModified(Waypoint* waypoint) {
    Q_UNUSED(waypoint);
    refreshWaypointList();
}

void WaypointPalettePanel::onWaypointsCleared() {
    refreshWaypointList();
}

// Quick edit handlers
void WaypointPalettePanel::onQuickNameChanged() {
    if (updatingUI_ || !selectedWaypoint_) return;
    applyQuickEditButton_->setEnabled(true);
}

void WaypointPalettePanel::onQuickTypeChanged() {
    if (updatingUI_ || !selectedWaypoint_) return;
    applyQuickEditButton_->setEnabled(true);
}

void WaypointPalettePanel::onQuickRadiusChanged(int radius) {
    Q_UNUSED(radius);
    if (updatingUI_ || !selectedWaypoint_) return;
    applyQuickEditButton_->setEnabled(true);
}

void WaypointPalettePanel::onQuickColorClicked() {
    if (!selectedWaypoint_) return;

    QColor currentColor = selectedWaypoint_->color();
    QColor newColor = QColorDialog::getColor(currentColor, this, "Select Waypoint Color");

    if (newColor.isValid() && newColor != currentColor) {
        quickColorButton_->setStyleSheet(QString("background-color: %1;").arg(newColor.name()));
        applyQuickEditButton_->setEnabled(true);
    }
}

void WaypointPalettePanel::onQuickScriptChanged() {
    if (updatingUI_ || !selectedWaypoint_) return;
    applyQuickEditButton_->setEnabled(true);
}

void WaypointPalettePanel::updateQuickEditPanel(Waypoint* waypoint) {
    updatingUI_ = true;

    if (!waypoint) {
        clearQuickEditPanel();
        updatingUI_ = false;
        return;
    }

    quickNameEdit_->setText(waypoint->name());
    quickTypeCombo_->setCurrentText(waypoint->type());
    quickRadiusSpin_->setValue(waypoint->radius());
    quickColorButton_->setStyleSheet(QString("background-color: %1;").arg(waypoint->color().name()));
    quickScriptEdit_->setPlainText(waypoint->scriptOrText());

    applyQuickEditButton_->setEnabled(false);
    updatingUI_ = false;
}

void WaypointPalettePanel::clearQuickEditPanel() {
    updatingUI_ = true;

    quickNameEdit_->clear();
    quickTypeCombo_->setCurrentText("generic");
    quickRadiusSpin_->setValue(1);
    quickColorButton_->setStyleSheet("background-color: blue;");
    quickScriptEdit_->clear();

    applyQuickEditButton_->setEnabled(false);
    updatingUI_ = false;
}

void WaypointPalettePanel::showEditDialog(Waypoint* waypoint) {
    if (!waypoint) return;

    if (!editDialog_) {
        editDialog_ = new EditWaypointDialog(this);
    }

    editDialog_->setWaypoint(waypoint);
    editDialog_->setEditMode(EditWaypointDialog::ModifyMode);

    if (editDialog_->exec() == QDialog::Accepted) {
        refreshWaypointList();
        updateQuickEditPanel(waypoint);
        emit waypointEditRequested(waypoint);
    }
}

// Additional utility methods
void WaypointPalettePanel::updateWaypointDetails(Waypoint* waypoint) {
    updateQuickEditPanel(waypoint);
}

void WaypointPalettePanel::clearSelection() {
    waypointList_->clearSelection();
    selectedWaypoint_ = nullptr;
    updateButtonStates();
    clearQuickEditPanel();
}

bool WaypointPalettePanel::isEnabled() const {
    return QWidget::isEnabled();
}

void WaypointPalettePanel::setEnabled(bool enabled) {
    QWidget::setEnabled(enabled);
    updateButtonStates();
}

// EditWaypointDialog implementation
EditWaypointDialog::EditWaypointDialog(QWidget* parent)
    : QDialog(parent),
      waypoint_(nullptr),
      originalWaypoint_(nullptr),
      editMode_(ModifyMode),
      dataChanged_(false) {

    setWindowTitle("Edit Waypoint");
    setModal(true);
    resize(400, 300);

    setupUI();
}

EditWaypointDialog::~EditWaypointDialog() {
    delete originalWaypoint_;
}

void EditWaypointDialog::setupUI() {
    mainLayout_ = new QVBoxLayout(this);

    // Tab widget for organized editing
    tabWidget_ = new QTabWidget(this);
    mainLayout_->addWidget(tabWidget_);

    setupBasicProperties();
    setupAdvancedProperties();
    setupButtons();
}

void EditWaypointDialog::setupBasicProperties() {
    basicTab_ = new QWidget();
    QFormLayout* formLayout = new QFormLayout(basicTab_);

    // Name
    nameEdit_ = new QLineEdit();
    connect(nameEdit_, &QLineEdit::textChanged, this, &EditWaypointDialog::onNameChanged);
    formLayout->addRow("Name:", nameEdit_);

    // Position
    QHBoxLayout* posLayout = new QHBoxLayout();
    posXSpin_ = new QSpinBox();
    posXSpin_->setRange(-32768, 32767);
    posYSpin_ = new QSpinBox();
    posYSpin_->setRange(-32768, 32767);
    posZSpin_ = new QSpinBox();
    posZSpin_->setRange(0, 15);

    connect(posXSpin_, QOverload<int>::of(&QSpinBox::valueChanged), this, &EditWaypointDialog::onPositionChanged);
    connect(posYSpin_, QOverload<int>::of(&QSpinBox::valueChanged), this, &EditWaypointDialog::onPositionChanged);
    connect(posZSpin_, QOverload<int>::of(&QSpinBox::valueChanged), this, &EditWaypointDialog::onPositionChanged);

    posLayout->addWidget(new QLabel("X:"));
    posLayout->addWidget(posXSpin_);
    posLayout->addWidget(new QLabel("Y:"));
    posLayout->addWidget(posYSpin_);
    posLayout->addWidget(new QLabel("Z:"));
    posLayout->addWidget(posZSpin_);
    posLayout->addStretch();

    formLayout->addRow("Position:", posLayout);

    // Type
    typeCombo_ = new QComboBox();
    typeCombo_->addItems({"generic", "npc_path", "quest_marker", "spawn_point", "treasure", "portal", "checkpoint"});
    typeCombo_->setEditable(true);
    connect(typeCombo_, QOverload<const QString&>::of(&QComboBox::currentTextChanged), this, &EditWaypointDialog::onTypeChanged);
    formLayout->addRow("Type:", typeCombo_);

    // Radius
    radiusSpin_ = new QSpinBox();
    radiusSpin_->setRange(0, 100);
    radiusSpin_->setSuffix(" tiles");
    connect(radiusSpin_, QOverload<int>::of(&QSpinBox::valueChanged), this, &EditWaypointDialog::onRadiusChanged);
    formLayout->addRow("Radius:", radiusSpin_);

    // Color
    colorButton_ = new QToolButton();
    colorButton_->setText("Select Color");
    colorButton_->setMinimumHeight(30);
    connect(colorButton_, &QToolButton::clicked, this, &EditWaypointDialog::onColorClicked);
    formLayout->addRow("Color:", colorButton_);

    // Icon
    iconCombo_ = new QComboBox();
    iconCombo_->addItems({"generic", "marker", "flag", "star", "diamond", "circle", "square", "triangle"});
    iconCombo_->setEditable(true);
    connect(iconCombo_, QOverload<const QString&>::of(&QComboBox::currentTextChanged), this, &EditWaypointDialog::onIconChanged);
    formLayout->addRow("Icon:", iconCombo_);

    tabWidget_->addTab(basicTab_, "Basic Properties");
}

void EditWaypointDialog::setupAdvancedProperties() {
    advancedTab_ = new QWidget();
    QVBoxLayout* advLayout = new QVBoxLayout(advancedTab_);

    // Script/Description
    QLabel* scriptLabel = new QLabel("Script/Description:");
    advLayout->addWidget(scriptLabel);

    scriptEdit_ = new QTextEdit();
    scriptEdit_->setPlaceholderText("Enter script code or description text");
    connect(scriptEdit_, &QTextEdit::textChanged, this, &EditWaypointDialog::onScriptChanged);
    advLayout->addWidget(scriptEdit_);

    // Additional properties
    QFormLayout* advFormLayout = new QFormLayout();

    descriptionEdit_ = new QLineEdit();
    descriptionEdit_->setPlaceholderText("Short description");
    advFormLayout->addRow("Description:", descriptionEdit_);

    visibleCheck_ = new QCheckBox("Visible on map");
    visibleCheck_->setChecked(true);
    advFormLayout->addRow("", visibleCheck_);

    selectableCheck_ = new QCheckBox("Selectable");
    selectableCheck_->setChecked(true);
    advFormLayout->addRow("", selectableCheck_);

    advLayout->addLayout(advFormLayout);

    tabWidget_->addTab(advancedTab_, "Advanced");
}

void EditWaypointDialog::setupButtons() {
    buttonBox_ = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);

    resetButton_ = new QPushButton("Reset");
    resetButton_->setToolTip("Reset to original values");
    connect(resetButton_, &QPushButton::clicked, this, &EditWaypointDialog::onResetClicked);
    buttonBox_->addButton(resetButton_, QDialogButtonBox::ResetRole);

    previewButton_ = new QPushButton("Preview");
    previewButton_->setToolTip("Preview waypoint on map");
    connect(previewButton_, &QPushButton::clicked, this, &EditWaypointDialog::onPreviewClicked);
    buttonBox_->addButton(previewButton_, QDialogButtonBox::ActionRole);

    connect(buttonBox_, &QDialogButtonBox::accepted, this, &EditWaypointDialog::accept);
    connect(buttonBox_, &QDialogButtonBox::rejected, this, &EditWaypointDialog::reject);

    mainLayout_->addWidget(buttonBox_);
}

void EditWaypointDialog::setWaypoint(Waypoint* waypoint) {
    waypoint_ = waypoint;

    // Create backup for reset functionality
    delete originalWaypoint_;
    originalWaypoint_ = waypoint ? waypoint->deepCopy() : nullptr;

    loadWaypointData();
    dataChanged_ = false;
    updateButtonStates();
}

void EditWaypointDialog::setEditMode(EditMode mode) {
    editMode_ = mode;

    switch (mode) {
        case CreateMode:
            setWindowTitle("Create Waypoint");
            break;
        case ModifyMode:
            setWindowTitle("Edit Waypoint");
            break;
        case ViewMode:
            setWindowTitle("View Waypoint");
            break;
    }

    bool editable = (mode != ViewMode);
    nameEdit_->setReadOnly(!editable);
    posXSpin_->setReadOnly(!editable);
    posYSpin_->setReadOnly(!editable);
    posZSpin_->setReadOnly(!editable);
    typeCombo_->setEnabled(editable);
    radiusSpin_->setReadOnly(!editable);
    colorButton_->setEnabled(editable);
    iconCombo_->setEnabled(editable);
    scriptEdit_->setReadOnly(!editable);
    descriptionEdit_->setReadOnly(!editable);
    visibleCheck_->setEnabled(editable);
    selectableCheck_->setEnabled(editable);

    buttonBox_->button(QDialogButtonBox::Ok)->setVisible(editable);
    resetButton_->setVisible(editable);
}

void EditWaypointDialog::loadWaypointData() {
    if (!waypoint_) {
        // Clear all fields for create mode
        nameEdit_->clear();
        posXSpin_->setValue(0);
        posYSpin_->setValue(0);
        posZSpin_->setValue(0);
        typeCombo_->setCurrentText("generic");
        radiusSpin_->setValue(1);
        colorButton_->setStyleSheet("background-color: blue;");
        iconCombo_->setCurrentText("generic");
        scriptEdit_->clear();
        descriptionEdit_->clear();
        visibleCheck_->setChecked(true);
        selectableCheck_->setChecked(true);
        return;
    }

    // Load waypoint data
    nameEdit_->setText(waypoint_->name());
    posXSpin_->setValue(waypoint_->position().x);
    posYSpin_->setValue(waypoint_->position().y);
    posZSpin_->setValue(waypoint_->position().z);
    typeCombo_->setCurrentText(waypoint_->type());
    radiusSpin_->setValue(waypoint_->radius());
    colorButton_->setStyleSheet(QString("background-color: %1;").arg(waypoint_->color().name()));
    iconCombo_->setCurrentText(waypoint_->iconType());
    scriptEdit_->setPlainText(waypoint_->scriptOrText());

    // Advanced properties (using defaults for now)
    descriptionEdit_->setText(waypoint_->scriptOrText().left(100)); // First 100 chars as description
    visibleCheck_->setChecked(true);
    selectableCheck_->setChecked(true);
}

void EditWaypointDialog::saveWaypointData() {
    if (!waypoint_) return;

    waypoint_->setName(nameEdit_->text());
    waypoint_->setPosition(MapPos(posXSpin_->value(), posYSpin_->value(), posZSpin_->value()));
    waypoint_->setType(typeCombo_->currentText());
    waypoint_->setRadius(radiusSpin_->value());
    waypoint_->setIconType(iconCombo_->currentText());
    waypoint_->setScriptOrText(scriptEdit_->toPlainText());

    // Extract color from button style
    QString styleSheet = colorButton_->styleSheet();
    QRegExp colorRegex("background-color:\\s*([^;]+);");
    if (colorRegex.indexIn(styleSheet) != -1) {
        QString colorName = colorRegex.cap(1).trimmed();
        QColor color(colorName);
        if (color.isValid()) {
            waypoint_->setColor(color);
        }
    }
}

bool EditWaypointDialog::validateInput() {
    validationError_.clear();

    // Check name
    if (nameEdit_->text().trimmed().isEmpty()) {
        validationError_ = "Waypoint name cannot be empty";
        return false;
    }

    // Check position bounds
    if (posXSpin_->value() < -32768 || posXSpin_->value() > 32767 ||
        posYSpin_->value() < -32768 || posYSpin_->value() > 32767 ||
        posZSpin_->value() < 0 || posZSpin_->value() > 15) {
        validationError_ = "Position values are out of valid range";
        return false;
    }

    // Check radius
    if (radiusSpin_->value() < 0 || radiusSpin_->value() > 100) {
        validationError_ = "Radius must be between 0 and 100";
        return false;
    }

    return true;
}

QString EditWaypointDialog::getValidationError() const {
    return validationError_;
}

void EditWaypointDialog::accept() {
    if (!validateInput()) {
        QMessageBox::warning(this, "Validation Error", validationError_);
        return;
    }

    saveWaypointData();
    QDialog::accept();
}

void EditWaypointDialog::reject() {
    if (dataChanged_) {
        int result = QMessageBox::question(
            this,
            "Discard Changes",
            "You have unsaved changes. Are you sure you want to discard them?",
            QMessageBox::Yes | QMessageBox::No,
            QMessageBox::No
        );

        if (result != QMessageBox::Yes) {
            return;
        }
    }

    QDialog::reject();
}

// Event handlers for EditWaypointDialog
void EditWaypointDialog::onNameChanged() {
    dataChanged_ = true;
    updateButtonStates();
}

void EditWaypointDialog::onPositionChanged() {
    dataChanged_ = true;
    updateButtonStates();
}

void EditWaypointDialog::onTypeChanged() {
    dataChanged_ = true;
    updateButtonStates();
}

void EditWaypointDialog::onRadiusChanged(int radius) {
    Q_UNUSED(radius);
    dataChanged_ = true;
    updateButtonStates();
}

void EditWaypointDialog::onColorClicked() {
    QColor currentColor = Qt::blue;

    // Extract current color from button style
    QString styleSheet = colorButton_->styleSheet();
    QRegExp colorRegex("background-color:\\s*([^;]+);");
    if (colorRegex.indexIn(styleSheet) != -1) {
        QString colorName = colorRegex.cap(1).trimmed();
        QColor color(colorName);
        if (color.isValid()) {
            currentColor = color;
        }
    }

    QColor newColor = QColorDialog::getColor(currentColor, this, "Select Waypoint Color");
    if (newColor.isValid() && newColor != currentColor) {
        colorButton_->setStyleSheet(QString("background-color: %1;").arg(newColor.name()));
        dataChanged_ = true;
        updateButtonStates();
    }
}

void EditWaypointDialog::onIconChanged() {
    dataChanged_ = true;
    updateButtonStates();
}

void EditWaypointDialog::onScriptChanged() {
    dataChanged_ = true;
    updateButtonStates();
}

void EditWaypointDialog::onResetClicked() {
    if (originalWaypoint_) {
        // Restore from backup
        waypoint_->setName(originalWaypoint_->name());
        waypoint_->setPosition(originalWaypoint_->position());
        waypoint_->setType(originalWaypoint_->type());
        waypoint_->setRadius(originalWaypoint_->radius());
        waypoint_->setColor(originalWaypoint_->color());
        waypoint_->setIconType(originalWaypoint_->iconType());
        waypoint_->setScriptOrText(originalWaypoint_->scriptOrText());

        loadWaypointData();
        dataChanged_ = false;
        updateButtonStates();
    }
}

void EditWaypointDialog::onPreviewClicked() {
    // TODO: Implement preview functionality
    // This would show the waypoint on the map temporarily
    QMessageBox::information(this, "Preview", "Preview functionality not yet implemented");
}

void EditWaypointDialog::updateButtonStates() {
    bool hasChanges = dataChanged_;
    resetButton_->setEnabled(hasChanges && originalWaypoint_);

    // Update OK button based on validation
    bool isValid = validateInput();
    buttonBox_->button(QDialogButtonBox::Ok)->setEnabled(isValid);
}
