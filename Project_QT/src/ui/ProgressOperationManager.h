#ifndef PROGRESSOPERATIONMANAGER_H
#define PROGRESSOPERATIONMANAGER_H

#include <QObject>
#include <QString>
#include <QVariantMap>
#include <QList>
#include <QHash>
#include <QTimer>
#include <QMutex>
#include <QThread>
#include <functional>

#include "ProgressDialog.h"
#include "threading/ProgressWorker.h"

/**
 * @brief Task 88: Progress Operation Manager for coordinating dialogs with workers
 * 
 * Complete operation management system:
 * - Coordinates ProgressDialog with ProgressWorker
 * - Manages long-running operations with progress indication
 * - Provides high-level API for common operations
 * - Handles threading, cancellation, and error recovery
 * - Replaces wxProgressDialog usage throughout application
 * - Consistent progress indication across all operations
 */

/**
 * @brief Progress Operation - represents a single long-running operation
 */
class ProgressOperation : public QObject
{
    Q_OBJECT

public:
    enum class Type {
        MapLoading,
        MapSaving,
        MapSearch,
        MapReplace,
        MapValidation,
        SpriteLoading,
        ItemTransformation,
        Custom
    };

    enum class State {
        Pending,
        Running,
        Paused,
        Finished,
        Cancelled,
        Error
    };

    explicit ProgressOperation(Type type, const QString& title, QObject* parent = nullptr);
    ~ProgressOperation() override;

    // Basic properties
    Type getType() const { return type_; }
    State getState() const { return state_; }
    QString getTitle() const { return title_; }
    QString getDescription() const { return description_; }
    
    void setDescription(const QString& description);
    void setEstimatedDuration(int seconds);
    int getEstimatedDuration() const;
    
    // Progress information
    int getCurrentProgress() const;
    int getMaximumProgress() const;
    QString getCurrentOperation() const;
    qint64 getElapsedTime() const;
    qint64 getEstimatedRemainingTime() const;
    
    // Control
    void start();
    void cancel();
    void pause();
    void resume();
    
    // Worker and dialog
    void setWorker(ProgressWorker* worker);
    ProgressWorker* getWorker() const { return worker_; }
    
    void setProgressDialog(ProgressDialog* dialog);
    ProgressDialog* getProgressDialog() const { return progressDialog_; }
    
    // Configuration
    void setShowDialog(bool show);
    bool isShowDialog() const { return showDialog_; }
    
    void setAutoClose(bool autoClose);
    bool isAutoClose() const { return autoClose_; }
    
    void setCancellable(bool cancellable);
    bool isCancellable() const { return cancellable_; }

signals:
    void started();
    void progressUpdated(int current, int maximum, const QString& operation);
    void finished(bool success, const QString& result);
    void cancelled();
    void error(const QString& error, const QString& details);
    void stateChanged(State newState);

private slots:
    void onWorkerProgressUpdated(int current, int maximum, const QString& operation, const QString& detail);
    void onWorkerFinished(bool success, const QString& result);
    void onWorkerError(const QString& error, const QString& details);
    void onDialogCancelled();

private:
    void setState(State newState);
    void connectWorkerSignals();
    void connectDialogSignals();
    void updateDialog();

private:
    Type type_;
    State state_;
    QString title_;
    QString description_;
    int estimatedDuration_;
    bool showDialog_;
    bool autoClose_;
    bool cancellable_;
    
    ProgressWorker* worker_;
    ProgressDialog* progressDialog_;
    QThread* workerThread_;
};

/**
 * @brief Progress Operation Manager - high-level API for progress operations
 */
class ProgressOperationManager : public QObject
{
    Q_OBJECT

public:
    static ProgressOperationManager* instance();
    
    // High-level operation methods
    ProgressOperation* loadMap(const QString& filePath, const QVariantMap& options = QVariantMap(), QWidget* parent = nullptr);
    ProgressOperation* saveMap(class Map* map, const QString& filePath, const QVariantMap& options = QVariantMap(), QWidget* parent = nullptr);
    ProgressOperation* searchMap(class Map* map, const QVariantMap& criteria, QWidget* parent = nullptr);
    ProgressOperation* replaceItems(class Map* map, const QVariantMap& operation, QWidget* parent = nullptr);
    ProgressOperation* validateMap(class Map* map, const QVariantMap& options = QVariantMap(), QWidget* parent = nullptr);
    ProgressOperation* loadSprites(const QStringList& spritePaths, const QVariantMap& options = QVariantMap(), QWidget* parent = nullptr);
    
    // Custom operation
    ProgressOperation* createCustomOperation(const QString& title, ProgressWorker* worker, QWidget* parent = nullptr);
    
    // Operation management
    void cancelOperation(ProgressOperation* operation);
    void cancelAllOperations();
    
    QList<ProgressOperation*> getActiveOperations() const;
    int getActiveOperationCount() const;
    bool hasActiveOperations() const;
    
