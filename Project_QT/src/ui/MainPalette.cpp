#include "MainPalette.h"
#include "Map.h"
#include "Brush.h"
#include "Item.h"
#include "ResourceManager.h"
#include <QApplication>
#include <QStyle>
#include <QPixmap>
#include <QIcon>
#include <QDebug>
#include <QMessageBox>
#include <QSettings>

// MainPalette implementation
MainPalette::MainPalette(QWidget* parent)
    : QWidget(parent)
    , tilesetManager_(nullptr)
    , brushManager_(nullptr)
    , itemManager_(nullptr)
    , map_(nullptr)
    , selectedBrush_(nullptr)
    , selectedBrushSize_(1)
    , currentPaletteType_(TilesetCategoryType::Terrain)
    , needsRefresh_(false)
    , actionId_(0)
    , actionIdEnabled_(false) {
    
    setupUI();
    connectSignals();
    
    // Initialize refresh timer
    refreshTimer_ = new QTimer(this);
    refreshTimer_->setSingleShot(true);
    refreshTimer_->setInterval(100); // 100ms delay for batch updates
    connect(refreshTimer_, &QTimer::timeout, this, &MainPalette::onRefreshTimer);
}

MainPalette::~MainPalette() {
    // Qt handles cleanup automatically
}

void MainPalette::setupUI() {
    mainLayout_ = new QVBoxLayout(this);
    mainLayout_->setContentsMargins(4, 4, 4, 4);
    mainLayout_->setSpacing(4);
    
    setupActionIdControls();
    setupTabWidget();
    
    setMinimumSize(225, 250);
    setMaximumWidth(400);
}

void MainPalette::setupActionIdControls() {
    // Action ID controls (from wxwidgets)
    QGroupBox* actionIdGroup = new QGroupBox("Action ID", this);
    actionIdLayout_ = new QHBoxLayout(actionIdGroup);
    
    actionIdSpinBox_ = new QSpinBox(actionIdGroup);
    actionIdSpinBox_->setRange(0, 65535);
    actionIdSpinBox_->setValue(0);
    actionIdSpinBox_->setFixedWidth(80);
    actionIdSpinBox_->setToolTip("Enter action ID (0-65535)");
    
    actionIdCheckBox_ = new QCheckBox("Enable Action ID", actionIdGroup);
    actionIdCheckBox_->setToolTip("When enabled, placed items will have this action ID");
    
    actionIdLayout_->addWidget(actionIdSpinBox_);
    actionIdLayout_->addWidget(actionIdCheckBox_);
    actionIdLayout_->addStretch();
    
    mainLayout_->addWidget(actionIdGroup);
}

void MainPalette::setupTabWidget() {
    tabWidget_ = new QTabWidget(this);
    tabWidget_->setTabPosition(QTabWidget::North);
    tabWidget_->setMovable(false);
    
    // Create palette tabs in order from wxwidgets
    terrainPalette_ = createTerrainPalette();
    tabWidget_->addTab(terrainPalette_, "Terrain");
    
    doodadPalette_ = createDoodadPalette();
    tabWidget_->addTab(doodadPalette_, "Doodads");
    
    collectionPalette_ = createCollectionPalette();
    tabWidget_->addTab(collectionPalette_, "Collections");
    
    itemPalette_ = createItemPalette();
    tabWidget_->addTab(itemPalette_, "Items");
    
    housePalette_ = createHousePalette();
    tabWidget_->addTab(housePalette_, "Houses");
    
    waypointPalette_ = createWaypointPalette();
    tabWidget_->addTab(waypointPalette_, "Waypoints");
    
    creaturePalette_ = createCreaturePalette();
    tabWidget_->addTab(creaturePalette_, "Creatures");
    
    rawPalette_ = createRawPalette();
    tabWidget_->addTab(rawPalette_, "RAW");
    
    mainLayout_->addWidget(tabWidget_, 1);
}

void MainPalette::connectSignals() {
    // Tab widget signals
    connect(tabWidget_, &QTabWidget::currentChanged, this, &MainPalette::onTabChanged);
    
    // Action ID signals
    connect(actionIdCheckBox_, &QCheckBox::toggled, this, &MainPalette::onActionIdToggled);
    connect(actionIdSpinBox_, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &MainPalette::onActionIdValueChanged);
}

// Integration methods
void MainPalette::setTilesetManager(TilesetManager* tilesetManager) {
    tilesetManager_ = tilesetManager;
    
    if (tilesetManager_) {
        // Connect tileset manager signals
        connect(tilesetManager_, &TilesetManager::tilesetAdded, this, &MainPalette::onTilesetChanged);
        connect(tilesetManager_, &TilesetManager::categoryAdded, this, &MainPalette::onTilesetChanged);
        connect(tilesetManager_, &TilesetManager::tilesetChanged, this, &MainPalette::onTilesetChanged);
    }
    
    // Update all category panels
    for (int i = 0; i < tabWidget_->count(); ++i) {
        TilesetCategoryPanel* panel = qobject_cast<TilesetCategoryPanel*>(tabWidget_->widget(i));
        if (panel) {
            panel->setTilesetManager(tilesetManager_);
        }
    }
}

