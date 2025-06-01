#ifndef PROGRESSINTEGRATION_H
#define PROGRESSINTEGRATION_H

#include <QObject>
#include <QString>
#include <QVariantMap>
#include <QList>
#include <QPoint>
#include <QRect>
#include <functional>

#include "ui/ProgressOperationManager.h"
#include "threading/ProgressWorker.h"

// Forward declarations
class Map;
class MapView;
class MapSearcher;
class ItemManager;
class SpriteManager;

/**
 * @brief Task 88: Progress Integration helpers for existing long operations
 * 
 * Integration layer for adding progress indication to existing operations:
 * - Map loading/saving operations (Task 84)
 * - Search operations (Task 82)
 * - Replace operations
 * - Validation routines
 * - Sprite loading operations
 * - Bulk item transformations
 * - Integration with existing classes
 */

/**
 * @brief Map Operations Progress Integration
 */
class MapOperationsProgress : public QObject
{
    Q_OBJECT

public:
    explicit MapOperationsProgress(QObject* parent = nullptr);
    ~MapOperationsProgress() override = default;

    // Map loading with progress
    static ProgressOperation* loadMapWithProgress(const QString& filePath, QWidget* parent = nullptr);
    static void loadMapWithProgress(const QString& filePath, 
                                   std::function<void(Map*)> onLoaded,
                                   std::function<void(const QString&)> onError = nullptr,
                                   QWidget* parent = nullptr);

    // Map saving with progress
    static ProgressOperation* saveMapWithProgress(Map* map, const QString& filePath, QWidget* parent = nullptr);
    static void saveMapWithProgress(Map* map, const QString& filePath,
                                   std::function<void()> onSaved,
                                   std::function<void(const QString&)> onError = nullptr,
                                   QWidget* parent = nullptr);

    // Map validation with progress
    static ProgressOperation* validateMapWithProgress(Map* map, const QVariantMap& options = QVariantMap(), QWidget* parent = nullptr);
    static void validateMapWithProgress(Map* map, const QVariantMap& options,
                                       std::function<void(const QStringList&)> onCompleted,
                                       std::function<void(const QString&)> onError = nullptr,
                                       QWidget* parent = nullptr);

signals:
    void mapLoadingStarted(const QString& filePath);
    void mapLoadingProgress(int percentage, const QString& stage);
    void mapLoadingCompleted(Map* map);
    void mapLoadingFailed(const QString& error);

    void mapSavingStarted(const QString& filePath);
    void mapSavingProgress(int percentage, const QString& stage);
    void mapSavingCompleted();
    void mapSavingFailed(const QString& error);
};

/**
 * @brief Search Operations Progress Integration
 */
class SearchOperationsProgress : public QObject
{
    Q_OBJECT

public:
    explicit SearchOperationsProgress(QObject* parent = nullptr);
    ~SearchOperationsProgress() override = default;

    // Search with progress
    static ProgressOperation* searchMapWithProgress(Map* map, const QVariantMap& criteria, QWidget* parent = nullptr);
    static void searchMapWithProgress(Map* map, const QVariantMap& criteria,
                                     std::function<void(const QList<QVariant>&)> onResults,
                                     std::function<void(const QString&)> onError = nullptr,
                                     QWidget* parent = nullptr);

    // Find all operations with progress
    static ProgressOperation* findAllItemsWithProgress(Map* map, quint16 itemId, const QRect& area = QRect(), QWidget* parent = nullptr);
    static void findAllItemsWithProgress(Map* map, quint16 itemId, const QRect& area,
                                        std::function<void(const QList<QPoint>&)> onResults,
                                        std::function<void(const QString&)> onError = nullptr,
                                        QWidget* parent = nullptr);

