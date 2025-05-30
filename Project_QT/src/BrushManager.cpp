#include "BrushManager.h"
#include "Brush.h" // For Brush::name()
#include <QDebug> // Optional: for logging

BrushManager::BrushManager(QObject *parent) : QObject(parent), currentBrush_(nullptr) {
    // Initialization if any beyond member initializers
}

BrushManager::~BrushManager() {
    // Remove and delete all brushes managed by this manager
    // qDeleteAll takes care of deleting the Brush objects.
    qDeleteAll(brushes_);
    brushes_.clear(); // Then clear the map.
    currentBrush_ = nullptr; // Not strictly necessary as object is being destroyed
}

void BrushManager::addBrush(Brush* brush) {
    if (!brush || brush->name().isEmpty()) {
        qWarning() << "BrushManager: Attempted to add null or unnamed brush.";
        return;
    }

    if (brushes_.contains(brush->name())) {
        Brush* oldBrush = brushes_.value(brush->name());
        if (oldBrush != brush) {
            // qWarning() << "BrushManager: Replacing existing brush with name:" << brush->name();
            delete oldBrush; // Delete the old brush object
        } else {
            // Same brush instance, do nothing
            return;
        }
    }
    brushes_.insert(brush->name(), brush);
}

void BrushManager::removeBrush(const QString& name) {
    if (!brushes_.contains(name)) {
        // qWarning() << "BrushManager: Attempted to remove non-existent brush:" << name;
        return;
    }

    Brush* brushToRemove = brushes_.value(name);

    if (currentBrush_ == brushToRemove) {
        setCurrentBrush(static_cast<Brush*>(nullptr)); // Explicitly cast nullptr if required by compiler/strictness, or just pass nullptr
    }

    brushes_.remove(name);
    delete brushToRemove; // Delete the brush object itself
    brushToRemove = nullptr; // Good practice
}

Brush* BrushManager::getBrush(const QString& name) const {
    return brushes_.value(name, nullptr);
}

QMap<QString, Brush*> BrushManager::getBrushes() const {
    return brushes_; // Returns a copy
}

void BrushManager::setCurrentBrush(const QString& name) {
    Brush* brush = getBrush(name);
    // If name is not found, getBrush returns nullptr, which is handled by the overloaded setCurrentBrush
    setCurrentBrush(brush);
}

void BrushManager::setCurrentBrush(Brush* brush) {
    if (currentBrush_ != brush) {
        currentBrush_ = brush;
        emit currentBrushChanged(currentBrush_);
    }
}

Brush* BrushManager::getCurrentBrush() const {
    return currentBrush_;
}