void MainPalette::setBrushManager(BrushManager* brushManager) {
    brushManager_ = brushManager;
    
    // Update all category panels
    for (int i = 0; i < tabWidget_->count(); ++i) {
        TilesetCategoryPanel* panel = qobject_cast<TilesetCategoryPanel*>(tabWidget_->widget(i));
        if (panel) {
            panel->setBrushManager(brushManager_);
        }
    }
}

void MainPalette::setItemManager(ItemManager* itemManager) {
    itemManager_ = itemManager;
    
    // Update all category panels
    for (int i = 0; i < tabWidget_->count(); ++i) {
        TilesetCategoryPanel* panel = qobject_cast<TilesetCategoryPanel*>(tabWidget_->widget(i));
        if (panel) {
            panel->setItemManager(itemManager_);
        }
    }
}

void MainPalette::setMap(Map* map) {
    map_ = map;
    
    // Update house and waypoint palettes which need map access
    // This would be implemented when those specific palettes are created
}

// Palette creation methods
QWidget* MainPalette::createTerrainPalette() {
    TilesetCategoryPanel* panel = new TilesetCategoryPanel(TilesetCategoryType::Terrain, this);
    
    // Add terrain-specific tools
    QVBoxLayout* layout = new QVBoxLayout();
    
    // Add brush tool panel
    BrushToolPanel* toolPanel = new BrushToolPanel(panel);
    layout->addWidget(toolPanel);
    
    // Add brush size panel
    BrushSizePanel* sizePanel = new BrushSizePanel(panel);
    layout->addWidget(sizePanel);
    
    // Add the category panel
    layout->addWidget(panel, 1);
    
    QWidget* container = new QWidget();
    container->setLayout(layout);
    
    // Connect signals
    connect(panel, &TilesetCategoryPanel::brushSelected, this, &MainPalette::brushSelected);
    connect(sizePanel, &BrushSizePanel::brushSizeChanged, this, &MainPalette::brushSizeChanged);
    
    return container;
}

QWidget* MainPalette::createDoodadPalette() {
    TilesetCategoryPanel* panel = new TilesetCategoryPanel(TilesetCategoryType::Doodad, this);
    
    QVBoxLayout* layout = new QVBoxLayout();
    
    // Add brush size panel
    BrushSizePanel* sizePanel = new BrushSizePanel(panel);
    layout->addWidget(sizePanel);
    
    layout->addWidget(panel, 1);
    
    QWidget* container = new QWidget();
    container->setLayout(layout);
    
    connect(panel, &TilesetCategoryPanel::brushSelected, this, &MainPalette::brushSelected);
    connect(sizePanel, &BrushSizePanel::brushSizeChanged, this, &MainPalette::brushSizeChanged);
    
    return container;
}

QWidget* MainPalette::createItemPalette() {
    TilesetCategoryPanel* panel = new TilesetCategoryPanel(TilesetCategoryType::Item, this);
    
    QVBoxLayout* layout = new QVBoxLayout();
    
    // Add brush size panel
    BrushSizePanel* sizePanel = new BrushSizePanel(panel);
    layout->addWidget(sizePanel);
    
    layout->addWidget(panel, 1);
    
    QWidget* container = new QWidget();
    container->setLayout(layout);
    
    connect(panel, &TilesetCategoryPanel::brushSelected, this, &MainPalette::brushSelected);
    connect(sizePanel, &BrushSizePanel::brushSizeChanged, this, &MainPalette::brushSizeChanged);
    
    return container;
}

QWidget* MainPalette::createCollectionPalette() {
    TilesetCategoryPanel* panel = new TilesetCategoryPanel(TilesetCategoryType::Collection, this);
    
    QVBoxLayout* layout = new QVBoxLayout();
    
    // Add brush tool panel
    BrushToolPanel* toolPanel = new BrushToolPanel(panel);
    layout->addWidget(toolPanel);
    
    // Add brush size panel
    BrushSizePanel* sizePanel = new BrushSizePanel(panel);
    layout->addWidget(sizePanel);
    
    layout->addWidget(panel, 1);
    
    QWidget* container = new QWidget();
    container->setLayout(layout);
    
    connect(panel, &TilesetCategoryPanel::brushSelected, this, &MainPalette::brushSelected);
    connect(sizePanel, &BrushSizePanel::brushSizeChanged, this, &MainPalette::brushSizeChanged);
    
    return container;
}

QWidget* MainPalette::createCreaturePalette() {
    TilesetCategoryPanel* panel = new TilesetCategoryPanel(TilesetCategoryType::Creature, this);
    
    // Creature palette is simpler - just the panel
    connect(panel, &TilesetCategoryPanel::brushSelected, this, &MainPalette::brushSelected);
    
    return panel;
}

