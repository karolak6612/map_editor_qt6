// WaypointEditorPanel_Events.cpp - Event handlers for WaypointEditorPanel (Task 71)

#include "WaypointEditorPanel.h"
#include "Map.h"
#include "Waypoint.h"
#include "Waypoints.h"
#include <QMessageBox>
#include <QInputDialog>
#include <QColorDialog>
#include <QFileDialog>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QXmlStreamWriter>
#include <QXmlStreamReader>
#include <QStandardPaths>
#include <QApplication>
#include <QClipboard>

// Event handlers implementation for WaypointEditorPanel

// Map change handlers
void WaypointEditorPanel::onMapChanged() {
    if (!refreshTimer_->isActive()) {
        refreshTimer_->start();
    }
}

void WaypointEditorPanel::onWaypointAdded(Waypoint* waypoint) {
    if (!waypoint || updatingUI_) {
        return;
    }
    
    // Add to list if it matches current filters
    if (matchesFilter(waypoint)) {
        QListWidgetItem* item = new QListWidgetItem();
        updateWaypointListItem(item, waypoint);
        waypointList_->addItem(item);
        
        // Update count
        int count = waypointList_->count();
        waypointCountLabel_->setText(QString("%1 waypoint%2").arg(count).arg(count == 1 ? "" : "s"));
    }
    
    updateButtonStates();
}

void WaypointEditorPanel::onWaypointRemoved(const QString& name) {
    QListWidgetItem* item = findWaypointItem(name);
    if (item) {
        if (getWaypointFromItem(item) == selectedWaypoint_) {
            clearSelection();
        }
        delete item;
        
        // Update count
        int count = waypointList_->count();
        waypointCountLabel_->setText(QString("%1 waypoint%2").arg(count).arg(count == 1 ? "" : "s"));
    }
    
    updateButtonStates();
}

void WaypointEditorPanel::onWaypointRemoved(Waypoint* waypoint) {
    if (!waypoint) {
        return;
    }
    
    QListWidgetItem* item = findWaypointItem(waypoint);
    if (item) {
        if (waypoint == selectedWaypoint_) {
            clearSelection();
        }
        delete item;
        
        // Update count
        int count = waypointList_->count();
        waypointCountLabel_->setText(QString("%1 waypoint%2").arg(count).arg(count == 1 ? "" : "s"));
    }
    
    updateButtonStates();
}

void WaypointEditorPanel::onWaypointModified(Waypoint* waypoint) {
    if (!waypoint) {
        return;
    }
    
    QListWidgetItem* item = findWaypointItem(waypoint);
    if (item) {
        updateWaypointListItem(item, waypoint);
        
        // Update property editor if this waypoint is selected
        if (waypoint == selectedWaypoint_) {
            updatePropertyEditor(waypoint);
        }
    }
}

void WaypointEditorPanel::onWaypointsCleared() {
    clearSelection();
    waypointList_->clear();
    waypointCountLabel_->setText("0 waypoints");
    updateButtonStates();
}

// UI event handlers
void WaypointEditorPanel::onWaypointListSelectionChanged() {
    if (updatingUI_) {
        return;
    }
    
    QListWidgetItem* currentItem = waypointList_->currentItem();
    Waypoint* waypoint = getWaypointFromItem(currentItem);
    
    if (waypoint != selectedWaypoint_) {
        selectedWaypoint_ = waypoint;
        updatePropertyEditor(waypoint);
        updateButtonStates();
        emit waypointSelected(waypoint);
    }
}

void WaypointEditorPanel::onWaypointListItemDoubleClicked(QListWidgetItem* item) {
    Waypoint* waypoint = getWaypointFromItem(item);
    if (waypoint) {
        emit waypointDoubleClicked(waypoint);
        emit centerOnWaypoint(waypoint);
    }
}

