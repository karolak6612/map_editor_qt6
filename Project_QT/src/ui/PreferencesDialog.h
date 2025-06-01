#ifndef PREFERENCESDIALOG_H
#define PREFERENCESDIALOG_H

#include <QDialog>
#include <QTabWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QCheckBox>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QComboBox>
#include <QLineEdit>
#include <QSlider>
#include <QLabel>
#include <QPushButton>
#include <QButtonGroup>
#include <QRadioButton>
#include <QTextEdit>
#include <QScrollArea>
#include <QSplitter>
#include <QTreeWidget>
#include <QListWidget>
#include <QTableWidget>
#include <QProgressBar>
#include <QFileDialog>
#include <QColorDialog>
#include <QFontDialog>

// Forward declarations
class SettingsManager;
class MainWindow;

/**
 * @brief Comprehensive preferences dialog for all application settings
 * 
 * Task 78: Complete migration of wxwidgets PreferencesWindow with all tabs:
 * - General settings (startup, backup, updates)
 * - Editor settings (undo, actions, drawing)
 * - Graphics settings (rendering, performance, visual effects)
 * - UI settings (palettes, toolbars, themes)
 * - Client Version settings (client paths, versions)
 * - LOD settings (level of detail, zoom thresholds)
 * - Automagic settings (border automation, ground borders)
 */
class PreferencesDialog : public QDialog {
    Q_OBJECT

public:
    explicit PreferencesDialog(QWidget* parent = nullptr);
    ~PreferencesDialog() override;

    // Settings management
    void loadSettings();
    void saveSettings();
    void resetToDefaults();
    void applySettings();

public slots:
    void accept() override;
    void reject() override;

private slots:
    // Tab change handling
    void onTabChanged(int index);
    
    // Button handlers
    void onDefaultsClicked();
    void onApplyClicked();
    void onOkClicked();
    void onCancelClicked();
    
    // General tab slots
    void onWelcomeDialogToggled(bool enabled);
    void onBackupToggled(bool enabled);
    void onUpdateCheckToggled(bool enabled);
    void onSingleInstanceToggled(bool enabled);
    void onTilesetEditingToggled(bool enabled);
    void onUndoSizeChanged(int size);
    void onUndoMemSizeChanged(int size);
    void onWorkerThreadsChanged(int threads);
    void onReplaceSizeChanged(int size);
    void onPositionFormatChanged(int format);
    void onAutoSaveToggled(bool enabled);
    void onAutoSaveIntervalChanged(int interval);
    
    // Editor tab slots
    void onGroupActionsToggled(bool enabled);
    void onDuplicateActionsToggled(bool enabled);
    void onHouseEditWarningToggled(bool enabled);
    void onWarnForDestructiveToggled(bool enabled);
    void onAutoSelectRawToggled(bool enabled);
    void onSwapMouseButtonsToggled(bool enabled);
    void onDoubleClickPropertiesToggled(bool enabled);
    void onInvertWheelToggled(bool enabled);
    void onScrollSpeedChanged(int speed);
    void onZoomSpeedChanged(int speed);
    
    // Graphics tab slots
    void onHardwareAccelerationToggled(bool enabled);
    void onVSyncToggled(bool enabled);
    void onAntiAliasingToggled(bool enabled);
    void onTextureFilteringToggled(bool enabled);
    void onAnimationsToggled(bool enabled);
    void onLightingToggled(bool enabled);
    void onShadowsToggled(bool enabled);
    void onTransparencyToggled(bool enabled);
    void onHighQualityToggled(bool enabled);
    void onFrameRateChanged(int fps);
    void onTextureQualityChanged(int quality);
    void onLightQualityChanged(int quality);
    
    // UI tab slots
    void onTerrainPaletteStyleChanged(const QString& style);
    void onCollectionPaletteStyleChanged(const QString& style);
    void onDoodadPaletteStyleChanged(const QString& style);
    void onItemPaletteStyleChanged(const QString& style);
    void onRawPaletteStyleChanged(const QString& style);
    void onPaletteColumnCountChanged(int count);
    void onLargeContainerIconsToggled(bool enabled);
    void onLargeChooseItemIconsToggled(bool enabled);
    void onLargeTerrainToolbarToggled(bool enabled);
    void onLargeCollectionToolbarToggled(bool enabled);
    void onGuiSelectionShadowToggled(bool enabled);
    void onDarkModeToggled(bool enabled);
    void onThemeChanged(const QString& theme);
    
