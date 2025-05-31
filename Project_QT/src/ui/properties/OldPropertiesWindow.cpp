#include "OldPropertiesWindow.h"
#include <QLabel>
#include <QVBoxLayout>
#include <QDebug> // For logging
#include <QObject> // For QObject::metaObject()->className()
#include <QMetaMethod> // For QObject::metaObject()

OldPropertiesWindow::OldPropertiesWindow(QWidget *parent)
    : ItemPropertyEditorBase(parent), m_placeholderLabel(nullptr) {

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    m_placeholderLabel = new QLabel("OldPropertiesWindow Stub (Generic Property Editor)");
    m_placeholderLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(m_placeholderLabel);
    setLayout(mainLayout);

    // m_editingObject is inherited from ItemPropertyEditorBase and initialized to nullptr there.
}

OldPropertiesWindow::~OldPropertiesWindow() {
    // m_placeholderLabel is a child of this widget, Qt will handle its deletion.
}

void OldPropertiesWindow::setEditingObject(QObject* object) {
    // Store the generic QObject pointer in the base class member
    m_editingObject = object;

    if (m_editingObject) {
        qDebug() << "OldPropertiesWindow: Editing object of type:" << m_editingObject->metaObject()->className();
    } else {
        qDebug() << "OldPropertiesWindow: Editing object set to nullptr.";
    }
    // Call loadPropertiesFromObject to refresh UI based on new object (or clear it if object is null)
    loadPropertiesFromObject();
}

void OldPropertiesWindow::loadPropertiesFromObject() {
    if (!m_editingObject) {
        qDebug() << "OldPropertiesWindow::loadPropertiesFromObject: No object to load from.";
        m_placeholderLabel->setText("No object selected.");
        // Potentially disable UI or clear fields here
        return;
    }

    qDebug() << "OldPropertiesWindow::loadPropertiesFromObject: Called for object type:" << m_editingObject->metaObject()->className();
    // In a real implementation, populate UI fields from 'm_editingObject' properties.
    // This would likely involve dynamic property access or type-specific handling.
    m_placeholderLabel->setText(QString("Editing Object (Type: %1)")
                                 .arg(m_editingObject->metaObject()->className()));
}

void OldPropertiesWindow::savePropertiesToObject() {
    if (!m_editingObject) {
        qDebug() << "OldPropertiesWindow::savePropertiesToObject: No object to save to.";
        return;
    }

    qDebug() << "OldPropertiesWindow::savePropertiesToObject: Called for object type:" << m_editingObject->metaObject()->className();
    // In a real implementation, get values from UI fields and set them on 'm_editingObject'.
    // This would involve dynamic property access or type-specific handling.
    // Remember to handle undo/redo.
}

// bool OldPropertiesWindow::hasPendingChanges() const {
//     // For a stub, can rely on base class returning false.
//     return ItemPropertyEditorBase::hasPendingChanges();
// }
