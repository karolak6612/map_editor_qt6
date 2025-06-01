# Task 84: Map Loaders and Converters Documentation

## Overview

Task 84 implements comprehensive map loaders and converters for the Qt6 map editor, providing complete replacement of the wxwidgets BinaryFile I/O system and support for all known map formats. This implementation includes format identification, version detection, conversion logic, and full structure restoration with MapView integration.

## Features Implemented

### 1. **Complete BinaryFile I/O Replacement with Qt**
- **QtBinaryFile System**: Complete replacement using QFile and QDataStream
- **Node-Based File Structure**: Full support for hierarchical binary file formats
- **Memory and Disk Operations**: Both file-based and memory-based I/O
- **Performance Optimization**: Efficient buffering and caching systems

### 2. **Support for All Known Map Formats**
- **OTBM Format**: Complete OpenTibia Binary Map support (versions 1-4)
- **OTMM Format**: Legacy OpenTibia Memory Map support (version 1)
- **XML Format**: Component files (spawns, houses, waypoints)
- **JSON Format**: Modern serialization support
- **Compressed Formats**: Support for compressed OTBM files

### 3. **Format Identification and Version Detection**
- **Automatic Detection**: Header-based format identification
- **Version Analysis**: Complete version detection for all formats
- **Compatibility Checking**: Version support validation
- **Metadata Extraction**: Format capabilities and limitations

### 4. **Version Conversion Logic**
- **OTBM Version Conversion**: Between all OTBM versions (1-4)
- **Client Version Conversion**: Between all client versions (740-1300+)
- **Item ID Mapping**: Complete item ID translation tables
- **Attribute Conversion**: Property and flag conversion logic

### 5. **Full Structure Restoration and MapView Integration**
- **Complete Map Loading**: All map components (tiles, items, spawns, houses, towns, waypoints)
- **Progress Tracking**: Real-time loading progress with cancellation
- **Error Handling**: Comprehensive error reporting and recovery
- **MapView Integration**: Seamless integration with map display system

## Architecture

### Core Components

#### MapFormatManager
```cpp
class MapFormatManager : public QObject
{
    // Format detection and identification
    FormatDetectionResult detectFormat(const QString& filePath) const;
    MapFormat getFormatFromExtension(const QString& extension) const;
    QStringList getSupportedFormats() const;
    
    // Map loading operations
    bool loadMap(Map* map, const QString& filePath);
    bool loadMapByFormat(Map* map, const QString& filePath, MapFormat format);
    
    // Map saving operations
    bool saveMap(Map* map, const QString& filePath) const;
    bool saveMapByFormat(Map* map, const QString& filePath, MapFormat format) const;
    
    // Version conversion operations
    bool convertMapVersion(Map* map, const MapVersion& fromVersion, const MapVersion& toVersion);
};
```

#### QtBinaryFile System
```cpp
// Basic file operations
class QtFileReadHandle : public QtFileHandle
{
    bool getU8(quint8& value);
    bool getU16(quint16& value);
    bool getU32(quint32& value);
    bool getString(QString& str);
    bool getRAW(QByteArray& data, qint64 size);
};

class QtFileWriteHandle : public QtFileHandle
{
    bool addU8(quint8 value);
    bool addU16(quint16 value);
    bool addU32(quint32 value);
    bool addString(const QString& str);
    bool addRAW(const QByteArray& data);
};

// Node-based operations
class QtNodeFileReadHandle : public QtFileHandle
{
    QtBinaryNode* getRootNode();
    bool open(const QString& fileName, const QStringList& acceptableIdentifiers);
};

class QtNodeFileWriteHandle : public QtFileHandle
{
    bool addNode(quint8 nodeType);
    bool endNode();
    bool open(const QString& fileName, const QString& identifier);
};
```

#### OTBM Map Loader
```cpp
class OTBMMapLoader : public QObject
{
    // Main loading operations
    bool loadMap(Map* map, const QString& filePath);
    bool loadMapFromHandle(Map* map, QtNodeFileReadHandle* handle);
    
    // Version detection
    OTBMVersion detectVersion(const QString& filePath) const;
    QList<OTBMVersion> getSupportedVersions() const;
    
    // Component loading
    bool loadSpawns(Map* map, const QString& filePath);
    bool loadHouses(Map* map, const QString& filePath);
    bool loadWaypoints(Map* map, const QString& filePath);
};
```

