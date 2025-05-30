#ifndef PLACEHOLDERPALETTEWIDGET_H
#define PLACEHOLDERPALETTEWIDGET_H

#include <QWidget>

class QLabel; // Forward declaration

class PlaceholderPaletteWidget : public QWidget {
    Q_OBJECT
public:
    explicit PlaceholderPaletteWidget(const QString& name, QWidget *parent = nullptr);
    ~PlaceholderPaletteWidget() override;
private:
    QLabel *label_;
};

#endif // PLACEHOLDERPALETTEWIDGET_H
