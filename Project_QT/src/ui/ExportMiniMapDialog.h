#ifndef EXPORTMINIMAPDIALOG_H
#define EXPORTMINIMAPDIALOG_H

#include <QDialog>

// Forward declarations
class QLineEdit;
class QPushButton;
class QSpinBox;
class QComboBox;
class QDialogButtonBox;
class QCheckBox;

class ExportMiniMapDialog : public QDialog {
    Q_OBJECT

public:
    explicit ExportMiniMapDialog(QWidget *parent = nullptr);
    ~ExportMiniMapDialog() override;

    // Getters for export settings
    QString getOutputDirectory() const;
    QString getFileName() const;
    int getStartFloor() const;
    int getEndFloor() const;
    int getExportType() const;
    bool shouldExportEmptyFloors() const;

    // Setters for default values
    void setOutputDirectory(const QString& directory);
    void setFileName(const QString& fileName);
    void setFloorRange(int startFloor, int endFloor);

private slots:
    void onBrowseClicked();
    void onExportTypeChanged();
    void onAccepted();

private:
    void setupUi();

    // UI elements
    QLineEdit* outputDirectoryLineEdit_;
    QPushButton* browseButton_;
    QLineEdit* fileNameLineEdit_;
    QSpinBox* startFloorSpinBox_;
    QSpinBox* endFloorSpinBox_;
    QComboBox* exportTypeComboBox_;
    QCheckBox* exportEmptyFloorsCheckBox_;
    QDialogButtonBox* buttonBox_;
};

#endif // EXPORTMINIMAPDIALOG_H