QWidget* MainPalette::createHousePalette() {
    // House palette would be a specialized panel
    // For now, create a placeholder
    QLabel* placeholder = new QLabel("House Palette\n(Specialized Implementation)", this);
    placeholder->setAlignment(Qt::AlignCenter);
    placeholder->setStyleSheet("color: gray; font-style: italic;");
    return placeholder;
}

QWidget* MainPalette::createWaypointPalette() {
    // Waypoint palette would be a specialized panel
    // For now, create a placeholder
    QLabel* placeholder = new QLabel("Waypoint Palette\n(Specialized Implementation)", this);
    placeholder->setAlignment(Qt::AlignCenter);
    placeholder->setStyleSheet("color: gray; font-style: italic;");
    return placeholder;
}

QWidget* MainPalette::createRawPalette() {
    TilesetCategoryPanel* panel = new TilesetCategoryPanel(TilesetCategoryType::Raw, this);
    
    QVBoxLayout* layout = new QVBoxLayout();
    
    // Add brush size panel
    BrushSizePanel* sizePanel = new BrushSizePanel(panel);
    layout->addWidget(sizePanel);
    
    layout->addWidget(panel, 1);
    
    QWidget* container = new QWidget();
    container->setLayout(layout);
    
    connect(panel, &TilesetCategoryPanel::brushSelected, this, &MainPalette::brushSelected);
    connect(sizePanel, &BrushSizePanel::brushSizeChanged, this, &MainPalette::brushSizeChanged);
    
    return container;
}

// Palette management
void MainPalette::initializePalettes() {
    if (!tilesetManager_) {
        qWarning() << "MainPalette: Cannot initialize palettes without TilesetManager";
        return;
    }
    
    // Populate each category panel
    for (int i = 0; i < tabWidget_->count(); ++i) {
        TilesetCategoryPanel* panel = qobject_cast<TilesetCategoryPanel*>(tabWidget_->widget(i));
        if (panel) {
            TilesetCategoryType categoryType = indexToTilesetType(i);
            QList<TilesetCategory*> categories = tilesetManager_->getCategoriesByType(categoryType);
            
            if (!categories.isEmpty()) {
                // Use the first category of this type
                panel->populateFromCategory(categories.first());
            }
        }
    }
    
    loadCurrentContents();
}

void MainPalette::reloadPalettes() {
    invalidateContents();
    initializePalettes();
}

void MainPalette::invalidateContents() {
    // Clear all category panels
    for (int i = 0; i < tabWidget_->count(); ++i) {
        TilesetCategoryPanel* panel = qobject_cast<TilesetCategoryPanel*>(tabWidget_->widget(i));
        if (panel) {
            panel->clearContents();
        }
    }
    
    needsRefresh_ = true;
}

void MainPalette::loadCurrentContents() {
    // Load current tab contents
    TilesetCategoryPanel* currentPanel = qobject_cast<TilesetCategoryPanel*>(tabWidget_->currentWidget());
    if (currentPanel) {
        currentPanel->refreshContents();
    }
    
    needsRefresh_ = false;
}

// Selection methods
Brush* MainPalette::getSelectedBrush() const {
    TilesetCategoryPanel* currentPanel = qobject_cast<TilesetCategoryPanel*>(tabWidget_->currentWidget());
    if (currentPanel) {
        return currentPanel->getSelectedBrush();
    }
    return selectedBrush_;
}

int MainPalette::getSelectedBrushSize() const {
    return selectedBrushSize_;
}

TilesetCategoryType MainPalette::getSelectedPaletteType() const {
    return indexToTilesetType(tabWidget_->currentIndex());
}

bool MainPalette::selectBrush(const Brush* brush, TilesetCategoryType preferredType) {
    if (!brush) {
        return false;
    }

    // Try preferred type first
    if (preferredType != TilesetCategoryType::Unknown) {
        int index = tilesetTypeToIndex(preferredType);
        if (index >= 0) {
            TilesetCategoryPanel* panel = qobject_cast<TilesetCategoryPanel*>(tabWidget_->widget(index));
            if (panel && panel->selectBrush(brush)) {
                tabWidget_->setCurrentIndex(index);
                return true;
            }
        }
    }

    // Search all panels
    for (int i = 0; i < tabWidget_->count(); ++i) {
        TilesetCategoryPanel* panel = qobject_cast<TilesetCategoryPanel*>(tabWidget_->widget(i));
        if (panel && panel->selectBrush(brush)) {
            tabWidget_->setCurrentIndex(i);
            return true;
        }
    }

    return false;
}

// Page selection
void MainPalette::selectPage(TilesetCategoryType type) {
    int index = tilesetTypeToIndex(type);
    if (index >= 0) {
        tabWidget_->setCurrentIndex(index);
    }
}

void MainPalette::selectPage(int index) {
    if (index >= 0 && index < tabWidget_->count()) {
        tabWidget_->setCurrentIndex(index);
    }
}

// Action ID support
void MainPalette::setActionIdEnabled(bool enabled) {
    actionIdEnabled_ = enabled;
    actionIdCheckBox_->setChecked(enabled);
    emit actionIdChanged(actionId_, actionIdEnabled_);
}

