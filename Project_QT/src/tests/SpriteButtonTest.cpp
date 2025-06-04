#include "SpriteButton.h"
#include <QApplication>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QPixmap>
#include <QPainter>
#include <QDebug>

// Simple test application to verify SpriteButton functionality
class SpriteButtonTestWidget : public QWidget {
    Q_OBJECT

public:
    SpriteButtonTestWidget(QWidget* parent = nullptr) : QWidget(parent) {
        setupUI();
        createTestButtons();
        connectSignals();
    }

private slots:
    void onNormalButtonClicked() {
        qDebug() << "Normal SpriteButton clicked!";
        statusLabel_->setText("Normal button clicked");
    }

    void onToggleButtonClicked() {
        qDebug() << "Toggle SpriteButton clicked! State:" << toggleButton_->getValue();
        statusLabel_->setText(QString("Toggle button clicked - State: %1")
                             .arg(toggleButton_->getValue() ? "ON" : "OFF"));
    }

    void onPixmapButtonClicked() {
        qDebug() << "Pixmap SpriteButton clicked!";
        statusLabel_->setText("Pixmap button clicked");
    }

private:
    void setupUI() {
        setWindowTitle("SpriteButton Test");
        setFixedSize(400, 300);
        
        QVBoxLayout* mainLayout = new QVBoxLayout(this);
        
        // Title
        QLabel* titleLabel = new QLabel("SpriteButton Test Application");
        titleLabel->setStyleSheet("font-weight: bold; font-size: 14px; margin: 10px;");
        titleLabel->setAlignment(Qt::AlignCenter);
        mainLayout->addWidget(titleLabel);
        
        // Button container
        QHBoxLayout* buttonLayout = new QHBoxLayout();
        buttonContainer_ = new QWidget();
        buttonContainer_->setLayout(buttonLayout);
        mainLayout->addWidget(buttonContainer_);
        
        // Status label
        statusLabel_ = new QLabel("Click buttons to test functionality");
        statusLabel_->setStyleSheet("border: 1px solid gray; padding: 5px; margin: 10px;");
        statusLabel_->setAlignment(Qt::AlignCenter);
        mainLayout->addWidget(statusLabel_);
        
        // Test controls
        QHBoxLayout* controlLayout = new QHBoxLayout();
        
        QPushButton* toggleStateBtn = new QPushButton("Toggle State");
        connect(toggleStateBtn, &QPushButton::clicked, [this]() {
            if (toggleButton_) {
                toggleButton_->setValue(!toggleButton_->getValue());
                statusLabel_->setText(QString("Toggle state set to: %1")
                                     .arg(toggleButton_->getValue() ? "ON" : "OFF"));
            }
        });
        controlLayout->addWidget(toggleStateBtn);
        
        QPushButton* exitBtn = new QPushButton("Exit");
        connect(exitBtn, &QPushButton::clicked, this, &QWidget::close);
        controlLayout->addWidget(exitBtn);
        
        mainLayout->addLayout(controlLayout);
    }
    
    void createTestButtons() {
        QHBoxLayout* layout = qobject_cast<QHBoxLayout*>(buttonContainer_->layout());
        
        // Create test pixmap
        QPixmap testPixmap = createTestPixmap(16, 16, Qt::blue);
        QPixmap testPixmap32 = createTestPixmap(32, 32, Qt::green);
        
        // Normal button with 16x16 size
        normalButton_ = new SpriteButton(buttonContainer_, SPRITE_BTN_NORMAL, SPRITE_SIZE_16x16, 123);
        normalButton_->setPixmap(testPixmap);
        layout->addWidget(normalButton_);
        
        // Toggle button with 32x32 size
        toggleButton_ = new SpriteButton(buttonContainer_, SPRITE_BTN_TOGGLE, SPRITE_SIZE_32x32, 456);
        toggleButton_->setPixmap(testPixmap32);
        layout->addWidget(toggleButton_);
        
        // Pixmap button (default size)
        QPixmap redPixmap = createTestPixmap(16, 16, Qt::red);
        pixmapButton_ = new SpriteButton(redPixmap, buttonContainer_);
        layout->addWidget(pixmapButton_);
        
        layout->addStretch(); // Add stretch to center buttons
    }
    
    void connectSignals() {
        connect(normalButton_, &SpriteButton::clicked, this, &SpriteButtonTestWidget::onNormalButtonClicked);
        connect(toggleButton_, &SpriteButton::clicked, this, &SpriteButtonTestWidget::onToggleButtonClicked);
        connect(pixmapButton_, &SpriteButton::clicked, this, &SpriteButtonTestWidget::onPixmapButtonClicked);
    }
    
    QPixmap createTestPixmap(int width, int height, const QColor& color) {
        QPixmap pixmap(width, height);
        pixmap.fill(Qt::transparent);
        
        QPainter painter(&pixmap);
        painter.setRenderHint(QPainter::Antialiasing, false);
        
        // Draw a simple test pattern
        painter.fillRect(0, 0, width, height, color);
        painter.setPen(Qt::white);
        painter.drawRect(0, 0, width-1, height-1);
        
        // Draw a cross pattern
        painter.drawLine(0, 0, width-1, height-1);
        painter.drawLine(0, height-1, width-1, 0);
        
        return pixmap;
    }
    
    QWidget* buttonContainer_;
    QLabel* statusLabel_;
    SpriteButton* normalButton_;
    SpriteButton* toggleButton_;
    SpriteButton* pixmapButton_;
};

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    
    SpriteButtonTestWidget testWidget;
    testWidget.show();
    
    return app.exec();
}

#include "SpriteButtonTest.moc"
