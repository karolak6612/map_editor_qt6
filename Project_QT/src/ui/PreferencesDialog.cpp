#include "PreferencesDialog.h"
#include "SettingsManager.h"
#include "MainWindow.h"
#include <QApplication>
#include <QMessageBox>
#include <QFileDialog>
#include <QStandardPaths>
#include <QDir>
#include <QDebug>

PreferencesDialog::PreferencesDialog(QWidget* parent)
    : QDialog(parent)
    , tabWidget_(nullptr)
    , mainLayout_(nullptr)
    , buttonLayout_(nullptr)
    , settingsManager_(SettingsManager::getInstance())
    , mainWindow_(qobject_cast<MainWindow*>(parent))
    , modified_(false)
    , loading_(false)
{
    setWindowTitle(tr("Preferences"));
    setWindowIcon(QIcon(":/icons/preferences.png"));
    resize(800, 600);
    setModal(true);
    
    setupUI();
    connectSignals();
    loadSettings();
}

PreferencesDialog::~PreferencesDialog() = default;

void PreferencesDialog::setupUI() {
    mainLayout_ = new QVBoxLayout(this);
    mainLayout_->setContentsMargins(10, 10, 10, 10);
    mainLayout_->setSpacing(10);
    
    setupTabWidget();
    setupButtonBox();
    
    mainLayout_->addWidget(tabWidget_);
    mainLayout_->addLayout(buttonLayout_);
}

void PreferencesDialog::setupTabWidget() {
    tabWidget_ = new QTabWidget(this);
    tabWidget_->setTabPosition(QTabWidget::North);
    tabWidget_->setMovable(false);
    tabWidget_->setTabsClosable(false);
    
    // Add all tabs
    tabWidget_->addTab(createGeneralTab(), tr("General"));
    tabWidget_->addTab(createEditorTab(), tr("Editor"));
    tabWidget_->addTab(createGraphicsTab(), tr("Graphics"));
    tabWidget_->addTab(createUITab(), tr("Interface"));
    tabWidget_->addTab(createClientVersionTab(), tr("Client Version"));
    tabWidget_->addTab(createLODTab(), tr("LOD"));
    tabWidget_->addTab(createAutomagicTab(), tr("Automagic"));
}

void PreferencesDialog::setupButtonBox() {
    buttonLayout_ = new QHBoxLayout();
    buttonLayout_->setSpacing(10);
    
    defaultsButton_ = new QPushButton(tr("&Defaults"), this);
    defaultsButton_->setToolTip(tr("Reset all settings to default values"));
    
    buttonLayout_->addWidget(defaultsButton_);
    buttonLayout_->addStretch();
    
    applyButton_ = new QPushButton(tr("&Apply"), this);
    applyButton_->setToolTip(tr("Apply changes without closing the dialog"));
    applyButton_->setEnabled(false);
    
    okButton_ = new QPushButton(tr("&OK"), this);
    okButton_->setToolTip(tr("Apply changes and close the dialog"));
    okButton_->setDefault(true);
    
    cancelButton_ = new QPushButton(tr("&Cancel"), this);
    cancelButton_->setToolTip(tr("Discard changes and close the dialog"));
    
    buttonLayout_->addWidget(applyButton_);
    buttonLayout_->addWidget(okButton_);
    buttonLayout_->addWidget(cancelButton_);
}

QWidget* PreferencesDialog::createGeneralTab() {
    QWidget* generalTab = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(generalTab);
    layout->setSpacing(15);
    
    // Startup group
    QGroupBox* startupGroup = new QGroupBox(tr("Startup"), generalTab);
    QVBoxLayout* startupLayout = new QVBoxLayout(startupGroup);
    
    welcomeDialogCheckBox_ = new QCheckBox(tr("Show welcome dialog on startup"), startupGroup);
    welcomeDialogCheckBox_->setToolTip(tr("Show welcome dialog when starting the editor"));
    startupLayout->addWidget(welcomeDialogCheckBox_);
    
    updateCheckCheckBox_ = new QCheckBox(tr("Check for updates on startup"), startupGroup);
    updateCheckCheckBox_->setToolTip(tr("Automatically check for application updates"));
    startupLayout->addWidget(updateCheckCheckBox_);
    
    singleInstanceCheckBox_ = new QCheckBox(tr("Only allow one instance"), startupGroup);
    singleInstanceCheckBox_->setToolTip(tr("Prevent multiple instances of the editor"));
    startupLayout->addWidget(singleInstanceCheckBox_);
    
    layout->addWidget(startupGroup);
    
    // File operations group
    QGroupBox* fileGroup = new QGroupBox(tr("File Operations"), generalTab);
    QVBoxLayout* fileLayout = new QVBoxLayout(fileGroup);
    
    alwaysBackupCheckBox_ = new QCheckBox(tr("Always make backup when saving"), fileGroup);
    alwaysBackupCheckBox_->setToolTip(tr("Create backup files before saving"));
    fileLayout->addWidget(alwaysBackupCheckBox_);
    
    // Auto-save settings
    QHBoxLayout* autoSaveLayout = new QHBoxLayout();
    autoSaveCheckBox_ = new QCheckBox(tr("Enable auto-save"), fileGroup);
    autoSaveCheckBox_->setToolTip(tr("Automatically save maps at regular intervals"));
    autoSaveLayout->addWidget(autoSaveCheckBox_);
    
    autoSaveLayout->addWidget(new QLabel(tr("Interval:"), fileGroup));
    autoSaveIntervalSpinBox_ = new QSpinBox(fileGroup);
    autoSaveIntervalSpinBox_->setRange(1, 60);
    autoSaveIntervalSpinBox_->setSuffix(tr(" minutes"));
    autoSaveIntervalSpinBox_->setToolTip(tr("Auto-save interval in minutes"));
    autoSaveLayout->addWidget(autoSaveIntervalSpinBox_);
    autoSaveLayout->addStretch();
    
    fileLayout->addLayout(autoSaveLayout);
    layout->addWidget(fileGroup);
    
    // Performance group
    QGroupBox* performanceGroup = new QGroupBox(tr("Performance"), generalTab);
    QFormLayout* performanceLayout = new QFormLayout(performanceGroup);
    
    undoSizeSpinBox_ = new QSpinBox(performanceGroup);
    undoSizeSpinBox_->setRange(10, 1000);
    undoSizeSpinBox_->setToolTip(tr("Maximum number of undo operations"));
    performanceLayout->addRow(tr("Undo size:"), undoSizeSpinBox_);
    
    undoMemSizeSpinBox_ = new QSpinBox(performanceGroup);
    undoMemSizeSpinBox_->setRange(10, 1000);
    undoMemSizeSpinBox_->setSuffix(tr(" MB"));
    undoMemSizeSpinBox_->setToolTip(tr("Maximum memory for undo operations"));
    performanceLayout->addRow(tr("Undo memory:"), undoMemSizeSpinBox_);
    
    workerThreadsSpinBox_ = new QSpinBox(performanceGroup);
    workerThreadsSpinBox_->setRange(1, QThread::idealThreadCount());
    workerThreadsSpinBox_->setToolTip(tr("Number of worker threads for background operations"));
    performanceLayout->addRow(tr("Worker threads:"), workerThreadsSpinBox_);
    
    replaceSizeSpinBox_ = new QSpinBox(performanceGroup);
    replaceSizeSpinBox_->setRange(100, 10000);
    replaceSizeSpinBox_->setToolTip(tr("Maximum items to process in replace operations"));
    performanceLayout->addRow(tr("Replace size:"), replaceSizeSpinBox_);
    
    layout->addWidget(performanceGroup);
    
    // Display group
    QGroupBox* displayGroup = new QGroupBox(tr("Display"), generalTab);
    QFormLayout* displayLayout = new QFormLayout(displayGroup);
    
    positionFormatComboBox_ = new QComboBox(displayGroup);
    positionFormatComboBox_->addItems({tr("Decimal"), tr("Hexadecimal"), tr("Tibia Format")});
    positionFormatComboBox_->setToolTip(tr("Format for displaying coordinates"));
    displayLayout->addRow(tr("Position format:"), positionFormatComboBox_);
    
    tilesetEditingCheckBox_ = new QCheckBox(tr("Enable tileset editing"), displayGroup);
    tilesetEditingCheckBox_->setToolTip(tr("Show tileset editing features in palettes"));
    displayLayout->addRow(tilesetEditingCheckBox_);
    
    layout->addWidget(displayGroup);
    
    layout->addStretch();
    return generalTab;
}

