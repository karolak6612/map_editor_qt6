#include "FindItemDialog.h"
#include "ItemManager.h"
#include "TilesetManager.h"
#include "Map.h"
#include "MapView.h"
#include "Tile.h"
#include "Item.h"
#include <QApplication>
#include <QHeaderView>
#include <QMessageBox>
#include <QDebug>
#include <QRegularExpression>
#include <QStringList>
#include <QSplitter>
#include <QTabWidget>

FindItemDialog::FindItemDialog(QWidget* parent, const QString& title, bool onlyPickupables)
    : QDialog(parent)
    , map_(nullptr)
    , mapView_(nullptr)
    , itemManager_(nullptr)
    , tilesetManager_(nullptr)
    , resultId_(0)
    , onlyPickupables_(onlyPickupables)
    , currentResultIndex_(-1)
    , searchInProgress_(false)
    , lastSearchMode_(SearchMode::ServerIDs) {
    
    setWindowTitle(title);
    setModal(false);
    resize(1000, 700);
    setMinimumSize(800, 600);
    
    // Initialize timer
    refreshTimer_ = new QTimer(this);
    refreshTimer_->setSingleShot(true);
    refreshTimer_->setInterval(500); // 500ms delay for auto-refresh
    
    setupUI();
    connectSignals();
    
    // Set initial search mode
    setSearchMode(SearchMode::ServerIDs);
    updateButtonStates();
}

FindItemDialog::~FindItemDialog() {
    // Qt handles cleanup automatically
}

void FindItemDialog::setupUI() {
    mainLayout_ = new QVBoxLayout(this);
    mainSplitter_ = new QSplitter(Qt::Horizontal, this);
    mainLayout_->addWidget(mainSplitter_);
    
    // Left side: Search controls
    QWidget* searchWidget = new QWidget();
    QVBoxLayout* searchLayout = new QVBoxLayout(searchWidget);
    
    setupSearchModeControls();
    setupIdSearchControls();
    setupNameSearchControls();
    setupTypeSearchControls();
    setupPropertySearchControls();
    setupAdvancedOptions();
    
    searchLayout->addWidget(searchModeGroup_);
    searchLayout->addWidget(idSearchGroup_);
    searchLayout->addWidget(rangeGroup_);
    searchLayout->addWidget(ignoredIdsGroup_);
    searchLayout->addWidget(nameSearchGroup_);
    searchLayout->addWidget(typeSearchGroup_);
    searchLayout->addWidget(propertyGroup_);
    searchLayout->addWidget(advancedGroup_);
    searchLayout->addStretch();
    
    mainSplitter_->addWidget(searchWidget);
    
    // Right side: Results and actions
    QWidget* resultsWidget = new QWidget();
    QVBoxLayout* resultsLayout = new QVBoxLayout(resultsWidget);
    
    setupResultsArea();
    setupActionButtons();
    
    resultsLayout->addWidget(resultsGroup_);
    resultsLayout->addLayout(buttonLayout_);
    
    mainSplitter_->addWidget(resultsWidget);
    
    // Set splitter proportions
    mainSplitter_->setStretchFactor(0, 1);
    mainSplitter_->setStretchFactor(1, 2);
}

void FindItemDialog::setupSearchModeControls() {
    searchModeGroup_ = new QGroupBox("Search Mode", this);
    QVBoxLayout* layout = new QVBoxLayout(searchModeGroup_);
    
    searchModeButtonGroup_ = new QButtonGroup(this);
    
    serverIdModeButton_ = new QRadioButton("Find by Server ID");
    clientIdModeButton_ = new QRadioButton("Find by Client ID");
    nameModeButton_ = new QRadioButton("Find by Name");
    typeModeButton_ = new QRadioButton("Find by Types");
    propertyModeButton_ = new QRadioButton("Find by Properties");
    
    searchModeButtonGroup_->addButton(serverIdModeButton_, static_cast<int>(SearchMode::ServerIDs));
    searchModeButtonGroup_->addButton(clientIdModeButton_, static_cast<int>(SearchMode::ClientIDs));
    searchModeButtonGroup_->addButton(nameModeButton_, static_cast<int>(SearchMode::Names));
    searchModeButtonGroup_->addButton(typeModeButton_, static_cast<int>(SearchMode::Types));
    searchModeButtonGroup_->addButton(propertyModeButton_, static_cast<int>(SearchMode::Properties));
    
    layout->addWidget(serverIdModeButton_);
    layout->addWidget(clientIdModeButton_);
    layout->addWidget(nameModeButton_);
    layout->addWidget(typeModeButton_);
    layout->addWidget(propertyModeButton_);
    
    serverIdModeButton_->setChecked(true);
}