    // Search by criteria with progress
    static ProgressOperation* searchByCriteriaWithProgress(Map* map, const QString& criteria, QWidget* parent = nullptr);
    static void searchByCriteriaWithProgress(Map* map, const QString& criteria,
                                            std::function<void(const QList<QVariant>&)> onResults,
                                            std::function<void(const QString&)> onError = nullptr,
                                            QWidget* parent = nullptr);

signals:
    void searchStarted(const QString& description);
    void searchProgress(int current, int total, const QString& currentItem);
    void searchCompleted(const QList<QVariant>& results);
    void searchFailed(const QString& error);
};

/**
 * @brief Replace Operations Progress Integration
 */
class ReplaceOperationsProgress : public QObject
{
    Q_OBJECT

public:
    explicit ReplaceOperationsProgress(QObject* parent = nullptr);
    ~ReplaceOperationsProgress() override = default;

    // Replace items with progress
    static ProgressOperation* replaceItemsWithProgress(Map* map, const QVariantMap& operation, QWidget* parent = nullptr);
    static void replaceItemsWithProgress(Map* map, const QVariantMap& operation,
                                        std::function<void(int)> onCompleted,
                                        std::function<void(const QString&)> onError = nullptr,
                                        QWidget* parent = nullptr);

    // Replace all instances with progress
    static ProgressOperation* replaceAllInstancesWithProgress(Map* map, quint16 oldItemId, quint16 newItemId, 
                                                             const QRect& area = QRect(), QWidget* parent = nullptr);
    static void replaceAllInstancesWithProgress(Map* map, quint16 oldItemId, quint16 newItemId, const QRect& area,
                                               std::function<void(int)> onCompleted,
                                               std::function<void(const QString&)> onError = nullptr,
                                               QWidget* parent = nullptr);

    // Bulk transformations with progress
    static ProgressOperation* bulkTransformWithProgress(Map* map, const QList<QPoint>& positions, 
                                                       std::function<void(Map*, const QPoint&)> transformation,
                                                       QWidget* parent = nullptr);

signals:
    void replaceStarted(const QString& description);
    void replaceProgress(int current, int total, const QString& currentItem);
    void replaceCompleted(int replacedCount);
    void replaceFailed(const QString& error);
};

/**
 * @brief Sprite Operations Progress Integration
 */
class SpriteOperationsProgress : public QObject
{
    Q_OBJECT

public:
    explicit SpriteOperationsProgress(QObject* parent = nullptr);
    ~SpriteOperationsProgress() override = default;

    // Sprite loading with progress
    static ProgressOperation* loadSpritesWithProgress(const QStringList& spritePaths, QWidget* parent = nullptr);
    static void loadSpritesWithProgress(const QStringList& spritePaths,
                                       std::function<void(const QStringList&)> onLoaded,
                                       std::function<void(const QString&)> onError = nullptr,
                                       QWidget* parent = nullptr);

    // Sprite cache building with progress
    static ProgressOperation* buildSpriteCacheWithProgress(SpriteManager* spriteManager, QWidget* parent = nullptr);
    static void buildSpriteCacheWithProgress(SpriteManager* spriteManager,
                                            std::function<void()> onCompleted,
                                            std::function<void(const QString&)> onError = nullptr,
                                            QWidget* parent = nullptr);

    // Sprite validation with progress
    static ProgressOperation* validateSpritesWithProgress(SpriteManager* spriteManager, QWidget* parent = nullptr);

signals:
    void spriteLoadingStarted(const QStringList& paths);
    void spriteLoadingProgress(int current, int total, const QString& currentSprite);
    void spriteLoadingCompleted(const QStringList& loadedPaths);
    void spriteLoadingFailed(const QString& error);
};

/**
 * @brief Integration with existing MapSearcher class
 */
class MapSearcherProgressIntegration : public QObject
{
    Q_OBJECT

public:
    explicit MapSearcherProgressIntegration(MapSearcher* mapSearcher, QObject* parent = nullptr);
    ~MapSearcherProgressIntegration() override = default;