QWidget* PreferencesDialog::createEditorTab() {
    QWidget* editorTab = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(editorTab);
    layout->setSpacing(15);
    
    // Actions group
    QGroupBox* actionsGroup = new QGroupBox(tr("Actions"), editorTab);
    QVBoxLayout* actionsLayout = new QVBoxLayout(actionsGroup);
    
    groupActionsCheckBox_ = new QCheckBox(tr("Group same-type actions"), actionsGroup);
    groupActionsCheckBox_->setToolTip(tr("Group consecutive actions of the same type for undo"));
    actionsLayout->addWidget(groupActionsCheckBox_);
    
    duplicateActionsCheckBox_ = new QCheckBox(tr("Warn for duplicate actions"), actionsGroup);
    duplicateActionsCheckBox_->setToolTip(tr("Show warning when performing duplicate actions"));
    actionsLayout->addWidget(duplicateActionsCheckBox_);
    
    warnForDestructiveCheckBox_ = new QCheckBox(tr("Warn for destructive actions"), actionsGroup);
    warnForDestructiveCheckBox_->setToolTip(tr("Show confirmation for destructive operations"));
    actionsLayout->addWidget(warnForDestructiveCheckBox_);
    
    layout->addWidget(actionsGroup);
    
    // Drawing group
    QGroupBox* drawingGroup = new QGroupBox(tr("Drawing"), editorTab);
    QVBoxLayout* drawingLayout = new QVBoxLayout(drawingGroup);
    
    autoSelectRawCheckBox_ = new QCheckBox(tr("Auto-select RAW brush"), drawingGroup);
    autoSelectRawCheckBox_->setToolTip(tr("Automatically select RAW brush when needed"));
    drawingLayout->addWidget(autoSelectRawCheckBox_);
    
    houseEditWarningCheckBox_ = new QCheckBox(tr("Warn when editing houses"), drawingGroup);
    houseEditWarningCheckBox_->setToolTip(tr("Show warning when editing house tiles"));
    drawingLayout->addWidget(houseEditWarningCheckBox_);
    
    layout->addWidget(drawingGroup);
    
    // Mouse group
    QGroupBox* mouseGroup = new QGroupBox(tr("Mouse"), editorTab);
    QVBoxLayout* mouseLayout = new QVBoxLayout(mouseGroup);
    
    swapMouseButtonsCheckBox_ = new QCheckBox(tr("Swap mouse buttons"), mouseGroup);
    swapMouseButtonsCheckBox_->setToolTip(tr("Swap left and right mouse button functions"));
    mouseLayout->addWidget(swapMouseButtonsCheckBox_);
    
    doubleClickPropertiesCheckBox_ = new QCheckBox(tr("Double-click opens properties"), mouseGroup);
    doubleClickPropertiesCheckBox_->setToolTip(tr("Double-click on items to open properties"));
    mouseLayout->addWidget(doubleClickPropertiesCheckBox_);
    
    invertWheelCheckBox_ = new QCheckBox(tr("Invert mouse wheel"), mouseGroup);
    invertWheelCheckBox_->setToolTip(tr("Invert mouse wheel zoom direction"));
    mouseLayout->addWidget(invertWheelCheckBox_);
    
    // Scroll speed
    QHBoxLayout* scrollLayout = new QHBoxLayout();
    scrollLayout->addWidget(new QLabel(tr("Scroll speed:"), mouseGroup));
    scrollSpeedSlider_ = new QSlider(Qt::Horizontal, mouseGroup);
    scrollSpeedSlider_->setRange(1, 10);
    scrollSpeedSlider_->setToolTip(tr("Mouse scroll speed"));
    scrollLayout->addWidget(scrollSpeedSlider_);
    scrollSpeedLabel_ = new QLabel("5", mouseGroup);
    scrollLayout->addWidget(scrollSpeedLabel_);
    mouseLayout->addLayout(scrollLayout);
    
    // Zoom speed
    QHBoxLayout* zoomLayout = new QHBoxLayout();
    zoomLayout->addWidget(new QLabel(tr("Zoom speed:"), mouseGroup));
    zoomSpeedSlider_ = new QSlider(Qt::Horizontal, mouseGroup);
    zoomSpeedSlider_->setRange(1, 10);
    zoomSpeedSlider_->setToolTip(tr("Mouse zoom speed"));
    zoomLayout->addWidget(zoomSpeedSlider_);
    zoomSpeedLabel_ = new QLabel("5", mouseGroup);
    zoomLayout->addWidget(zoomSpeedLabel_);
    mouseLayout->addLayout(zoomLayout);
    
    layout->addWidget(mouseGroup);
    
    layout->addStretch();
    return editorTab;
}

