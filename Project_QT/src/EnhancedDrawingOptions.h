#ifndef ENHANCEDDRAWINGOPTIONS_H
#define ENHANCEDDRAWINGOPTIONS_H

#include <QObject>
#include <QVariantMap>
#include <QStringList>
#include <QColor>
#include <QFont>
#include <QSettings>

// Forward declarations
class LODManager;
class EnhancedLightingSystem;
class TransparencyManager;

/**
 * @brief Enhanced Drawing Options for Task 83
 * 
 * Complete implementation of drawing options for Qt6 map editor:
 * - 1:1 compatibility with wxwidgets DrawingOptions
 * - Integration with LOD, lighting, and transparency systems
 * - Advanced rendering configuration and presets
 * - Performance optimization settings
 * - Persistence and configuration management
 */

/**
 * @brief Enhanced Drawing Options class (1:1 wxwidgets compatibility + extensions)
 */
class EnhancedDrawingOptions : public QObject
{
    Q_OBJECT

public:
    explicit EnhancedDrawingOptions(QObject* parent = nullptr);
    ~EnhancedDrawingOptions() override;

    // === 1:1 wxwidgets DrawingOptions compatibility ===
    
    // Floor and transparency settings
    bool transparent_floors = false;
    bool transparent_items = false;
    bool show_all_floors = false;
    bool show_shade = false;
    
    // Lighting settings
    bool show_lights = false;
    bool show_light_str = false;
    bool isDrawLight() const noexcept { return show_lights; }
    
    // Display elements
    bool show_ingame_box = false;
    bool show_tech_items = false;
    bool show_waypoints = false;
    bool show_creatures = false;
    bool show_spawns = false;
    bool show_houses = false;
    bool show_special_tiles = false;
    bool show_zone_areas = false;
    bool show_items = true;
    bool show_towns = false;
    bool show_hooks = false;
    
    // Grid and visual aids
    int show_grid = 0;  // 0=none, 1=light, 2=heavy
    bool show_blocking = false;
    bool show_tooltips = false;
    bool show_as_minimap = false;
    bool show_only_colors = false;
    bool show_only_modified = false;
    bool show_preview = false;
    bool always_show_zones = false;
    
    // Highlighting and effects
    bool highlight_items = false;
    bool highlight_locked_doors = false;
    bool extended_house_shader = false;
    bool experimental_fog = false;
    
    // State flags
    bool ingame = false;
    bool dragging = false;
    bool hide_items_when_zoomed = false;
    
    // === Enhanced features for Task 83 ===
    
    // LOD settings
    bool enableLOD = true;
    double lodZoomThreshold = 3.0;
    bool lodUseSimplifiedSprites = true;
    bool lodSkipAnimations = true;
    bool lodSkipEffects = true;
    
    // Advanced lighting settings
    bool enableAdvancedLighting = true;
    bool enableLightAnimation = true;
    bool enableLightBlending = true;
    bool enableLightReflection = false;
    QColor globalLightColor = QColor(50, 50, 50, 255);
    int globalLightIntensity = 140;
    bool enableFloorShading = true;
    int floorShadingIntensity = 128;
    
    // Advanced transparency settings
    bool enableAdvancedTransparency = true;
    bool enableTransparencyAnimation = true;
    double floorTransparencyFactor = 0.7;
    double maxFloorTransparency = 0.9;
    int transparentFloorRange = 3;
    bool fadeUpperFloors = true;
    bool fadeLowerFloors = false;
    
    // Performance settings
    bool enableRenderingOptimization = true;
    bool enableCaching = true;
    bool enableBatching = true;
    bool enableCulling = true;
    int maxItemsPerTile = 100;
    double minRenderThreshold = 0.01;
    
    // Visual quality settings
    bool enableAntialiasing = true;
    bool enableSmoothing = true;
    bool enableHighQualityRendering = false;
    bool enableTextureFiltering = true;
    
    // Color and appearance
    QColor backgroundColor = QColor(64, 64, 64);
    QColor gridColor = QColor(128, 128, 128, 128);
    QColor selectionColor = QColor(255, 255, 0, 128);
    QColor highlightColor = QColor(255, 0, 0, 128);
    
    // Font settings
    QFont tooltipFont = QFont("Arial", 9);
    QFont gridFont = QFont("Arial", 8);
    QFont debugFont = QFont("Consolas", 8);
    
    // Debug and diagnostic settings
    bool showDebugInfo = false;
    bool showPerformanceStats = false;
    bool showRenderingStats = false;
    bool showMemoryUsage = false;
    bool enableDebugOverlay = false;

public:
    // Configuration management
    void setDefault();
    void setIngame();
    void setMinimap();
    void setHighPerformance();
    void setHighQuality();
    void setDebugMode();
    
    // Preset management
    void loadPreset(const QString& presetName);
    void savePreset(const QString& presetName) const;
    QStringList getAvailablePresets() const;
    void deletePreset(const QString& presetName);
    
    // Integration with rendering systems
    void setLODManager(LODManager* lodManager) { lodManager_ = lodManager; }
    void setLightingSystem(EnhancedLightingSystem* lightingSystem) { lightingSystem_ = lightingSystem; }
    void setTransparencyManager(TransparencyManager* transparencyManager) { transparencyManager_ = transparencyManager; }
    
