#ifndef APPLICATIONSETTINGSMANAGER_H
#define APPLICATIONSETTINGSMANAGER_H

#include <QObject>
#include <QSettings>
#include <QMainWindow>
#include <QApplication>
#include <QTimer>
#include <QFileSystemWatcher>
#include <QMap>
#include <QVariant>
#include <QStringList>
#include <QByteArray>

// Forward declarations
class MainWindow;
class SettingsManager;
class BrushManager;
class AutomagicBorderController;
class MapView;
class DrawingOptions;

/**
 * @brief Task 97: Application Settings Manager for comprehensive settings integration
 * 
 * Complete settings management with wxwidgets compatibility:
 * - Full persistence and application across all components
 * - Window state and UI layout management
 * - Brush and tool settings persistence
 * - Automagic system configuration
 * - Client version and path management
 * - Recent files and directories tracking
 * - Hotkey configuration and management
 * - Settings import/export functionality
 * - Real-time settings application
 * - Complete wxwidgets g_settings migration
 */

/**
 * @brief Settings category for organized management
 */
enum class SettingsCategory {
    GENERAL,            // General application settings
    UI,                 // User interface settings
    EDITOR,             // Editor behavior settings
    GRAPHICS,           // Graphics and rendering settings
    AUTOMAGIC,          // Automagic system settings
    CLIENT_VERSION,     // Client version settings
    PATHS,              // File and directory paths
    LOD,                // Level of detail settings
    HOTKEYS,            // Keyboard shortcuts
    RECENT_FILES,       // Recent files and directories
    CUSTOM              // Custom application-specific settings
};

/**
 * @brief Settings change notification
 */
struct SettingsChangeNotification {
    QString key;
    QVariant oldValue;
    QVariant newValue;
    SettingsCategory category;
    QString description;
    QDateTime timestamp;
    
    SettingsChangeNotification(const QString& k = "", const QVariant& oldVal = QVariant(),
                              const QVariant& newVal = QVariant(), SettingsCategory cat = SettingsCategory::GENERAL,
                              const QString& desc = "")
        : key(k), oldValue(oldVal), newValue(newVal), category(cat), description(desc), timestamp(QDateTime::currentDateTime()) {}
};

/**
 * @brief Settings validation result
 */
struct SettingsValidationResult {
    bool isValid;
    QString errorMessage;
    QVariant suggestedValue;
    
    SettingsValidationResult(bool valid = true, const QString& error = "", const QVariant& suggested = QVariant())
        : isValid(valid), errorMessage(error), suggestedValue(suggested) {}
};

/**
 * @brief Main Application Settings Manager
 */
class ApplicationSettingsManager : public QObject
{
    Q_OBJECT

public:
    explicit ApplicationSettingsManager(QObject* parent = nullptr);
    ~ApplicationSettingsManager() override;

    // Component setup
    void setMainWindow(MainWindow* mainWindow);
    void setSettingsManager(SettingsManager* settingsManager);
    void setBrushManager(BrushManager* brushManager);
    void setAutomagicBorderController(AutomagicBorderController* automagicController);
    void setMapView(MapView* mapView);
    void setDrawingOptions(DrawingOptions* drawingOptions);

    // System control
    void enableSettingsManagement(bool enabled);
    bool isSettingsManagementEnabled() const { return settingsManagementEnabled_; }
    void enableAutoSave(bool enabled);
    bool isAutoSaveEnabled() const { return autoSaveEnabled_; }
    void setAutoSaveInterval(int seconds);
    int getAutoSaveInterval() const { return autoSaveInterval_; }

    // Application lifecycle
    void initializeSettings();
    void loadAllSettings();
    void saveAllSettings();
    void applyAllSettings();
    void resetAllToDefaults();

    // Window and UI state management
    void saveWindowState(QMainWindow* window);
    void restoreWindowState(QMainWindow* window);
    void saveDockWidgetLayout(QMainWindow* window);
    void restoreDockWidgetLayout(QMainWindow* window);
    void saveToolbarState(QMainWindow* window);
    void restoreToolbarState(QMainWindow* window);
    void savePaletteLayout();
    void restorePaletteLayout();

    // Settings validation
    SettingsValidationResult validateSetting(const QString& key, const QVariant& value) const;
    bool validateAllSettings();
    QStringList getValidationErrors() const;

    // Settings categories
    void loadCategorySettings(SettingsCategory category);
    void saveCategorySettings(SettingsCategory category);
    void applyCategorySettings(SettingsCategory category);
    void resetCategoryToDefaults(SettingsCategory category);

    // Settings monitoring
    void enableSettingsMonitoring(bool enabled);
    bool isSettingsMonitoringEnabled() const { return settingsMonitoringEnabled_; }
    QList<SettingsChangeNotification> getRecentChanges() const;
    void clearChangeHistory();

    // Settings import/export
    bool importSettings(const QString& filePath);
    bool exportSettings(const QString& filePath);
    bool importCategorySettings(const QString& filePath, SettingsCategory category);
    bool exportCategorySettings(const QString& filePath, SettingsCategory category);

    // Settings backup and restore
    void createSettingsBackup(const QString& backupName = "");
    bool restoreSettingsBackup(const QString& backupName);
    QStringList getAvailableBackups() const;
    void deleteSettingsBackup(const QString& backupName);

