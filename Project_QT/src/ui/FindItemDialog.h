#ifndef FINDITEMDIALOG_H
#define FINDITEMDIALOG_H

#include <QDialog>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QSplitter>
#include <QTabWidget>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QSpinBox>
#include <QCheckBox>
#include <QRadioButton>
#include <QButtonGroup>
#include <QComboBox>
#include <QListWidget>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QProgressBar>
#include <QTimer>
#include <QTextEdit>
#include <QScrollArea>
#include <QFrame>
#include "ItemFinder.h"
#include "MapSearcher.h" // Task 82: Enhanced search backend

// Forward declarations
class Map;
class MapView;
class ItemManager;
class TilesetManager;
class QTreeWidget;
class QTreeWidgetItem;
class QUndoStack;
class ReplaceItemCommand; // Task 82: Replace functionality

// Task 68: Find Item Dialog for Qt version
class FindItemDialog : public QDialog {
    Q_OBJECT

public:
    // Search modes (from wxwidgets)
    enum class SearchMode {
        ServerIDs = 0,
        ClientIDs = 1,
        Names = 2,
        Types = 3,
        Properties = 4
    };

    // Item types for type-based search (from wxwidgets)
    enum class SearchItemType {
        Depot = 0,
        Mailbox = 1,
        TrashHolder = 2,
        Container = 3,
        Door = 4,
        MagicField = 5,
        Teleport = 6,
        Bed = 7,
        Key = 8,
        Podium = 9
    };

    explicit FindItemDialog(QWidget* parent = nullptr, const QString& title = "Find Item", bool onlyPickupables = false);
    ~FindItemDialog() override;

    // Result access
    quint16 getResultId() const { return resultId_; }
    bool hasResult() const { return resultId_ != 0; }
    
    // Search mode management
    SearchMode getSearchMode() const;
    void setSearchMode(SearchMode mode);
    
    // Integration with map and managers
    void setMap(Map* map);
    void setMapView(MapView* mapView);
    void setItemManager(ItemManager* itemManager);
    void setTilesetManager(TilesetManager* tilesetManager);
    void setUndoStack(QUndoStack* undoStack); // Task 82: Undo support

    // Browse tile functionality
    void populateFromTile(const QPoint& tilePos, int floor = 0);
    void populateFromTileItems(const QList<quint16>& itemIds);

    // Task 82: Enhanced search functionality
    void setMapSearcher(MapSearcher* searcher) { mapSearcher_ = searcher; }
    MapSearcher* getMapSearcher() const { return mapSearcher_; }

    // Task 82: Find All functionality
    QList<SearchResult> findAllItems();
    QList<SearchResult> getCurrentResults() const { return currentSearchResults_; }
    int getResultCount() const { return currentSearchResults_.size(); }

    // Task 82: Replace functionality
    bool hasSelectedResults() const;
    QList<SearchResult> getSelectedResults() const;
    void showReplaceDialog();
    void showReplaceWithDialog();
    void showDeleteSelectedDialog();

signals:
    void itemFound(quint16 itemId, const QPoint& position, int floor);
    void goToRequested(const QPoint& position, int floor);
    void browseTileRequested();

    // Task 82: Enhanced search signals
    void findAllStarted(const QString& description);
    void findAllProgress(int current, int total, const QString& currentItem);
    void findAllCompleted(int resultCount, const QString& summary);
    void searchResultSelected(const SearchResult& result);
    void replaceOperationRequested(const ReplaceOperation& operation);
    void itemsReplaced(int count, const QString& summary);

public slots:
    void onFind();
    void onFindNext();
    void onGoToSelected();
    void onBrowseTile();
    void onRefresh();
    void onClearResults();

    // Task 82: Enhanced search slots
    void onFindAll();
    void onReplaceSelected();
    void onReplaceAll();
    void onDeleteSelected();
    void onSwapItems();
    void onExportResults();
    void onImportSearchCriteria();

private slots:
    // Search mode changes
    void onSearchModeChanged();
    void onServerIdChanged();
    void onClientIdChanged();
    void onNameTextChanged();
    void onTypeSelectionChanged();
    void onPropertyChanged();
    