void FindItemDialog::setupIdSearchControls() {
    idSearchGroup_ = new QGroupBox("ID Search", this);
    QVBoxLayout* layout = new QVBoxLayout(idSearchGroup_);
    
    // Server ID controls
    QHBoxLayout* serverIdLayout = new QHBoxLayout();
    serverIdLayout->addWidget(new QLabel("Server ID:"));
    serverIdSpinBox_ = new QSpinBox();
    serverIdSpinBox_->setRange(100, 65535);
    serverIdSpinBox_->setValue(100);
    serverIdLayout->addWidget(serverIdSpinBox_);
    serverIdLayout->addStretch();
    layout->addLayout(serverIdLayout);
    
    invalidItemCheckBox_ = new QCheckBox("Invalid Item");
    layout->addWidget(invalidItemCheckBox_);
    
    // Client ID controls
    QHBoxLayout* clientIdLayout = new QHBoxLayout();
    clientIdLayout->addWidget(new QLabel("Client ID:"));
    clientIdSpinBox_ = new QSpinBox();
    clientIdSpinBox_->setRange(100, 65535);
    clientIdSpinBox_->setValue(100);
    clientIdSpinBox_->setEnabled(false);
    clientIdLayout->addWidget(clientIdSpinBox_);
    clientIdLayout->addStretch();
    layout->addLayout(clientIdLayout);
    
    // Range controls
    rangeGroup_ = new QGroupBox("ID Range", this);
    QVBoxLayout* rangeLayout = new QVBoxLayout(rangeGroup_);
    
    useRangeCheckBox_ = new QCheckBox("Search by Range");
    rangeLayout->addWidget(useRangeCheckBox_);
    
    QHBoxLayout* rangeInputLayout = new QHBoxLayout();
    rangeInputLayout->addWidget(new QLabel("Ranges:"));
    rangeLineEdit_ = new QLineEdit();
    rangeLineEdit_->setPlaceholderText("e.g., 2222,2244-2266,5219");
    rangeLineEdit_->setToolTip("Enter IDs or ranges separated by commas");
    rangeInputLayout->addWidget(rangeLineEdit_);
    rangeLayout->addLayout(rangeInputLayout);
    
    // Ignored IDs controls
    ignoredIdsGroup_ = new QGroupBox("Ignored IDs", this);
    QVBoxLayout* ignoredLayout = new QVBoxLayout(ignoredIdsGroup_);
    
    ignoreIdsCheckBox_ = new QCheckBox("Enable Ignored IDs");
    ignoredLayout->addWidget(ignoreIdsCheckBox_);
    
    QHBoxLayout* ignoredInputLayout = new QHBoxLayout();
    ignoredInputLayout->addWidget(new QLabel("Ignore:"));
    ignoredIdsLineEdit_ = new QLineEdit();
    ignoredIdsLineEdit_->setPlaceholderText("e.g., 1212,1241,1256-1261");
    ignoredIdsLineEdit_->setToolTip("Enter IDs to ignore, separated by commas. Use '-' for ranges");
    ignoredInputLayout->addWidget(ignoredIdsLineEdit_);
    ignoredLayout->addLayout(ignoredInputLayout);
}

void FindItemDialog::setupNameSearchControls() {
    nameSearchGroup_ = new QGroupBox("Name Search", this);
    QVBoxLayout* layout = new QVBoxLayout(nameSearchGroup_);
    
    QHBoxLayout* nameLayout = new QHBoxLayout();
    nameLayout->addWidget(new QLabel("Name:"));
    nameLineEdit_ = new QLineEdit();
    nameLineEdit_->setPlaceholderText("Enter item name or partial name");
    nameLineEdit_->setEnabled(false);
    nameLayout->addWidget(nameLineEdit_);
    layout->addLayout(nameLayout);
    
    caseSensitiveCheckBox_ = new QCheckBox("Case Sensitive");
    layout->addWidget(caseSensitiveCheckBox_);
    
    wholeWordCheckBox_ = new QCheckBox("Whole Word Only");
    layout->addWidget(wholeWordCheckBox_);
}

void FindItemDialog::setupTypeSearchControls() {
    typeSearchGroup_ = new QGroupBox("Type Search", this);
    QVBoxLayout* layout = new QVBoxLayout(typeSearchGroup_);
    
    QHBoxLayout* typeLayout = new QHBoxLayout();
    typeLayout->addWidget(new QLabel("Item Type:"));
    typeComboBox_ = new QComboBox();
    typeComboBox_->setEnabled(false);
    populateTypeComboBox();
    typeLayout->addWidget(typeComboBox_);
    layout->addLayout(typeLayout);
}

