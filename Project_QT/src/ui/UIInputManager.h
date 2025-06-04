#ifndef UIINPUTMANAGER_H
#define UIINPUTMANAGER_H

#include <QObject>
#include <QWidget>
#include <QDialog>
#include <QDockWidget>
#include <QApplication>
#include <QKeyEvent>
#include <QFocusEvent>
#include <QEvent>
#include <QEventFilter>
#include <QMap>
#include <QSet>
#include <QList>
#include <QTimer>
#include <QShortcut>
#include <QKeySequence>
#include <QValidator>

// Forward declarations
class MainWindow;
class MapView;
class QLineEdit;
class QTextEdit;
class QSpinBox;
class QDoubleSpinBox;
class QComboBox;
class QTabWidget;
class QTreeWidget;
class QTableWidget;
class QListWidget;

/**
 * @brief Task 96: UI Input Manager for comprehensive keyboard focus and input handling
 * 
 * Complete input management system with wxwidgets compatibility:
 * - Keyboard focus management for all UI components
 * - Tab order and navigation control
 * - Input validation and specialized controls
 * - Global shortcut conflict resolution
 * - MapView focus isolation to prevent key stealing
 * - Complete palette and dialog input handling
 * - wxNumberTextCtrl equivalent validation
 * - Perfect replication of wxwidgets input behavior
 */

/**
 * @brief Focus context for different UI areas
 */
enum class FocusContext {
    UNKNOWN,
    MAP_VIEW,           // MapView has focus - map navigation keys active
    PALETTE,            // Palette controls have focus - input goes to controls
    DIALOG,             // Dialog has focus - dialog input handling
    DOCK_WIDGET,        // Dock widget has focus - dock input handling
    MENU_BAR,           // Menu bar has focus - menu navigation
    TOOL_BAR,           // Tool bar has focus - tool shortcuts
    STATUS_BAR,         // Status bar has focus - status input
    PROPERTY_EDITOR,    // Property editor has focus - property input
    TEXT_INPUT,         // Text input control has focus - text editing
    NUMERIC_INPUT,      // Numeric input control has focus - number editing
    LIST_WIDGET,        // List widget has focus - list navigation
    TREE_WIDGET,        // Tree widget has focus - tree navigation
    TABLE_WIDGET        // Table widget has focus - table navigation
};

/**
 * @brief Input validation mode for specialized controls
 */
enum class InputValidationMode {
    NONE,               // No validation
    INTEGER,            // Integer validation (wxNumberTextCtrl equivalent)
    DOUBLE,             // Double validation
    RANGE_INTEGER,      // Integer with range validation
    RANGE_DOUBLE,       // Double with range validation
    CUSTOM,             // Custom validation function
    REGEX,              // Regular expression validation
    ALPHANUMERIC,       // Alphanumeric only
    FILENAME,           // Valid filename characters
    PATH,               // Valid path characters
    HEX_COLOR,          // Hexadecimal color validation
    ITEM_ID,            // Item ID validation
    POSITION            // Map position validation
};

/**
 * @brief Tab navigation order configuration
 */
struct TabOrderConfiguration {
    QWidget* widget;
    int order;
    bool enabled;
    QString description;
    
    TabOrderConfiguration(QWidget* w = nullptr, int o = 0, bool e = true, const QString& d = "")
        : widget(w), order(o), enabled(e), description(d) {}
};

/**
 * @brief Input validation configuration
 */
struct InputValidationConfiguration {
    InputValidationMode mode;
    QVariant minValue;
    QVariant maxValue;
    QString regexPattern;
    QString errorMessage;
    bool allowEmpty;
    bool realTimeValidation;
    
    InputValidationConfiguration(InputValidationMode m = InputValidationMode::NONE)
        : mode(m), allowEmpty(true), realTimeValidation(true) {}
};

/**
 * @brief Global shortcut configuration
 */
struct GlobalShortcutConfiguration {
    QKeySequence keySequence;
    QString description;
    FocusContext allowedContext;
    bool overrideWidgetFocus;
    QObject* receiver;
    const char* slot;
    
