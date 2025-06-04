#ifndef REPLACEITEMSDIALOG_H
#define REPLACEITEMSDIALOG_H

#include <QDialog>

// Forward declarations for Qt classes to minimize include dependencies in header
class QLineEdit;
class QCheckBox;
class QComboBox;
class QPushButton;
class QListWidget;
class QGridLayout;
class QVBoxLayout;
class QFormLayout;
class QGroupBox;
class QDialogButtonBox;
class QSpinBox;
class QRadioButton;
class QButtonGroup;
class QProgressBar;
class QLabel;
class QSplitter;
class QTabWidget;
class QTextEdit;

/**
 * @brief Task 89: Complete Replace Item Functionality (Full Find Similar Dialog Logic)
 *
 * Complete migration of wxwidgets "Find Similar Items" / "Replace Item" dialog:
 * - Full UI for specifying find criteria (ID, name, flags/attributes, type)
 * - Complete replacement options (ID, delete, property changes)
 * - Comprehensive scope options (selection, whole map, visible layers)
 * - Advanced search options (match case, whole word, regex)
 * - Integration with MapSearcher and ItemManager
 * - Progress indication and cancellation support
 * - Undo/redo support with QUndoCommand
 * - 1:1 compatibility with wxwidgets replace functionality
 */

// Task 89: Enhanced ReplaceItemsDialog with complete find/replace functionality
class ReplaceItemsDialog : public QDialog {
    Q_OBJECT

public:
    explicit ReplaceItemsDialog(QWidget *parent = nullptr);
    ~ReplaceItemsDialog() override;

    // Task 89: Enhanced public interface for complete integration
    void setSearchScope(bool selectionOnly);
    void setInitialFindItemId(int itemId);
    void setInitialReplaceItemId(int itemId);

    // Task 89: Advanced configuration
    void setMap(class Map* map);
    void setMapView(class MapView* mapView);
    void setItemManager(class ItemManager* itemManager);
    void setMapSearcher(class MapSearcher* mapSearcher);
    void setUndoStack(class QUndoStack* undoStack);

    // Task 89: Find criteria configuration
    void setFindCriteria(const struct SearchCriteria& criteria);
    struct SearchCriteria getFindCriteria() const;

    // Task 89: Replace operation configuration
    void setReplaceOperation(const struct ReplaceOperation& operation);
    struct ReplaceOperation getReplaceOperation() const;

    // Task 89: Search results access
    QList<struct SearchResult> getSearchResults() const;
    QList<struct SearchResult> getSelectedResults() const;

    // Task 89: State management
    bool isSearchInProgress() const;
    bool isReplaceInProgress() const;
    int getFoundItemCount() const;
    int getReplacedItemCount() const;

signals:
    // Task 89: Enhanced signals for complete integration
    void searchRequested(const QString& criteria);
    void replaceRequested(const QString& findCriteria, const QString& replaceCriteria);
    void itemPropertyEditorRequested(bool forFindCriteria);

    // Task 89: Advanced search and replace signals
    void searchStarted(const struct SearchCriteria& criteria);
    void searchProgress(int current, int total, const QString& currentItem);
    void searchCompleted(const QList<struct SearchResult>& results);
    void searchCancelled();
    void searchError(const QString& error, const QString& details);

    void replaceStarted(const struct ReplaceOperation& operation);
    void replaceProgress(int current, int total, const QString& currentItem);
    void replaceCompleted(int replacedCount, const QString& summary);
    void replaceCancelled();
    void replaceError(const QString& error, const QString& details);

    // Task 89: Result interaction signals
    void resultSelected(const struct SearchResult& result);
    void resultDoubleClicked(const struct SearchResult& result);
    void resultsSelectionChanged(const QList<struct SearchResult>& selectedResults);

    // Task 89: Map interaction signals
    void requestMapHighlight(const QList<QPoint>& positions);
    void requestMapCenter(const QPoint& position);
    void requestMapSelection(const QList<QPoint>& positions);

private slots:
    // Task 89: Enhanced slot implementations
    void onFindClicked();
    void onFindAllClicked();
    void onReplaceSelectedClicked();
    void onReplaceAllClicked();
    void onPickFindItemClicked();
    void onPickReplaceItemClicked();
    void onClearResultsClicked();
    void onResultItemDoubleClicked();
    void onResultItemSelectionChanged();
    void onSearchScopeChanged();
    void onSearchOptionsChanged();
    void onPresetSelected();
    void onSavePresetClicked();
    void onLoadPresetClicked();
    void onDeletePresetClicked();
    void onSwapFindReplaceClicked();
    void onAdvancedOptionsToggled(bool visible);
    void onProgressCancelled();

    // Task 89: Advanced search and replace slots
    void onSearchModeChanged();
    void onFindCriteriaChanged();
    void onReplaceCriteriaChanged();
    void onSearchAreaChanged();
    void onFilterOptionsChanged();
    void onResultsFilterChanged();
    void onExportResultsClicked();
    void onImportCriteriaClicked();
    void onValidateCriteriaClicked();
    void onPreviewReplaceClicked();

    // Task 89: Map integration slots
    void onMapSearchProgress(int current, int total, const QString& currentItem);
    void onMapSearchCompleted(const QList<struct SearchResult>& results);
    void onMapSearchError(const QString& error, const QString& details);
    void onMapReplaceProgress(int current, int total, const QString& currentItem);
    void onMapReplaceCompleted(int replacedCount, const QString& summary);
    void onMapReplaceError(const QString& error, const QString& details);