QWidget* PreferencesDialog::createGraphicsTab() {
    QWidget* graphicsTab = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(graphicsTab);
    layout->setSpacing(15);

    // Rendering group
    QGroupBox* renderingGroup = new QGroupBox(tr("Rendering"), graphicsTab);
    QVBoxLayout* renderingLayout = new QVBoxLayout(renderingGroup);

    hardwareAccelerationCheckBox_ = new QCheckBox(tr("Hardware acceleration"), renderingGroup);
    hardwareAccelerationCheckBox_->setToolTip(tr("Use hardware acceleration for rendering"));
    renderingLayout->addWidget(hardwareAccelerationCheckBox_);

    vSyncCheckBox_ = new QCheckBox(tr("Vertical sync"), renderingGroup);
    vSyncCheckBox_->setToolTip(tr("Enable vertical synchronization"));
    renderingLayout->addWidget(vSyncCheckBox_);

    antiAliasingCheckBox_ = new QCheckBox(tr("Anti-aliasing"), renderingGroup);
    antiAliasingCheckBox_->setToolTip(tr("Enable anti-aliasing for smoother graphics"));
    renderingLayout->addWidget(antiAliasingCheckBox_);

    textureFilteringCheckBox_ = new QCheckBox(tr("Texture filtering"), renderingGroup);
    textureFilteringCheckBox_->setToolTip(tr("Enable texture filtering"));
    renderingLayout->addWidget(textureFilteringCheckBox_);

    layout->addWidget(renderingGroup);

    // Visual effects group
    QGroupBox* effectsGroup = new QGroupBox(tr("Visual Effects"), graphicsTab);
    QVBoxLayout* effectsLayout = new QVBoxLayout(effectsGroup);

    animationsCheckBox_ = new QCheckBox(tr("Animations"), effectsGroup);
    animationsCheckBox_->setToolTip(tr("Enable sprite animations"));
    effectsLayout->addWidget(animationsCheckBox_);

    lightingCheckBox_ = new QCheckBox(tr("Lighting effects"), effectsGroup);
    lightingCheckBox_->setToolTip(tr("Enable lighting and shadow effects"));
    effectsLayout->addWidget(lightingCheckBox_);

    shadowsCheckBox_ = new QCheckBox(tr("Shadows"), effectsGroup);
    shadowsCheckBox_->setToolTip(tr("Enable shadow rendering"));
    effectsLayout->addWidget(shadowsCheckBox_);

    transparencyCheckBox_ = new QCheckBox(tr("Transparency"), effectsGroup);
    transparencyCheckBox_->setToolTip(tr("Enable transparency effects"));
    effectsLayout->addWidget(transparencyCheckBox_);

    highQualityCheckBox_ = new QCheckBox(tr("High quality rendering"), effectsGroup);
    highQualityCheckBox_->setToolTip(tr("Enable high quality rendering mode"));
    effectsLayout->addWidget(highQualityCheckBox_);

    layout->addWidget(effectsGroup);

    // Performance group
    QGroupBox* performanceGroup = new QGroupBox(tr("Performance"), graphicsTab);
    QFormLayout* performanceLayout = new QFormLayout(performanceGroup);

    frameRateSpinBox_ = new QSpinBox(performanceGroup);
    frameRateSpinBox_->setRange(30, 120);
    frameRateSpinBox_->setSuffix(tr(" FPS"));
    frameRateSpinBox_->setToolTip(tr("Target frame rate"));
    performanceLayout->addRow(tr("Frame rate:"), frameRateSpinBox_);

    textureQualityComboBox_ = new QComboBox(performanceGroup);
    textureQualityComboBox_->addItems({tr("Low"), tr("Medium"), tr("High"), tr("Ultra")});
    textureQualityComboBox_->setToolTip(tr("Texture quality setting"));
    performanceLayout->addRow(tr("Texture quality:"), textureQualityComboBox_);

    lightQualityComboBox_ = new QComboBox(performanceGroup);
    lightQualityComboBox_->addItems({tr("Low"), tr("Medium"), tr("High"), tr("Ultra")});
    lightQualityComboBox_->setToolTip(tr("Lighting quality setting"));
    performanceLayout->addRow(tr("Light quality:"), lightQualityComboBox_);

    layout->addWidget(performanceGroup);

    layout->addStretch();
    return graphicsTab;
}

