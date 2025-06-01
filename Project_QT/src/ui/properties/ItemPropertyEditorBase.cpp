#include "ItemPropertyEditorBase.h"
#include <QDebug>

ItemPropertyEditorBase::ItemPropertyEditorBase(QWidget *parent)
    : QWidget(parent), m_editingObject(nullptr), m_hasPendingChanges(false), m_isModified(false) {
    // Constructor body can be empty for this abstract base, or setup common UI elements if any.
    qDebug() << "ItemPropertyEditorBase: Constructor called";
}

ItemPropertyEditorBase::~ItemPropertyEditorBase() {
    // Destructor body.
    // m_editingObject is not owned by the editor base by default, so not deleted here.
    // Subclasses might take ownership or handle it differently if needed.
    qDebug() << "ItemPropertyEditorBase: Destructor called";
}

bool ItemPropertyEditorBase::hasPendingChanges() const {
    // Default implementation: return the tracked pending changes state
    // Subclasses should override this if they have more complex change tracking
    return m_hasPendingChanges;
}

// Additional virtual methods for property manipulation (Task 45 requirement)
void ItemPropertyEditorBase::setProperty(const QString& key, const QVariant& value) {
    // Default implementation: emit signal for property change
    // Subclasses should override this to handle specific property setting
    qDebug() << "ItemPropertyEditorBase::setProperty:" << key << "=" << value;
    emit propertyChanged(key, value);
    markAsModified(true);
}

QVariant ItemPropertyEditorBase::getProperty(const QString& key) const {
    // Default implementation: return invalid variant
    // Subclasses should override this to return actual property values
    Q_UNUSED(key);
    qDebug() << "ItemPropertyEditorBase::getProperty:" << key << "(returning invalid variant)";
    return QVariant();
}

// Virtual methods for validation and change tracking
bool ItemPropertyEditorBase::validateInput() const {
    // Default implementation: assume input is always valid
    // Subclasses should override this to perform actual validation
    return true;
}

void ItemPropertyEditorBase::resetToOriginalValues() {
    // Default implementation: reload from object
    // Subclasses should override this to reset to specific original values
    qDebug() << "ItemPropertyEditorBase::resetToOriginalValues: Reloading from object";
    loadPropertiesFromObject();
    markAsModified(false);
}

void ItemPropertyEditorBase::markAsModified(bool modified) {
    if (m_isModified != modified) {
        m_isModified = modified;
        m_hasPendingChanges = modified;
        emit pendingChangesChanged(m_hasPendingChanges);
        qDebug() << "ItemPropertyEditorBase::markAsModified:" << modified;
    }
}

// Slots for common property editor operations
void ItemPropertyEditorBase::applyChanges() {
    // Default implementation: save properties and mark as not modified
    qDebug() << "ItemPropertyEditorBase::applyChanges: Saving properties to object";
    if (validateInput()) {
        savePropertiesToObject();
        markAsModified(false);
    } else {
        qDebug() << "ItemPropertyEditorBase::applyChanges: Validation failed, not saving";
    }
}

void ItemPropertyEditorBase::discardChanges() {
    // Default implementation: reset to original values
    qDebug() << "ItemPropertyEditorBase::discardChanges: Resetting to original values";
    resetToOriginalValues();
}

void ItemPropertyEditorBase::refreshFromObject() {
    // Default implementation: reload properties from object
    qDebug() << "ItemPropertyEditorBase::refreshFromObject: Reloading properties";
    loadPropertiesFromObject();
    markAsModified(false);
}