bool MainPalette::isActionIdEnabled() const {
    return actionIdEnabled_;
}

void MainPalette::setActionId(quint16 actionId) {
    actionId_ = actionId;
    actionIdSpinBox_->setValue(actionId);
    emit actionIdChanged(actionId_, actionIdEnabled_);
}

quint16 MainPalette::getActionId() const {
    return actionId_;
}

// Slots
void MainPalette::onMapChanged(Map* map) {
    setMap(map);
}

void MainPalette::onTilesetChanged() {
    if (!refreshTimer_->isActive()) {
        refreshTimer_->start();
    }
}

void MainPalette::onBrushSizeUpdate(int size) {
    selectedBrushSize_ = size;
    emit brushSizeChanged(size);
}

void MainPalette::onSettingsChanged() {
    // Reload display settings
    reloadPalettes();
}

void MainPalette::onTabChanged(int index) {
    currentPaletteType_ = indexToTilesetType(index);

    // Load contents for new tab
    TilesetCategoryPanel* panel = qobject_cast<TilesetCategoryPanel*>(tabWidget_->widget(index));
    if (panel) {
        panel->refreshContents();

        // Select first brush if none selected
        if (!panel->getSelectedBrush()) {
            panel->selectFirstBrush();
        }
    }

    emit paletteChanged(currentPaletteType_);
}

void MainPalette::onActionIdToggled(bool enabled) {
    actionIdEnabled_ = enabled;
    emit actionIdChanged(actionId_, actionIdEnabled_);
}

void MainPalette::onActionIdValueChanged(int value) {
    actionId_ = static_cast<quint16>(value);
    emit actionIdChanged(actionId_, actionIdEnabled_);
}

void MainPalette::onRefreshTimer() {
    if (needsRefresh_) {
        loadCurrentContents();
    }
}

// Helper methods
TilesetCategoryType MainPalette::indexToTilesetType(int index) const {
    switch (index) {
        case 0: return TilesetCategoryType::Terrain;
        case 1: return TilesetCategoryType::Doodad;
        case 2: return TilesetCategoryType::Collection;
        case 3: return TilesetCategoryType::Item;
        case 4: return TilesetCategoryType::House;
        case 5: return TilesetCategoryType::Waypoint;
        case 6: return TilesetCategoryType::Creature;
        case 7: return TilesetCategoryType::Raw;
        default: return TilesetCategoryType::Unknown;
    }
}

int MainPalette::tilesetTypeToIndex(TilesetCategoryType type) const {
    switch (type) {
        case TilesetCategoryType::Terrain: return 0;
        case TilesetCategoryType::Doodad: return 1;
        case TilesetCategoryType::Collection: return 2;
        case TilesetCategoryType::Item: return 3;
        case TilesetCategoryType::House: return 4;
        case TilesetCategoryType::Waypoint: return 5;
        case TilesetCategoryType::Creature: return 6;
        case TilesetCategoryType::Raw: return 7;
        default: return -1;
    }
}

// TilesetCategoryPanel implementation
TilesetCategoryPanel::TilesetCategoryPanel(TilesetCategoryType categoryType, QWidget* parent)
    : QWidget(parent)
    , categoryType_(categoryType)
    , tilesetManager_(nullptr)
    , brushManager_(nullptr)
    , itemManager_(nullptr)
    , currentCategory_(nullptr)
    , displayMode_(1) // Grid mode by default
    , showItemIds_(false)
    , iconSize_(32)
    , selectedBrush_(nullptr)
    , selectedItem_(nullptr) {

    setupUI();
    connectSignals();
}

TilesetCategoryPanel::~TilesetCategoryPanel() {
    // Qt handles cleanup automatically
}

void TilesetCategoryPanel::setupUI() {
    mainLayout_ = new QVBoxLayout(this);
    mainLayout_->setContentsMargins(2, 2, 2, 2);
    mainLayout_->setSpacing(2);

    setupToolbar();
    setupItemList();
}

void TilesetCategoryPanel::setupToolbar() {
    toolbar_ = new QToolBar(this);
    toolbar_->setToolButtonStyle(Qt::ToolButtonIconOnly);
    toolbar_->setIconSize(QSize(16, 16));

    // Display mode actions
    displayModeGroup_ = new QActionGroup(this);

    listModeAction_ = new QAction(QIcon(":/icons/list_mode.png"), "List Mode", this);
    listModeAction_->setCheckable(true);
    listModeAction_->setData(0);
    displayModeGroup_->addAction(listModeAction_);
    toolbar_->addAction(listModeAction_);

    gridModeAction_ = new QAction(QIcon(":/icons/grid_mode.png"), "Grid Mode", this);
    gridModeAction_->setCheckable(true);
    gridModeAction_->setChecked(true);
    gridModeAction_->setData(1);
    displayModeGroup_->addAction(gridModeAction_);
    toolbar_->addAction(gridModeAction_);

    largeModeAction_ = new QAction(QIcon(":/icons/large_mode.png"), "Large Icons", this);
    largeModeAction_->setCheckable(true);
    largeModeAction_->setData(2);
    displayModeGroup_->addAction(largeModeAction_);
    toolbar_->addAction(largeModeAction_);

    toolbar_->addSeparator();

    // Show IDs action
    showIdsAction_ = new QAction(QIcon(":/icons/show_ids.png"), "Show Item IDs", this);
    showIdsAction_->setCheckable(true);
    showIdsAction_->setChecked(showItemIds_);
    toolbar_->addAction(showIdsAction_);

    toolbar_->addSeparator();

    // Icon size combo
    QLabel* sizeLabel = new QLabel("Size:", this);
    toolbar_->addWidget(sizeLabel);

    iconSizeCombo_ = new QComboBox(this);
    iconSizeCombo_->addItem("16", 16);
    iconSizeCombo_->addItem("24", 24);
    iconSizeCombo_->addItem("32", 32);
    iconSizeCombo_->addItem("48", 48);
    iconSizeCombo_->addItem("64", 64);
    iconSizeCombo_->setCurrentText("32");
    toolbar_->addWidget(iconSizeCombo_);

    mainLayout_->addWidget(toolbar_);
}