QWidget* PreferencesDialog::createUITab() {
    QWidget* uiTab = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(uiTab);
    layout->setSpacing(15);

    // Palette styles group
    QGroupBox* paletteGroup = new QGroupBox(tr("Palette Styles"), uiTab);
    QFormLayout* paletteLayout = new QFormLayout(paletteGroup);

    QStringList paletteStyles = {tr("Large Icons"), tr("Small Icons"), tr("List"), tr("Detailed List")};

    terrainPaletteStyleComboBox_ = new QComboBox(paletteGroup);
    terrainPaletteStyleComboBox_->addItems(paletteStyles);
    terrainPaletteStyleComboBox_->setToolTip(tr("Terrain palette display style"));
    paletteLayout->addRow(tr("Terrain palette:"), terrainPaletteStyleComboBox_);

    collectionPaletteStyleComboBox_ = new QComboBox(paletteGroup);
    collectionPaletteStyleComboBox_->addItems(paletteStyles);
    collectionPaletteStyleComboBox_->setToolTip(tr("Collection palette display style"));
    paletteLayout->addRow(tr("Collection palette:"), collectionPaletteStyleComboBox_);

    doodadPaletteStyleComboBox_ = new QComboBox(paletteGroup);
    doodadPaletteStyleComboBox_->addItems(paletteStyles);
    doodadPaletteStyleComboBox_->setToolTip(tr("Doodad palette display style"));
    paletteLayout->addRow(tr("Doodad palette:"), doodadPaletteStyleComboBox_);

    itemPaletteStyleComboBox_ = new QComboBox(paletteGroup);
    itemPaletteStyleComboBox_->addItems(paletteStyles);
    itemPaletteStyleComboBox_->setToolTip(tr("Item palette display style"));
    paletteLayout->addRow(tr("Item palette:"), itemPaletteStyleComboBox_);

    rawPaletteStyleComboBox_ = new QComboBox(paletteGroup);
    rawPaletteStyleComboBox_->addItems(paletteStyles);
    rawPaletteStyleComboBox_->setToolTip(tr("RAW palette display style"));
    paletteLayout->addRow(tr("RAW palette:"), rawPaletteStyleComboBox_);

    paletteColumnCountSpinBox_ = new QSpinBox(paletteGroup);
    paletteColumnCountSpinBox_->setRange(1, 20);
    paletteColumnCountSpinBox_->setToolTip(tr("Number of columns in palette grids"));
    paletteLayout->addRow(tr("Palette columns:"), paletteColumnCountSpinBox_);

    layout->addWidget(paletteGroup);

    // Toolbar group
    QGroupBox* toolbarGroup = new QGroupBox(tr("Toolbars"), uiTab);
    QVBoxLayout* toolbarLayout = new QVBoxLayout(toolbarGroup);

    largeContainerIconsCheckBox_ = new QCheckBox(tr("Large container icons"), toolbarGroup);
    largeContainerIconsCheckBox_->setToolTip(tr("Use large icons in container toolbars"));
    toolbarLayout->addWidget(largeContainerIconsCheckBox_);

    largeChooseItemIconsCheckBox_ = new QCheckBox(tr("Large choose item icons"), toolbarGroup);
    largeChooseItemIconsCheckBox_->setToolTip(tr("Use large icons in item chooser"));
    toolbarLayout->addWidget(largeChooseItemIconsCheckBox_);

    largeTerrainToolbarCheckBox_ = new QCheckBox(tr("Large terrain toolbar"), toolbarGroup);
    largeTerrainToolbarCheckBox_->setToolTip(tr("Use large icons in terrain toolbar"));
    toolbarLayout->addWidget(largeTerrainToolbarCheckBox_);

    largeCollectionToolbarCheckBox_ = new QCheckBox(tr("Large collection toolbar"), toolbarGroup);
    largeCollectionToolbarCheckBox_->setToolTip(tr("Use large icons in collection toolbar"));
    toolbarLayout->addWidget(largeCollectionToolbarCheckBox_);

    layout->addWidget(toolbarGroup);

    // Appearance group
    QGroupBox* appearanceGroup = new QGroupBox(tr("Appearance"), uiTab);
    QVBoxLayout* appearanceLayout = new QVBoxLayout(appearanceGroup);

    guiSelectionShadowCheckBox_ = new QCheckBox(tr("GUI selection shadow"), appearanceGroup);
    guiSelectionShadowCheckBox_->setToolTip(tr("Show shadow effect for GUI selections"));
    appearanceLayout->addWidget(guiSelectionShadowCheckBox_);

    darkModeCheckBox_ = new QCheckBox(tr("Dark mode"), appearanceGroup);
    darkModeCheckBox_->setToolTip(tr("Enable dark theme"));
    appearanceLayout->addWidget(darkModeCheckBox_);

    QHBoxLayout* themeLayout = new QHBoxLayout();
    themeLayout->addWidget(new QLabel(tr("Theme:"), appearanceGroup));
    themeComboBox_ = new QComboBox(appearanceGroup);
    themeComboBox_->addItems({tr("Default"), tr("Dark"), tr("Light"), tr("System")});
    themeComboBox_->setToolTip(tr("Application theme"));
    themeLayout->addWidget(themeComboBox_);
    themeLayout->addStretch();
    appearanceLayout->addLayout(themeLayout);

    layout->addWidget(appearanceGroup);

    layout->addStretch();
    return uiTab;
}

QWidget* PreferencesDialog::createClientVersionTab() {
    QWidget* clientTab = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(clientTab);
    layout->setSpacing(15);

    // Current version group
    QGroupBox* currentGroup = new QGroupBox(tr("Current Version"), clientTab);
    QFormLayout* currentLayout = new QFormLayout(currentGroup);

    clientVersionComboBox_ = new QComboBox(currentGroup);
    clientVersionComboBox_->setToolTip(tr("Select client version"));
    currentLayout->addRow(tr("Client version:"), clientVersionComboBox_);

    checkSignaturesCheckBox_ = new QCheckBox(tr("Check file signatures"), currentGroup);
    checkSignaturesCheckBox_->setToolTip(tr("Verify client file signatures"));
    currentLayout->addRow(checkSignaturesCheckBox_);

    layout->addWidget(currentGroup);

    // Paths group
    QGroupBox* pathsGroup = new QGroupBox(tr("Paths"), clientTab);
    QGridLayout* pathsLayout = new QGridLayout(pathsGroup);

    pathsLayout->addWidget(new QLabel(tr("Client path:"), pathsGroup), 0, 0);
    clientPathLineEdit_ = new QLineEdit(pathsGroup);
    clientPathLineEdit_->setToolTip(tr("Path to client executable"));
    pathsLayout->addWidget(clientPathLineEdit_, 0, 1);
    browseClientPathButton_ = new QPushButton(tr("Browse..."), pathsGroup);
    pathsLayout->addWidget(browseClientPathButton_, 0, 2);

    pathsLayout->addWidget(new QLabel(tr("Data path:"), pathsGroup), 1, 0);
    dataPathLineEdit_ = new QLineEdit(pathsGroup);
    dataPathLineEdit_->setToolTip(tr("Path to client data files"));
    pathsLayout->addWidget(dataPathLineEdit_, 1, 1);
    browseDataPathButton_ = new QPushButton(tr("Browse..."), pathsGroup);
    pathsLayout->addWidget(browseDataPathButton_, 1, 2);

    pathsLayout->addWidget(new QLabel(tr("Extensions path:"), pathsGroup), 2, 0);
    extensionsPathLineEdit_ = new QLineEdit(pathsGroup);
    extensionsPathLineEdit_->setToolTip(tr("Path to client extensions"));
    pathsLayout->addWidget(extensionsPathLineEdit_, 2, 1);
    browseExtensionsPathButton_ = new QPushButton(tr("Browse..."), pathsGroup);
    pathsLayout->addWidget(browseExtensionsPathButton_, 2, 2);

    layout->addWidget(pathsGroup);

    // Version management group
    QGroupBox* managementGroup = new QGroupBox(tr("Version Management"), clientTab);
    QHBoxLayout* managementLayout = new QHBoxLayout(managementGroup);

    versionsListWidget_ = new QListWidget(managementGroup);
    versionsListWidget_->setToolTip(tr("Available client versions"));
    managementLayout->addWidget(versionsListWidget_);

    QVBoxLayout* buttonLayout = new QVBoxLayout();
    refreshVersionsButton_ = new QPushButton(tr("Refresh"), managementGroup);
    refreshVersionsButton_->setToolTip(tr("Refresh version list"));
    buttonLayout->addWidget(refreshVersionsButton_);

    addVersionButton_ = new QPushButton(tr("Add..."), managementGroup);
    addVersionButton_->setToolTip(tr("Add new version"));
    buttonLayout->addWidget(addVersionButton_);

    editVersionButton_ = new QPushButton(tr("Edit..."), managementGroup);
    editVersionButton_->setToolTip(tr("Edit selected version"));
    buttonLayout->addWidget(editVersionButton_);

    removeVersionButton_ = new QPushButton(tr("Remove"), managementGroup);
    removeVersionButton_->setToolTip(tr("Remove selected version"));
    buttonLayout->addWidget(removeVersionButton_);

    buttonLayout->addStretch();
    managementLayout->addLayout(buttonLayout);

    layout->addWidget(managementGroup);

    layout->addStretch();
    return clientTab;
}

