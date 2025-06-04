#ifndef INPUTVALIDATIONSYSTEM_H
#define INPUTVALIDATIONSYSTEM_H

#include <QObject>
#include <QWidget>
#include <QValidator>
#include <QRegularExpression>
#include <QRegularExpressionValidator>
#include <QIntValidator>
#include <QDoubleValidator>
#include <QLineEdit>
#include <QTextEdit>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QComboBox>
#include <QMap>
#include <QSet>
#include <QList>
#include <QVariant>
#include <QTimer>
#include <QColor>
#include <QFont>
#include <functional>

// Forward declarations
class MainWindow;
class NumberInputWidget;

/**
 * @brief Task 96: Input Validation System for comprehensive input control
 * 
 * Advanced input validation with wxwidgets compatibility:
 * - Real-time input validation with visual feedback
 * - Custom validation rules and functions
 * - wxNumberTextCtrl equivalent functionality
 * - Input formatting and masking
 * - Error highlighting and tooltips
 * - Validation groups and dependencies
 * - Form-level validation
 * - Complete wxwidgets validation behavior replication
 */

/**
 * @brief Validation result status
 */
enum class ValidationStatus {
    VALID,              // Input is valid
    INVALID,            // Input is invalid
    WARNING,            // Input has warnings but is acceptable
    PENDING,            // Validation is pending (async validation)
    UNKNOWN             // Validation status unknown
};

/**
 * @brief Validation trigger events
 */
enum class ValidationTrigger {
    ON_CHANGE,          // Validate on every change
    ON_FOCUS_LOST,      // Validate when focus is lost
    ON_ENTER_PRESSED,   // Validate when Enter is pressed
    ON_MANUAL,          // Validate only when manually triggered
    ON_FORM_SUBMIT,     // Validate when form is submitted
    ON_TIMER,           // Validate on timer (debounced)
    CUSTOM              // Custom validation trigger
};

/**
 * @brief Visual feedback style for validation
 */
struct ValidationFeedbackStyle {
    QColor validColor;
    QColor invalidColor;
    QColor warningColor;
    QColor pendingColor;
    QFont validFont;
    QFont invalidFont;
    QString validStyleSheet;
    QString invalidStyleSheet;
    QString warningStyleSheet;
    QString pendingStyleSheet;
    bool showTooltips;
    bool highlightBorder;
    bool changeBackground;
    int animationDuration;
    
    ValidationFeedbackStyle() 
        : validColor(Qt::green), invalidColor(Qt::red), warningColor(Qt::yellow), pendingColor(Qt::blue),
          showTooltips(true), highlightBorder(true), changeBackground(false), animationDuration(200) {}
};

/**
 * @brief Validation rule configuration
 */
struct ValidationRule {
    QString name;
    QString description;
    std::function<ValidationStatus(const QVariant&, QString&)> validator;
    ValidationTrigger trigger;
    bool enabled;
    int priority;
    QVariantMap parameters;
    
    ValidationRule(const QString& n = "", const QString& d = "",
                  ValidationTrigger t = ValidationTrigger::ON_CHANGE,
                  bool en = true, int p = 0)
        : name(n), description(d), trigger(t), enabled(en), priority(p) {}
};

/**
 * @brief Validation group for related widgets
 */
class ValidationGroup : public QObject
{
    Q_OBJECT

public:
    explicit ValidationGroup(const QString& name, QObject* parent = nullptr);
    ~ValidationGroup() override = default;

    // Group management
    QString getName() const { return name_; }
    void setDescription(const QString& description) { description_ = description; }
    QString getDescription() const { return description_; }

    // Widget management
    void addWidget(QWidget* widget);
    void removeWidget(QWidget* widget);
    bool containsWidget(QWidget* widget) const;
    QList<QWidget*> getWidgets() const { return widgets_; }

    // Validation operations
    bool validateAll();
    bool isValid() const;
    QStringList getErrors() const;
    QStringList getWarnings() const;
    void clearValidation();

