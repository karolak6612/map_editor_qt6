#include "ProgressIntegration.h"
#include "../Map.h"
#include "../MapSearcher.h"
#include "../ui/ProgressDialog.h"
#include "../ui/ProgressOperationManager.h"
#include "../threading/ProgressWorker.h"
#include "../io/MapFormatManager.h"
#include "../io/OTBMMapLoader.h"
#include <QApplication>
#include <QThread>
#include <QTimer>
#include <QMutexLocker>
#include <QDebug>
#include <QFileInfo>
#include <QDir>
#include <QVariant>
#include <QRect>
#include <QPoint>
#include <functional>

// MapOperationsProgress Implementation
MapOperationsProgress::MapOperationsProgress(QObject* parent)
    : QObject(parent)
{
}

ProgressOperation* MapOperationsProgress::loadMapWithProgress(const QString& filePath, QWidget* parent) {
    // Create map loading worker
    MapLoadingWorker* worker = new MapLoadingWorker();
    worker->setFilePath(filePath);
    
    // Create progress operation
    ProgressOperation* operation = ProgressOperationManager::instance()->createCustomOperation(
        QString("Loading Map: %1").arg(QFileInfo(filePath).fileName()), worker, parent);
    
    // Configure for map loading
    ProgressOperationFactory::configureForMapLoading(operation);
    
    return operation;
}

void MapOperationsProgress::loadMapWithProgress(const QString& filePath, 
                                               std::function<void(Map*)> onLoaded,
                                               std::function<void(const QString&)> onError,
                                               QWidget* parent) {
    ProgressOperation* operation = loadMapWithProgress(filePath, parent);
    
    // Connect completion callbacks
    QObject::connect(operation, &ProgressOperation::finished, 
                    [onLoaded, onError](bool success, const QString& result) {
        if (success && onLoaded) {
            // Extract Map* from result (implementation depends on worker)
            Map* map = nullptr; // Worker should provide map instance
            onLoaded(map);
        } else if (!success && onError) {
            onError(result);
        }
    });
    
    operation->start();
}

ProgressOperation* MapOperationsProgress::saveMapWithProgress(Map* map, const QString& filePath, QWidget* parent) {
    if (!map) {
        qWarning() << "MapOperationsProgress::saveMapWithProgress: null map provided";
        return nullptr;
    }
    
    // Create map saving worker
    MapSavingWorker* worker = new MapSavingWorker();
    worker->setMap(map);
    worker->setFilePath(filePath);
    
    // Create progress operation
    ProgressOperation* operation = ProgressOperationManager::instance()->createCustomOperation(
        QString("Saving Map: %1").arg(QFileInfo(filePath).fileName()), worker, parent);
    
    // Configure for map saving
    ProgressOperationFactory::configureForMapSaving(operation);
    
    return operation;
}

void MapOperationsProgress::saveMapWithProgress(Map* map, const QString& filePath,
                                               std::function<void()> onSaved,
                                               std::function<void(const QString&)> onError,
                                               QWidget* parent) {
    ProgressOperation* operation = saveMapWithProgress(map, filePath, parent);
    
    if (!operation) {
        if (onError) onError("Failed to create save operation");
        return;
    }
    
    // Connect completion callbacks
    QObject::connect(operation, &ProgressOperation::finished, 
                    [onSaved, onError](bool success, const QString& result) {
        if (success && onSaved) {
            onSaved();
        } else if (!success && onError) {
            onError(result);
        }
    });
    
    operation->start();
}

ProgressOperation* MapOperationsProgress::validateMapWithProgress(Map* map, const QVariantMap& options, QWidget* parent) {
    if (!map) {
        qWarning() << "MapOperationsProgress::validateMapWithProgress: null map provided";
        return nullptr;
    }
    
    // Create map validation worker
    MapValidationWorker* worker = new MapValidationWorker();
    worker->setMap(map);
    worker->setValidationOptions(options);
    
    // Create progress operation
    ProgressOperation* operation = ProgressOperationManager::instance()->createCustomOperation(
        "Validating Map", worker, parent);
    
    // Configure for validation
    ProgressOperationFactory::configureForValidation(operation);
    
    return operation;
}

