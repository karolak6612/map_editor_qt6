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

protected:
    QObject* m_editingObject = nullptr; // Initialize to nullptr

private:
    // No private members specific to the base class in this stub
};

#endif // ITEMPROPERTYEDITORBASE_H