    // Group behavior
    void setStopOnFirstError(bool stop);
    bool isStopOnFirstError() const { return stopOnFirstError_; }
    void setEnabled(bool enabled);
    bool isEnabled() const { return enabled_; }

public slots:
    void onWidgetValidationChanged(QWidget* widget, ValidationStatus status);

signals:
    void groupValidationChanged(bool isValid);
    void widgetValidationFailed(QWidget* widget, const QString& error);

private:
    QString name_;
    QString description_;
    QList<QWidget*> widgets_;
    bool stopOnFirstError_;
    bool enabled_;
};

/**
 * @brief Enhanced validator for complex validation rules
 */
class EnhancedValidator : public QValidator
{
    Q_OBJECT

public:
    explicit EnhancedValidator(QObject* parent = nullptr);
    ~EnhancedValidator() override = default;

    // QValidator interface
    State validate(QString& input, int& pos) const override;
    void fixup(QString& input) const override;

    // Rule management
    void addRule(const ValidationRule& rule);
    void removeRule(const QString& name);
    void clearRules();
    QList<ValidationRule> getRules() const { return rules_; }

    // Validation configuration
    void setAllowEmpty(bool allow);
    bool isAllowEmpty() const { return allowEmpty_; }
    void setTrimWhitespace(bool trim);
    bool isTrimWhitespace() const { return trimWhitespace_; }

    // Custom validation
    ValidationStatus validateValue(const QVariant& value, QString& error) const;
    bool isValueValid(const QVariant& value) const;

signals:
    void validationPerformed(const QVariant& value, ValidationStatus status, const QString& message);

private:
    void sortRulesByPriority();

private:
    QList<ValidationRule> rules_;
    bool allowEmpty_;
    bool trimWhitespace_;
};

/**
 * @brief Main Input Validation System
 */
class InputValidationSystem : public QObject
{
    Q_OBJECT

public:
    explicit InputValidationSystem(QObject* parent = nullptr);
    ~InputValidationSystem() override;

    // Component setup
    void setMainWindow(MainWindow* mainWindow);
    MainWindow* getMainWindow() const { return mainWindow_; }

    // System control
    void enableValidation(bool enabled);
    bool isValidationEnabled() const { return validationEnabled_; }
    void setRealTimeValidation(bool enabled);
    bool isRealTimeValidation() const { return realTimeValidation_; }

    // Widget registration
    void registerWidget(QWidget* widget, const QList<ValidationRule>& rules = QList<ValidationRule>());
    void unregisterWidget(QWidget* widget);
    bool isWidgetRegistered(QWidget* widget) const;

    // Validation rule management
    void addValidationRule(QWidget* widget, const ValidationRule& rule);
    void removeValidationRule(QWidget* widget, const QString& ruleName);
    void clearValidationRules(QWidget* widget);
    QList<ValidationRule> getValidationRules(QWidget* widget) const;

    // Validation group management
    ValidationGroup* createValidationGroup(const QString& name, const QString& description = "");
    void removeValidationGroup(const QString& name);
    ValidationGroup* getValidationGroup(const QString& name) const;
    QStringList getValidationGroupNames() const;

    // Validation operations
    bool validateWidget(QWidget* widget);
    bool validateAllWidgets();
    bool validateGroup(const QString& groupName);
    ValidationStatus getWidgetValidationStatus(QWidget* widget) const;
    QString getWidgetValidationError(QWidget* widget) const;

    // Visual feedback
    void setValidationFeedbackStyle(const ValidationFeedbackStyle& style);
    ValidationFeedbackStyle getValidationFeedbackStyle() const { return feedbackStyle_; }
    void enableVisualFeedback(bool enabled);
    bool isVisualFeedbackEnabled() const { return visualFeedbackEnabled_; }

    // Built-in validation rules
    static ValidationRule createIntegerRule(int min = INT_MIN, int max = INT_MAX);
    static ValidationRule createDoubleRule(double min = -DBL_MAX, double max = DBL_MAX, int decimals = -1);
    static ValidationRule createRangeRule(const QVariant& min, const QVariant& max);
    static ValidationRule createRegexRule(const QString& pattern, const QString& errorMessage = "");
    static ValidationRule createRequiredRule(const QString& errorMessage = "This field is required");
    static ValidationRule createLengthRule(int minLength, int maxLength = -1);
    static ValidationRule createEmailRule();
    static ValidationRule createUrlRule();
    static ValidationRule createFilePathRule();
    static ValidationRule createColorRule();
    static ValidationRule createItemIdRule();
    static ValidationRule createPositionRule();

