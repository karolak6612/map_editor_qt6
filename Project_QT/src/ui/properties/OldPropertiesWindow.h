#ifndef OLDPROPERTIESWINDOW_H
#define OLDPROPERTIESWINDOW_H

#include "ItemPropertyEditorBase.h" // Base class

// Forward declarations
class QLabel;   // For the placeholder UI element
class QVBoxLayout; // For layout
// QObject is forward declared/included via ItemPropertyEditorBase.h -> QWidget

class OldPropertiesWindow : public ItemPropertyEditorBase {
    Q_OBJECT

public:
    explicit OldPropertiesWindow(QWidget *parent = nullptr);
    ~OldPropertiesWindow() override;

    // Override pure virtual methods from ItemPropertyEditorBase
    void setEditingObject(QObject* object) override;
    void loadPropertiesFromObject() override;
    void savePropertiesToObject() override;

    // Override hasPendingChanges if specific logic is needed, otherwise base impl is fine for a stub
    // bool hasPendingChanges() const override;

private:
    QLabel* m_placeholderLabel = nullptr; // Initialize to nullptr
    // No specific members like m_currentItem, as this is a more generic editor stub
};

#endif // OLDPROPERTIESWINDOW_H