void FindItemDialog::setupPropertySearchControls() {
    propertyGroup_ = new QGroupBox("Property Search", this);
    QVBoxLayout* layout = new QVBoxLayout(propertyGroup_);
    
    propertyScrollArea_ = new QScrollArea();
    propertyWidget_ = new QWidget();
    propertyLayout_ = new QGridLayout(propertyWidget_);
    
    // Create property checkboxes with 3-state support
    int row = 0;
    
    // Basic properties
    unpassableCheckBox_ = new QCheckBox("Unpassable");
    unpassableCheckBox_->setTristate(true);
    propertyLayout_->addWidget(unpassableCheckBox_, row++, 0);
    
    unmovableCheckBox_ = new QCheckBox("Unmovable");
    unmovableCheckBox_->setTristate(true);
    propertyLayout_->addWidget(unmovableCheckBox_, row++, 0);
    
    blockMissilesCheckBox_ = new QCheckBox("Block Missiles");
    blockMissilesCheckBox_->setTristate(true);
    propertyLayout_->addWidget(blockMissilesCheckBox_, row++, 0);
    
    blockPathfinderCheckBox_ = new QCheckBox("Block Pathfinder");
    blockPathfinderCheckBox_->setTristate(true);
    propertyLayout_->addWidget(blockPathfinderCheckBox_, row++, 0);
    
    readableCheckBox_ = new QCheckBox("Readable");
    readableCheckBox_->setTristate(true);
    propertyLayout_->addWidget(readableCheckBox_, row++, 0);
    
    writeableCheckBox_ = new QCheckBox("Writeable");
    writeableCheckBox_->setTristate(true);
    propertyLayout_->addWidget(writeableCheckBox_, row++, 0);
    
    pickupableCheckBox_ = new QCheckBox("Pickupable");
    pickupableCheckBox_->setTristate(true);
    if (onlyPickupables_) {
        pickupableCheckBox_->setCheckState(Qt::Checked);
        pickupableCheckBox_->setEnabled(false);
    }
    propertyLayout_->addWidget(pickupableCheckBox_, row++, 0);
    
    stackableCheckBox_ = new QCheckBox("Stackable");
    stackableCheckBox_->setTristate(true);
    propertyLayout_->addWidget(stackableCheckBox_, row++, 0);
    
    // Reset row for second column
    row = 0;
    
    rotatableCheckBox_ = new QCheckBox("Rotatable");
    rotatableCheckBox_->setTristate(true);
    propertyLayout_->addWidget(rotatableCheckBox_, row++, 1);
    
    hangableCheckBox_ = new QCheckBox("Hangable");
    hangableCheckBox_->setTristate(true);
    propertyLayout_->addWidget(hangableCheckBox_, row++, 1);
    
    hookEastCheckBox_ = new QCheckBox("Hook East");
    hookEastCheckBox_->setTristate(true);
    propertyLayout_->addWidget(hookEastCheckBox_, row++, 1);
    
    hookSouthCheckBox_ = new QCheckBox("Hook South");
    hookSouthCheckBox_->setTristate(true);
    propertyLayout_->addWidget(hookSouthCheckBox_, row++, 1);
    
    hasElevationCheckBox_ = new QCheckBox("Has Elevation");
    hasElevationCheckBox_->setTristate(true);
    propertyLayout_->addWidget(hasElevationCheckBox_, row++, 1);
    
    ignoreLookCheckBox_ = new QCheckBox("Ignore Look");
    ignoreLookCheckBox_->setTristate(true);
    propertyLayout_->addWidget(ignoreLookCheckBox_, row++, 1);
    
    floorChangeCheckBox_ = new QCheckBox("Floor Change");
    floorChangeCheckBox_->setTristate(true);
    propertyLayout_->addWidget(floorChangeCheckBox_, row++, 1);
    
    hasLightCheckBox_ = new QCheckBox("Has Light");
    hasLightCheckBox_->setTristate(true);
    propertyLayout_->addWidget(hasLightCheckBox_, row++, 1);
    
    // Slot properties in third column
    row = 0;
    
    slotHeadCheckBox_ = new QCheckBox("Head Slot");
    slotHeadCheckBox_->setTristate(true);
    propertyLayout_->addWidget(slotHeadCheckBox_, row++, 2);
    
    slotNecklaceCheckBox_ = new QCheckBox("Necklace Slot");
    slotNecklaceCheckBox_->setTristate(true);
    propertyLayout_->addWidget(slotNecklaceCheckBox_, row++, 2);
    
    slotBackpackCheckBox_ = new QCheckBox("Backpack Slot");
    slotBackpackCheckBox_->setTristate(true);
    propertyLayout_->addWidget(slotBackpackCheckBox_, row++, 2);
    
    slotArmorCheckBox_ = new QCheckBox("Armor Slot");
    slotArmorCheckBox_->setTristate(true);
    propertyLayout_->addWidget(slotArmorCheckBox_, row++, 2);
    
    slotLegsCheckBox_ = new QCheckBox("Legs Slot");
    slotLegsCheckBox_->setTristate(true);
    propertyLayout_->addWidget(slotLegsCheckBox_, row++, 2);
    
    slotFeetCheckBox_ = new QCheckBox("Feet Slot");
    slotFeetCheckBox_->setTristate(true);
    propertyLayout_->addWidget(slotFeetCheckBox_, row++, 2);
    
    slotRingCheckBox_ = new QCheckBox("Ring Slot");
    slotRingCheckBox_->setTristate(true);
    propertyLayout_->addWidget(slotRingCheckBox_, row++, 2);
    
    slotAmmoCheckBox_ = new QCheckBox("Ammo Slot");
    slotAmmoCheckBox_->setTristate(true);
    propertyLayout_->addWidget(slotAmmoCheckBox_, row++, 2);
    
    propertyScrollArea_->setWidget(propertyWidget_);
    propertyScrollArea_->setWidgetResizable(true);
    propertyScrollArea_->setMaximumHeight(200);
    layout->addWidget(propertyScrollArea_);
    
    setupPropertyTooltips();
    enablePropertyControls(false);
}

void FindItemDialog::setupAdvancedOptions() {
    advancedGroup_ = new QGroupBox("Advanced Options", this);
    QVBoxLayout* layout = new QVBoxLayout(advancedGroup_);
    
    autoRefreshCheckBox_ = new QCheckBox("Auto Refresh (F5)");
    autoRefreshCheckBox_->setChecked(true);
    layout->addWidget(autoRefreshCheckBox_);
    
    QHBoxLayout* maxResultsLayout = new QHBoxLayout();
    maxResultsLayout->addWidget(new QLabel("Max Results:"));
    maxResultsSpinBox_ = new QSpinBox();
    maxResultsSpinBox_->setRange(100, 10000);
    maxResultsSpinBox_->setValue(1000);
    maxResultsLayout->addWidget(maxResultsSpinBox_);
    maxResultsLayout->addStretch();
    layout->addLayout(maxResultsLayout);
    
    searchInSelectionCheckBox_ = new QCheckBox("Search in selection only");
    layout->addWidget(searchInSelectionCheckBox_);
    
    browseTileButton_ = new QPushButton("Browse Tile");
    browseTileButton_->setToolTip("Click to select a tile on the map and populate search criteria from its items");
    layout->addWidget(browseTileButton_);
}

