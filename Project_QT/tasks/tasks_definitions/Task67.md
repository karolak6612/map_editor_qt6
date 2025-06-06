**Task67: Add `Tileset` Management to `ResourceManager` or create `TilesetManager` (Linking Items/Brushes by Category)**
- Task: **Implement `Tileset` management, mirroring how `g_materials` (from `wxwidgets`, potentially with extensions data for tile/doodad brush specific items or creatures) or `Brushes::TileSetCategory` organized items and brushes for editor palettes and tools. This involves creating a system (`TilesetManager` class, or extending `ResourceManager` from Task 32) that can group `ItemType`s and/or `Brush`es into logical categories (e.g., "Terrain," "Walls," "Doodads," "Creatures").**
    - **Analyze Existing Asset/Brush Management:** Review how `Project_QT/src` currently organizes items, brushes, and if any categorization ("tileset") concepts exist.
    - **`TilesetManager` Design (or `ResourceManager` Extension):**
        -   Define data structures to represent a "Tileset" or "Category." A category would typically hold a collection of `ItemType` IDs, actual `Item` template objects, or `Brush::Type` enums that belong to it.
        -   Implement methods to:
            -   `createTilesetCategory(const QString& categoryName)`
            -   `addItemToCategory(const QString& categoryName, ItemID itemID)` or `addBrushToCategory(const QString& categoryName, Brush::Type brushType)`
            -   `getItemsInCategory(const QString& categoryName) const`
            -   `getBrushTypesInCategory(const QString& categoryName) const`
    - **Data Source:** This manager might load its category definitions from a configuration file (XML/JSON) or be populated programmatically based on `ItemType` properties (e.g., all items with property `Category="Doodad"` go into the "Doodads" tileset). The original `updateBrushes` or `addToTileset` logic from `wxwidgets` (as referenced in `Task67.md`) should guide how items/brushes are assigned to categories.
    - **Integration with Item Loading:** When `ItemManager` loads an `Item`, or `SpriteManager` loads game info (Task 64), if an item's definition includes tileset/category information, it should be registered with this `TilesetManager`.
    - **Usage by UI Palettes:** UI palettes (e.g., `MainPalette`, developed in Task 70) will query this `TilesetManager` to populate their views with available items/brushes for each category tab.
    - **Preserving Lookup:** Preserve original category index-based lookups if they were essential for editor functionality, alongside type information by category.
    - **Decoupling:** `TilesetManager` should reduce complexity during lookup of `ItemProperties` on `Tiles` by providing a structured way to access items appropriate for a given context (e.g., what `wxwidgets` selection context and tile data did for `ItemType` for borders).
    - **Focus:** This task is primarily about the data handling and organization for `Item`/`Brush` categories. Actual loading of `ItemType` data is still `ItemManager`'s role. The `TilesetManager` provides the grouping. The actual GUI panels for these tilesets are Task 70.
    - **`Task67.md` needs to detail the structure of `g_materials` or `Brushes::TileSetCategory`, how items/brushes were categorized in `wxwidgets`, if category definitions were file-based, and the logic of `addToTileset` or `updateBrushes` which populated these categories.**
