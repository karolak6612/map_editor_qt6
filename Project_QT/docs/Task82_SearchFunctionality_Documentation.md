# Task 82: Migrate and Enhance Search Functionality Documentation

## Overview

Task 82 implements comprehensive search functionality for the Qt6 map editor, providing complete migration and enhancement of the original wxwidgets search system. This implementation includes enhanced backend search logic, "Find All" functionality, replace operations with undo support, and full ItemTypes integration.

## Features Implemented

### 1. **Enhanced Backend Search Logic (MapSearcher)**
- **MapSearcher Class**: Core search engine with advanced filtering capabilities
- **SearchCriteria Structure**: Comprehensive search parameter configuration
- **SearchResult Structure**: Detailed result information with metadata
- **Performance Optimization**: Efficient tile iteration and result caching

### 2. **"Find All" Functionality with Results Display**
- Complete search result collection and display
- Interactive results tree with detailed information
- Progress tracking and cancellation support
- Export/import functionality for search results

### 3. **Replace Functionality with QUndoCommand Support**
- **ReplaceItemCommand**: Full undo/redo support for replace operations
- **BatchReplaceCommand**: Batch operations with single undo command
- **SwapItemsCommand**: Bidirectional item swapping with undo
- **DeleteItemsCommand**: Item deletion with restoration capability

### 4. **ItemTypes Integration for Property-Based Searching**
- Property-based search with ItemTypes metadata
- Type category filtering and classification
- Advanced property matching and validation
- Integration with item attribute system

### 5. **Enhanced FindItemDialog UI**
- Enhanced UI with tabbed results display
- Real-time search progress and status updates
- Replace dialog integration with operation preview
- Context menus and keyboard shortcuts

## Architecture

### Core Components

#### MapSearcher
```cpp
class MapSearcher : public QObject
{
    // Main search operations
    QList<SearchResult> findAllItems(Map* map, const SearchCriteria& criteria);
    QList<SearchResult> findItemsInArea(Map* map, const QRect& area, const SearchCriteria& criteria);
    SearchResult findNextItem(Map* map, const SearchCriteria& criteria, const QPoint& startPos);
    
    // Specialized search methods (1:1 wxwidgets compatibility)
    QList<SearchResult> findItemsByServerId(Map* map, quint16 serverId, int maxResults);
    QList<SearchResult> findItemsByName(Map* map, const QString& name, bool exactMatch, int maxResults);
    
    // Replace operations
    int replaceItems(Map* map, const ReplaceOperation& operation);
    int deleteItems(Map* map, const QList<SearchResult>& results);
};
```

#### SearchCriteria
```cpp
struct SearchCriteria {
    enum SearchMode {
        SERVER_IDS,     // Search by server IDs
        CLIENT_IDS,     // Search by client IDs
        NAMES,          // Search by item names
        TYPES,          // Search by item types
        PROPERTIES      // Search by item properties
    };
    
    SearchMode mode;
    
    // ID-based search
    quint16 serverId;
    QList<quint16> serverIdList;
    QPair<quint16, quint16> serverIdRange;
    
    // Name-based search
    QString itemName;
    bool exactNameMatch;
    bool caseSensitive;
    bool wholeWordOnly;
    
    // Property-based search
    QVariantMap requiredProperties;
    QVariantMap excludedProperties;
    
    // Advanced filtering
    QStringList layers;
    QRect searchArea;
    bool searchInSelection;
    int maxResults;
};
```

#### ReplaceOperation
```cpp
struct ReplaceOperation {
    enum ReplaceMode {
        REPLACE_ITEM_ID,        // Replace with different item ID
        REPLACE_PROPERTIES,     // Replace specific properties
        DELETE_ITEMS,           // Delete matching items
        SWAP_ITEMS             // Swap two item types
    };
    
    ReplaceMode mode;
    SearchCriteria sourceCriteria;
    quint16 targetItemId;
    QVariantMap targetProperties;
    
    enum ReplaceScope {
        REPLACE_ALL_MATCHING,
        REPLACE_SELECTED_RESULTS,
        REPLACE_IN_SELECTION
    };
    
    ReplaceScope scope;
    bool createBackup;
    bool updateBorders;
};
```

