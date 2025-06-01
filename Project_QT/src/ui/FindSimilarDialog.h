#ifndef FINDSIMILARDIALOG_H
#define FINDSIMILARDIALOG_H

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QFormLayout>
#include <QTabWidget>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QComboBox>
#include <QCheckBox>
#include <QRadioButton>
#include <QButtonGroup>
#include <QPushButton>
#include <QToolButton>
#include <QListWidget>
#include <QListWidgetItem>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QTextEdit>
#include <QProgressBar>
#include <QSplitter>
#include <QDialogButtonBox>
#include <QTimer>
#include <QUndoStack>
#include <QUndoCommand>

#include "MapSearcher.h"
#include "ui/ProgressDialog.h"

/**
 * @brief Task 89: Find Similar Items Dialog
 * 
 * Complete "Find Similar Items" dialog implementation:
 * - Full UI for specifying find criteria (ID, name, flags/attributes, type)
 * - Advanced search options (match case, whole word, regex, property matching)
 * - Comprehensive scope options (selection, whole map, visible layers, floors)
 * - Results display with detailed information and interaction
 * - Integration with MapSearcher and ItemManager
 * - Progress indication and cancellation support
 * - Export/import functionality for search criteria
 * - 1:1 compatibility with wxwidgets find similar functionality
 */

class FindSimilarDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FindSimilarDialog(QWidget* parent = nullptr);
    ~FindSimilarDialog() override;

    // Configuration
    void setMap(class Map* map);
    void setMapView(class MapView* mapView);
    void setItemManager(class ItemManager* itemManager);
    void setMapSearcher(class MapSearcher* mapSearcher);
    void setUndoStack(QUndoStack* undoStack);

    // Search criteria
    void setSearchCriteria(const SearchCriteria& criteria);
    SearchCriteria getSearchCriteria() const;
    
    void setInitialItemId(quint16 itemId);
    void setInitialItemName(const QString& name);
    void setSearchScope(bool selectionOnly);

    // Results access
    QList<SearchResult> getSearchResults() const;
    QList<SearchResult> getSelectedResults() const;
    int getFoundItemCount() const;

    // State
    bool isSearchInProgress() const;

public slots:
    void findSimilarItems();
    void findAllItems();
    void clearResults();
    void exportResults();
    void importCriteria();

signals:
    void searchStarted(const SearchCriteria& criteria);
    void searchProgress(int current, int total, const QString& currentItem);
    void searchCompleted(const QList<SearchResult>& results);
    void searchCancelled();
    void searchError(const QString& error, const QString& details);
    
    void resultSelected(const SearchResult& result);
    void resultDoubleClicked(const SearchResult& result);
    void resultsSelectionChanged(const QList<SearchResult>& selectedResults);
    
    void requestMapHighlight(const QList<QPoint>& positions);
    void requestMapCenter(const QPoint& position);
    void requestReplaceDialog(const QList<SearchResult>& results);

private slots:
    // Search control
    void onFindClicked();
    void onFindAllClicked();
    void onCancelSearchClicked();
    void onClearResultsClicked();
    
    // Criteria input
    void onSearchModeChanged();
    void onItemIdChanged();
    void onItemNameChanged();
    void onItemTypeChanged();
    void onPropertyCriteriaChanged();
    void onScopeChanged();
    void onOptionsChanged();
    
    // Results interaction
    void onResultItemClicked(QListWidgetItem* item);
    void onResultItemDoubleClicked(QListWidgetItem* item);
    void onResultSelectionChanged();
    void onResultsFilterChanged();
    
    // Advanced features
    void onPickItemClicked();
    void onValidateCriteriaClicked();
    void onExportResultsClicked();
    void onImportCriteriaClicked();
    void onSavePresetClicked();
    void onLoadPresetClicked();
    void onOpenReplaceDialogClicked();
    
    // Map integration
    void onMapSearchProgress(int current, int total, const QString& currentItem);
    void onMapSearchCompleted(const QList<SearchResult>& results);
    void onMapSearchError(const QString& error, const QString& details);
    
    // UI updates
    void onUpdateButtonStates();
    void onUpdateResultsDisplay();
    void onUpdateStatistics();

private:
    void setupUI();
    void setupSearchCriteriaTab();
    void setupSearchOptionsTab();
    void setupSearchScopeTab();
    void setupResultsTab();
    void setupAdvancedTab();
    void connectSignals();
    
    void populateItemTypeComboBox();
    void populatePropertyComboBoxes();
    void updateButtonStates();
    void updateResultsDisplay();
    void updateStatistics();
    
    SearchCriteria buildSearchCriteria() const;
    void applyCriteriaToUI(const SearchCriteria& criteria);
    void addResultToList(const SearchResult& result);
    void highlightResultsOnMap();
    
    QString formatResultText(const SearchResult& result) const;
    QString formatStatistics() const;
    
    void savePreset(const QString& name);
    void loadPreset(const QString& name);

