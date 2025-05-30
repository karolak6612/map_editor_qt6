#ifndef ABOUTDIALOG_H
#define ABOUTDIALOG_H

#include <QDialog>

class QLabel; // Forward declare
class QPushButton; // Forward declare

class AboutDialog : public QDialog {
    Q_OBJECT

public:
    explicit AboutDialog(QWidget *parent = nullptr);
    ~AboutDialog() override;

private:
    QLabel *infoLabel_;
    QPushButton *okButton_;
};

#endif // ABOUTDIALOG_H
