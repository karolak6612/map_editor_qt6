#ifndef ITEMPROPERTYEDITORBASE_H
#define ITEMPROPERTYEDITORBASE_H

#include <QWidget> // Inherits from QWidget
#include <QString> // For method parameters/returns if any
#include <QVariant> // For method parameters/returns if any

// Forward declaration if needed, QObject is usually available via QWidget
// class QObject;

class ItemPropertyEditorBase : public QWidget {
    Q_OBJECT

public:
    explicit ItemPropertyEditorBase(QWidget *parent = nullptr);
    ~ItemPropertyEditorBase() override;

    // Pure virtual methods to be implemented by subclasses
    virtual void setEditingObject(QObject* object) = 0;
    virtual void loadPropertiesFromObject() = 0;
    virtual void savePropertiesToObject() = 0;

    // Virtual method with a default implementation
    virtual bool hasPendingChanges() const;

    // Additional virtual methods for property manipulation (Task 45 requirement)
    virtual void setProperty(const QString& key, const QVariant& value);
    virtual QVariant getProperty(const QString& key) const;

    // Virtual methods for validation and change tracking
    virtual bool validateInput() const;
    virtual void resetToOriginalValues();
    virtual void markAsModified(bool modified = true);

signals:
    // Signals for property editor events
    void propertyChanged(const QString& propertyName, const QVariant& newValue);
    void editingObjectChanged(QObject* object);
    void pendingChangesChanged(bool hasPendingChanges);

public slots:
    // Slots for common property editor operations
    virtual void applyChanges();
    virtual void discardChanges();
    virtual void refreshFromObject();

protected:
    QObject* m_editingObject = nullptr; // Initialize to nullptr
    bool m_hasPendingChanges = false;   // Track if there are unsaved changes
    bool m_isModified = false;          // Track if the editor has been modified

private:
    // No private members specific to the base class in this stub
};

#endif // ITEMPROPERTYEDITORBASE_H