void FindItemDialog::setupResultsArea() {
    resultsGroup_ = new QGroupBox("Search Results", this);
    QVBoxLayout* layout = new QVBoxLayout(resultsGroup_);
    
    // Results count and progress
    QHBoxLayout* statusLayout = new QHBoxLayout();
    resultsCountLabel_ = new QLabel("No results");
    statusLayout->addWidget(resultsCountLabel_);
    statusLayout->addStretch();
    
    searchProgressBar_ = new QProgressBar();
    searchProgressBar_->setVisible(false);
    statusLayout->addWidget(searchProgressBar_);
    layout->addLayout(statusLayout);
    
    // Results table
    resultsTable_ = new QTableWidget();
    resultsTable_->setColumnCount(5);
    QStringList headers = {"ID", "Name", "X", "Y", "Z"};
    resultsTable_->setHorizontalHeaderLabels(headers);
    resultsTable_->setSelectionBehavior(QAbstractItemView::SelectRows);
    resultsTable_->setAlternatingRowColors(true);
    resultsTable_->setSortingEnabled(true);
    
    // Set column widths
    resultsTable_->horizontalHeader()->setStretchLastSection(false);
    resultsTable_->setColumnWidth(0, 60);  // ID
    resultsTable_->setColumnWidth(1, 200); // Name
    resultsTable_->setColumnWidth(2, 60);  // X
    resultsTable_->setColumnWidth(3, 60);  // Y
    resultsTable_->setColumnWidth(4, 60);  // Z
    resultsTable_->horizontalHeader()->setStretchLastSection(true);
    
    layout->addWidget(resultsTable_);
}

void FindItemDialog::setupActionButtons() {
    buttonLayout_ = new QHBoxLayout();
    
    findButton_ = new QPushButton("Find");
    findButton_->setDefault(true);
    buttonLayout_->addWidget(findButton_);
    
    findNextButton_ = new QPushButton("Find Next");
    findNextButton_->setEnabled(false);
    buttonLayout_->addWidget(findNextButton_);
    
    goToSelectedButton_ = new QPushButton("Go to Selected");
    goToSelectedButton_->setEnabled(false);
    buttonLayout_->addWidget(goToSelectedButton_);
    
    refreshButton_ = new QPushButton("Refresh");
    buttonLayout_->addWidget(refreshButton_);
    
    clearResultsButton_ = new QPushButton("Clear Results");
    buttonLayout_->addWidget(clearResultsButton_);
    
    buttonLayout_->addStretch();
    
    closeButton_ = new QPushButton("Close");
    buttonLayout_->addWidget(closeButton_);
}

void FindItemDialog::connectSignals() {
    // Search mode changes
    connect(searchModeButtonGroup_, QOverload<int>::of(&QButtonGroup::buttonClicked),
            this, &FindItemDialog::onSearchModeChanged);

    // ID controls
    connect(serverIdSpinBox_, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &FindItemDialog::onServerIdChanged);
    connect(clientIdSpinBox_, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &FindItemDialog::onClientIdChanged);
    connect(invalidItemCheckBox_, &QCheckBox::toggled,
            this, &FindItemDialog::onPropertyChanged);

    // Name controls
    connect(nameLineEdit_, &QLineEdit::textChanged,
            this, &FindItemDialog::onNameTextChanged);
    connect(caseSensitiveCheckBox_, &QCheckBox::toggled,
            this, &FindItemDialog::onPropertyChanged);
    connect(wholeWordCheckBox_, &QCheckBox::toggled,
            this, &FindItemDialog::onPropertyChanged);

    // Type controls
    connect(typeComboBox_, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &FindItemDialog::onTypeSelectionChanged);

    // Range and ignore controls
    connect(useRangeCheckBox_, &QCheckBox::toggled,
            this, &FindItemDialog::onRangeToggled);
    connect(ignoreIdsCheckBox_, &QCheckBox::toggled,
            this, &FindItemDialog::onIgnoreIdsToggled);
    connect(rangeLineEdit_, &QLineEdit::textChanged,
            this, &FindItemDialog::onPropertyChanged);
    connect(ignoredIdsLineEdit_, &QLineEdit::textChanged,
            this, &FindItemDialog::onPropertyChanged);

    // Advanced options
    connect(autoRefreshCheckBox_, &QCheckBox::toggled,
            this, &FindItemDialog::onAutoRefreshToggled);
    connect(maxResultsSpinBox_, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &FindItemDialog::onMaxResultsChanged);
    connect(searchInSelectionCheckBox_, &QCheckBox::toggled,
            this, &FindItemDialog::onPropertyChanged);

    // Property checkboxes
    QList<QCheckBox*> propertyCheckBoxes = {
        unpassableCheckBox_, unmovableCheckBox_, blockMissilesCheckBox_, blockPathfinderCheckBox_,
        readableCheckBox_, writeableCheckBox_, pickupableCheckBox_, stackableCheckBox_,
        rotatableCheckBox_, hangableCheckBox_, hookEastCheckBox_, hookSouthCheckBox_,
        hasElevationCheckBox_, ignoreLookCheckBox_, floorChangeCheckBox_, hasLightCheckBox_,
        slotHeadCheckBox_, slotNecklaceCheckBox_, slotBackpackCheckBox_, slotArmorCheckBox_,
        slotLegsCheckBox_, slotFeetCheckBox_, slotRingCheckBox_, slotAmmoCheckBox_
    };

    for (QCheckBox* checkBox : propertyCheckBoxes) {
        connect(checkBox, &QCheckBox::stateChanged,
                this, &FindItemDialog::onPropertyChanged);
    }

    // Results handling
    connect(resultsTable_, &QTableWidget::itemClicked,
            this, &FindItemDialog::onResultItemClicked);
    connect(resultsTable_, &QTableWidget::itemDoubleClicked,
            this, &FindItemDialog::onResultItemDoubleClicked);
    connect(resultsTable_, &QTableWidget::itemSelectionChanged,
            this, &FindItemDialog::onResultSelectionChanged);

    // Action buttons
    connect(findButton_, &QPushButton::clicked, this, &FindItemDialog::onFind);
    connect(findNextButton_, &QPushButton::clicked, this, &FindItemDialog::onFindNext);
    connect(goToSelectedButton_, &QPushButton::clicked, this, &FindItemDialog::onGoToSelected);
    connect(browseTileButton_, &QPushButton::clicked, this, &FindItemDialog::onBrowseTile);
    connect(refreshButton_, &QPushButton::clicked, this, &FindItemDialog::onRefresh);
    connect(clearResultsButton_, &QPushButton::clicked, this, &FindItemDialog::onClearResults);
    connect(closeButton_, &QPushButton::clicked, this, &QDialog::close);

    // Auto-refresh timer
    connect(refreshTimer_, &QTimer::timeout, this, &FindItemDialog::onRefreshTimer);
}