## Search Functionality

### Basic Search Operations
```cpp
// Server ID search (1:1 wxwidgets compatibility)
QList<SearchResult> results = mapSearcher->findItemsByServerId(map, 100, 1000);

// Name search with options
QList<SearchResult> results = mapSearcher->findItemsByName(map, "stone", false, 500);

// Type-based search
QList<SearchResult> results = mapSearcher->findItemsByType(map, "ground", 200);
```

### Advanced Search Operations
```cpp
// Property-based search
SearchCriteria criteria;
criteria.mode = SearchCriteria::PROPERTIES;
criteria.requiredProperties["stackable"] = true;
criteria.requiredProperties["pickupable"] = true;
criteria.excludedProperties["movable"] = false;
criteria.maxResults = 1000;

QList<SearchResult> results = mapSearcher->findAllItems(map, criteria);
```

### Area and Selection Search
```cpp
// Search in specific area
QRect searchArea(10, 10, 100, 100);
QList<SearchResult> results = mapSearcher->findItemsInArea(map, searchArea, criteria);

// Search in current selection
criteria.searchInSelection = true;
QList<SearchResult> results = mapSearcher->findItemsInSelection(map, criteria);
```

## Replace Functionality

### Basic Replace Operations
```cpp
// Replace all items with specific ID
ReplaceOperation operation;
operation.mode = ReplaceOperation::REPLACE_ITEM_ID;
operation.sourceCriteria.mode = SearchCriteria::SERVER_IDS;
operation.sourceCriteria.serverId = 100;
operation.targetItemId = 101;
operation.scope = ReplaceOperation::REPLACE_ALL_MATCHING;

int replacedCount = mapSearcher->replaceItems(map, operation);
```

### Undo/Redo Support
```cpp
// Create replace command with undo support
ReplaceItemCommand* command = new ReplaceItemCommand(map, "Replace Items");
command->addReplacementsFromResults(searchResults, newItemId);
command->setUpdateBorders(true);
command->setMapView(mapView);

// Execute with undo stack
undoStack->push(command);

// Undo/redo operations
undoStack->undo(); // Undo replace
undoStack->redo(); // Redo replace
```

### Batch Operations
```cpp
// Batch replace with single undo command
BatchReplaceCommand* batchCommand = new BatchReplaceCommand(map, operation, "Batch Replace");
undoStack->push(batchCommand);

// Swap items across entire map
SwapItemsCommand* swapCommand = new SwapItemsCommand(map, itemId1, itemId2, "Swap Items");
swapCommand->setUpdateBorders(true);
undoStack->push(swapCommand);
```

## ItemTypes Integration

### Property-Based Search
```cpp
// Search using ItemTypes properties
SearchCriteria criteria;
criteria.mode = SearchCriteria::PROPERTIES;
criteria.requiredProperties["blockSolid"] = true;
criteria.requiredProperties["movable"] = false;
criteria.itemTypeCategories << "Ground" << "Wall";

QList<SearchResult> results = mapSearcher->findAllItems(map, criteria);
```

### Type Category Filtering
```cpp
// Search by item type categories
SearchCriteria criteria;
criteria.mode = SearchCriteria::TYPES;
criteria.itemTypeCategories << "Decoration" << "Container" << "Readable";
criteria.maxResults = 500;

QList<SearchResult> results = mapSearcher->findAllItems(map, criteria);
```

## Enhanced FindItemDialog