    // Recent files management
    void addRecentFile(const QString& filePath);
    void addRecentDirectory(const QString& directoryPath);
    QStringList getRecentFiles() const;
    QStringList getRecentDirectories() const;
    void clearRecentFiles();
    void clearRecentDirectories();
    void setMaxRecentFiles(int max);
    int getMaxRecentFiles() const;

    // Hotkey management
    void registerHotkey(const QString& action, const QString& defaultKeySequence, const QString& description = "");
    void setHotkey(const QString& action, const QString& keySequence);
    QString getHotkey(const QString& action) const;
    QMap<QString, QString> getAllHotkeys() const;
    void resetHotkeysToDefaults();
    bool isHotkeyConflict(const QString& keySequence, QString& conflictingAction) const;

    // Settings synchronization
    void enableSettingsSync(bool enabled);
    bool isSettingsSyncEnabled() const { return settingsSyncEnabled_; }
    void syncSettings();
    void forceSyncSettings();

    // Settings file management
    QString getSettingsFilePath() const;
    QString getSettingsDirectory() const;
    bool settingsFileExists() const;
    qint64 getSettingsFileSize() const;
    QDateTime getSettingsFileLastModified() const;

    // Debug and diagnostics
    void enableDebugMode(bool enabled);
    bool isDebugMode() const { return debugMode_; }
    QVariantMap getSettingsStatistics() const;
    void dumpAllSettings() const;
    void validateSettingsIntegrity();

public slots:
    void onApplicationAboutToQuit();
    void onMainWindowCloseEvent();
    void onSettingChanged(const QString& key, const QVariant& value);
    void onAutoSaveTimer();
    void onSettingsFileChanged(const QString& path);

signals:
    void settingsLoaded();
    void settingsSaved();
    void settingsApplied();
    void settingChanged(const QString& key, const QVariant& oldValue, const QVariant& newValue);
    void categorySettingsChanged(SettingsCategory category);
    void settingsValidationFailed(const QString& key, const QString& error);
    void recentFilesChanged();
    void hotkeyChanged(const QString& action, const QString& keySequence);
    void settingsBackupCreated(const QString& backupName);
    void settingsBackupRestored(const QString& backupName);
    void settingsSynced();

private:
    // Settings application implementation
    void applyGeneralSettings();
    void applyUISettings();
    void applyEditorSettings();
    void applyGraphicsSettings();
    void applyAutomagicSettings();
    void applyClientVersionSettings();
    void applyPathsSettings();
    void applyLODSettings();
    void applyHotkeySettings();

    // Settings loading implementation
    void loadGeneralSettings();
    void loadUISettings();
    void loadEditorSettings();
    void loadGraphicsSettings();
    void loadAutomagicSettings();
    void loadClientVersionSettings();
    void loadPathsSettings();
    void loadLODSettings();
    void loadHotkeySettings();

    // Settings saving implementation
    void saveGeneralSettings();
    void saveUISettings();
    void saveEditorSettings();
    void saveGraphicsSettings();
    void saveAutomagicSettings();
    void saveClientVersionSettings();
    void savePathsSettings();
    void saveLODSettings();
    void saveHotkeySettings();

    // Validation implementation
    bool validateGeneralSetting(const QString& key, const QVariant& value, QString& error) const;
    bool validateUISetting(const QString& key, const QVariant& value, QString& error) const;
    bool validateEditorSetting(const QString& key, const QVariant& value, QString& error) const;
    bool validateGraphicsSetting(const QString& key, const QVariant& value, QString& error) const;

    // Utility methods
    SettingsCategory getSettingCategory(const QString& key) const;
    QString getCategoryName(SettingsCategory category) const;
    QStringList getCategoryKeys(SettingsCategory category) const;
    void notifySettingChange(const QString& key, const QVariant& oldValue, const QVariant& newValue);
    void addToChangeHistory(const SettingsChangeNotification& notification);

    // File operations
    bool copySettingsFile(const QString& sourcePath, const QString& destinationPath);
    bool mergeSettingsFiles(const QString& basePath, const QString& overlayPath, const QString& outputPath);

private:
    // Core components
    MainWindow* mainWindow_;
    SettingsManager* settingsManager_;
    BrushManager* brushManager_;
    AutomagicBorderController* automagicController_;
    MapView* mapView_;
    DrawingOptions* drawingOptions_;

    // System state
    bool settingsManagementEnabled_;
    bool autoSaveEnabled_;
    int autoSaveInterval_;
    bool settingsMonitoringEnabled_;
    bool settingsSyncEnabled_;
    bool debugMode_;

    // Auto-save timer
    QTimer* autoSaveTimer_;

    // Settings file monitoring
    QFileSystemWatcher* settingsWatcher_;

    // Change tracking
    QList<SettingsChangeNotification> changeHistory_;
    int maxChangeHistorySize_;

    // Recent files
    QStringList recentFiles_;
    QStringList recentDirectories_;
    int maxRecentFiles_;

    // Hotkeys
    QMap<QString, QString> defaultHotkeys_;
    QMap<QString, QString> hotkeyDescriptions_;

    // Validation errors
    mutable QStringList validationErrors_;

    // Constants
    static const int DEFAULT_AUTO_SAVE_INTERVAL = 30; // seconds
    static const int DEFAULT_MAX_RECENT_FILES = 10;
    static const int DEFAULT_MAX_CHANGE_HISTORY = 100;
};

#endif // APPLICATIONSETTINGSMANAGER_H