void TilesetCategoryPanel::setupItemList() {
    scrollArea_ = new QScrollArea(this);
    scrollArea_->setWidgetResizable(true);
    scrollArea_->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollArea_->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    itemList_ = new QListWidget();
    itemList_->setSelectionMode(QAbstractItemView::SingleSelection);
    itemList_->setResizeMode(QListView::Adjust);
    itemList_->setViewMode(QListView::IconMode);
    itemList_->setMovement(QListView::Static);
    itemList_->setFlow(QListView::LeftToRight);
    itemList_->setWrapping(true);
    itemList_->setSpacing(2);
    itemList_->setUniformItemSizes(true);

    scrollArea_->setWidget(itemList_);
    mainLayout_->addWidget(scrollArea_, 1);
}

void TilesetCategoryPanel::connectSignals() {
    // Display mode signals
    connect(displayModeGroup_, &QActionGroup::triggered, this, &TilesetCategoryPanel::onDisplayModeChanged);

    // Show IDs signal
    connect(showIdsAction_, &QAction::toggled, this, [this](bool checked) {
        setShowItemIds(checked);
    });

    // Icon size signal
    connect(iconSizeCombo_, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int index) {
        int size = iconSizeCombo_->itemData(index).toInt();
        setIconSize(size);
    });

    // Item list signals
    connect(itemList_, &QListWidget::itemClicked, this, &TilesetCategoryPanel::onItemClicked);
    connect(itemList_, &QListWidget::itemDoubleClicked, this, &TilesetCategoryPanel::onItemDoubleClicked);
    connect(itemList_, &QListWidget::itemSelectionChanged, this, &TilesetCategoryPanel::selectionChanged);
}

// Setup and population
void TilesetCategoryPanel::setTilesetManager(TilesetManager* tilesetManager) {
    tilesetManager_ = tilesetManager;
}

void TilesetCategoryPanel::setBrushManager(BrushManager* brushManager) {
    brushManager_ = brushManager;
}

void TilesetCategoryPanel::setItemManager(ItemManager* itemManager) {
    itemManager_ = itemManager;
}

void TilesetCategoryPanel::populateFromCategory(TilesetCategory* category) {
    currentCategory_ = category;
    clearContents();

    if (!category) {
        return;
    }

    // Add brushes
    for (Brush* brush : category->getBrushes()) {
        if (brush) {
            addBrushToList(brush);
        }
    }

    // Add items
    for (quint16 itemId : category->getItemIds()) {
        addItemToList(itemId);
    }

    // Add creatures
    for (const QString& creatureName : category->getCreatureNames()) {
        addCreatureToList(creatureName);
    }

    // Select first item if available
    if (itemList_->count() > 0) {
        itemList_->setCurrentRow(0);
    }
}

void TilesetCategoryPanel::clearContents() {
    itemList_->clear();
    selectedBrush_ = nullptr;
    selectedItem_ = nullptr;
}

void TilesetCategoryPanel::refreshContents() {
    if (currentCategory_) {
        populateFromCategory(currentCategory_);
    }
}

// Selection methods
Brush* TilesetCategoryPanel::getSelectedBrush() const {
    return selectedBrush_;
}

bool TilesetCategoryPanel::selectBrush(const Brush* brush) {
    if (!brush) {
        return false;
    }

    // Search for the brush in the list
    for (int i = 0; i < itemList_->count(); ++i) {
        QListWidgetItem* item = itemList_->item(i);
        if (item) {
            Brush* itemBrush = item->data(Qt::UserRole).value<Brush*>();
            if (itemBrush == brush) {
                itemList_->setCurrentItem(item);
                selectedBrush_ = itemBrush;
                selectedItem_ = item;
                return true;
            }
        }
    }

    return false;
}

void TilesetCategoryPanel::selectFirstBrush() {
    if (itemList_->count() > 0) {
        itemList_->setCurrentRow(0);
        onItemClicked(itemList_->item(0));
    }
}

