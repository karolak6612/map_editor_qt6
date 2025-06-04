#ifndef GOTOPOSITIONDIALOG_H
#define GOTOPOSITIONDIALOG_H

#include <QDialog>
#include "Position.h"  // Use the proper Position class

// Forward declarations
class QTextEdit;
class QLabel;
class QDialogButtonBox;

class GotoPositionDialog : public QDialog {
    Q_OBJECT

public:
    explicit GotoPositionDialog(QWidget *parent = nullptr);
    ~GotoPositionDialog() override;

    // Getter for parsed position
    Position getPosition() const;
    
    // Setter for default position
    void setPosition(const Position& position);

private slots:
    void onAccepted();

private:
    void setupUi();
    Position parsePosition(const QString& input) const;

    // UI elements
    QTextEdit* positionTextEdit_;
    QLabel* formatHintLabel_;
    QDialogButtonBox* buttonBox_;
    
    // Parsed position
    Position position_;
};

#endif // GOTOPOSITIONDIALOG_H
