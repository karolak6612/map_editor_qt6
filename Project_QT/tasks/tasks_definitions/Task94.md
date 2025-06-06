**Task94: Implement and Integrate the `Houses` and `Towns` System (Full Life-Cycle, UI, Map Sync, OTBM/XML)**
- Task: **Fully integrate the Houses and Towns system into the `project_qt` application. This includes ensuring proper data management (`House`/`Town` objects within `Map`), complete UI interaction (dialogs/panels for editing all aspects), robust synchronization with the `Map` data model and `MapView`/`Minimap` visuals, and correct persistence (OTBM attributes or dedicated XML files). This task culminates all previous house/town work (Task 66, 73, 86).**
    - **Analyze Existing Systems:** Consolidate all `House`/`Town` related code from previous tasks in `Project_QT/src`.
    - **Data Management (`Map` Level):**
        -   `Map` class is the owner of `QList<House*>` and `QList<Town*>`.
        -   All operations (create, delete, modify house/town) go through `Map` methods, which then update these lists and associated `Tile` properties (`HouseID`, `TownID`, exit flags).
    - **UI Interaction (Full Editors):**
        -   Finalize `HouseEditorDialog/Panel` and `TownEditorDialog/Panel`. They must allow:
            -   Creating new houses/towns.
            -   Editing all properties (name, owner, rent, size, temple position, tile lists for houses, entry points).
            -   Deleting houses/towns.
        -   Connect these UIs to `MainWindow` (e.g., via menu actions from Task 86).
    - **Synchronization with `MapView`/`Minimap`:**
        -   Changes made in editors must reflect immediately on `MapView` (e.g., visual house borders, exit markers, highlighted town areas) and `Minimap`. This requires `Map` to emit signals when house/town data or linked `Tile` properties change.
    - **Persistence (OTBM and/or XML):**
        -   **OTBM:** If house/town data is stored as attributes on `Tile`s or within specific OTBM map nodes (e.g., `TAG_HOUSES`), ensure `Map::saveToOTBM` (Task 60) correctly serializes all house/town data derived from `Map`'s `House`/`Town` objects and associated `Tile`s. `Map::loadFromOTBM` must correctly reconstruct `House`/`Town` objects and link them to tiles.
        -   **Separate XML:** If `wxwidgets` used separate `houses.xml` / `towns.xml`, ensure full load/save functionality for these using `QXmlStreamReader/Writer`, and link this data to the `Map` on load.
    - **`Tile` Property Interaction (`HouseExits`, `hasStairs`):**
        -   Ensure `Tile` properties related to houses (e.g., `isHouseExit`, `hasStairsInsideHouse`) are correctly set/read and saved as part of OTBM tile attributes if that was the mechanism, or are derived from `ItemProperties` on items placed by `HouseBrush`. The new `QTile` must have helpers if needed.
        -   Original `g_materials::Houses/HouseTileset` logic (if it was about loading house data or applying "HouseTile" type to tiles) should be replaced by this integrated system where `Map` load processes create/link `House` objects.
    - **`HouseDoorID`:** Ensure logic for linking doors within a house (if `HouseDoorID` was moved as part_of items attributes) is fully handled by editing door items within the house context.
    - **Undo/Redo:** All house/town creation, deletion, and modification operations (including changes to tile assignments or entry points) must be encapsulated in `QUndoCommand`s.
    - **Minimap Layers:** Confirm if `Minimap` layers also contain tiles matching certain `House`/`Town` properties, and if these are now direct checks on `Tile` data.
    - **`Task94.md` must provide the definitive source for house/town data structures, persistence formats (OTBM nodes vs. XML schemas), UI for all editing operations, and all interactions with `Map` and `Tile` data, including how `wxwidgets`' `HouseTileset` or similar mechanisms worked.**