void WaypointEditorPanel::onWaypointListContextMenu(const QPoint& pos) {
    QListWidgetItem* item = waypointList_->itemAt(pos);
    if (item && contextMenu_) {
        contextMenu_->exec(waypointList_->mapToGlobal(pos));
    }
}

// Button handlers
void WaypointEditorPanel::onAddWaypointClicked() {
    if (!map_) {
        return;
    }
    
    emit newWaypointRequested();
    
    // For now, create a simple waypoint at map center
    MapPos centerPos(map_->getWidth() / 2, map_->getHeight() / 2, 7);
    
    WaypointCreationDialog dialog(map_, centerPos, this);
    if (dialog.exec() == QDialog::Accepted) {
        QString name = dialog.getWaypointName();
        QString type = dialog.getWaypointType();
        MapPos position = dialog.getWaypointPosition();
        int radius = dialog.getWaypointRadius();
        QColor color = dialog.getWaypointColor();
        QString script = dialog.getWaypointScript();
        
        // Create new waypoint
        Waypoint* waypoint = new Waypoint(name, position);
        waypoint->setType(type);
        waypoint->setRadius(radius);
        waypoint->setColor(color);
        waypoint->setScript(script);
        
        map_->addWaypoint(waypoint);
        selectWaypoint(waypoint);
    }
}

void WaypointEditorPanel::onRemoveWaypointClicked() {
    if (!selectedWaypoint_ || !map_) {
        return;
    }
    
    QString waypointName = selectedWaypoint_->name();
    
    int result = QMessageBox::question(this, "Remove Waypoint",
                                      QString("Are you sure you want to remove waypoint '%1'?").arg(waypointName),
                                      QMessageBox::Yes | QMessageBox::No,
                                      QMessageBox::No);
    
    if (result == QMessageBox::Yes) {
        emit waypointDeleteRequested(selectedWaypoint_);
        map_->removeWaypoint(selectedWaypoint_);
    }
}

void WaypointEditorPanel::onEditWaypointClicked() {
    if (!selectedWaypoint_) {
        return;
    }
    
    emit waypointEditRequested(selectedWaypoint_);
    
    // Switch to properties tab for editing
    tabWidget_->setCurrentIndex(1); // Properties tab
}

void WaypointEditorPanel::onDuplicateWaypointClicked() {
    if (!selectedWaypoint_ || !map_) {
        return;
    }
    
    // Generate unique name
    QString baseName = selectedWaypoint_->name() + " Copy";
    QString uniqueName = map_->generateUniqueWaypointName(baseName);
    
    // Create duplicate waypoint
    MapPos newPos = selectedWaypoint_->position();
    newPos.x += 5; // Offset slightly
    newPos.y += 5;
    
    Waypoint* duplicate = new Waypoint(uniqueName, newPos);
    duplicate->setType(selectedWaypoint_->type());
    duplicate->setRadius(selectedWaypoint_->radius());
    duplicate->setColor(selectedWaypoint_->color());
    duplicate->setScript(selectedWaypoint_->script());
    
    map_->addWaypoint(duplicate);
    selectWaypoint(duplicate);
}

void WaypointEditorPanel::onCenterOnWaypointClicked() {
    if (selectedWaypoint_) {
        emit centerOnWaypoint(selectedWaypoint_);
    }
}

void WaypointEditorPanel::onGoToWaypointClicked() {
    if (selectedWaypoint_) {
        emit goToWaypoint(selectedWaypoint_);
    }
}

void WaypointEditorPanel::onRefreshListClicked() {
    refreshWaypointList();
}

// Property editor handlers
void WaypointEditorPanel::onPropertyChanged() {
    if (!selectedWaypoint_ || !validateWaypointData()) {
        return;
    }
    
    applyPropertyChanges();
    applyButton_->setEnabled(false);
}

void WaypointEditorPanel::onNameChanged() {
    if (!updatingUI_ && selectedWaypoint_) {
        applyButton_->setEnabled(true);
    }
}