    // wxNumberTextCtrl equivalent
    NumberInputWidget* createNumberInputWidget(QWidget* parent = nullptr);
    void configureNumberInput(NumberInputWidget* widget, bool integersOnly = false, 
                             double min = -DBL_MAX, double max = DBL_MAX);

    // Validation triggers
    void setValidationTrigger(QWidget* widget, ValidationTrigger trigger);
    ValidationTrigger getValidationTrigger(QWidget* widget) const;
    void triggerValidation(QWidget* widget);

    // Error handling
    QStringList getAllErrors() const;
    QStringList getAllWarnings() const;
    void clearAllErrors();
    bool hasErrors() const;
    bool hasWarnings() const;

    // Statistics and monitoring
    QVariantMap getStatistics() const;
    void resetStatistics();
    int getValidationCount() const { return validationCount_; }
    int getErrorCount() const { return errorCount_; }

public slots:
    void onWidgetTextChanged();
    void onWidgetValueChanged();
    void onWidgetFocusLost();
    void onWidgetEditingFinished();
    void onValidationTimer();

signals:
    void widgetValidationChanged(QWidget* widget, ValidationStatus status, const QString& message);
    void validationGroupChanged(const QString& groupName, bool isValid);
    void validationError(QWidget* widget, const QString& error);
    void validationWarning(QWidget* widget, const QString& warning);
    void allValidationCompleted(bool allValid);

private:
    // Widget validation implementation
    void performWidgetValidation(QWidget* widget);
    void applyValidationFeedback(QWidget* widget, ValidationStatus status, const QString& message);
    void clearValidationFeedback(QWidget* widget);

    // Validation rule execution
    ValidationStatus executeValidationRules(QWidget* widget, const QVariant& value, QString& error);
    QVariant getWidgetValue(QWidget* widget) const;
    void setWidgetValue(QWidget* widget, const QVariant& value);

    // Visual feedback implementation
    void updateWidgetStyle(QWidget* widget, ValidationStatus status);
    void showValidationTooltip(QWidget* widget, const QString& message, ValidationStatus status);
    void hideValidationTooltip(QWidget* widget);

    // Widget analysis
    bool isInputWidget(QWidget* widget) const;
    bool isTextInputWidget(QWidget* widget) const;
    bool isNumericInputWidget(QWidget* widget) const;
    ValidationTrigger getDefaultTrigger(QWidget* widget) const;

    // Event handling
    void connectWidgetSignals(QWidget* widget);
    void disconnectWidgetSignals(QWidget* widget);

    // Statistics tracking
    void updateStatistics(ValidationStatus status);

private:
    // Core components
    MainWindow* mainWindow_;

    // System state
    bool validationEnabled_;
    bool realTimeValidation_;
    bool visualFeedbackEnabled_;

    // Widget management
    QMap<QWidget*, QList<ValidationRule>> widgetRules_;
    QMap<QWidget*, ValidationStatus> widgetStatus_;
    QMap<QWidget*, QString> widgetErrors_;
    QMap<QWidget*, ValidationTrigger> widgetTriggers_;
    QSet<QWidget*> registeredWidgets_;

    // Validation groups
    QMap<QString, ValidationGroup*> validationGroups_;

    // Visual feedback
    ValidationFeedbackStyle feedbackStyle_;
    QMap<QWidget*, QString> originalStyleSheets_;

    // Validation timers (for debounced validation)
    QMap<QWidget*, QTimer*> validationTimers_;

    // Statistics
    int validationCount_;
    int errorCount_;
    int warningCount_;
    QMap<QWidget*, int> widgetValidationCounts_;

    // Constants
    static const int DEFAULT_VALIDATION_DELAY = 500; // milliseconds
};

#endif // INPUTVALIDATIONSYSTEM_H