QWidget* PreferencesDialog::createLODTab() {
    QWidget* lodTab = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(lodTab);
    layout->setSpacing(15);

    // Zoom thresholds group
    QGroupBox* zoomGroup = new QGroupBox(tr("Zoom Thresholds"), lodTab);
    QFormLayout* zoomLayout = new QFormLayout(zoomGroup);

    tooltipMaxZoomSpinBox_ = new QSpinBox(zoomGroup);
    tooltipMaxZoomSpinBox_->setRange(1, 1000);
    tooltipMaxZoomSpinBox_->setSuffix("%");
    tooltipMaxZoomSpinBox_->setToolTip(tr("Maximum zoom for tooltips"));
    zoomLayout->addRow(tr("Tooltip max zoom:"), tooltipMaxZoomSpinBox_);

    groundOnlyZoomSpinBox_ = new QSpinBox(zoomGroup);
    groundOnlyZoomSpinBox_->setRange(1, 1000);
    groundOnlyZoomSpinBox_->setSuffix("%");
    groundOnlyZoomSpinBox_->setToolTip(tr("Zoom level for ground-only display"));
    zoomLayout->addRow(tr("Ground only zoom:"), groundOnlyZoomSpinBox_);

    itemDisplayZoomSpinBox_ = new QSpinBox(zoomGroup);
    itemDisplayZoomSpinBox_->setRange(1, 1000);
    itemDisplayZoomSpinBox_->setSuffix("%");
    itemDisplayZoomSpinBox_->setToolTip(tr("Minimum zoom for item display"));
    zoomLayout->addRow(tr("Item display zoom:"), itemDisplayZoomSpinBox_);

    specialFeaturesZoomSpinBox_ = new QSpinBox(zoomGroup);
    specialFeaturesZoomSpinBox_->setRange(1, 1000);
    specialFeaturesZoomSpinBox_->setSuffix("%");
    specialFeaturesZoomSpinBox_->setToolTip(tr("Minimum zoom for special features"));
    zoomLayout->addRow(tr("Special features zoom:"), specialFeaturesZoomSpinBox_);

    animationZoomSpinBox_ = new QSpinBox(zoomGroup);
    animationZoomSpinBox_->setRange(1, 1000);
    animationZoomSpinBox_->setSuffix("%");
    animationZoomSpinBox_->setToolTip(tr("Minimum zoom for animations"));
    zoomLayout->addRow(tr("Animation zoom:"), animationZoomSpinBox_);

    effectsZoomSpinBox_ = new QSpinBox(zoomGroup);
    effectsZoomSpinBox_->setRange(1, 1000);
    effectsZoomSpinBox_->setSuffix("%");
    effectsZoomSpinBox_->setToolTip(tr("Minimum zoom for effects"));
    zoomLayout->addRow(tr("Effects zoom:"), effectsZoomSpinBox_);

    lightZoomSpinBox_ = new QSpinBox(zoomGroup);
    lightZoomSpinBox_->setRange(1, 1000);
    lightZoomSpinBox_->setSuffix("%");
    lightZoomSpinBox_->setToolTip(tr("Minimum zoom for lighting"));
    zoomLayout->addRow(tr("Light zoom:"), lightZoomSpinBox_);

    shadeZoomSpinBox_ = new QSpinBox(zoomGroup);
    shadeZoomSpinBox_->setRange(1, 1000);
    shadeZoomSpinBox_->setSuffix("%");
    shadeZoomSpinBox_->setToolTip(tr("Minimum zoom for shading"));
    zoomLayout->addRow(tr("Shade zoom:"), shadeZoomSpinBox_);

    townZoneZoomSpinBox_ = new QSpinBox(zoomGroup);
    townZoneZoomSpinBox_->setRange(1, 1000);
    townZoneZoomSpinBox_->setSuffix("%");
    townZoneZoomSpinBox_->setToolTip(tr("Minimum zoom for town zones"));
    zoomLayout->addRow(tr("Town zone zoom:"), townZoneZoomSpinBox_);

    gridZoomSpinBox_ = new QSpinBox(zoomGroup);
    gridZoomSpinBox_->setRange(1, 1000);
    gridZoomSpinBox_->setSuffix("%");
    gridZoomSpinBox_->setToolTip(tr("Minimum zoom for grid display"));
    zoomLayout->addRow(tr("Grid zoom:"), gridZoomSpinBox_);

    layout->addWidget(zoomGroup);

    // Grid settings group
    QGroupBox* gridGroup = new QGroupBox(tr("Grid Settings"), lodTab);
    QFormLayout* gridLayout = new QFormLayout(gridGroup);

    gridChunkSizeSpinBox_ = new QSpinBox(gridGroup);
    gridChunkSizeSpinBox_->setRange(1, 100);
    gridChunkSizeSpinBox_->setToolTip(tr("Grid chunk size"));
    gridLayout->addRow(tr("Chunk size:"), gridChunkSizeSpinBox_);

    gridVisibleRowsSpinBox_ = new QSpinBox(gridGroup);
    gridVisibleRowsSpinBox_->setRange(1, 50);
    gridVisibleRowsSpinBox_->setToolTip(tr("Number of visible grid rows"));
    gridLayout->addRow(tr("Visible rows:"), gridVisibleRowsSpinBox_);

    layout->addWidget(gridGroup);

    layout->addStretch();
    return lodTab;
}

