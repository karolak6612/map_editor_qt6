#ifndef PROGRESSWORKER_H
#define PROGRESSWORKER_H

#include <QObject>
#include <QThread>
#include <QMutex>
#include <QMutexLocker>
#include <QWaitCondition>
#include <QTimer>
#include <QElapsedTimer>
#include <QVariant>
#include <QVariantMap>
#include <functional>

/**
 * @brief Task 88: Progress Worker system for threaded long operations
 * 
 * Complete worker thread implementation:
 * - Base worker class for long-running operations
 * - Progress reporting and cancellation support
 * - Thread-safe communication with UI
 * - Specialized workers for common operations
 * - Integration with ProgressDialog system
 * - Error handling and recovery
 */

/**
 * @brief Base class for progress-reporting worker operations
 */
class ProgressWorker : public QObject
{
    Q_OBJECT

public:
    explicit ProgressWorker(QObject* parent = nullptr);
    ~ProgressWorker() override;

    // Worker control
    virtual void start();
    virtual void cancel();
    virtual void pause();
    virtual void resume();
    
    // State management
    bool isRunning() const;
    bool isCancelled() const;
    bool isPaused() const;
    bool isFinished() const;
    
    // Progress information
    int getCurrentProgress() const;
    int getMaximumProgress() const;
    QString getCurrentOperation() const;
    QString getCurrentDetail() const;
    
    // Statistics
    qint64 getElapsedTime() const;
    qint64 getEstimatedRemainingTime() const;
    double getProgressRate() const;
    
    // Configuration
    void setProgressReportingInterval(int milliseconds);
    int getProgressReportingInterval() const;
    
    void setAutoFinish(bool autoFinish);
    bool isAutoFinish() const;

public slots:
    virtual void process() = 0; // Pure virtual - implement in derived classes

signals:
    void started();
    void progressUpdated(int current, int maximum, const QString& operation, const QString& detail = QString());
    void operationChanged(const QString& operation);
    void detailChanged(const QString& detail);
    void finished(bool success, const QString& result = QString());
    void cancelled();
    void paused();
    void resumed();
    void error(const QString& error, const QString& details = QString());

protected:
    // Progress reporting (thread-safe)
    void reportProgress(int current, int maximum = -1);
    void reportProgress(int current, const QString& operation, const QString& detail = QString());
    void reportOperation(const QString& operation);
    void reportDetail(const QString& detail);
    void reportError(const QString& error, const QString& details = QString());
    void reportFinished(bool success = true, const QString& result = QString());
    
    // Cancellation checking
    bool shouldCancel() const;
    void checkCancellation(); // Throws if cancelled
    
    // Pause handling
    void checkPause();
    void waitIfPaused();
    
    // Utility methods
    void sleep(int milliseconds);
    void processEvents();

private slots:
    void onProgressTimer();

private:
    void startProgressTimer();
    void stopProgressTimer();
    void updateStatistics();

private:
    // State
    mutable QMutex mutex_;
    bool running_;
    bool cancelled_;
    bool paused_;
    bool finished_;
    bool autoFinish_;
    
    // Progress data
    int currentProgress_;
    int maximumProgress_;
    QString currentOperation_;
    QString currentDetail_;
    
    // Timing
    QElapsedTimer elapsedTimer_;
    QTimer* progressTimer_;
    int progressReportingInterval_;
    qint64 lastProgressTime_;
    int lastProgressValue_;
    double progressRate_;
    
    // Pause handling
    QWaitCondition pauseCondition_;
};

/**
 * @brief Map Loading Worker
 */
class MapLoadingWorker : public ProgressWorker
{
    Q_OBJECT

public:
    explicit MapLoadingWorker(const QString& filePath, QObject* parent = nullptr);
    ~MapLoadingWorker() override = default;

    void setFilePath(const QString& filePath);
    QString getFilePath() const;
    
    void setLoadOptions(const QVariantMap& options);
    QVariantMap getLoadOptions() const;

public slots:
    void process() override;

signals:
    void mapLoaded(class Map* map);
    void loadingStageChanged(const QString& stage);

private:
    QString filePath_;
    QVariantMap loadOptions_;
};

/**
 * @brief Map Saving Worker
 */
class MapSavingWorker : public ProgressWorker
{
    Q_OBJECT

public:
    explicit MapSavingWorker(class Map* map, const QString& filePath, QObject* parent = nullptr);
    ~MapSavingWorker() override = default;

    void setMap(class Map* map);
    class Map* getMap() const;
    
    void setFilePath(const QString& filePath);
    QString getFilePath() const;
    
    void setSaveOptions(const QVariantMap& options);
    QVariantMap getSaveOptions() const;

public slots:
    void process() override;

signals:
    void mapSaved(const QString& filePath);
    void savingStageChanged(const QString& stage);

private:
    class Map* map_;
    QString filePath_;
    QVariantMap saveOptions_;
};

