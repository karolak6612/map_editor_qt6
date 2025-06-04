#include "ItemPropertyEditor.h"
#include "Item.h" // For casting and accessing Item properties (even if just for logging in stub)
#include <QLabel>
#include <QVBoxLayout>
#include <QDebug> // For logging
#include <QMetaMethod> // For className debug

ItemPropertyEditor::ItemPropertyEditor(QWidget *parent)
    : ItemPropertyEditorBase(parent), m_placeholderLabel(nullptr) {

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    m_placeholderLabel = new QLabel("Item Property Editor Stub", this);
    m_placeholderLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(m_placeholderLabel);
    setLayout(mainLayout);

    // m_editingObject is inherited from ItemPropertyEditorBase and initialized to nullptr there.
}

ItemPropertyEditor::~ItemPropertyEditor() {
    // m_placeholderLabel is a child of this widget, Qt will handle its deletion.
}

void ItemPropertyEditor::setEditingObject(QObject* object) {
    // Store the generic QObject pointer in the base class member
    if (m_editingObject != object) {
        m_editingObject = object;
        emit editingObjectChanged(object);

        Item* item = qobject_cast<Item*>(object);

        if (item) {
            qDebug() << "ItemPropertyEditor: Editing item with ID:" << item->getServerId() << "Name:" << item->name();
        } else if (object) {
            qDebug() << "ItemPropertyEditor: Received a QObject that is not an Item. ClassName:" << object->metaObject()->className();
        } else {
            qDebug() << "ItemPropertyEditor: Editing object set to nullptr.";
        }

        // Call loadPropertiesFromObject to refresh UI based on new object (or clear it if object is null)
        loadPropertiesFromObject();
        markAsModified(false); // Reset modification state for new object
    }
}

void ItemPropertyEditor::loadPropertiesFromObject() {
    if (!m_editingObject) {
        qDebug() << "ItemPropertyEditor::loadPropertiesFromObject: No object to load from.";
        m_placeholderLabel->setText("No item selected.");
        // In a real implementation, disable UI elements or clear them.
        return;
    }

    Item* item = qobject_cast<Item*>(m_editingObject);
    if (item) {
        qDebug() << "ItemPropertyEditor::loadPropertiesFromObject: Called for Item ID:" << item->getServerId();
        m_placeholderLabel->setText(QString("Editing Item: %1 (ID: %2)")
                                     .arg(item->name())
                                     .arg(item->getServerId()));
        // In a real implementation, populate UI fields from 'item' properties.
        // e.g., m_nameLineEdit->setText(item->name());
    } else {
        qDebug() << "ItemPropertyEditor::loadPropertiesFromObject: Editing object is not an Item. ClassName:" << m_editingObject->metaObject()->className();
        m_placeholderLabel->setText("Selected object is not an Item.");
    }
}

void ItemPropertyEditor::savePropertiesToObject() {
    if (!m_editingObject) {
        qDebug() << "ItemPropertyEditor::savePropertiesToObject: No object to save to.";
        return;
    }

    Item* item = qobject_cast<Item*>(m_editingObject);
    if (item) {
        qDebug() << "ItemPropertyEditor::savePropertiesToObject: Called for Item ID:" << item->getServerId();
        // In a real implementation, get values from UI fields and set them on 'item'.
        // e.g., item->setName(m_nameLineEdit->text());
        // This should ideally create an QUndoCommand.
    } else {
         qDebug() << "ItemPropertyEditor::savePropertiesToObject: Editing object is not an Item. ClassName:" << m_editingObject->metaObject()->className();
    }
}

// bool ItemPropertyEditor::hasPendingChanges() const {
//     // For a stub, can rely on base class returning false.
//     // If UI elements were added, this would compare their current state
//     // to the state of m_editingObject.
//     return ItemPropertyEditorBase::hasPendingChanges();
// }
