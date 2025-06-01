#ifndef GROUNDVALIDATIONDIALOG_H
#define GROUNDVALIDATIONDIALOG_H

#include <QDialog>

// Forward declarations
class QCheckBox;
class QLabel;
class QDialogButtonBox;

class GroundValidationDialog : public QDialog {
    Q_OBJECT

public:
    explicit GroundValidationDialog(QWidget *parent = nullptr);
    ~GroundValidationDialog() override;

    // Getters for validation options
    bool shouldValidateGroundStack() const;
    bool shouldGenerateEmptySurroundedGrounds() const;
    bool shouldRemoveDuplicateGrounds() const;

private slots:
    void onAccepted();

private:
    void setupUi();

    // UI elements
    QCheckBox* validateGroundStackBox_;
    QCheckBox* generateEmptySurroundedGroundsBox_;
    QCheckBox* removeDuplicateGroundsBox_;
    QLabel* warningLabel_;
    QDialogButtonBox* buttonBox_;

    // State variables
    bool validateGroundStack_;
    bool generateEmptySurroundedGrounds_;
    bool removeDuplicateGrounds_;
};

#endif // GROUNDVALIDATIONDIALOG_H
