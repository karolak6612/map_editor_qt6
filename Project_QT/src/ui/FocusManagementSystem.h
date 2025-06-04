#ifndef FOCUSMANAGEMENTSYSTEM_H
#define FOCUSMANAGEMENTSYSTEM_H

#include <QObject>
#include <QWidget>
#include <QApplication>
#include <QFocusEvent>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QEvent>
#include <QEventFilter>
#include <QTimer>
#include <QMap>
#include <QSet>
#include <QList>
#include <QStack>
#include <QVariantMap>

// Forward declarations
class MainWindow;
class MapView;
class QDialog;
class QDockWidget;

/**
 * @brief Task 96: Focus Management System for comprehensive focus control
 * 
 * Advanced focus management with wxwidgets compatibility:
 * - Hierarchical focus management for complex UI layouts
 * - Focus history and restoration
 * - Modal dialog focus handling
 * - Dock widget focus isolation
 * - Palette focus management
 * - MapView focus prevention when other widgets need input
 * - Focus debugging and monitoring
 * - Complete wxwidgets focus behavior replication
 */

/**
 * @brief Focus priority levels for different UI components
 */
enum class FocusPriority {
    LOWEST = 0,         // Background widgets
    LOW = 1,            // Secondary controls
    NORMAL = 2,         // Standard controls
    HIGH = 3,           // Important input controls
    HIGHEST = 4,        // Modal dialogs and critical input
    SYSTEM = 5          // System-level focus (menus, etc.)
};

/**
 * @brief Focus behavior configuration
 */
enum class FocusBehavior {
    STANDARD,           // Standard Qt focus behavior
    MODAL_EXCLUSIVE,    // Modal dialog exclusive focus
    DOCK_ISOLATED,      // Dock widget isolated focus
    PALETTE_CONTAINED,  // Palette contained focus
    MAP_EXCLUDED,       // Exclude MapView from focus
    TEXT_INPUT_PRIORITY,// Text input gets priority
    NUMERIC_INPUT_PRIORITY, // Numeric input gets priority
    CUSTOM              // Custom focus behavior
};

/**
 * @brief Focus state information
 */
struct FocusState {
    QWidget* widget;
    FocusPriority priority;
    FocusBehavior behavior;
    QDateTime timestamp;
    QString description;
    QVariantMap metadata;
    
    FocusState(QWidget* w = nullptr, FocusPriority p = FocusPriority::NORMAL, 
               FocusBehavior b = FocusBehavior::STANDARD, const QString& d = "")
        : widget(w), priority(p), behavior(b), timestamp(QDateTime::currentDateTime()), description(d) {}
};

/**
 * @brief Focus group for managing related widgets
 */
class FocusGroup : public QObject
{
    Q_OBJECT

public:
    explicit FocusGroup(const QString& name, QObject* parent = nullptr);
    ~FocusGroup() override = default;

    // Group management
    QString getName() const { return name_; }
    void setDescription(const QString& description) { description_ = description; }
    QString getDescription() const { return description_; }

    // Widget management
    void addWidget(QWidget* widget, FocusPriority priority = FocusPriority::NORMAL);
    void removeWidget(QWidget* widget);
    bool containsWidget(QWidget* widget) const;
    QList<QWidget*> getWidgets() const { return widgets_; }
    int getWidgetCount() const { return widgets_.size(); }

    // Focus behavior
    void setFocusBehavior(FocusBehavior behavior);
    FocusBehavior getFocusBehavior() const { return behavior_; }
    void setExclusive(bool exclusive);
    bool isExclusive() const { return exclusive_; }

    // Focus operations
    void focusFirst();
    void focusLast();
    void focusNext(QWidget* current);
    void focusPrevious(QWidget* current);
    QWidget* getFirstFocusableWidget() const;
    QWidget* getLastFocusableWidget() const;

