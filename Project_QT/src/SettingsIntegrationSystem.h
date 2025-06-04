#ifndef SETTINGSINTEGRATIONSYSTEM_H
#define SETTINGSINTEGRATIONSYSTEM_H

#include <QObject>
#include <QSettings>
#include <QVariant>
#include <QMap>
#include <QSet>
#include <QTimer>
#include <QMutex>
#include <QThread>
#include <functional>

// Forward declarations
class MainWindow;
class ApplicationSettingsManager;
class SettingsManager;

/**
 * @brief Task 97: Settings Integration System for comprehensive settings coordination
 * 
 * Advanced settings integration with wxwidgets compatibility:
 * - Real-time settings synchronization across components
 * - Settings dependency management and validation
 * - Automatic settings application and propagation
 * - Settings conflict resolution
 * - Performance-optimized settings updates
 * - Thread-safe settings access
 * - Settings change batching and optimization
 * - Complete component integration
 */

/**
 * @brief Settings dependency relationship
 */
enum class SettingsDependencyType {
    REQUIRES,           // Setting A requires setting B to be enabled
    CONFLICTS,          // Setting A conflicts with setting B
    IMPLIES,            // Setting A being true implies setting B should be true
    EXCLUDES,           // Setting A excludes setting B
    VALIDATES,          // Setting A validates the value of setting B
    TRIGGERS,           // Setting A triggers an update of setting B
    SYNCHRONIZES        // Setting A synchronizes with setting B
};

/**
 * @brief Settings dependency configuration
 */
struct SettingsDependency {
    QString sourceKey;
    QString targetKey;
    SettingsDependencyType type;
    QVariant condition;
    QString description;
    std::function<bool(const QVariant&, const QVariant&)> validator;
    
    SettingsDependency(const QString& source = "", const QString& target = "",
                      SettingsDependencyType t = SettingsDependencyType::REQUIRES,
                      const QVariant& cond = QVariant(), const QString& desc = "")
        : sourceKey(source), targetKey(target), type(t), condition(cond), description(desc) {}
};

/**
 * @brief Settings update batch for performance optimization
 */
struct SettingsUpdateBatch {
    QMap<QString, QVariant> changes;
    QSet<QString> affectedComponents;
    QDateTime timestamp;
    QString description;
    bool isAtomic;
    
    SettingsUpdateBatch(const QString& desc = "", bool atomic = false)
        : timestamp(QDateTime::currentDateTime()), description(desc), isAtomic(atomic) {}
};

/**
 * @brief Component settings interface
 */
class ISettingsComponent
{
public:
    virtual ~ISettingsComponent() = default;
    virtual void applySettings(const QMap<QString, QVariant>& settings) = 0;
    virtual QMap<QString, QVariant> getSettings() const = 0;
    virtual QStringList getSettingsKeys() const = 0;
    virtual bool validateSetting(const QString& key, const QVariant& value) const = 0;
    virtual QString getComponentName() const = 0;
};

/**
 * @brief Settings update worker for background processing
 */
class SettingsUpdateWorker : public QObject
{
    Q_OBJECT

public:
    explicit SettingsUpdateWorker(QObject* parent = nullptr);
    ~SettingsUpdateWorker() override = default;

public slots:
    void processUpdateBatch(const SettingsUpdateBatch& batch);
    void validateSettings(const QMap<QString, QVariant>& settings);

signals:
    void batchProcessed(const SettingsUpdateBatch& batch, bool success);
    void settingsValidated(const QMap<QString, QVariant>& settings, bool isValid, const QStringList& errors);
    void componentUpdated(const QString& componentName, bool success);

private:
    void applySettingsToComponent(const QString& componentName, const QMap<QString, QVariant>& settings);
    bool validateComponentSettings(const QString& componentName, const QMap<QString, QVariant>& settings, QStringList& errors);
};

/**
 * @brief Main Settings Integration System
 */
class SettingsIntegrationSystem : public QObject
{
    Q_OBJECT

public:
    explicit SettingsIntegrationSystem(QObject* parent = nullptr);
    ~SettingsIntegrationSystem() override;

    // Component setup
    void setMainWindow(MainWindow* mainWindow);
    void setApplicationSettingsManager(ApplicationSettingsManager* appSettingsManager);
    void setSettingsManager(SettingsManager* settingsManager);

    // System control
    void enableIntegration(bool enabled);
    bool isIntegrationEnabled() const { return integrationEnabled_; }
    void enableBackgroundProcessing(bool enabled);
    bool isBackgroundProcessingEnabled() const { return backgroundProcessingEnabled_; }
    void enableBatchUpdates(bool enabled);
    bool isBatchUpdatesEnabled() const { return batchUpdatesEnabled_; }

    // Component registration
    void registerComponent(const QString& name, ISettingsComponent* component);
    void unregisterComponent(const QString& name);
    bool isComponentRegistered(const QString& name) const;
    QStringList getRegisteredComponents() const;

    // Settings dependency management
    void addDependency(const SettingsDependency& dependency);
    void removeDependency(const QString& sourceKey, const QString& targetKey);
    void clearDependencies();
    QList<SettingsDependency> getDependencies(const QString& key) const;
    bool hasDependencies(const QString& key) const;

    // Settings synchronization
    void synchronizeSettings();
    void synchronizeComponent(const QString& componentName);
    void synchronizeSetting(const QString& key);
    void forceSynchronization();

