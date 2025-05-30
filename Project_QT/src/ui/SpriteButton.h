#ifndef QT_SPRITEBUTTON_H
#define QT_SPRITEBUTTON_H

#include <QPushButton>
#include <QPixmap>
#include <QIcon> // Might be useful for different states

// Forward declarations
class QPaintEvent;
class QPainter;

class SpriteButton : public QPushButton {
    Q_OBJECT

public:
    explicit SpriteButton(QWidget *parent = nullptr);
    explicit SpriteButton(const QPixmap& pixmap, QWidget *parent = nullptr);
    explicit SpriteButton(const QIcon& icon, const QString& text, QWidget *parent = nullptr); // Standard QPushButton constructor
    ~SpriteButton() override;

    void setPixmap(const QPixmap& pixmap);
    QPixmap pixmap() const;

    // To allow different pixmaps for different states, like QToolButton or custom logic
    // void setPixmapForState(const QPixmap& pixmap, QIcon::Mode mode, QIcon::State state = QIcon::Off);

    QSize sizeHint() const override;

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QPixmap m_currentPixmap; // The pixmap to display
    // QMap<QPair<QIcon::Mode, QIcon::State>, QPixmap> m_statePixmaps; // For more complex state handling

    void init(); // Common initialization
};

#endif // QT_SPRITEBUTTON_H