// Display options
void TilesetCategoryPanel::setDisplayMode(int mode) {
    displayMode_ = mode;

    switch (mode) {
        case 0: // List mode
            itemList_->setViewMode(QListView::ListMode);
            itemList_->setFlow(QListView::TopToBottom);
            break;
        case 1: // Grid mode
            itemList_->setViewMode(QListView::IconMode);
            itemList_->setFlow(QListView::LeftToRight);
            break;
        case 2: // Large icons
            itemList_->setViewMode(QListView::IconMode);
            itemList_->setFlow(QListView::LeftToRight);
            setIconSize(64);
            break;
    }

    refreshContents();
}

void TilesetCategoryPanel::setShowItemIds(bool show) {
    showItemIds_ = show;
    showIdsAction_->setChecked(show);
    refreshContents();
}

void TilesetCategoryPanel::setIconSize(int size) {
    iconSize_ = size;
    itemList_->setIconSize(QSize(size, size));
    itemList_->setGridSize(QSize(size + 8, size + 8));

    // Update combo box
    for (int i = 0; i < iconSizeCombo_->count(); ++i) {
        if (iconSizeCombo_->itemData(i).toInt() == size) {
            iconSizeCombo_->setCurrentIndex(i);
            break;
        }
    }

    refreshContents();
}

// Slots
void TilesetCategoryPanel::onItemClicked(QListWidgetItem* item) {
    if (!item) {
        return;
    }

    selectedItem_ = item;

    // Get brush from item data
    Brush* brush = item->data(Qt::UserRole).value<Brush*>();
    if (brush) {
        selectedBrush_ = brush;
        emit brushSelected(brush);
    }
}

void TilesetCategoryPanel::onItemDoubleClicked(QListWidgetItem* item) {
    onItemClicked(item);

    // Double-click could trigger additional actions
    if (selectedBrush_) {
        // For example, open brush properties dialog
        qDebug() << "Double-clicked brush:" << selectedBrush_->getName();
    }
}

void TilesetCategoryPanel::onDisplayModeChanged() {
    QAction* action = displayModeGroup_->checkedAction();
    if (action) {
        int mode = action->data().toInt();
        setDisplayMode(mode);
    }
}

// Item management
void TilesetCategoryPanel::addBrushToList(Brush* brush) {
    if (!brush) {
        return;
    }

    QListWidgetItem* item = createBrushItem(brush);
    if (item) {
        itemList_->addItem(item);
    }
}

void TilesetCategoryPanel::addItemToList(quint16 itemId) {
    QListWidgetItem* item = createItemItem(itemId);
    if (item) {
        itemList_->addItem(item);
    }
}

void TilesetCategoryPanel::addCreatureToList(const QString& creatureName) {
    QListWidgetItem* item = createCreatureItem(creatureName);
    if (item) {
        itemList_->addItem(item);
    }
}

QListWidgetItem* TilesetCategoryPanel::createBrushItem(Brush* brush) {
    if (!brush) {
        return nullptr;
    }

    QListWidgetItem* item = new QListWidgetItem();

    // Set icon
    QPixmap icon = getBrushIcon(brush, iconSize_);
    item->setIcon(QIcon(icon));

    // Set text
    QString text = brush->getName();
    if (showItemIds_ && brush->isRaw()) {
        // For RAW brushes, show item ID
        text += QString(" [%1]").arg(brush->getLookID());
    }
    item->setText(text);

    // Set tooltip
    item->setToolTip(getBrushTooltip(brush));

    // Store brush pointer
    item->setData(Qt::UserRole, QVariant::fromValue(brush));

    return item;
}

QListWidgetItem* TilesetCategoryPanel::createItemItem(quint16 itemId) {
    QListWidgetItem* item = new QListWidgetItem();

    // Set icon
    QPixmap icon = getItemIcon(itemId, iconSize_);
    item->setIcon(QIcon(icon));

    // Set text
    QString text = QString("Item %1").arg(itemId);
    if (itemManager_) {
        const ItemProperties& props = itemManager_->getItemProperties(itemId);
        if (!props.name.isEmpty()) {
            text = props.name;
        }
    }

    if (showItemIds_) {
        text += QString(" [%1]").arg(itemId);
    }
    item->setText(text);

    // Set tooltip
    item->setToolTip(getItemTooltip(itemId));

    // Store item ID
    item->setData(Qt::UserRole + 1, itemId);

    return item;
}

QListWidgetItem* TilesetCategoryPanel::createCreatureItem(const QString& creatureName) {
    QListWidgetItem* item = new QListWidgetItem();

    // Set icon
    QPixmap icon = getCreatureIcon(creatureName, iconSize_);
    item->setIcon(QIcon(icon));

    // Set text
    item->setText(creatureName);

    // Set tooltip
    item->setToolTip(getCreatureTooltip(creatureName));

    // Store creature name
    item->setData(Qt::UserRole + 2, creatureName);

    return item;
}