void MapOperationsProgress::validateMapWithProgress(Map* map, const QVariantMap& options,
                                                   std::function<void(const QStringList&)> onCompleted,
                                                   std::function<void(const QString&)> onError,
                                                   QWidget* parent) {
    ProgressOperation* operation = validateMapWithProgress(map, options, parent);
    
    if (!operation) {
        if (onError) onError("Failed to create validation operation");
        return;
    }
    
    // Connect completion callbacks
    QObject::connect(operation, &ProgressOperation::finished, 
                    [onCompleted, onError](bool success, const QString& result) {
        if (success && onCompleted) {
            // Parse validation results from result string
            QStringList validationResults = result.split('\n', Qt::SkipEmptyParts);
            onCompleted(validationResults);
        } else if (!success && onError) {
            onError(result);
        }
    });
    
    operation->start();
}

// SearchOperationsProgress Implementation
SearchOperationsProgress::SearchOperationsProgress(QObject* parent)
    : QObject(parent)
{
}

ProgressOperation* SearchOperationsProgress::searchWithProgress(Map* map, const QVariantMap& criteria, QWidget* parent) {
    if (!map) {
        qWarning() << "SearchOperationsProgress::searchWithProgress: null map provided";
        return nullptr;
    }
    
    // Create map search worker
    MapSearchWorker* worker = new MapSearchWorker(map);
    worker->setSearchCriteria(criteria);
    
    // Set search area if specified
    if (criteria.contains("searchArea")) {
        QRect area = criteria.value("searchArea").toRect();
        worker->setSearchArea(area);
    }
    
    // Set max results if specified
    if (criteria.contains("maxResults")) {
        int maxResults = criteria.value("maxResults").toInt();
        worker->setMaxResults(maxResults);
    }
    
    // Create progress operation
    ProgressOperation* operation = ProgressOperationManager::instance()->createCustomOperation(
        "Searching Map", worker, parent);
    
    // Configure for search
    ProgressOperationFactory::configureForSearch(operation);
    
    return operation;
}

ProgressOperation* SearchOperationsProgress::findAllItemsWithProgress(Map* map, quint16 itemId, const QRect& area, QWidget* parent) {
    QVariantMap criteria;
    criteria["itemId"] = itemId;
    if (area.isValid()) {
        criteria["searchArea"] = area;
    }
    
    return searchWithProgress(map, criteria, parent);
}

ProgressOperation* SearchOperationsProgress::findItemsByTypeWithProgress(Map* map, const QString& itemType, const QRect& area, QWidget* parent) {
    QVariantMap criteria;
    criteria["itemType"] = itemType;
    if (area.isValid()) {
        criteria["searchArea"] = area;
    }
    
    return searchWithProgress(map, criteria, parent);
}

ProgressOperation* SearchOperationsProgress::findItemsByPropertiesWithProgress(Map* map, const QVariantMap& properties, const QRect& area, QWidget* parent) {
    QVariantMap criteria = properties;
    if (area.isValid()) {
        criteria["searchArea"] = area;
    }
    
    return searchWithProgress(map, criteria, parent);
}

void SearchOperationsProgress::searchWithProgress(Map* map, const QVariantMap& criteria,
                                                 std::function<void(const QList<QVariant>&)> onResults,
                                                 std::function<void(const QString&)> onError,
                                                 QWidget* parent) {
    ProgressOperation* operation = searchWithProgress(map, criteria, parent);
    
    if (!operation) {
        if (onError) onError("Failed to create search operation");
        return;
    }
    
    // Connect completion callbacks
    QObject::connect(operation, &ProgressOperation::finished, 
                    [onResults, onError](bool success, const QString& result) {
        if (success && onResults) {
            // Parse search results from result string
            // This would need to be implemented based on how MapSearchWorker returns results
            QList<QVariant> searchResults;
            onResults(searchResults);
        } else if (!success && onError) {
            onError(result);
        }
    });
    
    operation->start();
}

// ReplaceOperationsProgress Implementation
ReplaceOperationsProgress::ReplaceOperationsProgress(QObject* parent)
    : QObject(parent)
{
}