    // Task 89: UI update slots
    void onUpdateButtonStates();
    void onUpdateResultsDisplay();
    void onUpdateProgressDisplay();
    void onUpdateStatistics();

private:
    // Task 50: UI Setup Helpers
    void setupUi();
    void setupFindCriteriaTab();
    void setupReplacementTab();
    void setupSearchScopeTab();
    void setupAdvancedOptionsTab();
    void setupResultsArea();
    void setupPresetManagement();
    void connectSignals();
    void updateButtonStates();
    void populateItemTypeComboBox();
    void populatePresetComboBox();

    // Task 50: Data management
    QString buildSearchCriteria() const;
    QString buildReplacementCriteria() const;
    void clearResults();
    void addResultItem(const QString& description, int x, int y, int z);
    void saveCurrentPreset(const QString& name);
    void loadPreset(const QString& name);

    // Task 50: Main UI Structure
    QTabWidget* m_tabWidget;
    QSplitter* m_mainSplitter;
    QVBoxLayout* m_mainLayout;

    // Task 50: Find Criteria Tab
    QWidget* m_findCriteriaTab;
    QGroupBox* m_findCriteriaGroup;
    QLineEdit* m_findItemIdLineEdit;
    QLineEdit* m_findNameLineEdit;
    QCheckBox* m_findMatchCaseCheckBox;
    QCheckBox* m_findWholeWordCheckBox;
    QCheckBox* m_findIsMoveableCheckBox;
    QCheckBox* m_findIsBlockingCheckBox;
    QCheckBox* m_findIsStackableCheckBox;
    QCheckBox* m_findIsPickupableCheckBox;
    QCheckBox* m_findIsReadableCheckBox;
    QCheckBox* m_findIsWriteableCheckBox;
    QCheckBox* m_findIsRotateableCheckBox;
    QCheckBox* m_findIsHangableCheckBox;
    QCheckBox* m_findIsVerticalCheckBox;
    QCheckBox* m_findIsHorizontalCheckBox;
    QComboBox* m_findItemTypeComboBox;
    QSpinBox* m_findMinCountSpinBox;
    QSpinBox* m_findMaxCountSpinBox;
    QLineEdit* m_findActionIdLineEdit;
    QLineEdit* m_findUniqueIdLineEdit;
    QPushButton* m_pickFindItemButton;

    // Task 50: Replacement Tab
    QWidget* m_replacementTab;
    QGroupBox* m_replaceRuleGroup;
    QLineEdit* m_replaceItemIdLineEdit;
    QCheckBox* m_deleteFoundItemsCheckBox;
    QPushButton* m_pickReplaceItemButton;
    QPushButton* m_swapFindReplaceButton;

    // Task 50: Search Scope Tab
    QWidget* m_searchScopeTab;
    QGroupBox* m_scopeGroup;
    QRadioButton* m_scopeWholeMapRadio;
    QRadioButton* m_scopeSelectionRadio;
    QRadioButton* m_scopeVisibleAreaRadio;
    QRadioButton* m_scopeCurrentFloorRadio;
    QButtonGroup* m_scopeButtonGroup;
    QCheckBox* m_includeGroundItemsCheckBox;
    QCheckBox* m_includeTopItemsCheckBox;
    QCheckBox* m_includeCreaturesCheckBox;
    QSpinBox* m_searchRadiusSpinBox;

    // Task 50: Advanced Options Tab
    QWidget* m_advancedOptionsTab;
    QGroupBox* m_advancedGroup;
    QSpinBox* m_maxResultsSpinBox;
    QCheckBox* m_caseSensitiveCheckBox;
    QCheckBox* m_regexSearchCheckBox;
    QCheckBox* m_searchInContainersCheckBox;
    QCheckBox* m_searchInDepotCheckBox;
    QCheckBox* m_ignoreUniqueItemsCheckBox;
    QCheckBox* m_showProgressCheckBox;

    // Task 50: Results Area
    QGroupBox* m_resultsGroup;
    QListWidget* m_resultsListWidget;
    QLabel* m_resultsCountLabel;
    QPushButton* m_clearResultsButton;
    QPushButton* m_exportResultsButton;
    QProgressBar* m_searchProgressBar;

    // Task 50: Action Buttons
    QGroupBox* m_actionsGroup;
    QPushButton* m_findButton;
    QPushButton* m_replaceSelectedButton;
    QPushButton* m_replaceAllButton;
    QPushButton* m_cancelButton;

    // Task 50: Preset Management
    QGroupBox* m_presetGroup;
    QComboBox* m_presetComboBox;
    QPushButton* m_savePresetButton;
    QPushButton* m_loadPresetButton;
    QPushButton* m_deletePresetButton;

    // Task 50: Dialog Buttons
    QDialogButtonBox* m_dialogButtonBox;

    // Task 89: Core integration components
    class Map* m_map;
    class MapView* m_mapView;
    class ItemManager* m_itemManager;
    class MapSearcher* m_mapSearcher;
    class QUndoStack* m_undoStack;

    // Task 89: Search and replace state
    bool m_searchInProgress;
    bool m_replaceInProgress;
    int m_totalResultsFound;
    int m_totalItemsReplaced;
    QString m_lastSearchCriteria;
    struct SearchCriteria m_currentSearchCriteria;
    struct ReplaceOperation m_currentReplaceOperation;
    QList<struct SearchResult> m_searchResults;
    QList<struct SearchResult> m_selectedResults;

    // Task 89: Progress tracking
    class ProgressDialog* m_progressDialog;
    QTimer* m_progressUpdateTimer;
    int m_currentProgress;
    int m_maxProgress;
    QString m_currentOperation;

    // Task 89: Statistics and analysis
    QVariantMap m_searchStatistics;
    QVariantMap m_replaceStatistics;
    QStringList m_foundItemTypes;
    QMap<quint16, int> m_itemIdCounts;

    // Task 019: Helper methods
    QString getItemDisplayName(quint16 itemId);
};

#endif // REPLACEITEMSDIALOG_H
