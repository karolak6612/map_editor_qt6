#ifndef GOTOPOSITIONDIALOG_H
#define GOTOPOSITIONDIALOG_H

#include <QDialog>

// Forward declarations
class QTextEdit;
class QLabel;
class QDialogButtonBox;

struct Position {
    int x = 0;
    int y = 0;
    int z = 0;
    
    Position() = default;
    Position(int x, int y, int z) : x(x), y(y), z(z) {}
    
    bool isValid() const {
        // Basic validation - could be enhanced based on map constraints
        return x >= 0 && y >= 0 && z >= 0 && z <= 15;
    }
};

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