    LODManager* getLODManager() const { return lodManager_; }
    EnhancedLightingSystem* getLightingSystem() const { return lightingSystem_; }
    TransparencyManager* getTransparencyManager() const { return transparencyManager_; }
    
    // Synchronization with rendering systems
    void syncWithLODManager();
    void syncWithLightingSystem();
    void syncWithTransparencyManager();
    void syncAllSystems();
    
    // Validation and constraints
    bool validateOptions() const;
    void enforceConstraints();
    QStringList getValidationErrors() const;
    
    // Serialization
    QVariantMap toVariantMap() const;
    void fromVariantMap(const QVariantMap& map);
    QString toJsonString() const;
    void fromJsonString(const QString& json);
    
    // Persistence
    void loadFromSettings(QSettings& settings, const QString& group = "DrawingOptions");
    void saveToSettings(QSettings& settings, const QString& group = "DrawingOptions") const;
    void loadFromFile(const QString& filePath);
    void saveToFile(const QString& filePath) const;
    
    // Comparison and copying
    bool operator==(const EnhancedDrawingOptions& other) const;
    bool operator!=(const EnhancedDrawingOptions& other) const;
    EnhancedDrawingOptions& operator=(const EnhancedDrawingOptions& other);
    
    // Utility methods
    QString toString() const;
    QVariantMap getDifferences(const EnhancedDrawingOptions& other) const;
    void copyFrom(const EnhancedDrawingOptions& other);
    void reset();
    
    // Performance analysis
    double estimateRenderingComplexity() const;
    QStringList getPerformanceRecommendations() const;
    bool isHighPerformanceMode() const;
    bool isHighQualityMode() const;
    
    // Debug and diagnostics
    QString getDiagnosticInfo() const;
    QVariantMap getDebugInfo() const;
    void dumpConfiguration() const;

signals:
    void optionsChanged();
    void presetLoaded(const QString& presetName);
    void presetSaved(const QString& presetName);
    void validationFailed(const QStringList& errors);
    void performanceWarning(const QString& warning);

public slots:
    void onLODSettingsChanged();
    void onLightingSettingsChanged();
    void onTransparencySettingsChanged();
    void onPerformanceSettingsChanged();
    void validateAndEmitChanges();

private:
    // Rendering system integration
    LODManager* lodManager_;
    EnhancedLightingSystem* lightingSystem_;
    TransparencyManager* transparencyManager_;
    
    // Preset management
    mutable QSettings* presetSettings_;
    QString currentPreset_;
    
    // Validation
    mutable QStringList validationErrors_;
    
    // Performance tracking
    mutable double lastComplexityEstimate_;
    mutable QStringList lastPerformanceRecommendations_;
    
    // Helper methods
    void initializeDefaults();
    void setupConnections();
    void updateRenderingSystems();
    void validatePerformanceSettings();
    void validateQualitySettings();
    void validateCompatibilitySettings();
    
    // Preset helpers
    QString getPresetFilePath(const QString& presetName) const;
    QVariantMap getPresetData(const QString& presetName) const;
    void savePresetData(const QString& presetName, const QVariantMap& data) const;
    
    // Serialization helpers
    QVariantMap serializeBasicOptions() const;
    QVariantMap serializeEnhancedOptions() const;
    void deserializeBasicOptions(const QVariantMap& map);
    void deserializeEnhancedOptions(const QVariantMap& map);
    
    // Constants
    static const QString DEFAULT_PRESET_NAME;
    static const QString INGAME_PRESET_NAME;
    static const QString MINIMAP_PRESET_NAME;
    static const QString HIGH_PERFORMANCE_PRESET_NAME;
    static const QString HIGH_QUALITY_PRESET_NAME;
    static const QString DEBUG_PRESET_NAME;
};

/**
 * @brief Drawing options preset manager
 */
class DrawingOptionsPresetManager : public QObject
{
    Q_OBJECT

public:
    explicit DrawingOptionsPresetManager(QObject* parent = nullptr);
    ~DrawingOptionsPresetManager() override;

    // Preset management
    void createPreset(const QString& name, const EnhancedDrawingOptions& options);
    bool loadPreset(const QString& name, EnhancedDrawingOptions& options) const;
    bool deletePreset(const QString& name);
    QStringList getPresetNames() const;
    bool hasPreset(const QString& name) const;
    
    // Built-in presets
    void createBuiltInPresets();
    void resetBuiltInPresets();
    bool isBuiltInPreset(const QString& name) const;
    
    // Import/export
    bool exportPreset(const QString& name, const QString& filePath) const;
    bool importPreset(const QString& filePath, const QString& newName = QString());
    bool exportAllPresets(const QString& directoryPath) const;
    bool importPresetsFromDirectory(const QString& directoryPath);

signals:
    void presetCreated(const QString& name);
    void presetDeleted(const QString& name);
    void presetImported(const QString& name);
    void presetExported(const QString& name);

private:
    QSettings* presetSettings_;
    QStringList builtInPresets_;
    
    // Helper methods
    QString getPresetKey(const QString& name) const;
    bool validatePresetName(const QString& name) const;
    void initializeBuiltInPresets();
};

#endif // ENHANCEDDRAWINGOPTIONS_H