// Search mode management
FindItemDialog::SearchMode FindItemDialog::getSearchMode() const {
    int buttonId = searchModeButtonGroup_->checkedId();
    return static_cast<SearchMode>(buttonId);
}

void FindItemDialog::setSearchMode(SearchMode mode) {
    QAbstractButton* button = searchModeButtonGroup_->button(static_cast<int>(mode));
    if (button) {
        button->setChecked(true);
        onSearchModeChanged();
    }
}

// Integration methods
void FindItemDialog::setMap(Map* map) {
    map_ = map;
}

void FindItemDialog::setMapView(MapView* mapView) {
    mapView_ = mapView;
}

void FindItemDialog::setItemManager(ItemManager* itemManager) {
    itemManager_ = itemManager;
    if (itemManager_) {
        // Update max values for spin boxes
        quint16 maxId = itemManager_->getMaxServerId();
        serverIdSpinBox_->setMaximum(maxId);
        clientIdSpinBox_->setMaximum(maxId);
    }
}

void FindItemDialog::setTilesetManager(TilesetManager* tilesetManager) {
    tilesetManager_ = tilesetManager;
}

// Browse tile functionality
void FindItemDialog::populateFromTile(const QPoint& tilePos, int floor) {
    if (!map_) {
        QMessageBox::warning(this, "Warning", "No map available for tile browsing.");
        return;
    }

    Tile* tile = map_->getTile(tilePos.x(), tilePos.y(), floor);
    if (!tile) {
        QMessageBox::information(this, "Information", "No tile found at the specified position.");
        return;
    }

    QList<quint16> itemIds;
    for (Item* item : tile->getItems()) {
        if (item) {
            itemIds.append(item->getServerId());
        }
    }

    populateFromTileItems(itemIds);
}

void FindItemDialog::populateFromTileItems(const QList<quint16>& itemIds) {
    if (itemIds.isEmpty()) {
        QMessageBox::information(this, "Information", "No items found on the selected tile.");
        return;
    }

    // Set search mode to Server IDs
    setSearchMode(SearchMode::ServerIDs);

    // If only one item, set it directly
    if (itemIds.size() == 1) {
        serverIdSpinBox_->setValue(itemIds.first());
    } else {
        // Multiple items - create a range
        QString rangeText;
        for (int i = 0; i < itemIds.size(); ++i) {
            if (i > 0) rangeText += ",";
            rangeText += QString::number(itemIds[i]);
        }

        useRangeCheckBox_->setChecked(true);
        rangeLineEdit_->setText(rangeText);
    }

    // Trigger search
    onFind();
}

// Main action slots
void FindItemDialog::onFind() {
    performSearch();
}

void FindItemDialog::onFindNext() {
    if (currentResults_.isEmpty()) {
        onFind();
        return;
    }

    currentResultIndex_++;
    if (currentResultIndex_ >= currentResults_.size()) {
        currentResultIndex_ = 0;
    }

    // Select the next result in the table
    if (currentResultIndex_ < resultsTable_->rowCount()) {
        resultsTable_->selectRow(currentResultIndex_);
        onGoToSelected();
    }
}

void FindItemDialog::onGoToSelected() {
    int currentRow = resultsTable_->currentRow();
    if (currentRow < 0 || currentRow >= currentResults_.size()) {
        return;
    }

    const ItemFinder::ItemResult& result = currentResults_[currentRow];
    QPoint position(result.position.x, result.position.y);
    int floor = result.position.z;

    emit goToRequested(position, floor);

    if (mapView_) {
        // Center the map view on the selected position
        // This would need to be implemented in MapView
        qDebug() << "FindItemDialog: Go to position" << position << "floor" << floor;
    }
}

void FindItemDialog::onBrowseTile() {
    emit browseTileRequested();

    // Hide dialog temporarily to allow tile selection
    hide();

    QMessageBox::information(this, "Browse Tile",
        "Click on a tile in the map view to populate search criteria from its items.\n"
        "This feature requires MapView integration.");

    show();
}

void FindItemDialog::onRefresh() {
    if (!currentResults_.isEmpty()) {
        performSearch();
    }
}

void FindItemDialog::onClearResults() {
    clearResults();
    updateButtonStates();
}

