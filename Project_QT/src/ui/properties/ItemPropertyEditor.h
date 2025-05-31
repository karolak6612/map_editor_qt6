#ifndef ITEMPROPERTYEDITOR_H
#define ITEMPROPERTYEDITOR_H

#include "ItemPropertyEditorBase.h" // Base class

// Forward declarations
class Item;     // For casting m_editingObject or a more specific member
class QLabel;   // For the placeholder UI element
class QVBoxLayout; // For layout

class ItemPropertyEditor : public ItemPropertyEditorBase {
    Q_OBJECT

public:
    explicit ItemPropertyEditor(QWidget *parent = nullptr);
    ~ItemPropertyEditor() override;

    // Override pure virtual methods from ItemPropertyEditorBase
    void setEditingObject(QObject* object) override;
    void loadPropertiesFromObject() override;
    void savePropertiesToObject() override;

    // Override hasPendingChanges if specific logic is needed, otherwise base impl is fine for a stub
    // bool hasPendingChanges() const override;

private:
    QLabel* m_placeholderLabel = nullptr; // Initialize to nullptr
    // Item* m_currentItem = nullptr; // Optional: store the casted object if frequently accessed
};

#endif // ITEMPROPERTYEDITOR_H
