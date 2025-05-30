#ifndef AUTOMAGICSETTINGSDIALOG_H
#define AUTOMAGICSETTINGSDIALOG_H

#include <QDialog>

// Forward declarations for pointer members to reduce include dependencies
class QCheckBox;
class QSpinBox;
class QLabel;
class QPushButton;
// class QDialogButtonBox; // Alternative

class AutomagicSettingsDialog : public QDialog {
    Q_OBJECT

public:
    explicit AutomagicSettingsDialog(QWidget* parent = nullptr);
    ~AutomagicSettingsDialog() override;

    // Methods to get current settings
    bool isAutomagicEnabled() const;
    bool isSameGroundTypeBorderEnabled() const;
    bool isWallsRepelBordersEnabled() const;
    bool isLayerCarpetsEnabled() const;
    bool isBorderizeDeleteEnabled() const;
    bool isCustomBorderEnabled() const;
    int getCustomBorderId() const;

    // Method to set initial values
    void setSettings(bool automagicEnabled, bool sameGround, bool wallsRepel, 
                     bool layerCarpets, bool borderizeDelete, bool customBorder, 
                     int customBorderId);

private slots:
    void onAutomagicEnabledChanged(int state);
    void onCustomBorderEnabledChanged(int state);
    // void onOkClicked(); // If using custom OK button
    // void onCancelClicked(); // If using custom Cancel button

private:
    void updateDependentControlStates();

    // UI element pointers
    QCheckBox* automagicEnabledCheckBox_ = nullptr;
    QCheckBox* sameGroundTypeBorderCheckBox_ = nullptr;
    QCheckBox* wallsRepelBordersCheckBox_ = nullptr;
    QCheckBox* layerCarpetsCheckBox_ = nullptr;
    QCheckBox* borderizeDeleteCheckBox_ = nullptr;
    QCheckBox* customBorderCheckBox_ = nullptr;
    QLabel* customBorderIdLabel_ = nullptr;
    QSpinBox* customBorderIdSpinBox_ = nullptr;
    QLabel* descriptionLabel_ = nullptr;
    QPushButton* okButton_ = nullptr;
    QPushButton* cancelButton_ = nullptr;
    // QDialogButtonBox* buttonBox_ = nullptr; // Alternative
};

#endif // AUTOMAGICSETTINGSDIALOG_H