// Helper methods
QPixmap TilesetCategoryPanel::getBrushIcon(Brush* brush, int size) {
    if (!brush) {
        return QPixmap(size, size);
    }

    // This would integrate with ResourceManager to get brush icons
    // For now, create a placeholder
    QPixmap pixmap(size, size);
    pixmap.fill(Qt::lightGray);

    // Draw brush type indicator
    QPainter painter(&pixmap);
    painter.setPen(Qt::black);
    painter.drawRect(0, 0, size - 1, size - 1);

    // Draw brush name initial
    if (!brush->getName().isEmpty()) {
        painter.drawText(pixmap.rect(), Qt::AlignCenter, brush->getName().left(1).toUpper());
    }

    return pixmap;
}

QPixmap TilesetCategoryPanel::getItemIcon(quint16 itemId, int size) {
    // This would integrate with ResourceManager to get item sprites
    // For now, create a placeholder
    QPixmap pixmap(size, size);
    pixmap.fill(Qt::white);

    QPainter painter(&pixmap);
    painter.setPen(Qt::black);
    painter.drawRect(0, 0, size - 1, size - 1);

    // Draw item ID
    painter.drawText(pixmap.rect(), Qt::AlignCenter, QString::number(itemId));

    return pixmap;
}

QPixmap TilesetCategoryPanel::getCreatureIcon(const QString& creatureName, int size) {
    // This would integrate with ResourceManager to get creature sprites
    // For now, create a placeholder
    QPixmap pixmap(size, size);
    pixmap.fill(Qt::yellow);

    QPainter painter(&pixmap);
    painter.setPen(Qt::black);
    painter.drawRect(0, 0, size - 1, size - 1);

    // Draw creature name initial
    if (!creatureName.isEmpty()) {
        painter.drawText(pixmap.rect(), Qt::AlignCenter, creatureName.left(1).toUpper());
    }

    return pixmap;
}

QString TilesetCategoryPanel::getBrushTooltip(Brush* brush) {
    if (!brush) {
        return QString();
    }

    QString tooltip = QString("Brush: %1").arg(brush->getName());

    if (brush->isRaw()) {
        tooltip += QString("\nItem ID: %1").arg(brush->getLookID());
    }

    // Add brush type information
    switch (brush->getType()) {
        case Brush::Type::Ground:
            tooltip += "\nType: Ground";
            break;
        case Brush::Type::Wall:
            tooltip += "\nType: Wall";
            break;
        case Brush::Type::Door:
            tooltip += "\nType: Door";
            break;
        case Brush::Type::Doodad:
            tooltip += "\nType: Doodad";
            break;
        default:
            tooltip += "\nType: Unknown";
            break;
    }

    return tooltip;
}

QString TilesetCategoryPanel::getItemTooltip(quint16 itemId) {
    QString tooltip = QString("Item ID: %1").arg(itemId);

    if (itemManager_) {
        const ItemProperties& props = itemManager_->getItemProperties(itemId);
        if (!props.name.isEmpty()) {
            tooltip = QString("%1\nID: %2").arg(props.name).arg(itemId);
        }

        // Add item properties
        QStringList properties;
        if (props.moveable) properties << "Moveable";
        if (props.stackable) properties << "Stackable";
        if (props.rotatable) properties << "Rotatable";
        if (props.readable) properties << "Readable";
        if (props.writeable) properties << "Writeable";

        if (!properties.isEmpty()) {
            tooltip += "\nProperties: " + properties.join(", ");
        }
    }

    return tooltip;
}

QString TilesetCategoryPanel::getCreatureTooltip(const QString& creatureName) {
    QString tooltip = QString("Creature: %1").arg(creatureName);

    // This would integrate with creature database for additional info
    // For now, just return the name

    return tooltip;
}

// BrushSizePanel implementation
BrushSizePanel::BrushSizePanel(QWidget* parent)
    : QWidget(parent)
    , brushSize_(1)
    , brushShape_(0) // Square
    , largeIcons_(false) {

    setupUI();
    connectSignals();
}

BrushSizePanel::~BrushSizePanel() {
    // Qt handles cleanup automatically
}

void BrushSizePanel::setupUI() {
    layout_ = new QHBoxLayout(this);
    layout_->setContentsMargins(2, 2, 2, 2);
    layout_->setSpacing(4);

    // Size controls
    sizeLabel_ = new QLabel("Size:", this);
    layout_->addWidget(sizeLabel_);

    sizeSpinBox_ = new QSpinBox(this);
    sizeSpinBox_->setRange(1, 20);
    sizeSpinBox_->setValue(brushSize_);
    sizeSpinBox_->setFixedWidth(60);
    layout_->addWidget(sizeSpinBox_);

    layout_->addSpacing(10);

    // Shape controls
    shapeGroup_ = new QButtonGroup(this);

    squareButton_ = new QPushButton("□", this);
    squareButton_->setCheckable(true);
    squareButton_->setChecked(true);
    squareButton_->setFixedSize(24, 24);
    squareButton_->setToolTip("Square brush");
    shapeGroup_->addButton(squareButton_, 0);
    layout_->addWidget(squareButton_);

    circleButton_ = new QPushButton("○", this);
    circleButton_->setCheckable(true);
    circleButton_->setFixedSize(24, 24);
    circleButton_->setToolTip("Circle brush");
    shapeGroup_->addButton(circleButton_, 1);
    layout_->addWidget(circleButton_);

    customButton_ = new QPushButton("◊", this);
    customButton_->setCheckable(true);
    customButton_->setFixedSize(24, 24);
    customButton_->setToolTip("Custom brush");
    shapeGroup_->addButton(customButton_, 2);
    layout_->addWidget(customButton_);

    layout_->addStretch();
}