#### Map Version Converter
```cpp
class MapVersionConverter : public QObject
{
    // Main conversion operations
    bool convertMap(Map* map, const MapVersion& targetVersion);
    bool convertMapOTBMVersion(Map* map, MapVersion::OTBMVersion targetVersion);
    bool convertMapClientVersion(Map* map, MapVersion::ClientVersion targetVersion);
    
    // Item ID conversion
    bool convertItemIds(Map* map, MapVersion::ClientVersion sourceVersion, MapVersion::ClientVersion targetVersion);
    quint16 convertItemId(quint16 sourceId, MapVersion::ClientVersion sourceVersion, MapVersion::ClientVersion targetVersion) const;
    
    // Conversion path analysis
    QList<MapVersion> getConversionPath(const MapVersion& sourceVersion, const MapVersion& targetVersion) const;
    bool isConversionSupported(const MapVersion& sourceVersion, const MapVersion& targetVersion) const;
};
```

## Qt Binary File I/O System

### Basic File Operations (1:1 wxwidgets compatibility)
```cpp
// Reading operations
QtFileReadHandle readHandle;
if (readHandle.open("map.otbm", QIODevice::ReadOnly)) {
    quint32 signature;
    quint16 version;
    QString description;
    
    readHandle.getU32(signature);
    readHandle.getU16(version);
    readHandle.getString(description);
    
    readHandle.close();
}

// Writing operations
QtFileWriteHandle writeHandle;
if (writeHandle.open("map.otbm", QIODevice::WriteOnly)) {
    writeHandle.addU32(0x12345678);
    writeHandle.addU16(1);
    writeHandle.addString("Test Map");
    
    writeHandle.close();
}
```

### Node-Based File Operations
```cpp
// Reading node-based files
QStringList acceptableIds = {"OTBM"};
QtNodeFileReadHandle nodeHandle;
if (nodeHandle.open("map.otbm", acceptableIds)) {
    QtBinaryNode* rootNode = nodeHandle.getRootNode();
    if (rootNode) {
        quint32 version;
        rootNode->getU32(version);
        
        QtBinaryNode* childNode = rootNode->getChild();
        while (childNode) {
            // Process child node
            childNode = childNode->advance();
        }
    }
    nodeHandle.close();
}

// Writing node-based files
QtNodeFileWriteHandle nodeWriteHandle;
if (nodeWriteHandle.open("map.otbm", "OTBM")) {
    nodeWriteHandle.addNode(1); // Root node
    nodeWriteHandle.addU32(4); // Version
    
    nodeWriteHandle.addNode(2); // Child node
    nodeWriteHandle.addString("Map Data");
    nodeWriteHandle.endNode(); // End child
    
    nodeWriteHandle.endNode(); // End root
    nodeWriteHandle.close();
}
```

### Memory-Based Operations
```cpp
// Memory-based reading
QByteArray mapData = loadMapDataFromSomewhere();
QtMemoryNodeFileReadHandle memoryHandle;
if (memoryHandle.open(mapData, acceptableIds)) {
    QtBinaryNode* rootNode = memoryHandle.getRootNode();
    // Process nodes...
    memoryHandle.close();
}

// Memory-based writing
QtMemoryNodeFileWriteHandle memoryWriteHandle;
memoryWriteHandle.addNode(1);
memoryWriteHandle.addString("Test");
memoryWriteHandle.endNode();
QByteArray result = memoryWriteHandle.getData();
```

## Map Format Support

### OTBM Format (OpenTibia Binary Map)
```cpp
// OTBM version enumeration (1:1 wxwidgets compatibility)
enum class OTBMVersion : quint32 {
    OTBM_VERSION_1 = 0,
    OTBM_VERSION_2 = 1,
    OTBM_VERSION_3 = 2,
    OTBM_VERSION_4 = 3
};

// OTBM node types (1:1 wxwidgets compatibility)
enum class OTBMNodeType : quint8 {
    OTBM_ROOT_V1 = 1,
    OTBM_MAP_DATA = 2,
    OTBM_ITEM_DEF = 3,
    OTBM_TILE_AREA = 4,
    OTBM_TILE = 5,
    OTBM_ITEM = 6,
    OTBM_SPAWNS = 9,
    OTBM_TOWNS = 12,
    OTBM_WAYPOINTS = 15,
    OTBM_ROOT_V4 = 19
};

// Loading OTBM maps
OTBMMapLoader loader;
if (loader.loadMap(map, "map.otbm")) {
    OTBMLoadingStatistics stats = loader.getLoadingStatistics();
    qDebug() << "Loaded" << stats.totalTiles << "tiles," << stats.totalItems << "items";
}
```