    // Integrate existing MapSearcher with progress system
    void enableProgressReporting(bool enabled);
    bool isProgressReportingEnabled() const;

    void setProgressDialog(ProgressDialog* dialog);
    ProgressDialog* getProgressDialog() const;

    // Search operations with progress
    ProgressOperation* searchWithProgress(const QVariantMap& criteria, QWidget* parent = nullptr);
    ProgressOperation* findAllWithProgress(quint16 itemId, const QRect& area = QRect(), QWidget* parent = nullptr);

private slots:
    void onSearchProgress(int current, int total, const QString& currentItem);
    void onSearchCompleted(int resultCount, const QString& summary);
    void onSearchError(const QString& error, const QString& details);

private:
    MapSearcher* mapSearcher_;
    ProgressDialog* progressDialog_;
    bool progressReportingEnabled_;
};

/**
 * @brief Legacy Progress Replacement Helper
 */
class LegacyProgressReplacement
{
public:
    // Replace wxProgressDialog usage patterns
    static void replaceCreateLoadBar(const QString& message, bool canCancel = false, QWidget* parent = nullptr);
    static bool replaceSetLoadDone(int percentage, const QString& newMessage = QString());
    static void replaceSetLoadScale(int from, int to);
    static void replaceDestroyLoadBar();

    // Replace ScopedLoadingBar usage
    class ScopedLoadingBarReplacement {
    public:
        explicit ScopedLoadingBarReplacement(const QString& message, bool canCancel = false, QWidget* parent = nullptr);
        ~ScopedLoadingBarReplacement();

        void setLoadDone(int percentage, const QString& newMessage = QString());
        void setLoadScale(int from, int to);

    private:
        ProgressDialog* dialog_;
        int scaleFrom_;
        int scaleTo_;
    };

    // Integration with existing progress patterns
    static void integrateWithExistingProgress(std::function<void()> operation, 
                                            const QString& title, const QString& description,
                                            QWidget* parent = nullptr);

    // Batch operation helpers
    template<typename Container, typename Operation>
    static void processBatchWithProgress(const Container& items, Operation operation,
                                       const QString& title, const QString& description,
                                       QWidget* parent = nullptr) {
        auto* dialog = ProgressDialogManager::instance()->createProgressDialog(title, description, 0, items.size(), parent);
        dialog->show();

        int current = 0;
        for (const auto& item : items) {
            if (dialog->wasCanceled()) {
                break;
            }

            operation(item);
            dialog->setProgress(++current);
            QApplication::processEvents();
        }

        ProgressDialogManager::instance()->destroyProgressDialog(dialog);
    }

private:
    static ProgressDialog* currentLegacyDialog_;
    static int currentScaleFrom_;
    static int currentScaleTo_;
};

/**
 * @brief Progress Integration Factory
 */
class ProgressIntegrationFactory
{
public:
    // Create integrated operations
    static ProgressOperation* createIntegratedMapLoading(const QString& filePath, QWidget* parent = nullptr);
    static ProgressOperation* createIntegratedMapSaving(Map* map, const QString& filePath, QWidget* parent = nullptr);
    static ProgressOperation* createIntegratedMapSearch(Map* map, const QVariantMap& criteria, QWidget* parent = nullptr);
    static ProgressOperation* createIntegratedMapReplace(Map* map, const QVariantMap& operation, QWidget* parent = nullptr);
    static ProgressOperation* createIntegratedMapValidation(Map* map, const QVariantMap& options, QWidget* parent = nullptr);
    static ProgressOperation* createIntegratedSpriteLoading(const QStringList& paths, QWidget* parent = nullptr);

    // Integration helpers
    static void integrateExistingOperation(std::function<void(ProgressDialog*)> operation,
                                         const QString& title, const QString& description,
                                         QWidget* parent = nullptr);

private:
    ProgressIntegrationFactory() = default; // Static factory class
};

#endif // PROGRESSINTEGRATION_H