    // Client Version tab slots
    void onClientVersionChanged(const QString& version);
    void onClientPathChanged(const QString& path);
    void onDataPathChanged(const QString& path);
    void onExtensionsPathChanged(const QString& path);
    void onCheckSignaturesToggled(bool enabled);
    void onBrowseClientPath();
    void onBrowseDataPath();
    void onBrowseExtensionsPath();
    void onRefreshVersions();
    void onAddVersion();
    void onRemoveVersion();
    void onEditVersion();
    
    // LOD tab slots
    void onTooltipMaxZoomChanged(int zoom);
    void onGroundOnlyZoomChanged(int zoom);
    void onItemDisplayZoomChanged(int zoom);
    void onSpecialFeaturesZoomChanged(int zoom);
    void onAnimationZoomChanged(int zoom);
    void onEffectsZoomChanged(int zoom);
    void onLightZoomChanged(int zoom);
    void onShadeZoomChanged(int zoom);
    void onTownZoneZoomChanged(int zoom);
    void onGridZoomChanged(int zoom);
    void onGridChunkSizeChanged(int size);
    void onGridVisibleRowsChanged(int rows);
    
    // Automagic tab slots
    void onAutomagicEnabledToggled(bool enabled);
    void onSameGroundTypeBorderToggled(bool enabled);
    void onWallsRepelBordersToggled(bool enabled);
    void onLayerCarpetsToggled(bool enabled);
    void onBorderizeDeleteToggled(bool enabled);
    void onCustomBorderToggled(bool enabled);
    void onCustomBorderIdChanged(int id);
    void onBorderQualityChanged(int quality);
    void onBorderThicknessChanged(int thickness);
    void onBorderRandomnessChanged(int randomness);

private:
    // UI setup methods
    void setupUI();
    void setupTabWidget();
    void setupButtonBox();
    void connectSignals();
    
    // Tab creation methods
    QWidget* createGeneralTab();
    QWidget* createEditorTab();
    QWidget* createGraphicsTab();
    QWidget* createUITab();
    QWidget* createClientVersionTab();
    QWidget* createLODTab();
    QWidget* createAutomagicTab();
    
    // Helper methods
    void updateDependentControls();
    void validateSettings();
    bool hasUnsavedChanges() const;
    void markAsModified();
    void resetModifiedFlag();
    
    // Settings integration
    void loadGeneralSettings();
    void loadEditorSettings();
    void loadGraphicsSettings();
    void loadUISettings();
    void loadClientVersionSettings();
    void loadLODSettings();
    void loadAutomagicSettings();
    
    void saveGeneralSettings();
    void saveEditorSettings();
    void saveGraphicsSettings();
    void saveUISettings();
    void saveClientVersionSettings();
    void saveLODSettings();
    void saveAutomagicSettings();
    
    // UI components
    QTabWidget* tabWidget_;
    QVBoxLayout* mainLayout_;
    QHBoxLayout* buttonLayout_;
    
    // Buttons
    QPushButton* defaultsButton_;
    QPushButton* applyButton_;
    QPushButton* okButton_;
    QPushButton* cancelButton_;
    
    // General tab controls
    QCheckBox* welcomeDialogCheckBox_;
    QCheckBox* alwaysBackupCheckBox_;
    QCheckBox* updateCheckCheckBox_;
    QCheckBox* singleInstanceCheckBox_;
    QCheckBox* tilesetEditingCheckBox_;
    QSpinBox* undoSizeSpinBox_;
    QSpinBox* undoMemSizeSpinBox_;
    QSpinBox* workerThreadsSpinBox_;
    QSpinBox* replaceSizeSpinBox_;
    QComboBox* positionFormatComboBox_;
    QCheckBox* autoSaveCheckBox_;
    QSpinBox* autoSaveIntervalSpinBox_;
    