ProgressOperation* ReplaceOperationsProgress::replaceAllInstancesWithProgress(Map* map, quint16 oldItemId, quint16 newItemId, 
                                                                             const QRect& area, QWidget* parent) {
    if (!map) {
        qWarning() << "ReplaceOperationsProgress::replaceAllInstancesWithProgress: null map provided";
        return nullptr;
    }
    
    // Create map replace worker
    MapReplaceWorker* worker = new MapReplaceWorker();
    worker->setMap(map);
    
    QVariantMap operation;
    operation["type"] = "replaceItems";
    operation["oldItemId"] = oldItemId;
    operation["newItemId"] = newItemId;
    if (area.isValid()) {
        operation["area"] = area;
    }
    
    worker->setReplaceOperation(operation);
    
    // Create progress operation
    ProgressOperation* progressOp = ProgressOperationManager::instance()->createCustomOperation(
        QString("Replacing Items: %1 → %2").arg(oldItemId).arg(newItemId), worker, parent);
    
    // Configure for replace
    ProgressOperationFactory::configureForReplace(progressOp);
    
    return progressOp;
}

void ReplaceOperationsProgress::replaceAllInstancesWithProgress(Map* map, quint16 oldItemId, quint16 newItemId, const QRect& area,
                                                               std::function<void(int)> onCompleted,
                                                               std::function<void(const QString&)> onError,
                                                               QWidget* parent) {
    ProgressOperation* operation = replaceAllInstancesWithProgress(map, oldItemId, newItemId, area, parent);
    
    if (!operation) {
        if (onError) onError("Failed to create replace operation");
        return;
    }
    
    // Connect completion callbacks
    QObject::connect(operation, &ProgressOperation::finished, 
                    [onCompleted, onError](bool success, const QString& result) {
        if (success && onCompleted) {
            // Parse replacement count from result
            int replacementCount = result.toInt();
            onCompleted(replacementCount);
        } else if (!success && onError) {
            onError(result);
        }
    });
    
    operation->start();
}

ProgressOperation* ReplaceOperationsProgress::bulkTransformWithProgress(Map* map, const QList<QPoint>& positions, 
                                                                       std::function<void(Map*, const QPoint&)> transformation,
                                                                       QWidget* parent) {
    if (!map) {
        qWarning() << "ReplaceOperationsProgress::bulkTransformWithProgress: null map provided";
        return nullptr;
    }
    
    // Create bulk transform worker
    BulkTransformWorker* worker = new BulkTransformWorker();
    worker->setMap(map);
    worker->setPositions(positions);
    worker->setTransformation(transformation);
    
    // Create progress operation
    ProgressOperation* operation = ProgressOperationManager::instance()->createCustomOperation(
        QString("Bulk Transform: %1 positions").arg(positions.size()), worker, parent);
    
    // Configure for bulk operations
    operation->setEstimatedDuration(positions.size() / 100); // Rough estimate
    operation->setCancellable(true);
    operation->setAutoClose(true);
    
    return operation;
}

// MapSearcherProgressIntegration Implementation
MapSearcherProgressIntegration::MapSearcherProgressIntegration(MapSearcher* mapSearcher, QObject* parent)
    : QObject(parent)
    , mapSearcher_(mapSearcher)
    , progressDialog_(nullptr)
    , progressReportingEnabled_(true)
{
    if (mapSearcher_) {
        // Connect to existing MapSearcher signals for progress integration
        connect(mapSearcher_, &MapSearcher::searchStarted, this, &MapSearcherProgressIntegration::onSearchStarted);
        connect(mapSearcher_, &MapSearcher::searchProgress, this, &MapSearcherProgressIntegration::onSearchProgress);
        connect(mapSearcher_, &MapSearcher::searchCompleted, this, &MapSearcherProgressIntegration::onSearchCompleted);
        connect(mapSearcher_, &MapSearcher::searchError, this, &MapSearcherProgressIntegration::onSearchError);
    }
}

void MapSearcherProgressIntegration::enableProgressReporting(bool enabled) {
    progressReportingEnabled_ = enabled;
}