### Find All Functionality
```cpp
// Enhanced dialog with Find All support
FindItemDialog* dialog = new FindItemDialog(parent, "Enhanced Find Item Dialog");
dialog->setMapSearcher(mapSearcher);
dialog->setUndoStack(undoStack);

// Find All operation
QList<SearchResult> allResults = dialog->findAllItems();

// Get selected results for operations
if (dialog->hasSelectedResults()) {
    QList<SearchResult> selected = dialog->getSelectedResults();
    dialog->showReplaceDialog(); // Show replace options
}
```

### Replace Dialog Integration
```cpp
// Show replace dialog for selected results
dialog->showReplaceWithDialog();
dialog->showDeleteSelectedDialog();

// Handle replace operations
connect(dialog, &FindItemDialog::replaceOperationRequested,
        this, &MainWindow::executeReplaceOperation);
```

## Testing

### Test Application
The comprehensive test application (`SearchFunctionalityTest.cpp`) provides:

- **Interactive UI**: Full-featured test interface with search controls
- **Real-time Testing**: Live search operations with progress tracking
- **Results Display**: Tree view with detailed search result information
- **Replace Testing**: Complete replace functionality testing with undo/redo

### Test Features
- ✅ Basic search functionality (ID, name, type)
- ✅ Advanced search with properties and areas
- ✅ Find All functionality with results display
- ✅ Replace functionality with undo/redo support
- ✅ ItemTypes integration for property-based searching
- ✅ UI integration with MapView updates
- ✅ Performance testing and search limits
- ✅ Error handling and cancellation

### Running Tests
```bash
# Build test application
mkdir build && cd build
cmake -f ../tests/CMakeLists_SearchFunctionalityTest.txt ..
cmake --build .

# Run test application
./SearchFunctionalityTest
```

## Integration Points

### MapView Integration
- Real-time result highlighting and navigation
- Search result visualization with overlays
- Interactive result selection and manipulation

### ItemManager Integration
- ItemTypes property access and validation
- Item creation and modification support
- Dependency resolution and validation

### Undo System Integration
- Complete undo/redo support for all operations
- Command merging for batch operations
- State preservation and restoration

## Performance Considerations

### Optimization Features
- **Efficient Tile Iteration**: Optimized map traversal algorithms
- **Result Caching**: Smart caching of search results
- **Progress Tracking**: Real-time progress updates with cancellation
- **Memory Management**: Efficient memory usage for large result sets

### Search Limits
- Configurable maximum result limits
- Timeout protection for long-running searches
- Memory usage monitoring and optimization
- Background search processing support

## Error Handling

### Comprehensive Error Management
- **Search Errors**: Graceful handling of search failures
- **Replace Errors**: Validation and rollback for failed operations
- **UI Errors**: User-friendly error messages and recovery
- **Performance Errors**: Timeout and resource limit handling

### User Feedback
- Real-time progress tracking with cancellation
- Detailed error messages with context
- Operation summaries and statistics
- Recovery suggestions and options

## Future Enhancements

### Planned Features
- **Regex Search**: Regular expression support for advanced patterns
- **Saved Searches**: Search criteria persistence and reuse
- **Search History**: Recent search tracking and quick access
- **Plugin System**: Extensible search criteria and operations

### Performance Improvements
- **Parallel Search**: Multi-threaded search processing
- **Index-based Search**: Pre-built search indices for faster results
- **Incremental Search**: Real-time search as user types
- **Smart Caching**: Intelligent result caching and invalidation

## Conclusion

Task 82 provides a complete, production-ready search functionality system that:

1. **Maintains wxwidgets Compatibility**: Full 1:1 migration of original functionality
2. **Adds Modern Features**: Enhanced UI, undo support, advanced filtering
3. **Ensures Performance**: Optimized algorithms and progress tracking
4. **Provides Extensibility**: Pluggable architecture for future enhancements
5. **Enables Integration**: Seamless integration with MapView and ItemManager

The implementation successfully migrates all search functionality from the original wxwidgets system while adding significant improvements in usability, performance, and maintainability.
