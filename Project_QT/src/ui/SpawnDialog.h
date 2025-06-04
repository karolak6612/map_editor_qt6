#ifndef SPAWNDIALOG_H
#define SPAWNDIALOG_H

#include <QDialog>
#include <QStringList> // For passing creature names, though not directly stored

// Forward declarations
class QLineEdit;
class QSpinBox;
class QListWidget;
class QPushButton;
class QDialogButtonBox;
class Spawn; // Forward declare the data class

class SpawnDialog : public QDialog {
    Q_OBJECT

public:
    explicit SpawnDialog(QWidget* parent = nullptr);
    SpawnDialog(Spawn* spawnToEdit, QWidget* parent = nullptr); // Constructor to load an existing spawn
    ~SpawnDialog() override;

    // Method to load spawn data into the dialog
    void setSpawnData(Spawn* spawn);
    // Method to apply dialog data back to a spawn object
    void applyToSpawn(Spawn* spawn) const;

    // Task 019: Get spawn data without modifying the spawn (for undo integration)
    struct SpawnData {
        int radius;
        int interval;
        int maxCreatures;
        QStringList creatureNames;
    };
    SpawnData getSpawnData() const;

private slots:
    void onAddCreatureClicked();
    void onRemoveCreatureClicked();

private:
    // UI element pointers
    QSpinBox* radiusSpinBox_ = nullptr;
    QSpinBox* intervalSpinBox_ = nullptr; // For spawn interval in ms
    QSpinBox* maxCreaturesSpinBox_ = nullptr;
    QListWidget* creatureListWidget_ = nullptr; // To display/edit creature names
    QLineEdit* addCreatureLineEdit_ = nullptr; // To type new creature name
    QPushButton* addCreatureButton_ = nullptr;
    QPushButton* removeCreatureButton_ = nullptr; // To remove selected creature from listWidget
    QDialogButtonBox* buttonBox_ = nullptr;

    Spawn* currentSpawn_ = nullptr; // Pointer to the spawn being edited (not owned)
};

#endif // SPAWNDIALOG_H