    // State management
    void setEnabled(bool enabled);
    bool isEnabled() const { return enabled_; }
    void setVisible(bool visible);
    bool isVisible() const { return visible_; }

public slots:
    void onWidgetDestroyed(QObject* widget);

signals:
    void widgetFocused(QWidget* widget);
    void groupFocusEntered();
    void groupFocusLeft();

private:
    QString name_;
    QString description_;
    QList<QWidget*> widgets_;
    QMap<QWidget*, FocusPriority> widgetPriorities_;
    FocusBehavior behavior_;
    bool exclusive_;
    bool enabled_;
    bool visible_;
};

/**
 * @brief Main Focus Management System
 */
class FocusManagementSystem : public QObject
{
    Q_OBJECT

public:
    explicit FocusManagementSystem(QObject* parent = nullptr);
    ~FocusManagementSystem() override;

    // Component setup
    void setMainWindow(MainWindow* mainWindow);
    void setMapView(MapView* mapView);
    MainWindow* getMainWindow() const { return mainWindow_; }
    MapView* getMapView() const { return mapView_; }

    // System control
    void enableFocusManagement(bool enabled);
    bool isFocusManagementEnabled() const { return focusManagementEnabled_; }
    void setDebugMode(bool enabled);
    bool isDebugMode() const { return debugMode_; }

    // Widget registration
    void registerWidget(QWidget* widget, FocusPriority priority = FocusPriority::NORMAL, 
                       FocusBehavior behavior = FocusBehavior::STANDARD);
    void unregisterWidget(QWidget* widget);
    void updateWidgetPriority(QWidget* widget, FocusPriority priority);
    void updateWidgetBehavior(QWidget* widget, FocusBehavior behavior);

    // Focus group management
    FocusGroup* createFocusGroup(const QString& name, const QString& description = "");
    void removeFocusGroup(const QString& name);
    FocusGroup* getFocusGroup(const QString& name) const;
    QStringList getFocusGroupNames() const;

    // Modal dialog management
    void registerModalDialog(QDialog* dialog);
    void unregisterModalDialog(QDialog* dialog);
    bool hasActiveModalDialog() const;
    QDialog* getActiveModalDialog() const;

    // Dock widget management
    void registerDockWidget(QDockWidget* dockWidget);
    void unregisterDockWidget(QDockWidget* dockWidget);
    void setDockWidgetFocusIsolation(QDockWidget* dockWidget, bool isolated);

    // Focus state management
    QWidget* getCurrentFocusWidget() const;
    FocusState getCurrentFocusState() const;
    QList<FocusState> getFocusHistory() const;
    void clearFocusHistory();
    void saveFocusState(const QString& name);
    void restoreFocusState(const QString& name);

    // Focus operations
    void setFocus(QWidget* widget, Qt::FocusReason reason = Qt::OtherFocusReason);
    void clearFocus();
    void focusNext();
    void focusPrevious();
    void focusFirstWidget();
    void focusLastWidget();

    // MapView focus isolation
    void enableMapViewFocusIsolation(bool enabled);
    bool isMapViewFocusIsolationEnabled() const { return mapViewFocusIsolation_; }
    void setMapViewFocusPolicy(Qt::FocusPolicy policy);
    bool shouldMapViewReceiveFocus() const;

    // Focus validation
    bool canWidgetReceiveFocus(QWidget* widget) const;
    bool isWidgetFocusable(QWidget* widget) const;
    QWidget* findNextFocusableWidget(QWidget* current) const;
    QWidget* findPreviousFocusableWidget(QWidget* current) const;

    // Event handling
    void installEventFilters();
    void removeEventFilters();