### OTMM Format (OpenTibia Memory Map)
```cpp
// OTMM version enumeration
enum class OTMMVersion : quint32 {
    OTMM_VERSION_1 = 0
};

// Loading OTMM maps
OTMMMapLoader loader;
if (loader.loadMap(map, "map.otmm")) {
    OTMMLoadingStatistics stats = loader.getLoadingStatistics();
    qDebug() << "Loaded" << stats.totalTiles << "tiles," << stats.totalItems << "items";
}
```

### Format Detection
```cpp
// Automatic format detection
MapFormatManager formatManager;
FormatDetectionResult result = formatManager.detectFormat("unknown_map_file");

if (result.isValid) {
    qDebug() << "Detected format:" << formatManager.getFormatName(result.format);
    qDebug() << "Version:" << result.version.toString();
    qDebug() << "Description:" << result.description;
} else {
    qDebug() << "Detection failed:" << result.errorMessage;
}

// Extension-based detection
MapFormat format = formatManager.getFormatFromExtension("otbm");
QStringList supportedExtensions = formatManager.getSupportedExtensions();
```

## Version Conversion System

### Map Version Structure
```cpp
struct MapVersion {
    enum OTBMVersion {
        OTBM_VERSION_1 = 0,
        OTBM_VERSION_2 = 1,
        OTBM_VERSION_3 = 2,
        OTBM_VERSION_4 = 3
    };
    
    enum ClientVersion {
        CLIENT_VERSION_740 = 740,
        CLIENT_VERSION_800 = 800,
        CLIENT_VERSION_860 = 860,
        CLIENT_VERSION_1200 = 1200,
        CLIENT_VERSION_1300 = 1300
        // ... all client versions supported
    };
    
    OTBMVersion otbm;
    ClientVersion client;
};
```

### Version Conversion Operations
```cpp
// Basic version conversion
MapVersionConverter converter;
MapVersion sourceVersion(MapVersion::OTBM_VERSION_3, MapVersion::CLIENT_VERSION_860);
MapVersion targetVersion(MapVersion::OTBM_VERSION_4, MapVersion::CLIENT_VERSION_1200);

if (converter.convertMap(map, targetVersion)) {
    ConversionStatistics stats = converter.getConversionStatistics();
    qDebug() << "Converted" << stats.itemsConverted << "items";
} else {
    qDebug() << "Conversion failed:" << converter.getLastError();
}

// Item ID conversion
quint16 oldItemId = 100;
quint16 newItemId = converter.convertItemId(oldItemId, 
                                           MapVersion::CLIENT_VERSION_860,
                                           MapVersion::CLIENT_VERSION_1200);

// Conversion path analysis
QList<MapVersion> path = converter.getConversionPath(sourceVersion, targetVersion);
for (const MapVersion& step : path) {
    qDebug() << "Conversion step:" << step.toString();
}
```

### Item ID Mapping
```cpp
// Item ID mapping structure
struct ItemIdMapping {
    quint16 sourceId;
    quint16 targetId;
    QString sourceName;
    QString targetName;
    QVariantMap attributeChanges;
};

// Managing item ID mappings
ItemIdMappingManager mappingManager;
mappingManager.loadMappingTable("item_mappings.xml");

ItemIdMapping mapping = mappingManager.getMapping(100, 
                                                 MapVersion::CLIENT_VERSION_860,
                                                 MapVersion::CLIENT_VERSION_1200);
if (mapping.isValid) {
    qDebug() << "Item" << mapping.sourceId << "maps to" << mapping.targetId;
}
```

## Map Loading and Saving

### Complete Map Loading
```cpp
// Using MapFormatManager for automatic format detection
MapFormatManager formatManager;
Map* map = new Map();

if (formatManager.loadMap(map, "map.otbm")) {
    MapLoadingStatistics stats = formatManager.getLastLoadingStatistics();
    qDebug() << "Map loaded successfully:";
    qDebug() << "  Tiles:" << stats.totalTiles;
    qDebug() << "  Items:" << stats.totalItems;
    qDebug() << "  Spawns:" << stats.totalSpawns;
    qDebug() << "  Houses:" << stats.totalHouses;
    qDebug() << "  Towns:" << stats.totalTowns;
    qDebug() << "  Waypoints:" << stats.totalWaypoints;
    qDebug() << "  Loading time:" << stats.loadingTime << "ms";
    
    if (!stats.warnings.isEmpty()) {
        qDebug() << "Warnings:" << stats.warnings;
    }
} else {
    qDebug() << "Loading failed:" << formatManager.getLastError();
}
```