void WaypointEditorPanel::onTypeChanged() {
    if (!updatingUI_ && selectedWaypoint_) {
        applyButton_->setEnabled(true);
    }
}

void WaypointEditorPanel::onPositionChanged() {
    if (!updatingUI_ && selectedWaypoint_) {
        applyButton_->setEnabled(true);
    }
}

void WaypointEditorPanel::onRadiusChanged(int radius) {
    Q_UNUSED(radius)
    if (!updatingUI_ && selectedWaypoint_) {
        applyButton_->setEnabled(true);
    }
}

void WaypointEditorPanel::onColorChanged() {
    if (!selectedWaypoint_) {
        return;
    }
    
    QColor currentColor = selectedWaypoint_->color();
    if (!currentColor.isValid()) {
        currentColor = Qt::red;
    }
    
    QColor newColor = QColorDialog::getColor(currentColor, this, "Select Waypoint Color");
    if (newColor.isValid()) {
        colorButton_->setStyleSheet(QString("background-color: %1; border: 1px solid gray;").arg(newColor.name()));
        
        if (!updatingUI_) {
            applyButton_->setEnabled(true);
        }
    }
}

void WaypointEditorPanel::onScriptChanged() {
    if (!updatingUI_ && selectedWaypoint_) {
        applyButton_->setEnabled(true);
    }
}

// Search and filter handlers
void WaypointEditorPanel::onSearchTextChanged(const QString& text) {
    searchFilter_ = text;
    applyFilters();
}

void WaypointEditorPanel::onTypeFilterChanged(const QString& type) {
    typeFilter_ = type;
    applyFilters();
}

void WaypointEditorPanel::onClearFiltersClicked() {
    clearFilters();
}

// Helper methods for property changes
void WaypointEditorPanel::applyPropertyChanges() {
    if (!selectedWaypoint_ || !map_) {
        return;
    }
    
    // Store old position for signal
    MapPos oldPosition = selectedWaypoint_->position();
    
    // Apply changes
    QString newName = nameEdit_->text().trimmed();
    if (!newName.isEmpty() && newName != selectedWaypoint_->name()) {
        if (map_->hasWaypoint(newName)) {
            QMessageBox::warning(this, "Invalid Name", 
                                QString("A waypoint named '%1' already exists.").arg(newName));
            return;
        }
        selectedWaypoint_->setName(newName);
    }
    
    selectedWaypoint_->setType(typeCombo_->currentText());
    
    MapPos newPosition(xSpinBox_->value(), ySpinBox_->value(), zSpinBox_->value());
    selectedWaypoint_->setPosition(newPosition);
    
    selectedWaypoint_->setRadius(radiusSpinBox_->value());
    
    // Get color from button style
    QString styleSheet = colorButton_->styleSheet();
    QRegExp colorRegex("background-color:\\s*(#[0-9A-Fa-f]{6})");
    if (colorRegex.indexIn(styleSheet) != -1) {
        QColor color(colorRegex.cap(1));
        selectedWaypoint_->setColor(color);
    }
    
    selectedWaypoint_->setScript(scriptEdit_->toPlainText());
    
    // Emit signals
    if (oldPosition != newPosition) {
        emit map_->waypointMoved(selectedWaypoint_, oldPosition, newPosition);
    }
    emit map_->waypointModified(selectedWaypoint_);
    
    map_->setModified(true);
}

bool WaypointEditorPanel::validateWaypointData() const {
    QString name = nameEdit_->text().trimmed();
    if (name.isEmpty()) {
        QMessageBox::warning(const_cast<WaypointEditorPanel*>(this), "Invalid Data", "Waypoint name cannot be empty.");
        return false;
    }
    
    if (selectedWaypoint_ && name != selectedWaypoint_->name() && map_ && map_->hasWaypoint(name)) {
        QMessageBox::warning(const_cast<WaypointEditorPanel*>(this), "Invalid Data", 
                            QString("A waypoint named '%1' already exists.").arg(name));
        return false;
    }
    
    return true;
}
