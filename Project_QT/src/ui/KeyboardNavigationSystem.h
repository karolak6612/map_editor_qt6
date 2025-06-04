#ifndef KEYBOARDNAVIGATIONSYSTEM_H
#define KEYBOARDNAVIGATIONSYSTEM_H

#include <QObject>
#include <QWidget>
#include <QKeyEvent>
#include <QKeySequence>
#include <QShortcut>
#include <QAction>
#include <QMap>
#include <QSet>
#include <QList>
#include <QTimer>
#include <QVariantMap>

// Forward declarations
class MainWindow;
class MapView;
class UIInputManager;
class FocusManagementSystem;

/**
 * @brief Task 96: Keyboard Navigation System for comprehensive keyboard control
 * 
 * Advanced keyboard navigation with wxwidgets compatibility:
 * - Tab order management and customization
 * - Arrow key navigation for complex layouts
 * - Shortcut key conflict resolution
 * - Context-sensitive navigation
 * - Accelerator key support
 * - Mnemonic handling
 * - Navigation between different UI areas
 * - Complete wxwidgets navigation behavior replication
 */

/**
 * @brief Navigation direction for directional navigation
 */
enum class NavigationDirection {
    NEXT,               // Tab or right/down
    PREVIOUS,           // Shift+Tab or left/up
    UP,                 // Up arrow
    DOWN,               // Down arrow
    LEFT,               // Left arrow
    RIGHT,              // Right arrow
    FIRST,              // Home or Ctrl+Home
    LAST,               // End or Ctrl+End
    PAGE_UP,            // Page Up
    PAGE_DOWN,          // Page Down
    PARENT,             // Escape to parent
    CHILD               // Enter to child
};

/**
 * @brief Navigation context for different UI areas
 */
enum class NavigationContext {
    GLOBAL,             // Global application navigation
    DIALOG,             // Dialog navigation
    DOCK_WIDGET,        // Dock widget navigation
    PALETTE,            // Palette navigation
    TREE_VIEW,          // Tree view navigation
    TABLE_VIEW,         // Table view navigation
    LIST_VIEW,          // List view navigation
    TAB_WIDGET,         // Tab widget navigation
    MENU,               // Menu navigation
    TOOLBAR,            // Toolbar navigation
    PROPERTY_EDITOR,    // Property editor navigation
    TEXT_EDITOR,        // Text editor navigation
    CUSTOM              // Custom navigation
};

/**
 * @brief Navigation behavior configuration
 */
enum class NavigationBehavior {
    STANDARD,           // Standard Qt navigation
    WRAP_AROUND,        // Wrap to beginning/end
    STOP_AT_EDGES,      // Stop at first/last widget
    SKIP_DISABLED,      // Skip disabled widgets
    SKIP_INVISIBLE,     // Skip invisible widgets
    ACCELERATOR_KEYS,   // Support accelerator keys
    MNEMONIC_KEYS,      // Support mnemonic keys
    ARROW_KEYS,         // Support arrow key navigation
    CUSTOM              // Custom navigation behavior
};

/**
 * @brief Navigation shortcut configuration
 */
struct NavigationShortcut {
    QKeySequence keySequence;
    NavigationDirection direction;
    NavigationContext context;
    QString description;
    bool enabled;
    bool overrideDefault;
    
    NavigationShortcut(const QKeySequence& seq = QKeySequence(),
                      NavigationDirection dir = NavigationDirection::NEXT,
                      NavigationContext ctx = NavigationContext::GLOBAL,
                      const QString& desc = "",
                      bool en = true,
                      bool override = false)
        : keySequence(seq), direction(dir), context(ctx), description(desc), 
          enabled(en), overrideDefault(override) {}
};

/**
 * @brief Tab order configuration for widgets
 */
struct TabOrderItem {
    QWidget* widget;
    int order;
    bool enabled;
    NavigationBehavior behavior;
    QString description;
    QVariantMap metadata;
    
    TabOrderItem(QWidget* w = nullptr, int o = 0, bool en = true,
                NavigationBehavior b = NavigationBehavior::STANDARD,
                const QString& desc = "")
        : widget(w), order(o), enabled(en), behavior(b), description(desc) {}
};

/**
 * @brief Accelerator key configuration
 */
struct AcceleratorKey {
    QChar key;
    QWidget* targetWidget;
    QString description;
    bool enabled;
    bool caseSensitive;
    
    AcceleratorKey(QChar k = QChar(), QWidget* w = nullptr, 
                  const QString& desc = "", bool en = true, bool cs = false)
        : key(k), targetWidget(w), description(desc), enabled(en), caseSensitive(cs) {}
};