QWidget* PreferencesDialog::createAutomagicTab() {
    QWidget* automagicTab = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(automagicTab);
    layout->setSpacing(15);

    // Automagic settings group
    QGroupBox* automagicGroup = new QGroupBox(tr("Automagic Settings"), automagicTab);
    QVBoxLayout* automagicLayout = new QVBoxLayout(automagicGroup);

    automagicEnabledCheckBox_ = new QCheckBox(tr("Enable automagic"), automagicGroup);
    automagicEnabledCheckBox_->setToolTip(tr("Enable automatic border generation"));
    automagicLayout->addWidget(automagicEnabledCheckBox_);

    sameGroundTypeBorderCheckBox_ = new QCheckBox(tr("Same ground type border"), automagicGroup);
    sameGroundTypeBorderCheckBox_->setToolTip(tr("Create borders between same ground types"));
    automagicLayout->addWidget(sameGroundTypeBorderCheckBox_);

    wallsRepelBordersCheckBox_ = new QCheckBox(tr("Walls repel borders"), automagicGroup);
    wallsRepelBordersCheckBox_->setToolTip(tr("Walls prevent border generation"));
    automagicLayout->addWidget(wallsRepelBordersCheckBox_);

    layerCarpetsCheckBox_ = new QCheckBox(tr("Layer carpets"), automagicGroup);
    layerCarpetsCheckBox_->setToolTip(tr("Layer carpet items automatically"));
    automagicLayout->addWidget(layerCarpetsCheckBox_);

    borderizeDeleteCheckBox_ = new QCheckBox(tr("Borderize on delete"), automagicGroup);
    borderizeDeleteCheckBox_->setToolTip(tr("Update borders when deleting items"));
    automagicLayout->addWidget(borderizeDeleteCheckBox_);

    layout->addWidget(automagicGroup);

    // Custom border group
    QGroupBox* borderGroup = new QGroupBox(tr("Custom Border"), automagicTab);
    QVBoxLayout* borderLayout = new QVBoxLayout(borderGroup);

    customBorderCheckBox_ = new QCheckBox(tr("Use custom border"), borderGroup);
    customBorderCheckBox_->setToolTip(tr("Use custom border item"));
    borderLayout->addWidget(customBorderCheckBox_);

    QHBoxLayout* borderIdLayout = new QHBoxLayout();
    borderIdLayout->addWidget(new QLabel(tr("Border item ID:"), borderGroup));
    customBorderIdSpinBox_ = new QSpinBox(borderGroup);
    customBorderIdSpinBox_->setRange(1, 65535);
    customBorderIdSpinBox_->setToolTip(tr("Item ID for custom border"));
    borderIdLayout->addWidget(customBorderIdSpinBox_);
    borderIdLayout->addStretch();
    borderLayout->addLayout(borderIdLayout);

    layout->addWidget(borderGroup);

    // Border quality group
    QGroupBox* qualityGroup = new QGroupBox(tr("Border Quality"), automagicTab);
    QVBoxLayout* qualityLayout = new QVBoxLayout(qualityGroup);

    // Quality slider
    QHBoxLayout* qualitySliderLayout = new QHBoxLayout();
    qualitySliderLayout->addWidget(new QLabel(tr("Quality:"), qualityGroup));
    borderQualitySlider_ = new QSlider(Qt::Horizontal, qualityGroup);
    borderQualitySlider_->setRange(1, 10);
    borderQualitySlider_->setToolTip(tr("Border generation quality"));
    qualitySliderLayout->addWidget(borderQualitySlider_);
    borderQualityLabel_ = new QLabel("5", qualityGroup);
    qualitySliderLayout->addWidget(borderQualityLabel_);
    qualityLayout->addLayout(qualitySliderLayout);

    // Thickness slider
    QHBoxLayout* thicknessSliderLayout = new QHBoxLayout();
    thicknessSliderLayout->addWidget(new QLabel(tr("Thickness:"), qualityGroup));
    borderThicknessSlider_ = new QSlider(Qt::Horizontal, qualityGroup);
    borderThicknessSlider_->setRange(1, 5);
    borderThicknessSlider_->setToolTip(tr("Border thickness"));
    thicknessSliderLayout->addWidget(borderThicknessSlider_);
    borderThicknessLabel_ = new QLabel("2", qualityGroup);
    thicknessSliderLayout->addWidget(borderThicknessLabel_);
    qualityLayout->addLayout(thicknessSliderLayout);

    // Randomness slider
    QHBoxLayout* randomnessSliderLayout = new QHBoxLayout();
    randomnessSliderLayout->addWidget(new QLabel(tr("Randomness:"), qualityGroup));
    borderRandomnessSlider_ = new QSlider(Qt::Horizontal, qualityGroup);
    borderRandomnessSlider_->setRange(0, 10);
    borderRandomnessSlider_->setToolTip(tr("Border randomness factor"));
    randomnessSliderLayout->addWidget(borderRandomnessSlider_);
    borderRandomnessLabel_ = new QLabel("3", qualityGroup);
    randomnessSliderLayout->addWidget(borderRandomnessLabel_);
    qualityLayout->addLayout(randomnessSliderLayout);

    layout->addWidget(qualityGroup);

    layout->addStretch();
    return automagicTab;
}

