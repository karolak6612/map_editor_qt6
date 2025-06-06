**Task57: Migrate `FlagBrush` (Zones - Tile Attribute Setting & Menu/Tool Integration)**
- Task: **Migrate `FlagBrush` functionality to `project_qt`. This brush is used to apply zone settings and attributes (e.g., `ZoneID`, Protection Zone (PZ), No-Logout Zone, other `mapFlags`) to tiles, by setting attributes on the `Tile` itself or by placing special "zone items" that confer these properties.**
    - **Analyze Existing Zone/Flag Logic:** Review `Project_QT/src` for any existing zone or tile flag management.
    - **Mechanism for Setting Zone Attributes:**
        -   Determine if zone flags (like PZ, No-Logout) will be direct properties/flags on the `Tile` object or represented by special invisible `Item`s placed by the `FlagBrush` (with specific `ItemProperties` or attributes via the `Item` attribute map). `Task57.md` should clarify this based on original `wxwidgets` design or preferred Qt approach.
        -   `FlagBrush::draw()` should implement the chosen mechanism to apply the selected zone flag/`ZoneID` to the target `Tile`(s).
    - **`mapFlags` vs. Tile Properties:** Differentiate between map-wide flags (`BaseMap::mapFlags` if these were static members like `BORDERTYPE`) and flags applied to individual `TileLocation`s. This brush primarily deals with tile-specific flags.
    - **Menu/Tool Integration:**
        -   Create and load any related menu entries (e.g., under "Tools" or "Map Properties") or toolbar buttons for activating different `FlagBrush` modes (e.g., a "Set PZ Brush," "Set No-Logout Brush," "Set Zone ID Brush").
        -   These actions should activate the `FlagBrush` in `BrushManager` and configure it with the specific flag/zone type to apply.
    - **Visual Indicators (Conceptual):** While full visual rendering of zones is later, `FlagBrush` interactions should conceptually update the state that rendering will use (e.g., setting `Tile::isPZ(true)`). The original drawing logic might have used special states in `MapView` or `Item->flags` that triggered visual indicators for zones (e.g., colored borders); this brush now sets the underlying data for that.
    - **`Task57.md` must define all zone types/map flags handled by `FlagBrush`, how they were stored (on tile, as special items), how the brush was configured for different flags (e.g., if it used sub-types or parameters), and any UI elements used to activate/configure it.**