/**
 * @brief Navigation area for grouping related widgets
 */
class NavigationArea : public QObject
{
    Q_OBJECT

public:
    explicit NavigationArea(const QString& name, QObject* parent = nullptr);
    ~NavigationArea() override = default;

    // Area management
    QString getName() const { return name_; }
    void setDescription(const QString& description) { description_ = description; }
    QString getDescription() const { return description_; }

    // Widget management
    void addWidget(QWidget* widget, int order = -1);
    void removeWidget(QWidget* widget);
    void setWidgetOrder(QWidget* widget, int order);
    QList<QWidget*> getWidgets() const;
    QList<QWidget*> getOrderedWidgets() const;

    // Navigation behavior
    void setNavigationBehavior(NavigationBehavior behavior);
    NavigationBehavior getNavigationBehavior() const { return behavior_; }
    void setWrapAround(bool wrap);
    bool isWrapAround() const { return wrapAround_; }

    // Navigation operations
    QWidget* getFirstWidget() const;
    QWidget* getLastWidget() const;
    QWidget* getNextWidget(QWidget* current) const;
    QWidget* getPreviousWidget(QWidget* current) const;
    QWidget* getWidgetInDirection(QWidget* current, NavigationDirection direction) const;

    // State management
    void setEnabled(bool enabled);
    bool isEnabled() const { return enabled_; }

public slots:
    void onWidgetDestroyed(QObject* widget);

signals:
    void widgetNavigated(QWidget* from, QWidget* to, NavigationDirection direction);
    void areaEntered(QWidget* widget);
    void areaLeft(QWidget* widget);

private:
    void sortWidgetsByOrder();
    bool isWidgetNavigable(QWidget* widget) const;

private:
    QString name_;
    QString description_;
    QMap<QWidget*, int> widgetOrder_;
    QList<QWidget*> orderedWidgets_;
    NavigationBehavior behavior_;
    bool wrapAround_;
    bool enabled_;
    bool orderDirty_;
};

/**
 * @brief Main Keyboard Navigation System
 */
class KeyboardNavigationSystem : public QObject
{
    Q_OBJECT

public:
    explicit KeyboardNavigationSystem(QObject* parent = nullptr);
    ~KeyboardNavigationSystem() override;

    // Component setup
    void setMainWindow(MainWindow* mainWindow);
    void setMapView(MapView* mapView);
    void setUIInputManager(UIInputManager* inputManager);
    void setFocusManagementSystem(FocusManagementSystem* focusSystem);

    // System control
    void enableNavigation(bool enabled);
    bool isNavigationEnabled() const { return navigationEnabled_; }
    void setDebugMode(bool enabled);
    bool isDebugMode() const { return debugMode_; }

    // Navigation area management
    NavigationArea* createNavigationArea(const QString& name, const QString& description = "");
    void removeNavigationArea(const QString& name);
    NavigationArea* getNavigationArea(const QString& name) const;
    QStringList getNavigationAreaNames() const;

    // Tab order management
    void setTabOrder(QWidget* parent, const QList<TabOrderItem>& items);
    void addTabOrderItem(QWidget* widget, int order, NavigationBehavior behavior = NavigationBehavior::STANDARD);
    void removeTabOrderItem(QWidget* widget);
    void updateTabOrder(QWidget* parent);
    QList<TabOrderItem> getTabOrder(QWidget* parent) const;

    // Navigation shortcuts
    void registerNavigationShortcut(const NavigationShortcut& shortcut);
    void unregisterNavigationShortcut(const QKeySequence& keySequence);
    void enableNavigationShortcuts(bool enabled);
    bool areNavigationShortcutsEnabled() const { return navigationShortcutsEnabled_; }

    // Accelerator keys
    void registerAcceleratorKey(const AcceleratorKey& accelerator);
    void unregisterAcceleratorKey(QChar key);
    void enableAcceleratorKeys(bool enabled);
    bool areAcceleratorKeysEnabled() const { return acceleratorKeysEnabled_; }

    // Navigation operations
    bool navigateInDirection(NavigationDirection direction);
    bool navigateToWidget(QWidget* widget);
    bool navigateToNextWidget(QWidget* current = nullptr);
    bool navigateToPreviousWidget(QWidget* current = nullptr);
    bool navigateToFirstWidget(QWidget* parent = nullptr);
    bool navigateToLastWidget(QWidget* parent = nullptr);

    // Context-sensitive navigation
    void setNavigationContext(NavigationContext context);
    NavigationContext getNavigationContext() const { return currentContext_; }
    void pushNavigationContext(NavigationContext context);
    void popNavigationContext();