void BrushSizePanel::connectSignals() {
    connect(sizeSpinBox_, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &BrushSizePanel::onSizeChanged);
    connect(shapeGroup_, QOverload<int>::of(&QButtonGroup::buttonClicked),
            this, &BrushSizePanel::onShapeChanged);
}

void BrushSizePanel::setBrushSize(int size) {
    brushSize_ = size;
    sizeSpinBox_->setValue(size);
    updateSizeDisplay();
}

int BrushSizePanel::getBrushSize() const {
    return brushSize_;
}

void BrushSizePanel::setBrushShape(int shape) {
    brushShape_ = shape;
    QAbstractButton* button = shapeGroup_->button(shape);
    if (button) {
        button->setChecked(true);
    }
}

int BrushSizePanel::getBrushShape() const {
    return brushShape_;
}

void BrushSizePanel::setToolbarIconSize(bool large) {
    largeIcons_ = large;
    int size = large ? 32 : 24;

    squareButton_->setFixedSize(size, size);
    circleButton_->setFixedSize(size, size);
    customButton_->setFixedSize(size, size);
}

void BrushSizePanel::onSizeChanged(int size) {
    brushSize_ = size;
    updateSizeDisplay();
    emit brushSizeChanged(size);
}

void BrushSizePanel::onShapeChanged() {
    brushShape_ = shapeGroup_->checkedId();
    emit brushShapeChanged(brushShape_);
}

void BrushSizePanel::updateSizeDisplay() {
    // Update size label or other visual indicators
    sizeLabel_->setText(QString("Size: %1x%1").arg(brushSize_));
}

// BrushToolPanel implementation
BrushToolPanel::BrushToolPanel(QWidget* parent)
    : QWidget(parent)
    , selectedTool_(0) // Paint tool
    , largeIcons_(false) {

    setupUI();
    connectSignals();
}

BrushToolPanel::~BrushToolPanel() {
    // Qt handles cleanup automatically
}

void BrushToolPanel::setupUI() {
    layout_ = new QHBoxLayout(this);
    layout_->setContentsMargins(2, 2, 2, 2);
    layout_->setSpacing(4);

    // Tool selection buttons
    toolGroup_ = new QButtonGroup(this);

    paintButton_ = new QPushButton("🖌", this);
    paintButton_->setCheckable(true);
    paintButton_->setChecked(true);
    paintButton_->setFixedSize(24, 24);
    paintButton_->setToolTip("Paint tool");
    toolGroup_->addButton(paintButton_, 0);
    layout_->addWidget(paintButton_);

    fillButton_ = new QPushButton("🪣", this);
    fillButton_->setCheckable(true);
    fillButton_->setFixedSize(24, 24);
    fillButton_->setToolTip("Fill tool");
    toolGroup_->addButton(fillButton_, 1);
    layout_->addWidget(fillButton_);

    replaceButton_ = new QPushButton("🔄", this);
    replaceButton_->setCheckable(true);
    replaceButton_->setFixedSize(24, 24);
    replaceButton_->setToolTip("Replace tool");
    toolGroup_->addButton(replaceButton_, 2);
    layout_->addWidget(replaceButton_);

    selectButton_ = new QPushButton("📋", this);
    selectButton_->setCheckable(true);
    selectButton_->setFixedSize(24, 24);
    selectButton_->setToolTip("Select tool");
    toolGroup_->addButton(selectButton_, 3);
    layout_->addWidget(selectButton_);

    layout_->addStretch();
}

void BrushToolPanel::connectSignals() {
    connect(toolGroup_, QOverload<int>::of(&QButtonGroup::buttonClicked),
            this, &BrushToolPanel::onToolChanged);
}

void BrushToolPanel::setSelectedTool(int tool) {
    selectedTool_ = tool;
    QAbstractButton* button = toolGroup_->button(tool);
    if (button) {
        button->setChecked(true);
    }
}

int BrushToolPanel::getSelectedTool() const {
    return selectedTool_;
}

void BrushToolPanel::setToolbarIconSize(bool large) {
    largeIcons_ = large;
    int size = large ? 32 : 24;

    paintButton_->setFixedSize(size, size);
    fillButton_->setFixedSize(size, size);
    replaceButton_->setFixedSize(size, size);
    selectButton_->setFixedSize(size, size);
}

void BrushToolPanel::onToolChanged() {
    selectedTool_ = toolGroup_->checkedId();
    emit toolSelected(selectedTool_);
}

#include "MainPalette.moc"
