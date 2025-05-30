#include "PlaceholderPaletteWidget.h"
#include <QLabel>
#include <QVBoxLayout>
#include <QDebug>
#include <QPalette> // For background color
#include <QColor>   // For QColor

PlaceholderPaletteWidget::PlaceholderPaletteWidget(const QString& name, QWidget *parent) : QWidget(parent) {
    QVBoxLayout *layout = new QVBoxLayout(this);
    label_ = new QLabel(QString("%1 Placeholder").arg(name), this);
    label_->setAlignment(Qt::AlignCenter);
    layout->addWidget(label_);
    setLayout(layout);
    
    // Optional: Give it a distinct background color for easy identification
    setAutoFillBackground(true);
    QPalette pal = palette();
    // Using a slightly different color for each placeholder for better visual distinction
    if (name.compare("Palette", Qt::CaseInsensitive) == 0) {
        pal.setColor(QPalette::Window, QColor(220, 220, 240)); // Light Lavender
    } else if (name.compare("Minimap", Qt::CaseInsensitive) == 0) {
        pal.setColor(QPalette::Window, QColor(220, 240, 220)); // Light Mint
    } else if (name.compare("Properties", Qt::CaseInsensitive) == 0) {
        pal.setColor(QPalette::Window, QColor(240, 220, 220)); // Light Rose
    } else {
        pal.setColor(QPalette::Window, QColor(Qt::lightGray)); 
    }
    setPalette(pal);

    qDebug() << QString("Placeholder%1Widget created.").arg(name);
}

PlaceholderPaletteWidget::~PlaceholderPaletteWidget() {
    // No need to delete label_ or layout, Qt's parent-child system handles it.
    qDebug() << QString("Placeholder%1Widget (specifically '%2') destroyed.").arg(label_->text().split(" ").first()).arg(label_->text());
}