    // Arrow key navigation
    void enableArrowKeyNavigation(bool enabled);
    bool isArrowKeyNavigationEnabled() const { return arrowKeyNavigation_; }
    void setArrowKeyBehavior(NavigationBehavior behavior);
    NavigationBehavior getArrowKeyBehavior() const { return arrowKeyBehavior_; }

    // Mnemonic support
    void enableMnemonicSupport(bool enabled);
    bool isMnemonicSupportEnabled() const { return mnemonicSupport_; }
    void registerMnemonic(QChar key, QWidget* widget);
    void unregisterMnemonic(QChar key);
    bool activateMnemonic(QChar key);

    // Event handling
    bool handleKeyPressEvent(QKeyEvent* event, QWidget* widget);
    bool handleNavigationKey(QKeyEvent* event, QWidget* widget);
    bool handleAcceleratorKey(QKeyEvent* event);
    bool handleMnemonicKey(QKeyEvent* event);

    // Statistics and monitoring
    QVariantMap getStatistics() const;
    void resetStatistics();
    int getNavigationCount() const { return navigationCount_; }

public slots:
    void onWidgetFocusChanged(QWidget* old, QWidget* now);
    void onNavigationShortcutActivated();
    void onAcceleratorKeyActivated();
    void updateNavigationState();

signals:
    void navigationPerformed(QWidget* from, QWidget* to, NavigationDirection direction);
    void navigationFailed(QWidget* widget, NavigationDirection direction, const QString& reason);
    void acceleratorKeyActivated(QChar key, QWidget* widget);
    void mnemonicActivated(QChar key, QWidget* widget);
    void navigationContextChanged(NavigationContext oldContext, NavigationContext newContext);

private:
    // Navigation implementation
    QWidget* findNavigationTarget(QWidget* current, NavigationDirection direction) const;
    QWidget* findNextTabWidget(QWidget* current) const;
    QWidget* findPreviousTabWidget(QWidget* current) const;
    QWidget* findArrowNavigationTarget(QWidget* current, NavigationDirection direction) const;

    // Widget analysis
    bool isWidgetNavigable(QWidget* widget) const;
    bool canWidgetReceiveFocus(QWidget* widget) const;
    NavigationContext determineWidgetContext(QWidget* widget) const;
    NavigationArea* findWidgetNavigationArea(QWidget* widget) const;

    // Tab order implementation
    void buildTabOrderChain(QWidget* parent);
    void sortTabOrderItems(QList<TabOrderItem>& items);
    QList<QWidget*> getTabOrderChain(QWidget* parent) const;

    // Shortcut management
    void setupNavigationShortcuts();
    void cleanupNavigationShortcuts();
    bool isNavigationShortcut(QKeyEvent* event) const;

    // Context management
    NavigationBehavior getContextBehavior(NavigationContext context) const;
    bool isContextActive(NavigationContext context) const;

    // Debug and monitoring
    void logNavigation(QWidget* from, QWidget* to, NavigationDirection direction);
    void logNavigationFailure(QWidget* widget, NavigationDirection direction, const QString& reason);
    void updateStatistics();

private:
    // Core components
    MainWindow* mainWindow_;
    MapView* mapView_;
    UIInputManager* inputManager_;
    FocusManagementSystem* focusSystem_;

    // System state
    bool navigationEnabled_;
    bool debugMode_;
    NavigationContext currentContext_;
    QList<NavigationContext> contextStack_;

    // Navigation areas
    QMap<QString, NavigationArea*> navigationAreas_;

    // Tab order management
    QMap<QWidget*, QList<TabOrderItem>> tabOrderItems_;
    QMap<QWidget*, QList<QWidget*>> tabOrderChains_;

    // Navigation shortcuts
    QList<NavigationShortcut> navigationShortcuts_;
    QMap<QKeySequence, QShortcut*> shortcutObjects_;
    bool navigationShortcutsEnabled_;

    // Accelerator keys
    QMap<QChar, AcceleratorKey> acceleratorKeys_;
    bool acceleratorKeysEnabled_;

    // Arrow key navigation
    bool arrowKeyNavigation_;
    NavigationBehavior arrowKeyBehavior_;

    // Mnemonic support
    bool mnemonicSupport_;
    QMap<QChar, QWidget*> mnemonicMap_;

    // Statistics
    int navigationCount_;
    QMap<NavigationDirection, int> directionCounts_;
    QMap<QWidget*, int> widgetNavigationCounts_;

    // Update timer
    QTimer* updateTimer_;
};

#endif // KEYBOARDNAVIGATIONSYSTEM_H