    // Advanced options
    void onRangeToggled(bool enabled);
    void onIgnoreIdsToggled(bool enabled);
    void onAutoRefreshToggled(bool enabled);
    void onMaxResultsChanged(int value);
    
    // Results handling
    void onResultItemClicked();
    void onResultItemDoubleClicked();
    void onResultSelectionChanged();

    // Timer for auto-refresh
    void onRefreshTimer();

    // Task 82: Enhanced search slots
    void onSearchProgress(int current, int total, const QString& currentItem);
    void onSearchCompleted(int resultCount, const QString& summary);
    void onSearchError(const QString& error, const QString& details);
    void onResultFound(const SearchResult& result);
    void onReplaceProgress(int current, int total, const QString& currentItem);
    void onReplaceCompleted(int replacedCount, const QString& summary);
    void onItemReplaced(const SearchResult& oldResult, const SearchResult& newResult);

    // Results tree handling
    void onResultTreeItemClicked(QTreeWidgetItem* item, int column);
    void onResultTreeItemDoubleClicked(QTreeWidgetItem* item, int column);
    void onResultTreeSelectionChanged();
    void onResultTreeContextMenu(const QPoint& pos);

private:
    // UI setup methods
    void setupUI();
    void setupSearchModeControls();
    void setupIdSearchControls();
    void setupNameSearchControls();
    void setupTypeSearchControls();
    void setupPropertySearchControls();
    void setupAdvancedOptions();
    void setupResultsArea();
    void setupActionButtons();
    void connectSignals();
    
    // Search functionality
    void performSearch();
    void buildSearchCriteria(ItemFinder::SearchCriteria& criteria);
    void populateResults(const QList<ItemFinder::ItemResult>& results);
    void clearResults();
    void updateResultsCount();
    void updateButtonStates();

    // Task 82: Enhanced search functionality
    void buildEnhancedSearchCriteria(SearchCriteria& criteria);
    void populateEnhancedResults(const QList<SearchResult>& results);
    void setupResultsTree();
    void addResultToTree(const SearchResult& result);
    void updateResultsTree();
    void clearResultsTree();
    void exportResultsToFile(const QString& filePath);
    void importSearchCriteriaFromFile(const QString& filePath);

    // Task 82: Replace functionality
    void setupReplaceControls();
    void showReplaceDialog(const QList<SearchResult>& results);
    void executeReplaceOperation(const ReplaceOperation& operation);
    void validateReplaceOperation(const ReplaceOperation& operation);
    bool confirmReplaceOperation(const ReplaceOperation& operation);
    
    // Property management
    void enablePropertyControls(bool enabled);
    void resetPropertyControls();
    void setupPropertyTooltips();
    
    // Range and ignore ID parsing
    QVector<QPair<quint16, quint16>> parseIdRanges(const QString& rangeText);
    QSet<quint16> parseIgnoredIds(const QString& ignoredText);
    bool isIdInRanges(quint16 id, const QVector<QPair<quint16, quint16>>& ranges);
    bool isIdIgnored(quint16 id);
    
    // Utility methods
    void populateTypeComboBox();
    void populateTilesetCategories();
    QString getItemDisplayName(quint16 itemId);
    QString getItemTypeName(SearchItemType type);
    SearchItemType getSelectedItemType();
    
    // Main UI structure
    QVBoxLayout* mainLayout_;
    QSplitter* mainSplitter_;
    
    // Search mode controls
    QGroupBox* searchModeGroup_;
    QButtonGroup* searchModeButtonGroup_;
    QRadioButton* serverIdModeButton_;
    QRadioButton* clientIdModeButton_;
    QRadioButton* nameModeButton_;
    QRadioButton* typeModeButton_;
    QRadioButton* propertyModeButton_;
    
    // ID search controls
    QGroupBox* idSearchGroup_;
    QSpinBox* serverIdSpinBox_;
    QSpinBox* clientIdSpinBox_;
    QCheckBox* invalidItemCheckBox_;
    
    // Range search controls
    QGroupBox* rangeGroup_;
    QCheckBox* useRangeCheckBox_;
    QLineEdit* rangeLineEdit_;
    
    // Ignored IDs controls
    QGroupBox* ignoredIdsGroup_;
    QCheckBox* ignoreIdsCheckBox_;
    QLineEdit* ignoredIdsLineEdit_;
    