    // Batch operations
    void beginBatch(const QString& description = "");
    void endBatch();
    void cancelBatch();
    bool isBatchActive() const { return batchActive_; }
    void addToBatch(const QString& key, const QVariant& value);

    // Settings validation
    bool validateAllSettings();
    bool validateComponentSettings(const QString& componentName);
    bool validateSetting(const QString& key, const QVariant& value);
    QStringList getValidationErrors() const;

    // Settings application
    void applyAllSettings();
    void applyComponentSettings(const QString& componentName);
    void applySetting(const QString& key, const QVariant& value);
    void applySettingsBatch(const SettingsUpdateBatch& batch);

    // Dependency resolution
    bool resolveDependencies(const QString& key, const QVariant& value);
    QMap<QString, QVariant> calculateDependentChanges(const QString& key, const QVariant& value);
    bool checkDependencyConflicts(const QString& key, const QVariant& value);

    // Performance optimization
    void enableUpdateThrottling(bool enabled);
    bool isUpdateThrottlingEnabled() const { return updateThrottlingEnabled_; }
    void setUpdateThrottleInterval(int milliseconds);
    int getUpdateThrottleInterval() const { return updateThrottleInterval_; }
    void flushPendingUpdates();

    // Thread safety
    void enableThreadSafeAccess(bool enabled);
    bool isThreadSafeAccessEnabled() const { return threadSafeAccess_; }
    QVariant getSettingThreadSafe(const QString& key) const;
    void setSettingThreadSafe(const QString& key, const QVariant& value);

    // Statistics and monitoring
    QVariantMap getStatistics() const;
    void resetStatistics();
    int getUpdateCount() const { return updateCount_; }
    int getValidationCount() const { return validationCount_; }
    qint64 getAverageUpdateTime() const;

public slots:
    void onSettingChanged(const QString& key, const QVariant& value);
    void onComponentSettingsChanged(const QString& componentName);
    void onBatchProcessed(const SettingsUpdateBatch& batch, bool success);
    void onUpdateThrottleTimer();
    void onValidationTimer();

signals:
    void settingsSynchronized();
    void componentSynchronized(const QString& componentName);
    void settingApplied(const QString& key, const QVariant& value);
    void batchApplied(const SettingsUpdateBatch& batch);
    void dependencyResolved(const QString& sourceKey, const QString& targetKey);
    void dependencyConflict(const QString& sourceKey, const QString& targetKey, const QString& reason);
    void validationFailed(const QString& key, const QString& error);
    void integrationError(const QString& error);

private:
    // Integration implementation
    void initializeIntegration();
    void shutdownIntegration();
    void setupWorkerThread();
    void cleanupWorkerThread();

    // Dependency processing
    void processDependencies(const QString& key, const QVariant& value);
    bool evaluateDependency(const SettingsDependency& dependency, const QVariant& sourceValue, const QVariant& targetValue);
    void applyDependencyAction(const SettingsDependency& dependency, const QVariant& sourceValue);

    // Batch processing
    void processBatch();
    void optimizeBatch(SettingsUpdateBatch& batch);
    void validateBatch(const SettingsUpdateBatch& batch);

    // Update optimization
    void addPendingUpdate(const QString& key, const QVariant& value);
    void processPendingUpdates();
    bool shouldThrottleUpdate(const QString& key) const;

    // Component management
    ISettingsComponent* getComponent(const QString& name) const;
    void notifyComponent(const QString& name, const QString& key, const QVariant& value);
    void updateComponentSettings(const QString& name, const QMap<QString, QVariant>& settings);

    // Validation implementation
    bool validateDependencies(const QString& key, const QVariant& value, QStringList& errors);
    bool validateComponentIntegrity(const QString& componentName, QStringList& errors);

    // Thread safety implementation
    void lockSettings() const;
    void unlockSettings() const;

    // Statistics tracking
    void updateStatistics(const QString& operation, qint64 duration);

private:
    // Core components
    MainWindow* mainWindow_;
    ApplicationSettingsManager* appSettingsManager_;
    SettingsManager* settingsManager_;

    // System state
    bool integrationEnabled_;
    bool backgroundProcessingEnabled_;
    bool batchUpdatesEnabled_;
    bool updateThrottlingEnabled_;
    bool threadSafeAccess_;

    // Component registry
    QMap<QString, ISettingsComponent*> registeredComponents_;

    // Dependency management
    QList<SettingsDependency> dependencies_;
    QMap<QString, QList<SettingsDependency>> dependencyIndex_;

    // Batch processing
    bool batchActive_;
    SettingsUpdateBatch currentBatch_;

    // Background processing
    QThread* workerThread_;
    SettingsUpdateWorker* worker_;

    // Update throttling
    QTimer* updateThrottleTimer_;
    int updateThrottleInterval_;
    QMap<QString, QVariant> pendingUpdates_;

    // Validation
    QTimer* validationTimer_;
    QStringList validationErrors_;

    // Thread safety
    mutable QMutex settingsMutex_;

    // Statistics
    int updateCount_;
    int validationCount_;
    qint64 totalUpdateTime_;
    QMap<QString, int> componentUpdateCounts_;

    // Constants
    static const int DEFAULT_THROTTLE_INTERVAL = 100; // milliseconds
    static const int DEFAULT_VALIDATION_INTERVAL = 1000; // milliseconds
};

#endif // SETTINGSINTEGRATIONSYSTEM_H