// Search mode change slots
void FindItemDialog::onSearchModeChanged() {
    SearchMode mode = getSearchMode();

    // Enable/disable controls based on search mode
    bool isServerIds = (mode == SearchMode::ServerIDs);
    bool isClientIds = (mode == SearchMode::ClientIDs);
    bool isNames = (mode == SearchMode::Names);
    bool isTypes = (mode == SearchMode::Types);
    bool isProperties = (mode == SearchMode::Properties);

    // ID controls
    serverIdSpinBox_->setEnabled(isServerIds);
    invalidItemCheckBox_->setEnabled(isServerIds);
    clientIdSpinBox_->setEnabled(isClientIds);

    // Range controls (for ID modes)
    useRangeCheckBox_->setEnabled(isServerIds || isClientIds);
    rangeLineEdit_->setEnabled((isServerIds || isClientIds) && useRangeCheckBox_->isChecked());

    // Name controls
    nameLineEdit_->setEnabled(isNames);
    caseSensitiveCheckBox_->setEnabled(isNames);
    wholeWordCheckBox_->setEnabled(isNames);

    // Type controls
    typeComboBox_->setEnabled(isTypes);

    // Property controls
    enablePropertyControls(isProperties);

    // Set focus to appropriate control
    if (isServerIds) {
        serverIdSpinBox_->setFocus();
        serverIdSpinBox_->selectAll();
    } else if (isClientIds) {
        clientIdSpinBox_->setFocus();
        clientIdSpinBox_->selectAll();
    } else if (isNames) {
        nameLineEdit_->setFocus();
        nameLineEdit_->selectAll();
    }

    // Auto-refresh if enabled
    if (autoRefreshCheckBox_->isChecked()) {
        refreshTimer_->start();
    }
}

void FindItemDialog::onServerIdChanged() {
    if (autoRefreshCheckBox_->isChecked()) {
        refreshTimer_->start();
    }
}

void FindItemDialog::onClientIdChanged() {
    if (autoRefreshCheckBox_->isChecked()) {
        refreshTimer_->start();
    }
}

void FindItemDialog::onNameTextChanged() {
    if (autoRefreshCheckBox_->isChecked()) {
        refreshTimer_->start();
    }
}

void FindItemDialog::onTypeSelectionChanged() {
    if (autoRefreshCheckBox_->isChecked()) {
        refreshTimer_->start();
    }
}

void FindItemDialog::onPropertyChanged() {
    if (autoRefreshCheckBox_->isChecked()) {
        refreshTimer_->start();
    }
}

// Advanced option slots
void FindItemDialog::onRangeToggled(bool enabled) {
    rangeLineEdit_->setEnabled(enabled && (getSearchMode() == SearchMode::ServerIDs || getSearchMode() == SearchMode::ClientIDs));
    if (autoRefreshCheckBox_->isChecked()) {
        refreshTimer_->start();
    }
}

void FindItemDialog::onIgnoreIdsToggled(bool enabled) {
    ignoredIdsLineEdit_->setEnabled(enabled);
    if (autoRefreshCheckBox_->isChecked()) {
        refreshTimer_->start();
    }
}

void FindItemDialog::onAutoRefreshToggled(bool enabled) {
    if (enabled) {
        refreshTimer_->start();
    } else {
        refreshTimer_->stop();
    }
}

void FindItemDialog::onMaxResultsChanged(int value) {
    Q_UNUSED(value)
    if (autoRefreshCheckBox_->isChecked()) {
        refreshTimer_->start();
    }
}

// Results handling slots
void FindItemDialog::onResultItemClicked() {
    updateButtonStates();
}

void FindItemDialog::onResultItemDoubleClicked() {
    onGoToSelected();
}

void FindItemDialog::onResultSelectionChanged() {
    updateButtonStates();
}

void FindItemDialog::onRefreshTimer() {
    performSearch();
}

// Core search functionality
void FindItemDialog::performSearch() {
    if (!map_) {
        QMessageBox::warning(this, "Warning", "No map available for search.");
        return;
    }

    searchInProgress_ = true;
    searchProgressBar_->setVisible(true);
    searchProgressBar_->setRange(0, 0); // Indeterminate progress

    // Build search criteria
    ItemFinder::SearchCriteria criteria;
    buildSearchCriteria(criteria);

    // Perform search using ItemFinder
    QList<ItemFinder::ItemResult> results;

    try {
        if (searchInSelectionCheckBox_->isChecked()) {
            // Search in selection only - this would need MapView integration
            qDebug() << "FindItemDialog: Search in selection not yet implemented";
            results = ItemFinder::findAllItemsInMap(map_, criteria);
        } else {
            // Search entire map
            results = ItemFinder::findAllItemsInMap(map_, criteria);
        }

        // Limit results
        int maxResults = maxResultsSpinBox_->value();
        if (results.size() > maxResults) {
            results = results.mid(0, maxResults);
        }

        // Filter ignored IDs
        if (ignoreIdsCheckBox_->isChecked()) {
            QSet<quint16> ignoredIds = parseIgnoredIds(ignoredIdsLineEdit_->text());
            results.erase(std::remove_if(results.begin(), results.end(),
                [&ignoredIds](const ItemFinder::ItemResult& result) {
                    return result.item && ignoredIds.contains(result.item->getServerId());
                }), results.end());
        }

        populateResults(results);

    } catch (const std::exception& e) {
        QMessageBox::critical(this, "Search Error", QString("Search failed: %1").arg(e.what()));
        clearResults();
    }

    searchInProgress_ = false;
    searchProgressBar_->setVisible(false);
    updateButtonStates();
}