    // Global settings
    void setDefaultShowDialog(bool show);
    bool getDefaultShowDialog() const;
    
    void setDefaultAutoClose(bool autoClose);
    bool getDefaultAutoClose() const;
    
    void setDefaultCancellable(bool cancellable);
    bool getDefaultCancellable() const;
    
    // Convenience methods with simplified API
    void showMapLoadingProgress(const QString& filePath, std::function<void(class Map*)> onLoaded, 
                               std::function<void(const QString&)> onError = nullptr, QWidget* parent = nullptr);
    void showMapSavingProgress(class Map* map, const QString& filePath, std::function<void()> onSaved,
                              std::function<void(const QString&)> onError = nullptr, QWidget* parent = nullptr);
    void showSearchProgress(class Map* map, const QVariantMap& criteria, std::function<void(const QList<QVariant>&)> onResults,
                           std::function<void(const QString&)> onError = nullptr, QWidget* parent = nullptr);

signals:
    void operationStarted(ProgressOperation* operation);
    void operationFinished(ProgressOperation* operation);
    void operationCancelled(ProgressOperation* operation);
    void activeOperationCountChanged(int count);

private slots:
    void onOperationFinished();
    void onOperationCancelled();

private:
    explicit ProgressOperationManager(QObject* parent = nullptr);
    ~ProgressOperationManager() override;

    ProgressOperation* createOperation(ProgressOperation::Type type, const QString& title, 
                                     ProgressWorker* worker, QWidget* parent);
    void registerOperation(ProgressOperation* operation);
    void unregisterOperation(ProgressOperation* operation);

private:
    static ProgressOperationManager* instance_;
    
    QList<ProgressOperation*> activeOperations_;
    
    // Default settings
    bool defaultShowDialog_;
    bool defaultAutoClose_;
    bool defaultCancellable_;
    
    mutable QMutex mutex_;
};

/**
 * @brief Scoped Progress Operation for RAII-style usage
 */
class ScopedProgressOperation
{
public:
    explicit ScopedProgressOperation(ProgressOperation::Type type, const QString& title, 
                                   ProgressWorker* worker, QWidget* parent = nullptr);
    ~ScopedProgressOperation();

    // Operation access
    ProgressOperation* operation() const { return operation_; }
    ProgressOperation* operator->() const { return operation_; }
    ProgressOperation& operator*() const { return *operation_; }

    // Convenience methods
    void start() { operation_->start(); }
    void cancel() { operation_->cancel(); }
    bool isFinished() const { return operation_->getState() == ProgressOperation::State::Finished; }
    bool wasCancelled() const { return operation_->getState() == ProgressOperation::State::Cancelled; }

private:
    ProgressOperation* operation_;
};

/**
 * @brief Progress Operation Factory for creating common operations
 */
class ProgressOperationFactory
{
public:
    // Factory methods for common operations
    static ProgressOperation* createMapLoadingOperation(const QString& filePath, const QVariantMap& options = QVariantMap(), QWidget* parent = nullptr);
    static ProgressOperation* createMapSavingOperation(class Map* map, const QString& filePath, const QVariantMap& options = QVariantMap(), QWidget* parent = nullptr);
    static ProgressOperation* createMapSearchOperation(class Map* map, const QVariantMap& criteria, QWidget* parent = nullptr);
    static ProgressOperation* createMapReplaceOperation(class Map* map, const QVariantMap& operation, QWidget* parent = nullptr);
    static ProgressOperation* createMapValidationOperation(class Map* map, const QVariantMap& options = QVariantMap(), QWidget* parent = nullptr);
    static ProgressOperation* createSpriteLoadingOperation(const QStringList& spritePaths, const QVariantMap& options = QVariantMap(), QWidget* parent = nullptr);
    
    // Configuration presets
    static void configureForMapLoading(ProgressOperation* operation);
    static void configureForMapSaving(ProgressOperation* operation);
    static void configureForSearch(ProgressOperation* operation);
    static void configureForReplace(ProgressOperation* operation);
    static void configureForValidation(ProgressOperation* operation);

private:
    ProgressOperationFactory() = default; // Static factory class
};

/**
 * @brief Legacy compatibility layer for wxProgressDialog replacement
 */
class LegacyProgressAdapter
{
public:
    // wxProgressDialog-compatible API
    static void createLoadBar(const QString& message, bool canCancel = false, QWidget* parent = nullptr);
    static bool setLoadDone(int percentage, const QString& newMessage = QString());
    static void setLoadScale(int from, int to);
    static void destroyLoadBar();
    
    // Current operation access
    static ProgressDialog* getCurrentProgressDialog();
    static ProgressOperation* getCurrentOperation();

private:
    static ProgressDialog* currentDialog_;
    static ProgressOperation* currentOperation_;
    static int scaleFrom_;
    static int scaleTo_;
};

#endif // PROGRESSOPERATIONMANAGER_H