void PreferencesDialog::connectSignals() {
    // Tab widget signals
    connect(tabWidget_, &QTabWidget::currentChanged, this, &PreferencesDialog::onTabChanged);

    // Button signals
    connect(defaultsButton_, &QPushButton::clicked, this, &PreferencesDialog::onDefaultsClicked);
    connect(applyButton_, &QPushButton::clicked, this, &PreferencesDialog::onApplyClicked);
    connect(okButton_, &QPushButton::clicked, this, &PreferencesDialog::onOkClicked);
    connect(cancelButton_, &QPushButton::clicked, this, &PreferencesDialog::onCancelClicked);

    // General tab signals
    connect(welcomeDialogCheckBox_, &QCheckBox::toggled, this, &PreferencesDialog::onWelcomeDialogToggled);
    connect(alwaysBackupCheckBox_, &QCheckBox::toggled, this, &PreferencesDialog::onBackupToggled);
    connect(updateCheckCheckBox_, &QCheckBox::toggled, this, &PreferencesDialog::onUpdateCheckToggled);
    connect(singleInstanceCheckBox_, &QCheckBox::toggled, this, &PreferencesDialog::onSingleInstanceToggled);
    connect(tilesetEditingCheckBox_, &QCheckBox::toggled, this, &PreferencesDialog::onTilesetEditingToggled);
    connect(undoSizeSpinBox_, QOverload<int>::of(&QSpinBox::valueChanged), this, &PreferencesDialog::onUndoSizeChanged);
    connect(undoMemSizeSpinBox_, QOverload<int>::of(&QSpinBox::valueChanged), this, &PreferencesDialog::onUndoMemSizeChanged);
    connect(workerThreadsSpinBox_, QOverload<int>::of(&QSpinBox::valueChanged), this, &PreferencesDialog::onWorkerThreadsChanged);
    connect(replaceSizeSpinBox_, QOverload<int>::of(&QSpinBox::valueChanged), this, &PreferencesDialog::onReplaceSizeChanged);
    connect(positionFormatComboBox_, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &PreferencesDialog::onPositionFormatChanged);
    connect(autoSaveCheckBox_, &QCheckBox::toggled, this, &PreferencesDialog::onAutoSaveToggled);
    connect(autoSaveIntervalSpinBox_, QOverload<int>::of(&QSpinBox::valueChanged), this, &PreferencesDialog::onAutoSaveIntervalChanged);

    // Editor tab signals
    connect(groupActionsCheckBox_, &QCheckBox::toggled, this, &PreferencesDialog::onGroupActionsToggled);
    connect(duplicateActionsCheckBox_, &QCheckBox::toggled, this, &PreferencesDialog::onDuplicateActionsToggled);
    connect(houseEditWarningCheckBox_, &QCheckBox::toggled, this, &PreferencesDialog::onHouseEditWarningToggled);
    connect(warnForDestructiveCheckBox_, &QCheckBox::toggled, this, &PreferencesDialog::onWarnForDestructiveToggled);
    connect(autoSelectRawCheckBox_, &QCheckBox::toggled, this, &PreferencesDialog::onAutoSelectRawToggled);
    connect(swapMouseButtonsCheckBox_, &QCheckBox::toggled, this, &PreferencesDialog::onSwapMouseButtonsToggled);
    connect(doubleClickPropertiesCheckBox_, &QCheckBox::toggled, this, &PreferencesDialog::onDoubleClickPropertiesToggled);
    connect(invertWheelCheckBox_, &QCheckBox::toggled, this, &PreferencesDialog::onInvertWheelToggled);
    connect(scrollSpeedSlider_, &QSlider::valueChanged, this, &PreferencesDialog::onScrollSpeedChanged);
    connect(zoomSpeedSlider_, &QSlider::valueChanged, this, &PreferencesDialog::onZoomSpeedChanged);

    // Graphics tab signals
    connect(hardwareAccelerationCheckBox_, &QCheckBox::toggled, this, &PreferencesDialog::onHardwareAccelerationToggled);
    connect(vSyncCheckBox_, &QCheckBox::toggled, this, &PreferencesDialog::onVSyncToggled);
    connect(antiAliasingCheckBox_, &QCheckBox::toggled, this, &PreferencesDialog::onAntiAliasingToggled);
    connect(textureFilteringCheckBox_, &QCheckBox::toggled, this, &PreferencesDialog::onTextureFilteringToggled);
    connect(animationsCheckBox_, &QCheckBox::toggled, this, &PreferencesDialog::onAnimationsToggled);
    connect(lightingCheckBox_, &QCheckBox::toggled, this, &PreferencesDialog::onLightingToggled);
    connect(shadowsCheckBox_, &QCheckBox::toggled, this, &PreferencesDialog::onShadowsToggled);
    connect(transparencyCheckBox_, &QCheckBox::toggled, this, &PreferencesDialog::onTransparencyToggled);
    connect(highQualityCheckBox_, &QCheckBox::toggled, this, &PreferencesDialog::onHighQualityToggled);
    connect(frameRateSpinBox_, QOverload<int>::of(&QSpinBox::valueChanged), this, &PreferencesDialog::onFrameRateChanged);
    connect(textureQualityComboBox_, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &PreferencesDialog::onTextureQualityChanged);
    connect(lightQualityComboBox_, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &PreferencesDialog::onLightQualityChanged);

    // UI tab signals
    connect(terrainPaletteStyleComboBox_, &QComboBox::currentTextChanged, this, &PreferencesDialog::onTerrainPaletteStyleChanged);
    connect(collectionPaletteStyleComboBox_, &QComboBox::currentTextChanged, this, &PreferencesDialog::onCollectionPaletteStyleChanged);
    connect(doodadPaletteStyleComboBox_, &QComboBox::currentTextChanged, this, &PreferencesDialog::onDoodadPaletteStyleChanged);
    connect(itemPaletteStyleComboBox_, &QComboBox::currentTextChanged, this, &PreferencesDialog::onItemPaletteStyleChanged);
    connect(rawPaletteStyleComboBox_, &QComboBox::currentTextChanged, this, &PreferencesDialog::onRawPaletteStyleChanged);
    connect(paletteColumnCountSpinBox_, QOverload<int>::of(&QSpinBox::valueChanged), this, &PreferencesDialog::onPaletteColumnCountChanged);
    connect(largeContainerIconsCheckBox_, &QCheckBox::toggled, this, &PreferencesDialog::onLargeContainerIconsToggled);
    connect(largeChooseItemIconsCheckBox_, &QCheckBox::toggled, this, &PreferencesDialog::onLargeChooseItemIconsToggled);
    connect(largeTerrainToolbarCheckBox_, &QCheckBox::toggled, this, &PreferencesDialog::onLargeTerrainToolbarToggled);
    connect(largeCollectionToolbarCheckBox_, &QCheckBox::toggled, this, &PreferencesDialog::onLargeCollectionToolbarToggled);
    connect(guiSelectionShadowCheckBox_, &QCheckBox::toggled, this, &PreferencesDialog::onGuiSelectionShadowToggled);
    connect(darkModeCheckBox_, &QCheckBox::toggled, this, &PreferencesDialog::onDarkModeToggled);
    connect(themeComboBox_, &QComboBox::currentTextChanged, this, &PreferencesDialog::onThemeChanged);

    // Client Version tab signals
    connect(clientVersionComboBox_, &QComboBox::currentTextChanged, this, &PreferencesDialog::onClientVersionChanged);
    connect(clientPathLineEdit_, &QLineEdit::textChanged, this, &PreferencesDialog::onClientPathChanged);
    connect(dataPathLineEdit_, &QLineEdit::textChanged, this, &PreferencesDialog::onDataPathChanged);
    connect(extensionsPathLineEdit_, &QLineEdit::textChanged, this, &PreferencesDialog::onExtensionsPathChanged);
    connect(checkSignaturesCheckBox_, &QCheckBox::toggled, this, &PreferencesDialog::onCheckSignaturesToggled);
    connect(browseClientPathButton_, &QPushButton::clicked, this, &PreferencesDialog::onBrowseClientPath);
    connect(browseDataPathButton_, &QPushButton::clicked, this, &PreferencesDialog::onBrowseDataPath);
    connect(browseExtensionsPathButton_, &QPushButton::clicked, this, &PreferencesDialog::onBrowseExtensionsPath);
    connect(refreshVersionsButton_, &QPushButton::clicked, this, &PreferencesDialog::onRefreshVersions);
    connect(addVersionButton_, &QPushButton::clicked, this, &PreferencesDialog::onAddVersion);
    connect(removeVersionButton_, &QPushButton::clicked, this, &PreferencesDialog::onRemoveVersion);
    connect(editVersionButton_, &QPushButton::clicked, this, &PreferencesDialog::onEditVersion);

    // LOD tab signals
    connect(tooltipMaxZoomSpinBox_, QOverload<int>::of(&QSpinBox::valueChanged), this, &PreferencesDialog::onTooltipMaxZoomChanged);
    connect(groundOnlyZoomSpinBox_, QOverload<int>::of(&QSpinBox::valueChanged), this, &PreferencesDialog::onGroundOnlyZoomChanged);
    connect(itemDisplayZoomSpinBox_, QOverload<int>::of(&QSpinBox::valueChanged), this, &PreferencesDialog::onItemDisplayZoomChanged);
    connect(specialFeaturesZoomSpinBox_, QOverload<int>::of(&QSpinBox::valueChanged), this, &PreferencesDialog::onSpecialFeaturesZoomChanged);
    connect(animationZoomSpinBox_, QOverload<int>::of(&QSpinBox::valueChanged), this, &PreferencesDialog::onAnimationZoomChanged);
    connect(effectsZoomSpinBox_, QOverload<int>::of(&QSpinBox::valueChanged), this, &PreferencesDialog::onEffectsZoomChanged);
    connect(lightZoomSpinBox_, QOverload<int>::of(&QSpinBox::valueChanged), this, &PreferencesDialog::onLightZoomChanged);
    connect(shadeZoomSpinBox_, QOverload<int>::of(&QSpinBox::valueChanged), this, &PreferencesDialog::onShadeZoomChanged);
    connect(townZoneZoomSpinBox_, QOverload<int>::of(&QSpinBox::valueChanged), this, &PreferencesDialog::onTownZoneZoomChanged);
    connect(gridZoomSpinBox_, QOverload<int>::of(&QSpinBox::valueChanged), this, &PreferencesDialog::onGridZoomChanged);
    connect(gridChunkSizeSpinBox_, QOverload<int>::of(&QSpinBox::valueChanged), this, &PreferencesDialog::onGridChunkSizeChanged);
    connect(gridVisibleRowsSpinBox_, QOverload<int>::of(&QSpinBox::valueChanged), this, &PreferencesDialog::onGridVisibleRowsChanged);

    // Automagic tab signals
    connect(automagicEnabledCheckBox_, &QCheckBox::toggled, this, &PreferencesDialog::onAutomagicEnabledToggled);
    connect(sameGroundTypeBorderCheckBox_, &QCheckBox::toggled, this, &PreferencesDialog::onSameGroundTypeBorderToggled);
    connect(wallsRepelBordersCheckBox_, &QCheckBox::toggled, this, &PreferencesDialog::onWallsRepelBordersToggled);
    connect(layerCarpetsCheckBox_, &QCheckBox::toggled, this, &PreferencesDialog::onLayerCarpetsToggled);
    connect(borderizeDeleteCheckBox_, &QCheckBox::toggled, this, &PreferencesDialog::onBorderizeDeleteToggled);
    connect(customBorderCheckBox_, &QCheckBox::toggled, this, &PreferencesDialog::onCustomBorderToggled);
    connect(customBorderIdSpinBox_, QOverload<int>::of(&QSpinBox::valueChanged), this, &PreferencesDialog::onCustomBorderIdChanged);
    connect(borderQualitySlider_, &QSlider::valueChanged, this, &PreferencesDialog::onBorderQualityChanged);
    connect(borderThicknessSlider_, &QSlider::valueChanged, this, &PreferencesDialog::onBorderThicknessChanged);
    connect(borderRandomnessSlider_, &QSlider::valueChanged, this, &PreferencesDialog::onBorderRandomnessChanged);

    // Update slider labels
    connect(scrollSpeedSlider_, &QSlider::valueChanged, scrollSpeedLabel_, QOverload<int>::of(&QLabel::setNum));
    connect(zoomSpeedSlider_, &QSlider::valueChanged, zoomSpeedLabel_, QOverload<int>::of(&QLabel::setNum));
    connect(borderQualitySlider_, &QSlider::valueChanged, borderQualityLabel_, QOverload<int>::of(&QLabel::setNum));
    connect(borderThicknessSlider_, &QSlider::valueChanged, borderThicknessLabel_, QOverload<int>::of(&QLabel::setNum));
    connect(borderRandomnessSlider_, &QSlider::valueChanged, borderRandomnessLabel_, QOverload<int>::of(&QLabel::setNum));
}