    GlobalShortcutConfiguration(const QKeySequence& seq = QKeySequence(), 
                               const QString& desc = "",
                               FocusContext context = FocusContext::UNKNOWN,
                               bool override = false)
        : keySequence(seq), description(desc), allowedContext(context), 
          overrideWidgetFocus(override), receiver(nullptr), slot(nullptr) {}
};

/**
 * @brief Enhanced Number Input Widget (wxNumberTextCtrl equivalent)
 */
class NumberInputWidget : public QLineEdit
{
    Q_OBJECT

public:
    explicit NumberInputWidget(QWidget* parent = nullptr);
    ~NumberInputWidget() override = default;

    // Value access
    void setValue(double value);
    double getValue() const;
    void setIntegerValue(int value);
    int getIntegerValue() const;

    // Validation configuration
    void setValidationMode(InputValidationMode mode);
    void setRange(double min, double max);
    void setIntegerRange(int min, int max);
    void setDecimalPlaces(int places);
    void setAllowEmpty(bool allow);
    void setRealTimeValidation(bool enabled);

    // Validation state
    bool isValid() const;
    QString getValidationError() const;
    void clearValidationError();

    // wxwidgets compatibility
    void setNumbersOnly(bool numbersOnly);
    void setFloatingPoint(bool floatingPoint);

protected:
    void keyPressEvent(QKeyEvent* event) override;
    void focusInEvent(QFocusEvent* event) override;
    void focusOutEvent(QFocusEvent* event) override;

private slots:
    void onTextChanged(const QString& text);
    void validateInput();

signals:
    void valueChanged(double value);
    void integerValueChanged(int value);
    void validationStateChanged(bool isValid);
    void validationError(const QString& error);

private:
    void setupValidator();
    void updateValidationState();
    bool validateCurrentText() const;
    void formatDisplayText();

private:
    InputValidationMode validationMode_;
    double minValue_;
    double maxValue_;
    int decimalPlaces_;
    bool allowEmpty_;
    bool realTimeValidation_;
    bool isValid_;
    QString validationError_;
    QValidator* validator_;
};

/**
 * @brief Main UI Input Manager
 */
class UIInputManager : public QObject
{
    Q_OBJECT

public:
    explicit UIInputManager(QObject* parent = nullptr);
    ~UIInputManager() override;

    // Component setup
    void setMainWindow(MainWindow* mainWindow);
    void setMapView(MapView* mapView);
    MainWindow* getMainWindow() const { return mainWindow_; }
    MapView* getMapView() const { return mapView_; }

    // Focus management
    void enableFocusManagement(bool enabled);
    bool isFocusManagementEnabled() const { return focusManagementEnabled_; }
    FocusContext getCurrentFocusContext() const { return currentFocusContext_; }
    QWidget* getCurrentFocusWidget() const;
    void setFocusContext(FocusContext context);

    // Widget registration
    void registerWidget(QWidget* widget, FocusContext context);
    void unregisterWidget(QWidget* widget);
    void registerDialog(QDialog* dialog);
    void registerDockWidget(QDockWidget* dockWidget);
    void registerPalette(QWidget* palette);

    // Tab order management
    void configureTabOrder(QWidget* parent);
    void setTabOrderConfiguration(const QList<TabOrderConfiguration>& configuration);
    void addTabOrderWidget(QWidget* widget, int order, const QString& description = "");
    void removeTabOrderWidget(QWidget* widget);
    void updateTabOrder();

    // Input validation
    void configureInputValidation(QWidget* widget, const InputValidationConfiguration& config);
    void removeInputValidation(QWidget* widget);
    bool validateWidget(QWidget* widget);
    bool validateAllWidgets();
    QStringList getValidationErrors() const;

    // Global shortcuts
    void registerGlobalShortcut(const GlobalShortcutConfiguration& config);
    void unregisterGlobalShortcut(const QKeySequence& keySequence);
    void enableGlobalShortcuts(bool enabled);
    bool areGlobalShortcutsEnabled() const { return globalShortcutsEnabled_; }