    // Editor tab controls
    QCheckBox* groupActionsCheckBox_;
    QCheckBox* duplicateActionsCheckBox_;
    QCheckBox* houseEditWarningCheckBox_;
    QCheckBox* warnForDestructiveCheckBox_;
    QCheckBox* autoSelectRawCheckBox_;
    QCheckBox* swapMouseButtonsCheckBox_;
    QCheckBox* doubleClickPropertiesCheckBox_;
    QCheckBox* invertWheelCheckBox_;
    QSlider* scrollSpeedSlider_;
    QSlider* zoomSpeedSlider_;
    QLabel* scrollSpeedLabel_;
    QLabel* zoomSpeedLabel_;
    
    // Graphics tab controls
    QCheckBox* hardwareAccelerationCheckBox_;
    QCheckBox* vSyncCheckBox_;
    QCheckBox* antiAliasingCheckBox_;
    QCheckBox* textureFilteringCheckBox_;
    QCheckBox* animationsCheckBox_;
    QCheckBox* lightingCheckBox_;
    QCheckBox* shadowsCheckBox_;
    QCheckBox* transparencyCheckBox_;
    QCheckBox* highQualityCheckBox_;
    QSpinBox* frameRateSpinBox_;
    QComboBox* textureQualityComboBox_;
    QComboBox* lightQualityComboBox_;
    
    // UI tab controls
    QComboBox* terrainPaletteStyleComboBox_;
    QComboBox* collectionPaletteStyleComboBox_;
    QComboBox* doodadPaletteStyleComboBox_;
    QComboBox* itemPaletteStyleComboBox_;
    QComboBox* rawPaletteStyleComboBox_;
    QSpinBox* paletteColumnCountSpinBox_;
    QCheckBox* largeContainerIconsCheckBox_;
    QCheckBox* largeChooseItemIconsCheckBox_;
    QCheckBox* largeTerrainToolbarCheckBox_;
    QCheckBox* largeCollectionToolbarCheckBox_;
    QCheckBox* guiSelectionShadowCheckBox_;
    QCheckBox* darkModeCheckBox_;
    QComboBox* themeComboBox_;
    
    // Client Version tab controls
    QComboBox* clientVersionComboBox_;
    QLineEdit* clientPathLineEdit_;
    QLineEdit* dataPathLineEdit_;
    QLineEdit* extensionsPathLineEdit_;
    QCheckBox* checkSignaturesCheckBox_;
    QPushButton* browseClientPathButton_;
    QPushButton* browseDataPathButton_;
    QPushButton* browseExtensionsPathButton_;
    QPushButton* refreshVersionsButton_;
    QPushButton* addVersionButton_;
    QPushButton* removeVersionButton_;
    QPushButton* editVersionButton_;
    QListWidget* versionsListWidget_;
    
    // LOD tab controls
    QSpinBox* tooltipMaxZoomSpinBox_;
    QSpinBox* groundOnlyZoomSpinBox_;
    QSpinBox* itemDisplayZoomSpinBox_;
    QSpinBox* specialFeaturesZoomSpinBox_;
    QSpinBox* animationZoomSpinBox_;
    QSpinBox* effectsZoomSpinBox_;
    QSpinBox* lightZoomSpinBox_;
    QSpinBox* shadeZoomSpinBox_;
    QSpinBox* townZoneZoomSpinBox_;
    QSpinBox* gridZoomSpinBox_;
    QSpinBox* gridChunkSizeSpinBox_;
    QSpinBox* gridVisibleRowsSpinBox_;
    
    // Automagic tab controls
    QCheckBox* automagicEnabledCheckBox_;
    QCheckBox* sameGroundTypeBorderCheckBox_;
    QCheckBox* wallsRepelBordersCheckBox_;
    QCheckBox* layerCarpetsCheckBox_;
    QCheckBox* borderizeDeleteCheckBox_;
    QCheckBox* customBorderCheckBox_;
    QSpinBox* customBorderIdSpinBox_;
    QSlider* borderQualitySlider_;
    QSlider* borderThicknessSlider_;
    QSlider* borderRandomnessSlider_;
    QLabel* borderQualityLabel_;
    QLabel* borderThicknessLabel_;
    QLabel* borderRandomnessLabel_;
    
    // State management
    SettingsManager* settingsManager_;
    MainWindow* mainWindow_;
    bool modified_;
    bool loading_;
};

#endif // PREFERENCESDIALOG_H
