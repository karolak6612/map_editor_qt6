# CMakeLists.txt (Generated based on repomix-output-src.zip.xml)

cmake_minimum_required(VERSION 3.16)
project(MapEditorQt6 VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable Qt's MOC, RCC, and UIC
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Find required Qt6 packages
find_package(Qt6 COMPONENTS Core Gui Widgets Xml Network REQUIRED)

# --- Header Files ---
set(PROJECT_HEADERS
    src/Animator.h
    src/ApplicationSettingsManager.h
    src/AutoBorder.h
    src/AutoBorderData.h
    src/BorderSystem.h
    src/Brush.h
    src/BrushCommon.h
    src/BrushInteractionController.h
    src/BrushManager.h
    src/BrushPersistence.h
    src/CarpetBrush.h
    src/ClipboardData.h
    src/ClipboardManager.h
    src/ContainerItem.h
    src/Creature.h
    src/CreatureBrush.h
    src/CreatureManager.h
    src/CreatureProperties.h
    src/CreatureSpriteManager.h
    src/DepotItem.h
    src/DeselectCommand.h
    src/DoodadBrush.h
    src/DoorBrush.h
    src/DoorItem.h
    src/DrawingOptions.h
    src/EditorSprite.h
    src/EditorSpriteIds.h
    src/EnhancedDrawingOptions.h
    src/EnhancedLightingSystem.h
    src/EnhancedMapRenderer.h
    src/EraseCommand.h
    src/EraserBrush.h
    src/FlagBrush.h
    src/GameSprite.h
    src/GraphicManager.h
    src/GroundBorderTool.h
    src/GroundBrush.h
    src/House.h
    src/HouseBrush.h
    src/HouseExitBrush.h
    src/ImageSpace.h
    src/Item.h
    src/ItemFactory.h
    src/ItemFinder.h
    src/ItemManager.h
    src/ItemRenderer.h
    src/LightDrawer.h
    src/LODManager.h
    src/Map.h
    src/MapDrawingPrimitives.h
    src/MapEntityManager.h
    src/MapIO.h
    src/MapIterator.h
    # src/MapPos.h  # REMOVED: Conflicts with MapPos struct in Map.h
    src/MapSearcher.h
    src/MapUtilities.h
    src/MapView.h
    src/MapViewEnhancements.h
    src/MapViewInputHandler.h
    src/MapViewZoomHandler.h
    src/MenuBarActionIDs.h
    src/OptionalBorderBrush.h
    src/OtbmTypes.h
    src/Outfit.h
    src/PasteCommand.h
    src/PixelBrush.h
    src/PlaceCreatureCommand.h
    src/PlaceDecorationCommand.h
    src/PlaceDoorCommand.h
    src/PlaceItemCommand.h
    src/PlaceWallCommand.h
    src/PodiumItem.h
    src/Position.h
    src/RAWBrush.h
    src/RemoveCreatureCommand.h
    src/ReplaceItemCommand.h
    src/ResourceManager.h
    src/ScreenshotManager.h
    src/SelectCommand.h
    src/Selection.h
    src/SelectionBrush.h
    src/SelectionTransformationCommand.h
    src/SetGroundItemCommand.h
    src/SetTileColorCommand.h
    src/SettingsIntegrationSystem.h
    src/SettingsManager.h
    src/Spawn.h
    src/SpawnBrush.h
    src/SpawnCommand.h
    src/Spawns.h
    src/Sprite.h
    src/SpriteFileParser.h
    src/SpriteLight.h
    src/SpriteManager.h
    src/TableBrush.h
    src/TeleportItem.h
    src/TerrainBrush.h
    src/Tile.h
    src/TileRenderer.h
    src/TilesetManager.h
    src/Town.h
    src/TransparencyManager.h
    src/WallBrush.h
    src/WallDecorationBrush.h
    src/Waypoint.h
    src/WaypointBrush.h
    src/Waypoints.h

    src/commands/HouseTownCommands.h
    src/commands/ModifySpawnCommand.h
    src/commands/SelectionCommands.h
    src/commands/SetItemAttributeCommand.h
    src/commands/TileLockCommands.h
    src/commands/TilePropertyCommand.h

    src/integration/HouseTownIntegrationManager.h
    src/integration/MinimapIntegration.h
    src/integration/ProgressIntegration.h
    src/integration/SpawnIntegrationManager.h
    src/integration/TilesetIntegrationManager.h

    src/io/MapFormatManager.h
    src/io/MapVersionConverter.h
    src/io/OTBMMapLoader.h
    src/io/OtbmReader.h
    src/io/OtbmVersionConverter.h
    src/io/OtbmWriter.h
    src/io/OTMMMapLoader.h
    src/io/QtBinaryFile.h

    src/managers/TilesetBehaviorManager.h

    src/network/LiveCursor.h
    src/network/LivePackets.h
    src/network/NetworkController.h
    src/network/NetworkMessage.h

    src/operations/ReplaceOperationManager.h

    src/persistence/HouseTownPersistenceManager.h

    src/rendering/MinimapRenderer.h
    src/rendering/SpawnVisualizationManager.h

    src/search/ItemPropertyMatcher.h

    src/selection/EnhancedSelectionBrush.h
    src/selection/SelectionTransformationEngine.h
    src/selection/SelectionVisualFeedback.h

    src/threading/ProgressWorker.h

    src/ui/properties/ItemPropertyEditor.h
    src/ui/properties/ItemPropertyEditorBase.h
    src/ui/properties/OldPropertiesWindow.h

    src/ui/AboutDialog.h
    src/ui/AutomagicSettingsDialog.h
    src/ui/BrushPalettePanel.h
    src/ui/BrushPanel.h
    src/ui/BrushSizePanel.h
    src/ui/CreaturePalette.h
    src/ui/DialogManager.h
    src/ui/EnhancedMapView.h
    src/ui/ExportMiniMapDialog.h
    src/ui/FindItemDialog.h
    src/ui/FindSimilarDialog.h
    src/ui/FocusManagementSystem.h
    src/ui/GotoPositionDialog.h
    src/ui/GroundValidationDialog.h
    src/ui/HelpSystem.h
    src/ui/HouseEditorDialog.h
    src/ui/HouseTownActions.h
    src/ui/ImportMapDialog.h
    src/ui/InputValidationSystem.h
    src/ui/KeyboardNavigationSystem.h
    src/ui/MainPalette.h
    src/ui/MainWindow.h
    src/ui/MapTileItem.h
    src/ui/MenuActionHandler.h
    src/ui/MenuBuilder.h
    src/ui/MinimapWindow.h
    src/ui/PerspectiveManager.h
    src/ui/PlaceholderMinimapWidget.h
    src/ui/PreferencesDialog.h
    src/ui/ProgressDialog.h
    src/ui/ProgressOperationManager.h
    src/ui/ReplaceItemsDialog.h
    src/ui/SelectionItem.h
    src/ui/SpawnDialog.h
    src/ui/SpawnEditorDialog.h
    src/ui/SpawnItem.h
    src/ui/SpriteButton.h
    src/ui/StatusBarManager.h
    src/ui/TileLockActions.h
    src/ui/TilePropertyDialog.h
    src/ui/TilePropertyEditor.h
    src/ui/TilesetEditorDialog.h
    src/ui/TilesetWindow.h
    src/ui/ToolBarManager.h
    src/ui/TownEditorDialog.h
    src/ui/UIInputManager.h
    src/ui/WaypointCreationDialog.h
    src/ui/WaypointEditorPanel.h
    src/ui/WaypointItem.h
    src/ui/WaypointPalettePanel.h
    src/ui/ZoneBrushPanel.h
)

# --- Source Files ---
set(PROJECT_SOURCES
    src/Animator.cpp
    src/AutoBorder.cpp
    src/AutoBorderData.cpp
    src/BorderSystem.cpp
    src/Brush.cpp
    src/BrushInteractionController.cpp
    src/BrushManager.cpp
    src/BrushManagerHelpers.cpp
    src/BrushPersistence.cpp
    src/CarpetBrush.cpp
    src/ClipboardData.cpp
    src/ClipboardManager.cpp
    src/ContainerItem.cpp
    src/Creature.cpp
    src/CreatureBrush.cpp
    src/CreatureManager.cpp
    src/CreatureSpriteManager.cpp
    src/DepotItem.cpp
    src/DeselectCommand.cpp  # FIXED: Now exists - created implementation
    src/DoodadBrush.cpp
    src/DoorBrush.cpp
    src/DoorItem.cpp
    src/DrawingOptions.cpp
    src/EditorSprite.cpp
    src/EditorSpriteIds.cpp
    src/EnhancedLightingSystem.cpp
    src/EnhancedMapRenderer.cpp
    src/EraseCommand.cpp
    src/EraserBrush.cpp
    src/FlagBrush.cpp
    src/GameSprite.cpp
    src/GraphicManager.cpp
    src/GroundBorderTool.cpp
    src/GroundBrush.cpp
    src/House.cpp
    src/HouseBrush.cpp
    src/HouseExitBrush.cpp
    src/ImageSpace.cpp
    src/Item.cpp
    src/ItemFactory.cpp
    src/ItemFinder.cpp
    src/ItemManager.cpp
    src/ItemRenderer.cpp
    src/LightDrawer.cpp
    src/LODManager.cpp
    src/main.cpp # Entry point
    src/Map.cpp
    src/MapDrawingPrimitives_Enhanced.cpp
    src/MapDrawingPrimitives.cpp
    src/MapEntityManager.cpp
    src/MapIO.cpp
    src/MapIterator.cpp
    src/MapSearcher.cpp
    src/MapView.cpp
    src/MapViewEnhancements.cpp
    src/MapViewInputHandler.cpp
    src/MapViewInputHandlerBrushInteraction.cpp
    src/MapViewInputHandlerTileLogic.cpp
    src/MapViewZoomHandler.cpp
    src/OptionalBorderBrush.cpp
    src/PasteCommand.cpp
    src/PixelBrush.cpp
    src/PlaceCreatureCommand.cpp
    src/PlaceDecorationCommand.cpp
    src/PlaceDoorCommand.cpp
    src/PlaceItemCommand.cpp
    src/PlaceWallCommand.cpp
    src/PodiumItem.cpp
    src/Position.cpp
    src/RAWBrush.cpp
    src/RemoveCreatureCommand.cpp
    src/ReplaceItemCommand.cpp  # FIXED: Now exists - created implementation
    src/ResourceManager.cpp
    src/ScreenshotManager.cpp
    src/SelectCommand.cpp  # FIXED: Now exists - created implementation
    src/Selection.cpp
    src/SelectionBrush.cpp
    src/SelectionCommands.cpp
    src/SelectionTransformationCommand.cpp
    src/SetGroundItemCommand.cpp
    src/SetTileColorCommand.cpp
    src/SettingsManager.cpp
    src/Spawn.cpp
    src/SpawnBrush.cpp
    src/SpawnCommand.cpp
    src/Spawns.cpp
    src/Sprite.cpp
    src/SpriteFileParser.cpp
    src/SpriteManager.cpp
    src/TableBrush.cpp
    src/TeleportItem.cpp
    src/TerrainBrush.cpp
    src/Tile.cpp
    src/TileLockManager.cpp  # FIXED: Now exists - was missing from CMakeLists.txt
    src/TileRenderer.cpp
    src/TilesetManager.cpp
    src/Town.cpp
    src/TransparencyManager.cpp
    src/WallBrush.cpp
    src/WallDecorationBrush.cpp
    src/Waypoint.cpp
    src/WaypointBrush.cpp
    src/Waypoints.cpp

    src/commands/HouseTownCommands.cpp  # FIXED: Now exists - created implementation
    src/commands/ModifySpawnCommand.cpp
    src/commands/SetItemAttributeCommand.cpp
    src/commands/TileLockCommands.cpp
    src/commands/TilePropertyCommand.cpp  # FIXED: Now exists - created implementation
    # src/commands/SelectionCommands.cpp is already listed under src/

    src/integration/ProgressIntegration.cpp
    src/integration/TilesetIntegrationManager.cpp
    # src/integration/HouseTownIntegrationManager.h (no .cpp in XML)
    # src/integration/MinimapIntegration.h (no .cpp in XML)
    # src/integration/SpawnIntegrationManager.h (no .cpp in XML)

    src/io/MapFormatManager.cpp
    src/io/MapVersionConverter.cpp
    src/io/OTBMMapLoader.cpp
    src/io/OtbmReader.cpp
    src/io/OtbmVersionConverter.cpp
    src/io/OtbmWriter.cpp
    src/io/OTMMMapLoader.cpp
    src/io/QtBinaryFile.cpp

    # src/managers/ (TilesetBehaviorManager.h has no .cpp in XML)

    src/network/LiveCursor.cpp
    src/network/NetworkController.cpp
    src/network/NetworkMessage.cpp

    # src/operations/ (ReplaceOperationManager.h has no .cpp in XML)
    # src/persistence/ (HouseTownPersistenceManager.h has no .cpp in XML)
    # src/rendering/ (MinimapRenderer.h, SpawnVisualizationManager.h have no .cpp in XML)
    # src/search/ (ItemPropertyMatcher.h has no .cpp in XML)
    # src/selection/ (EnhancedSelectionBrush.h, SelectionTransformationEngine.h, SelectionVisualFeedback.h have no .cpp in XML)
    # src/threading/ (ProgressWorker.h has no .cpp in XML)

    src/ui/properties/ItemPropertyEditor.cpp
    src/ui/properties/ItemPropertyEditorBase.cpp
    src/ui/properties/OldPropertiesWindow.cpp

    src/ui/AboutDialog.cpp
    src/ui/AutomagicSettingsDialog.cpp
    src/ui/BrushPalettePanel.cpp
    src/ui/BrushPanel.cpp
    src/ui/BrushSizePanel.cpp
    src/ui/DialogManager.cpp
    src/ui/ExportMiniMapDialog.cpp
    src/ui/FindItemDialog.cpp
    src/ui/GotoPositionDialog.cpp
    src/ui/GroundValidationDialog.cpp
    src/ui/ImportMapDialog.cpp
    src/ui/MainPalette.cpp
    src/ui/MainWindow.cpp
    src/ui/MapTileItem.cpp
    src/ui/MenuActionHandler.cpp
    src/ui/MenuBuilder.cpp
    src/ui/PerspectiveManager.cpp
    src/ui/PlaceholderMinimapWidget.cpp
    src/ui/PreferencesDialog.cpp
    src/ui/ReplaceItemsDialog.cpp
    src/ui/SelectionItem.cpp
    src/ui/SpawnDialog.cpp
   # src/ui/SpawnEditorDialog.cpp
    src/ui/SpriteButton.cpp
    src/ui/StatusBarManager.cpp
    src/ui/TileLockActions.cpp
    src/ui/TilePropertyEditor.cpp
    src/ui/ToolBarManager.cpp
    src/ui/WaypointCreationDialog.cpp  # FIXED: Now exists - was missing from CMakeLists.txt
    src/ui/WaypointEditorPanel_Events.cpp
    src/ui/WaypointEditorPanel.cpp
    src/ui/WaypointItem.cpp
    src/ui/WaypointPalettePanel.cpp
    #src/ui/WaypointSystemTest.cpp # Test file
    src/ui/ZoneBrushPanel.cpp
)

# Add the executable target
add_executable(MapEditorQt6 MACOSX_BUNDLE ${PROJECT_SOURCES}) # <-- REMOVED ${PROJECT_HEADERS}

# Link Qt6 libraries
target_link_libraries(MapEditorQt6 PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Xml
    Qt6::Network
)

# If you have .qrc files for resources (e.g., icons):
# qt_add_resources(MapEditorQt6 "resources.qrc")
# Make sure resources.qrc exists and lists your resource files.

# Include directories
target_include_directories(MapEditorQt6 PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/commands
    ${CMAKE_CURRENT_SOURCE_DIR}/src/integration
    ${CMAKE_CURRENT_SOURCE_DIR}/src/io
    ${CMAKE_CURRENT_SOURCE_DIR}/src/managers
    ${CMAKE_CURRENT_SOURCE_DIR}/src/network
    ${CMAKE_CURRENT_SOURCE_DIR}/src/operations
    ${CMAKE_CURRENT_SOURCE_DIR}/src/persistence
    ${CMAKE_CURRENT_SOURCE_DIR}/src/rendering
    ${CMAKE_CURRENT_SOURCE_DIR}/src/search
    ${CMAKE_CURRENT_SOURCE_DIR}/src/selection
    ${CMAKE_CURRENT_SOURCE_DIR}/src/threading
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ui
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ui/properties
)