void FindItemDialog::buildSearchCriteria(ItemFinder::SearchCriteria& criteria) {
    SearchMode mode = getSearchMode();

    switch (mode) {
        case SearchMode::ServerIDs: {
            if (useRangeCheckBox_->isChecked() && !rangeLineEdit_->text().isEmpty()) {
                // Parse range
                QVector<QPair<quint16, quint16>> ranges = parseIdRanges(rangeLineEdit_->text());
                for (const auto& range : ranges) {
                    for (quint16 id = range.first; id <= range.second; ++id) {
                        criteria.serverIds.insert(id);
                    }
                }
            } else {
                criteria.serverIds.insert(static_cast<quint16>(serverIdSpinBox_->value()));
            }
            break;
        }

        case SearchMode::ClientIDs: {
            if (useRangeCheckBox_->isChecked() && !rangeLineEdit_->text().isEmpty()) {
                // Parse range
                QVector<QPair<quint16, quint16>> ranges = parseIdRanges(rangeLineEdit_->text());
                for (const auto& range : ranges) {
                    for (quint16 id = range.first; id <= range.second; ++id) {
                        criteria.clientIds.insert(id);
                    }
                }
            } else {
                criteria.clientIds.insert(static_cast<quint16>(clientIdSpinBox_->value()));
            }
            break;
        }

        case SearchMode::Names: {
            QString searchText = nameLineEdit_->text();
            if (!searchText.isEmpty()) {
                // This would need to be implemented as a custom predicate
                // For now, store in typeNames for processing
                criteria.typeNames.append(searchText);
            }
            break;
        }

        case SearchMode::Types: {
            SearchItemType itemType = getSelectedItemType();
            // Convert to ItemTypes_t - this would need proper mapping
            // For now, use a placeholder
            qDebug() << "FindItemDialog: Type search for" << getItemTypeName(itemType);
            break;
        }

        case SearchMode::Properties: {
            // Build property criteria from checkboxes
            if (pickupableCheckBox_->checkState() == Qt::Checked) {
                criteria.requiresMoveable = true;
            }
            if (stackableCheckBox_->checkState() == Qt::Checked) {
                criteria.requiresStackable = true;
            }
            if (blockMissilesCheckBox_->checkState() == Qt::Checked) {
                criteria.requiresBlocking = true;
            }
            // Add more property mappings as needed
            break;
        }
    }
}

// Results management
void FindItemDialog::populateResults(const QList<ItemFinder::ItemResult>& results) {
    currentResults_ = results;
    currentResultIndex_ = -1;

    resultsTable_->setRowCount(results.size());

    for (int i = 0; i < results.size(); ++i) {
        const ItemFinder::ItemResult& result = results[i];

        // ID column
        QTableWidgetItem* idItem = new QTableWidgetItem(QString::number(result.item ? result.item->getServerId() : 0));
        idItem->setData(Qt::UserRole, i); // Store result index
        resultsTable_->setItem(i, 0, idItem);

        // Name column
        QString itemName = getItemDisplayName(result.item ? result.item->getServerId() : 0);
        QTableWidgetItem* nameItem = new QTableWidgetItem(itemName);
        resultsTable_->setItem(i, 1, nameItem);

        // Position columns
        QTableWidgetItem* xItem = new QTableWidgetItem(QString::number(result.position.x));
        QTableWidgetItem* yItem = new QTableWidgetItem(QString::number(result.position.y));
        QTableWidgetItem* zItem = new QTableWidgetItem(QString::number(result.position.z));

        resultsTable_->setItem(i, 2, xItem);
        resultsTable_->setItem(i, 3, yItem);
        resultsTable_->setItem(i, 4, zItem);
    }

    updateResultsCount();

    // Select first result if available
    if (!results.isEmpty()) {
        resultsTable_->selectRow(0);
        currentResultIndex_ = 0;
    }
}

void FindItemDialog::clearResults() {
    currentResults_.clear();
    currentResultIndex_ = -1;
    resultsTable_->setRowCount(0);
    updateResultsCount();
}

void FindItemDialog::updateResultsCount() {
    int count = currentResults_.size();
    if (count == 0) {
        resultsCountLabel_->setText("No results");
    } else if (count == 1) {
        resultsCountLabel_->setText("1 result found");
    } else {
        resultsCountLabel_->setText(QString("%1 results found").arg(count));
    }
}

void FindItemDialog::updateButtonStates() {
    bool hasResults = !currentResults_.isEmpty();
    bool hasSelection = resultsTable_->currentRow() >= 0;

    findNextButton_->setEnabled(hasResults);
    goToSelectedButton_->setEnabled(hasResults && hasSelection);
    clearResultsButton_->setEnabled(hasResults);
}

// Property management
void FindItemDialog::enablePropertyControls(bool enabled) {
    QList<QCheckBox*> propertyCheckBoxes = {
        unpassableCheckBox_, unmovableCheckBox_, blockMissilesCheckBox_, blockPathfinderCheckBox_,
        readableCheckBox_, writeableCheckBox_, pickupableCheckBox_, stackableCheckBox_,
        rotatableCheckBox_, hangableCheckBox_, hookEastCheckBox_, hookSouthCheckBox_,
        hasElevationCheckBox_, ignoreLookCheckBox_, floorChangeCheckBox_, hasLightCheckBox_,
        slotHeadCheckBox_, slotNecklaceCheckBox_, slotBackpackCheckBox_, slotArmorCheckBox_,
        slotLegsCheckBox_, slotFeetCheckBox_, slotRingCheckBox_, slotAmmoCheckBox_
    };

    for (QCheckBox* checkBox : propertyCheckBoxes) {
        checkBox->setEnabled(enabled && (!onlyPickupables_ || checkBox != pickupableCheckBox_));
    }
}

void FindItemDialog::resetPropertyControls() {
    QList<QCheckBox*> propertyCheckBoxes = {
        unpassableCheckBox_, unmovableCheckBox_, blockMissilesCheckBox_, blockPathfinderCheckBox_,
        readableCheckBox_, writeableCheckBox_, pickupableCheckBox_, stackableCheckBox_,
        rotatableCheckBox_, hangableCheckBox_, hookEastCheckBox_, hookSouthCheckBox_,
        hasElevationCheckBox_, ignoreLookCheckBox_, floorChangeCheckBox_, hasLightCheckBox_,
        slotHeadCheckBox_, slotNecklaceCheckBox_, slotBackpackCheckBox_, slotArmorCheckBox_,
        slotLegsCheckBox_, slotFeetCheckBox_, slotRingCheckBox_, slotAmmoCheckBox_
    };

    for (QCheckBox* checkBox : propertyCheckBoxes) {
        checkBox->setCheckState(Qt::PartiallyChecked);
    }

    if (onlyPickupables_) {
        pickupableCheckBox_->setCheckState(Qt::Checked);
    }
}