/**
 * @brief Map Search Worker
 */
class MapSearchWorker : public ProgressWorker
{
    Q_OBJECT

public:
    explicit MapSearchWorker(class Map* map, QObject* parent = nullptr);
    ~MapSearchWorker() override = default;

    void setMap(class Map* map);
    class Map* getMap() const;
    
    void setSearchCriteria(const QVariantMap& criteria);
    QVariantMap getSearchCriteria() const;
    
    void setSearchArea(const QRect& area);
    QRect getSearchArea() const;
    
    void setMaxResults(int maxResults);
    int getMaxResults() const;

public slots:
    void process() override;

signals:
    void searchResultFound(const QVariant& result);
    void searchCompleted(const QList<QVariant>& results);

private:
    class Map* map_;
    QVariantMap searchCriteria_;
    QRect searchArea_;
    int maxResults_;
    QList<QVariant> results_;
};

/**
 * @brief Map Replace Worker
 */
class MapReplaceWorker : public ProgressWorker
{
    Q_OBJECT

public:
    explicit MapReplaceWorker(class Map* map, QObject* parent = nullptr);
    ~MapReplaceWorker() override = default;

    void setMap(class Map* map);
    class Map* getMap() const;
    
    void setReplaceOperation(const QVariantMap& operation);
    QVariantMap getReplaceOperation() const;
    
    void setTargetPositions(const QList<QPoint>& positions);
    QList<QPoint> getTargetPositions() const;

public slots:
    void process() override;

signals:
    void itemReplaced(const QPoint& position, const QVariant& oldItem, const QVariant& newItem);
    void replaceCompleted(int replacedCount);

private:
    class Map* map_;
    QVariantMap replaceOperation_;
    QList<QPoint> targetPositions_;
    int replacedCount_;
};

/**
 * @brief Map Validation Worker
 */
class MapValidationWorker : public ProgressWorker
{
    Q_OBJECT

public:
    explicit MapValidationWorker(class Map* map, QObject* parent = nullptr);
    ~MapValidationWorker() override = default;

    void setMap(class Map* map);
    class Map* getMap() const;
    
    void setValidationOptions(const QVariantMap& options);
    QVariantMap getValidationOptions() const;

public slots:
    void process() override;

signals:
    void validationIssueFound(const QString& issue, const QPoint& position);
    void validationCompleted(const QStringList& issues);

private:
    class Map* map_;
    QVariantMap validationOptions_;
    QStringList issues_;
};

/**
 * @brief Sprite Loading Worker
 */
class SpriteLoadingWorker : public ProgressWorker
{
    Q_OBJECT

public:
    explicit SpriteLoadingWorker(const QStringList& spritePaths, QObject* parent = nullptr);
    ~SpriteLoadingWorker() override = default;

    void setSpritePaths(const QStringList& paths);
    QStringList getSpritePaths() const;
    
    void setLoadingOptions(const QVariantMap& options);
    QVariantMap getLoadingOptions() const;

public slots:
    void process() override;

signals:
    void spriteLoaded(const QString& path, const QPixmap& sprite);
    void spritesLoaded(const QStringList& loadedPaths);

private:
    QStringList spritePaths_;
    QVariantMap loadingOptions_;
    QStringList loadedPaths_;
};

/**
 * @brief Worker Thread Manager
 */
class WorkerThreadManager : public QObject
{
    Q_OBJECT

public:
    static WorkerThreadManager* instance();
    
    // Thread management
    QThread* createWorkerThread(const QString& name = QString());
    void destroyWorkerThread(QThread* thread);
    
    // Worker execution
    void executeWorker(ProgressWorker* worker, QThread* thread = nullptr);
    void executeWorkerWithProgress(ProgressWorker* worker, class ProgressDialog* progressDialog, QThread* thread = nullptr);
    
    // Active workers
    QList<ProgressWorker*> getActiveWorkers() const;
    int getActiveWorkerCount() const;
    bool hasActiveWorkers() const;
    
    // Thread pool management
    void setMaxThreads(int maxThreads);
    int getMaxThreads() const;
    int getAvailableThreads() const;
    
    // Cleanup
    void cancelAllWorkers();
    void waitForAllWorkers(int timeout = 30000);

signals:
    void workerStarted(ProgressWorker* worker);
    void workerFinished(ProgressWorker* worker);
    void activeWorkerCountChanged(int count);

private slots:
    void onWorkerFinished();
    void onThreadFinished();

private:
    explicit WorkerThreadManager(QObject* parent = nullptr);
    ~WorkerThreadManager() override;

    static WorkerThreadManager* instance_;
    
    QList<QThread*> workerThreads_;
    QList<ProgressWorker*> activeWorkers_;
    int maxThreads_;
    
    mutable QMutex mutex_;
};

#endif // PROGRESSWORKER_H