### Map Saving
```cpp
// Save in different formats
if (formatManager.saveMapByFormat(map, "map.otbm", MapFormat::OTBM)) {
    qDebug() << "Map saved in OTBM format";
}

if (formatManager.saveMapByFormat(map, "map.xml", MapFormat::XML)) {
    qDebug() << "Map saved in XML format";
}

// Save with specific version
MapVersion targetVersion(MapVersion::OTBM_VERSION_4, MapVersion::CLIENT_VERSION_1200);
if (formatManager.saveMapWithVersion(map, "map_v4.otbm", targetVersion)) {
    qDebug() << "Map saved with version" << targetVersion.toString();
}
```

### Component Loading
```cpp
// Load individual components
OTBMMapLoader loader;

// Load spawns
if (loader.loadSpawns(map, "spawns.xml")) {
    qDebug() << "Spawns loaded successfully";
}

// Load houses
if (loader.loadHouses(map, "houses.xml")) {
    qDebug() << "Houses loaded successfully";
}

// Load waypoints
if (loader.loadWaypoints(map, "waypoints.xml")) {
    qDebug() << "Waypoints loaded successfully";
}
```

## Testing

### Test Application
The comprehensive test application (`MapIOTest.cpp`) provides:

- **Interactive UI**: Full-featured test interface with I/O controls
- **Real-time Testing**: Live map loading and conversion operations
- **Format Testing**: Complete testing of all supported formats
- **Performance Testing**: Large file handling and optimization testing

### Test Features
- ✅ Qt Binary File I/O system (QFile, QDataStream replacement)
- ✅ Format detection and identification
- ✅ OTBM map loading (all versions 1-4)
- ✅ OTMM map loading (version 1)
- ✅ Version conversion logic with item ID mapping
- ✅ Map saving in multiple formats
- ✅ Performance and limits testing
- ✅ Error handling and recovery

### Running Tests
```bash
# Build test application
mkdir build && cd build
cmake -f ../tests/CMakeLists_MapIOTest.txt ..
cmake --build .

# Run test application
./MapIOTest
```

## Performance Considerations

### Optimization Features
- **Efficient Buffering**: Optimized buffer sizes for different file operations
- **Memory Management**: Smart memory usage for large map files
- **Caching Systems**: Intelligent caching of frequently accessed data
- **Progress Tracking**: Real-time progress updates with cancellation support

### Performance Settings
```cpp
// Optimize file access
QtBinaryFileUtils::optimizeFileAccess(file);
qint64 optimalBufferSize = QtBinaryFileUtils::getOptimalBufferSize(fileSize);

// Enable caching for better performance
nodeHandle.setCacheSize(1024 * 1024); // 1MB cache
nodeHandle.renewCache();

// Batch operations for efficiency
loader.setValidationEnabled(false); // Skip validation for speed
loader.setStrictMode(false); // Allow minor format variations
```

## Integration Points

### MapView Integration
- Real-time loading progress with visual feedback
- Automatic map display updates after loading
- Error reporting with user-friendly messages

### ItemManager Integration
- Item type validation during loading
- Item property extraction and conversion
- Dependency resolution for complex items

### Error Handling Integration
- Comprehensive error reporting with context
- Recovery mechanisms for corrupted files
- Validation and integrity checking

## Future Enhancements

### Planned Features
- **Parallel Loading**: Multi-threaded map loading for improved performance
- **Streaming Support**: Large map streaming for memory efficiency
- **Plugin System**: Extensible format support through plugins
- **Advanced Compression**: Better compression algorithms for smaller files

### Performance Improvements
- **Memory Mapping**: Direct memory mapping for very large files
- **Incremental Loading**: Load only visible map sections
- **Background Processing**: Background loading and conversion operations
- **Smart Caching**: Intelligent cache management based on usage patterns

## Conclusion

Task 84 provides a complete, production-ready map I/O and conversion system that:

1. **Maintains wxwidgets Compatibility**: Full 1:1 migration of original functionality
2. **Adds Modern Features**: Qt-based I/O, enhanced error handling, progress tracking
3. **Ensures Performance**: Optimized algorithms and efficient memory usage
4. **Provides Extensibility**: Modular architecture for future format support
5. **Enables Integration**: Seamless integration with MapView and ItemManager

The implementation successfully replaces the entire wxwidgets BinaryFile system while adding significant improvements in reliability, performance, and maintainability.
