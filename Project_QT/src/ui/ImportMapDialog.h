#ifndef IMPORTMAPDIALOG_H
#define IMPORTMAPDIALOG_H

#include <QDialog>

// Forward declarations
class QLineEdit;
class QPushButton;
class QSpinBox;
class QComboBox;
class QDialogButtonBox;

class ImportMapDialog : public QDialog {
    Q_OBJECT

public:
    explicit ImportMapDialog(QWidget *parent = nullptr);
    ~ImportMapDialog() override;

    // Getters for import settings
    QString getFilePath() const;
    int getXOffset() const;
    int getYOffset() const;
    int getHouseOption() const;
    int getSpawnOption() const;

    // Setters for default values
    void setFilePath(const QString& path);
    void setXOffset(int offset);
    void setYOffset(int offset);

private slots:
    void onBrowseClicked();
    void onAccepted();

private:
    void setupUi();

    // UI elements
    QLineEdit* filePathLineEdit_;
    QPushButton* browseButton_;
    QSpinBox* xOffsetSpinBox_;
    QSpinBox* yOffsetSpinBox_;
    QComboBox* houseOptionsComboBox_;
    QComboBox* spawnOptionsComboBox_;
    QDialogButtonBox* buttonBox_;
};

#endif // IMPORTMAPDIALOG_H