// Task 78: Settings management implementation
void PreferencesDialog::loadSettings() {
    if (!settingsManager_) {
        qWarning() << "PreferencesDialog::loadSettings: SettingsManager not available";
        return;
    }

    loading_ = true;

    loadGeneralSettings();
    loadEditorSettings();
    loadGraphicsSettings();
    loadUISettings();
    loadClientVersionSettings();
    loadLODSettings();
    loadAutomagicSettings();

    loading_ = false;
    resetModifiedFlag();
}

void PreferencesDialog::saveSettings() {
    if (!settingsManager_) {
        qWarning() << "PreferencesDialog::saveSettings: SettingsManager not available";
        return;
    }

    saveGeneralSettings();
    saveEditorSettings();
    saveGraphicsSettings();
    saveUISettings();
    saveClientVersionSettings();
    saveLODSettings();
    saveAutomagicSettings();

    resetModifiedFlag();
}

void PreferencesDialog::resetToDefaults() {
    int result = QMessageBox::question(this, tr("Reset to Defaults"),
                                      tr("Are you sure you want to reset all settings to their default values?\n"
                                         "This action cannot be undone."),
                                      QMessageBox::Yes | QMessageBox::No,
                                      QMessageBox::No);

    if (result == QMessageBox::Yes) {
        if (settingsManager_) {
            settingsManager_->resetToDefaults();
            loadSettings();
        }
    }
}

void PreferencesDialog::applySettings() {
    saveSettings();
    if (applyButton_) {
        applyButton_->setEnabled(false);
    }
}