void FindItemDialog::setupPropertyTooltips() {
    QString tooltip = "Click to cycle through states:\n"
                     "☐ Ignore this property\n"
                     "☑ Must have this property\n"
                     "☒ Must NOT have this property";

    QList<QCheckBox*> propertyCheckBoxes = {
        unpassableCheckBox_, unmovableCheckBox_, blockMissilesCheckBox_, blockPathfinderCheckBox_,
        readableCheckBox_, writeableCheckBox_, pickupableCheckBox_, stackableCheckBox_,
        rotatableCheckBox_, hangableCheckBox_, hookEastCheckBox_, hookSouthCheckBox_,
        hasElevationCheckBox_, ignoreLookCheckBox_, floorChangeCheckBox_, hasLightCheckBox_,
        slotHeadCheckBox_, slotNecklaceCheckBox_, slotBackpackCheckBox_, slotArmorCheckBox_,
        slotLegsCheckBox_, slotFeetCheckBox_, slotRingCheckBox_, slotAmmoCheckBox_
    };

    for (QCheckBox* checkBox : propertyCheckBoxes) {
        checkBox->setToolTip(tooltip);
    }
}

// Range and ID parsing
QVector<QPair<quint16, quint16>> FindItemDialog::parseIdRanges(const QString& rangeText) {
    QVector<QPair<quint16, quint16>> ranges;

    QStringList parts = rangeText.split(',', Qt::SkipEmptyParts);
    for (const QString& part : parts) {
        QString trimmed = part.trimmed();
        if (trimmed.contains('-')) {
            QStringList rangeParts = trimmed.split('-');
            if (rangeParts.size() == 2) {
                bool ok1, ok2;
                quint16 start = rangeParts[0].trimmed().toUShort(&ok1);
                quint16 end = rangeParts[1].trimmed().toUShort(&ok2);
                if (ok1 && ok2 && start <= end) {
                    ranges.append(qMakePair(start, end));
                }
            }
        } else {
            bool ok;
            quint16 id = trimmed.toUShort(&ok);
            if (ok) {
                ranges.append(qMakePair(id, id));
            }
        }
    }

    return ranges;
}

QSet<quint16> FindItemDialog::parseIgnoredIds(const QString& ignoredText) {
    QSet<quint16> ignoredIds;

    QVector<QPair<quint16, quint16>> ranges = parseIdRanges(ignoredText);
    for (const auto& range : ranges) {
        for (quint16 id = range.first; id <= range.second; ++id) {
            ignoredIds.insert(id);
        }
    }

    return ignoredIds;
}

bool FindItemDialog::isIdInRanges(quint16 id, const QVector<QPair<quint16, quint16>>& ranges) {
    for (const auto& range : ranges) {
        if (id >= range.first && id <= range.second) {
            return true;
        }
    }
    return false;
}

bool FindItemDialog::isIdIgnored(quint16 id) {
    if (!ignoreIdsCheckBox_->isChecked()) {
        return false;
    }

    QSet<quint16> ignoredIds = parseIgnoredIds(ignoredIdsLineEdit_->text());
    return ignoredIds.contains(id);
}

// Utility methods
void FindItemDialog::populateTypeComboBox() {
    typeComboBox_->clear();

    typeComboBox_->addItem("Depot", static_cast<int>(SearchItemType::Depot));
    typeComboBox_->addItem("Mailbox", static_cast<int>(SearchItemType::Mailbox));
    typeComboBox_->addItem("Trash Holder", static_cast<int>(SearchItemType::TrashHolder));
    typeComboBox_->addItem("Container", static_cast<int>(SearchItemType::Container));
    typeComboBox_->addItem("Door", static_cast<int>(SearchItemType::Door));
    typeComboBox_->addItem("Magic Field", static_cast<int>(SearchItemType::MagicField));
    typeComboBox_->addItem("Teleport", static_cast<int>(SearchItemType::Teleport));
    typeComboBox_->addItem("Bed", static_cast<int>(SearchItemType::Bed));
    typeComboBox_->addItem("Key", static_cast<int>(SearchItemType::Key));
    typeComboBox_->addItem("Podium", static_cast<int>(SearchItemType::Podium));
}

QString FindItemDialog::getItemDisplayName(quint16 itemId) {
    if (itemManager_) {
        const ItemProperties& props = itemManager_->getItemProperties(itemId);
        if (!props.name.isEmpty()) {
            return QString("%1").arg(props.name);
        }
    }

    return QString("Item %1").arg(itemId);
}

QString FindItemDialog::getItemTypeName(SearchItemType type) {
    switch (type) {
        case SearchItemType::Depot: return "Depot";
        case SearchItemType::Mailbox: return "Mailbox";
        case SearchItemType::TrashHolder: return "Trash Holder";
        case SearchItemType::Container: return "Container";
        case SearchItemType::Door: return "Door";
        case SearchItemType::MagicField: return "Magic Field";
        case SearchItemType::Teleport: return "Teleport";
        case SearchItemType::Bed: return "Bed";
        case SearchItemType::Key: return "Key";
        case SearchItemType::Podium: return "Podium";
        default: return "Unknown";
    }
}

FindItemDialog::SearchItemType FindItemDialog::getSelectedItemType() {
    int index = typeComboBox_->currentData().toInt();
    return static_cast<SearchItemType>(index);
}