bool MapSearcherProgressIntegration::isProgressReportingEnabled() const {
    return progressReportingEnabled_;
}

void MapSearcherProgressIntegration::setProgressDialog(ProgressDialog* dialog) {
    progressDialog_ = dialog;
}

ProgressDialog* MapSearcherProgressIntegration::getProgressDialog() const {
    return progressDialog_;
}

ProgressOperation* MapSearcherProgressIntegration::searchWithProgress(const QVariantMap& criteria, QWidget* parent) {
    if (!mapSearcher_) {
        qWarning() << "MapSearcherProgressIntegration::searchWithProgress: null MapSearcher";
        return nullptr;
    }

    // Create search worker that integrates with existing MapSearcher
    MapSearcherIntegrationWorker* worker = new MapSearcherIntegrationWorker(mapSearcher_);
    worker->setSearchCriteria(criteria);

    // Create progress operation
    ProgressOperation* operation = ProgressOperationManager::instance()->createCustomOperation(
        "Searching Map", worker, parent);

    // Configure for search
    ProgressOperationFactory::configureForSearch(operation);

    return operation;
}

ProgressOperation* MapSearcherProgressIntegration::findAllWithProgress(quint16 itemId, const QRect& area, QWidget* parent) {
    QVariantMap criteria;
    criteria["itemId"] = itemId;
    if (area.isValid()) {
        criteria["searchArea"] = area;
    }

    return searchWithProgress(criteria, parent);
}

void MapSearcherProgressIntegration::onSearchProgress(int current, int total, const QString& currentItem) {
    if (!progressReportingEnabled_ || !progressDialog_) {
        return;
    }

    // Update progress dialog
    int percentage = (total > 0) ? (current * 100 / total) : 0;
    progressDialog_->updateProgress(percentage, QString("Searching: %1").arg(currentItem));
}

void MapSearcherProgressIntegration::onSearchCompleted(int resultCount, const QString& summary) {
    if (!progressReportingEnabled_ || !progressDialog_) {
        return;
    }

    // Update progress dialog with completion
    progressDialog_->updateProgress(100, QString("Search completed: %1 results").arg(resultCount));
    progressDialog_->finish();
}

void MapSearcherProgressIntegration::onSearchError(const QString& error, const QString& details) {
    if (!progressReportingEnabled_ || !progressDialog_) {
        return;
    }

    // Update progress dialog with error
    progressDialog_->updateProgress(0, QString("Search failed: %1").arg(error));
    progressDialog_->cancel();
}

void MapSearcherProgressIntegration::onSearchStarted(const QString& description) {
    if (!progressReportingEnabled_ || !progressDialog_) {
        return;
    }

    // Initialize progress dialog
    progressDialog_->updateProgress(0, QString("Starting search: %1").arg(description));
}

// LegacyProgressReplacement Implementation - 100% wxwidgets compatibility
static ProgressDialog* g_legacyProgressDialog = nullptr;
static ProgressOperation* g_legacyProgressOperation = nullptr;
static int g_legacyScaleFrom = 0;
static int g_legacyScaleTo = 100;

void LegacyProgressReplacement::replaceCreateLoadBar(const QString& message, bool canCancel, QWidget* parent) {
    // Clean up any existing dialog
    replaceDestroyLoadBar();

    // Create new progress dialog matching wxProgressDialog behavior exactly
    g_legacyProgressDialog = new ProgressDialog("Loading", message, 0, 100, parent);
    g_legacyProgressDialog->setCancellable(canCancel);
    g_legacyProgressDialog->setShowElapsedTime(true);
    g_legacyProgressDialog->setAutoClose(false);
    g_legacyProgressDialog->show();

    // Reset scale to match wxwidgets defaults
    g_legacyScaleFrom = 0;
    g_legacyScaleTo = 100;

    qDebug() << "LegacyProgressReplacement: Created load bar:" << message;
}