    // Name search controls
    QGroupBox* nameSearchGroup_;
    QLineEdit* nameLineEdit_;
    QCheckBox* caseSensitiveCheckBox_;
    QCheckBox* wholeWordCheckBox_;
    
    // Type search controls
    QGroupBox* typeSearchGroup_;
    QComboBox* typeComboBox_;
    
    // Property search controls
    QGroupBox* propertyGroup_;
    QScrollArea* propertyScrollArea_;
    QWidget* propertyWidget_;
    QGridLayout* propertyLayout_;
    
    // Property checkboxes (3-state from wxwidgets)
    QCheckBox* unpassableCheckBox_;
    QCheckBox* unmovableCheckBox_;
    QCheckBox* blockMissilesCheckBox_;
    QCheckBox* blockPathfinderCheckBox_;
    QCheckBox* readableCheckBox_;
    QCheckBox* writeableCheckBox_;
    QCheckBox* pickupableCheckBox_;
    QCheckBox* stackableCheckBox_;
    QCheckBox* rotatableCheckBox_;
    QCheckBox* hangableCheckBox_;
    QCheckBox* hookEastCheckBox_;
    QCheckBox* hookSouthCheckBox_;
    QCheckBox* hasElevationCheckBox_;
    QCheckBox* ignoreLookCheckBox_;
    QCheckBox* floorChangeCheckBox_;
    QCheckBox* hasLightCheckBox_;
    
    // Slot property checkboxes
    QCheckBox* slotHeadCheckBox_;
    QCheckBox* slotNecklaceCheckBox_;
    QCheckBox* slotBackpackCheckBox_;
    QCheckBox* slotArmorCheckBox_;
    QCheckBox* slotLegsCheckBox_;
    QCheckBox* slotFeetCheckBox_;
    QCheckBox* slotRingCheckBox_;
    QCheckBox* slotAmmoCheckBox_;
    
    // Advanced options
    QGroupBox* advancedGroup_;
    QCheckBox* autoRefreshCheckBox_;
    QSpinBox* maxResultsSpinBox_;
    QCheckBox* searchInSelectionCheckBox_;
    QPushButton* browseTileButton_;
    
    // Results area
    QGroupBox* resultsGroup_;
    QTableWidget* resultsTable_;
    QLabel* resultsCountLabel_;
    QProgressBar* searchProgressBar_;
    
    // Action buttons
    QHBoxLayout* buttonLayout_;
    QPushButton* findButton_;
    QPushButton* findNextButton_;
    QPushButton* goToSelectedButton_;
    QPushButton* refreshButton_;
    QPushButton* clearResultsButton_;
    QPushButton* closeButton_;

    // Task 82: Enhanced action buttons
    QPushButton* findAllButton_;
    QPushButton* replaceSelectedButton_;
    QPushButton* replaceAllButton_;
    QPushButton* deleteSelectedButton_;
    QPushButton* swapItemsButton_;
    QPushButton* exportResultsButton_;
    QPushButton* importCriteriaButton_;

    // Task 82: Enhanced results display
    QTabWidget* resultsTabWidget_;
    QTreeWidget* resultsTree_;
    QTextEdit* resultsDetailsText_;
    QLabel* searchStatusLabel_;
    
    // Data members
    Map* map_;
    MapView* mapView_;
    ItemManager* itemManager_;
    TilesetManager* tilesetManager_;
    QUndoStack* undoStack_; // Task 82: Undo support

    quint16 resultId_;
    bool onlyPickupables_;
    QList<ItemFinder::ItemResult> currentResults_;
    int currentResultIndex_;

    // Auto-refresh timer
    QTimer* refreshTimer_;

    // Search state
    bool searchInProgress_;
    QString lastSearchText_;
    SearchMode lastSearchMode_;

    // Task 82: Enhanced search components
    MapSearcher* mapSearcher_;
    QList<SearchResult> currentSearchResults_;
    QList<SearchResult> selectedSearchResults_;

    // Task 82: Replace state
    bool replaceInProgress_;
    ReplaceOperation currentReplaceOperation_;

    // Task 82: Statistics
    int totalSearchTime_;
    int lastSearchResultCount_;
    QString lastSearchSummary_;
};

#endif // FINDITEMDIALOG_H