    // Statistics and monitoring
    QVariantMap getStatistics() const;
    void resetStatistics();
    int getFocusChangeCount() const { return focusChangeCount_; }
    qint64 getAverageFocusTime() const;

public slots:
    void onApplicationFocusChanged(QWidget* old, QWidget* now);
    void onWidgetDestroyed(QObject* widget);
    void onModalDialogFinished(int result);
    void onDockWidgetVisibilityChanged(bool visible);
    void updateFocusState();

signals:
    void focusChanged(QWidget* oldWidget, QWidget* newWidget);
    void focusStateChanged(const FocusState& state);
    void modalDialogActivated(QDialog* dialog);
    void modalDialogDeactivated(QDialog* dialog);
    void dockWidgetFocusChanged(QDockWidget* dockWidget, bool hasFocus);
    void mapViewFocusBlocked(QWidget* requestingWidget);
    void focusValidationFailed(QWidget* widget, const QString& reason);

protected:
    bool eventFilter(QObject* watched, QEvent* event) override;

private:
    // Focus state management
    void updateCurrentFocusState(QWidget* widget);
    void addToFocusHistory(const FocusState& state);
    void cleanupFocusHistory();
    FocusPriority getWidgetPriority(QWidget* widget) const;
    FocusBehavior getWidgetBehavior(QWidget* widget) const;

    // Focus validation implementation
    bool validateFocusChange(QWidget* from, QWidget* to) const;
    bool isModalDialogActive() const;
    bool isDockWidgetIsolated(QWidget* widget) const;
    bool isMapViewFocusBlocked() const;

    // Event handling implementation
    bool handleFocusInEvent(QFocusEvent* event, QWidget* widget);
    bool handleFocusOutEvent(QFocusEvent* event, QWidget* widget);
    bool handleKeyPressEvent(QKeyEvent* event, QWidget* widget);
    bool handleMousePressEvent(QMouseEvent* event, QWidget* widget);

    // Widget analysis
    bool isTextInputWidget(QWidget* widget) const;
    bool isNumericInputWidget(QWidget* widget) const;
    bool isListWidget(QWidget* widget) const;
    bool isDialogWidget(QWidget* widget) const;
    bool isDockWidget(QWidget* widget) const;
    bool isPaletteWidget(QWidget* widget) const;

    // Focus chain management
    void buildFocusChain();
    void updateFocusChain();
    QList<QWidget*> getFocusChain() const;
    int getWidgetIndexInChain(QWidget* widget) const;

    // Debug and monitoring
    void logFocusChange(QWidget* from, QWidget* to, const QString& reason);
    void logFocusEvent(const QString& event, QWidget* widget);
    void updateStatistics();

private:
    // Core components
    MainWindow* mainWindow_;
    MapView* mapView_;

    // System state
    bool focusManagementEnabled_;
    bool debugMode_;
    bool mapViewFocusIsolation_;
    Qt::FocusPolicy originalMapViewFocusPolicy_;

    // Widget management
    QMap<QWidget*, FocusPriority> widgetPriorities_;
    QMap<QWidget*, FocusBehavior> widgetBehaviors_;
    QSet<QWidget*> registeredWidgets_;

    // Focus groups
    QMap<QString, FocusGroup*> focusGroups_;

    // Modal dialogs
    QStack<QDialog*> modalDialogStack_;
    QSet<QDialog*> registeredDialogs_;

    // Dock widgets
    QSet<QDockWidget*> registeredDockWidgets_;
    QMap<QDockWidget*, bool> dockWidgetIsolation_;

    // Focus state
    FocusState currentFocusState_;
    QList<FocusState> focusHistory_;
    QMap<QString, FocusState> savedFocusStates_;
    int maxHistorySize_;

    // Focus chain
    QList<QWidget*> focusChain_;
    bool focusChainDirty_;

    // Event filtering
    QSet<QObject*> filteredObjects_;

    // Statistics
    int focusChangeCount_;
    qint64 totalFocusTime_;
    QMap<QWidget*, int> widgetFocusCount_;
    QMap<QWidget*, qint64> widgetFocusTime_;

    // Update timer
    QTimer* updateTimer_;

    // Constants
    static const int DEFAULT_MAX_HISTORY_SIZE = 100;
    static const int UPDATE_INTERVAL = 100; // milliseconds
};

#endif // FOCUSMANAGEMENTSYSTEM_H