private:
    // Core components
    class Map* map_;
    class MapView* mapView_;
    class ItemManager* itemManager_;
    class MapSearcher* mapSearcher_;
    QUndoStack* undoStack_;
    
    // UI structure
    QTabWidget* tabWidget_;
    QSplitter* mainSplitter_;
    
    // Search Criteria Tab
    QWidget* searchCriteriaTab_;
    QGroupBox* searchModeGroup_;
    QRadioButton* searchByIdRadio_;
    QRadioButton* searchByNameRadio_;
    QRadioButton* searchByTypeRadio_;
    QRadioButton* searchByPropertiesRadio_;
    QButtonGroup* searchModeButtonGroup_;
    
    QGroupBox* criteriaGroup_;
    QLineEdit* itemIdLineEdit_;
    QLineEdit* itemNameLineEdit_;
    QComboBox* itemTypeComboBox_;
    QPushButton* pickItemButton_;
    
    // Property criteria
    QGroupBox* propertyGroup_;
    QCheckBox* moveableCheckBox_;
    QCheckBox* blockingCheckBox_;
    QCheckBox* stackableCheckBox_;
    QCheckBox* pickupableCheckBox_;
    QCheckBox* readableCheckBox_;
    QCheckBox* writeableCheckBox_;
    QCheckBox* rotateableCheckBox_;
    QCheckBox* hangableCheckBox_;
    QCheckBox* verticalCheckBox_;
    QCheckBox* horizontalCheckBox_;
    
    // Search Options Tab
    QWidget* searchOptionsTab_;
    QGroupBox* textOptionsGroup_;
    QCheckBox* matchCaseCheckBox_;
    QCheckBox* wholeWordCheckBox_;
    QCheckBox* regexSearchCheckBox_;
    
    QGroupBox* filterOptionsGroup_;
    QSpinBox* minCountSpinBox_;
    QSpinBox* maxCountSpinBox_;
    QLineEdit* actionIdLineEdit_;
    QLineEdit* uniqueIdLineEdit_;
    
    // Search Scope Tab
    QWidget* searchScopeTab_;
    QGroupBox* scopeGroup_;
    QRadioButton* wholeMapRadio_;
    QRadioButton* selectionRadio_;
    QRadioButton* visibleAreaRadio_;
    QRadioButton* currentFloorRadio_;
    QRadioButton* customAreaRadio_;
    QButtonGroup* scopeButtonGroup_;
    
    QGroupBox* layerGroup_;
    QCheckBox* includeGroundCheckBox_;
    QCheckBox* includeItemsCheckBox_;
    QCheckBox* includeCreaturesCheckBox_;
    QCheckBox* includeSpawnsCheckBox_;
    
    // Results Tab
    QWidget* resultsTab_;
    QGroupBox* resultsGroup_;
    QListWidget* resultsListWidget_;
    QLabel* resultsCountLabel_;
    QLabel* statisticsLabel_;
    QTextEdit* statisticsTextEdit_;
    
    // Advanced Tab
    QWidget* advancedTab_;
    QGroupBox* presetGroup_;
    QComboBox* presetComboBox_;
    QPushButton* savePresetButton_;
    QPushButton* loadPresetButton_;
    QPushButton* deletePresetButton_;
    
    QGroupBox* exportGroup_;
    QPushButton* exportResultsButton_;
    QPushButton* importCriteriaButton_;
    QPushButton* exportCriteriaButton_;
    
    // Control buttons
    QGroupBox* controlGroup_;
    QPushButton* findButton_;
    QPushButton* findAllButton_;
    QPushButton* cancelButton_;
    QPushButton* clearResultsButton_;
    QPushButton* replaceButton_;
    QPushButton* validateButton_;
    
    // Progress
    QProgressBar* progressBar_;
    ProgressDialog* progressDialog_;
    
    // State
    bool searchInProgress_;
    QList<SearchResult> searchResults_;
    QList<SearchResult> selectedResults_;
    SearchCriteria currentCriteria_;
    
    // Statistics
    QVariantMap searchStatistics_;
    QStringList foundItemTypes_;
    QMap<quint16, int> itemIdCounts_;
    
    // Timers
    QTimer* updateTimer_;
};

/**
 * @brief Search Result Item Widget for detailed display
 */
class SearchResultItem : public QListWidgetItem
{
public:
    explicit SearchResultItem(const SearchResult& result, QListWidget* parent = nullptr);
    ~SearchResultItem() override = default;

    const SearchResult& getSearchResult() const { return result_; }
    void updateDisplay();

private:
    SearchResult result_;
};

/**
 * @brief Search Criteria Preset Manager
 */
class SearchCriteriaPresetManager : public QObject
{
    Q_OBJECT

public:
    explicit SearchCriteriaPresetManager(QObject* parent = nullptr);
    ~SearchCriteriaPresetManager() override = default;

    // Preset management
    void savePreset(const QString& name, const SearchCriteria& criteria);
    SearchCriteria loadPreset(const QString& name) const;
    void deletePreset(const QString& name);
    QStringList getPresetNames() const;
    bool hasPreset(const QString& name) const;

    // Import/export
    bool exportPresets(const QString& filePath) const;
    bool importPresets(const QString& filePath);
    bool exportCriteria(const QString& filePath, const SearchCriteria& criteria) const;
    SearchCriteria importCriteria(const QString& filePath) const;

signals:
    void presetSaved(const QString& name);
    void presetDeleted(const QString& name);
    void presetsChanged();

private:
    void loadPresets();
    void savePresets();
    QString getPresetsFilePath() const;

private:
    QMap<QString, SearchCriteria> presets_;
};

#endif // FINDSIMILARDIALOG_H