bool LegacyProgressReplacement::replaceSetLoadDone(int percentage, const QString& newMessage) {
    if (!g_legacyProgressDialog) {
        return false;
    }

    // Handle completion (matching wxwidgets behavior)
    if (percentage >= 100) {
        replaceDestroyLoadBar();
        return true;
    }

    // Calculate scaled progress (exact wxwidgets algorithm)
    int scaledProgress = g_legacyScaleFrom + (percentage * (g_legacyScaleTo - g_legacyScaleFrom) / 100);
    scaledProgress = qBound(0, scaledProgress, 100);

    // Update dialog with exact wxwidgets format
    QString displayMessage = newMessage.isEmpty() ?
        g_legacyProgressDialog->getLabelText() : newMessage;

    g_legacyProgressDialog->updateProgress(scaledProgress,
        QString("%1 (%2%)").arg(displayMessage).arg(scaledProgress));

    // Process events to keep UI responsive (matching wxwidgets behavior exactly)
    QApplication::processEvents();

    // Check for cancellation (matching wxwidgets return value)
    return !g_legacyProgressDialog->wasCancelled();
}

void LegacyProgressReplacement::replaceSetLoadScale(int from, int to) {
    g_legacyScaleFrom = qBound(0, from, 100);
    g_legacyScaleTo = qBound(g_legacyScaleFrom, to, 100);

    qDebug() << "LegacyProgressReplacement: Set scale from" << g_legacyScaleFrom << "to" << g_legacyScaleTo;
}

void LegacyProgressReplacement::replaceDestroyLoadBar() {
    if (g_legacyProgressDialog) {
        g_legacyProgressDialog->close();
        g_legacyProgressDialog->deleteLater();
        g_legacyProgressDialog = nullptr;
    }

    if (g_legacyProgressOperation) {
        g_legacyProgressOperation->cancel();
        g_legacyProgressOperation = nullptr;
    }

    qDebug() << "LegacyProgressReplacement: Destroyed load bar";
}

// ScopedLoadingBarReplacement Implementation
LegacyProgressReplacement::ScopedLoadingBarReplacement::ScopedLoadingBarReplacement(const QString& message, bool canCancel, QWidget* parent) {
    LegacyProgressReplacement::replaceCreateLoadBar(message, canCancel, parent);
}

LegacyProgressReplacement::ScopedLoadingBarReplacement::~ScopedLoadingBarReplacement() {
    LegacyProgressReplacement::replaceDestroyLoadBar();
}

void LegacyProgressReplacement::ScopedLoadingBarReplacement::setLoadDone(int percentage, const QString& newMessage) {
    LegacyProgressReplacement::replaceSetLoadDone(percentage, newMessage);
}

void LegacyProgressReplacement::ScopedLoadingBarReplacement::setLoadScale(int from, int to) {
    LegacyProgressReplacement::replaceSetLoadScale(from, to);
}

// ProgressIntegrationFactory Implementation
ProgressOperation* ProgressIntegrationFactory::createIntegratedMapLoading(const QString& filePath, QWidget* parent) {
    return MapOperationsProgress::loadMapWithProgress(filePath, parent);
}

ProgressOperation* ProgressIntegrationFactory::createIntegratedMapSaving(Map* map, const QString& filePath, QWidget* parent) {
    return MapOperationsProgress::saveMapWithProgress(map, filePath, parent);
}

ProgressOperation* ProgressIntegrationFactory::createIntegratedMapSearch(Map* map, const QVariantMap& criteria, QWidget* parent) {
    return SearchOperationsProgress::searchWithProgress(map, criteria, parent);
}

ProgressOperation* ProgressIntegrationFactory::createIntegratedMapReplace(Map* map, const QVariantMap& operation, QWidget* parent) {
    quint16 oldItemId = operation.value("oldItemId").toUInt();
    quint16 newItemId = operation.value("newItemId").toUInt();
    QRect area = operation.value("area", QRect()).toRect();

    return ReplaceOperationsProgress::replaceAllInstancesWithProgress(map, oldItemId, newItemId, area, parent);
}

ProgressOperation* ProgressIntegrationFactory::createIntegratedMapValidation(Map* map, const QVariantMap& options, QWidget* parent) {
    return MapOperationsProgress::validateMapWithProgress(map, options, parent);
}
