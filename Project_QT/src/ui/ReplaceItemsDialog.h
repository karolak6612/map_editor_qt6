#ifndef REPLACEITEMSDIALOG_H
#define REPLACEITEMSDIALOG_H

#include <QDialog>

// Forward declarations for Qt classes to minimize include dependencies in header
class QLineEdit;
class QCheckBox;
class QComboBox;
class QPushButton;
class QListWidget;
class QGridLayout;
class QVBoxLayout;
class QFormLayout;
class QGroupBox;
class QDialogButtonBox; // For standard OK/Cancel buttons if needed, or custom buttons

class ReplaceItemsDialog : public QDialog {
    Q_OBJECT

public:
    explicit ReplaceItemsDialog(QWidget *parent = nullptr);
    ~ReplaceItemsDialog() override;

private slots:
    void onFindClicked();
    void onReplaceSelectedClicked();
    void onReplaceAllClicked();
    void onPickFindItemClicked();    // Placeholder
    void onPickReplaceItemClicked(); // Placeholder
    // void onContextItemPropertyChanged(); // Slot if ItemPropertyEditor emits a signal

private:
    // UI Setup Helper
    void setupUi();

    // Find Criteria
    QGroupBox* m_findCriteriaGroup;
    QLineEdit* m_findItemIdLineEdit;
    QLineEdit* m_findNameLineEdit;
    QCheckBox* m_findIsMoveableCheckBox;
    QCheckBox* m_findIsBlockingCheckBox;
    // Add more common item property checkboxes if desired (e.g., isStackable, isPickupable)
    // QCheckBox* m_findIsStackableCheckBox;
    // QCheckBox* m_findIsPickupableCheckBox;
    QComboBox* m_findItemTypeComboBox;     // Placeholder for item types
    QPushButton* m_pickFindItemButton;     // Button to use ItemPropertyEditor for find criteria

    // Replacement Rule
    QGroupBox* m_replaceRuleGroup;
    QLineEdit* m_replaceItemIdLineEdit;
    QCheckBox* m_deleteFoundItemsCheckBox; // Option to delete instead of replace
    QPushButton* m_pickReplaceItemButton;  // Button to use ItemPropertyEditor for replacement item

    // Actions
    QGroupBox* m_actionsGroup; // Optional grouping for action buttons
    QPushButton* m_findButton;
    QPushButton* m_replaceSelectedButton;
    QPushButton* m_replaceAllButton;

    // Results (Placeholder for now)
    QGroupBox* m_resultsGroup;
    QListWidget* m_resultsListWidget;

    // Dialog Buttons
    QDialogButtonBox* m_dialogButtonBox; // For OK/Cancel or just Close

    // Main Layout
    QVBoxLayout* m_mainLayout;
};

#endif // REPLACEITEMSDIALOG_H