    // MapView focus isolation
    void enableMapViewFocusIsolation(bool enabled);
    bool isMapViewFocusIsolationEnabled() const { return mapViewFocusIsolation_; }
    void setMapViewFocusPolicy(Qt::FocusPolicy policy);

    // Event filtering
    void enableEventFiltering(bool enabled);
    bool isEventFilteringEnabled() const { return eventFilteringEnabled_; }

    // Navigation helpers
    void navigateToNextWidget();
    void navigateToPreviousWidget();
    void navigateToWidget(QWidget* widget);
    void focusFirstInputWidget(QWidget* parent);
    void focusLastInputWidget(QWidget* parent);

    // Specialized input widgets
    NumberInputWidget* createNumberInputWidget(QWidget* parent = nullptr);
    void configureNumberInput(NumberInputWidget* widget, InputValidationMode mode, 
                             double min = 0.0, double max = 999999.0);

public slots:
    void onFocusChanged(QWidget* old, QWidget* now);
    void onApplicationStateChanged(Qt::ApplicationState state);
    void updateFocusContext();
    void handleGlobalShortcut();

signals:
    void focusContextChanged(FocusContext oldContext, FocusContext newContext);
    void widgetFocusChanged(QWidget* widget, FocusContext context);
    void validationStateChanged(QWidget* widget, bool isValid);
    void globalShortcutActivated(const QKeySequence& sequence);

protected:
    bool eventFilter(QObject* watched, QEvent* event) override;

private:
    // Focus management implementation
    FocusContext determineFocusContext(QWidget* widget) const;
    bool isWidgetInContext(QWidget* widget, FocusContext context) const;
    void updateWidgetFocusPolicy(QWidget* widget, FocusContext context);
    void applyFocusContextSettings(FocusContext context);

    // Event handling implementation
    bool handleKeyPressEvent(QKeyEvent* event, QWidget* widget);
    bool handleFocusEvent(QFocusEvent* event, QWidget* widget);
    bool shouldMapViewHandleKey(QKeyEvent* event) const;
    bool isGlobalShortcut(QKeyEvent* event) const;

    // Tab order implementation
    void buildTabOrderChain(QWidget* parent);
    void sortTabOrderWidgets();
    QList<QWidget*> getTabOrderWidgets(QWidget* parent) const;

    // Validation implementation
    void setupWidgetValidation(QWidget* widget, const InputValidationConfiguration& config);
    void validateWidgetInput(QWidget* widget);
    bool isInputWidget(QWidget* widget) const;

    // Utility methods
    bool isTextInputWidget(QWidget* widget) const;
    bool isNumericInputWidget(QWidget* widget) const;
    bool isListWidget(QWidget* widget) const;
    QWidget* findParentDialog(QWidget* widget) const;
    QDockWidget* findParentDockWidget(QWidget* widget) const;

private:
    // Core components
    MainWindow* mainWindow_;
    MapView* mapView_;

    // Focus management
    bool focusManagementEnabled_;
    FocusContext currentFocusContext_;
    QMap<QWidget*, FocusContext> widgetContextMap_;
    QSet<QWidget*> registeredWidgets_;
    QSet<QDialog*> registeredDialogs_;
    QSet<QDockWidget*> registeredDockWidgets_;
    QSet<QWidget*> registeredPalettes_;

    // Tab order management
    QList<TabOrderConfiguration> tabOrderConfiguration_;
    QMap<QWidget*, int> widgetTabOrder_;

    // Input validation
    QMap<QWidget*, InputValidationConfiguration> validationConfiguration_;
    QMap<QWidget*, QValidator*> widgetValidators_;
    QStringList validationErrors_;

    // Global shortcuts
    QList<GlobalShortcutConfiguration> globalShortcuts_;
    QMap<QKeySequence, QShortcut*> shortcutObjects_;
    bool globalShortcutsEnabled_;

    // MapView focus isolation
    bool mapViewFocusIsolation_;
    Qt::FocusPolicy originalMapViewFocusPolicy_;

    // Event filtering
    bool eventFilteringEnabled_;
    QSet<QObject*> filteredObjects_;

    // Update timer
    QTimer* updateTimer_;
};

#endif // UIINPUTMANAGER_H
