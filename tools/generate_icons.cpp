#include <QApplication>
#include <QPainter>
#include <QIcon>
#include <QDir>

void createIcon(const QString &name, const QColor &color, const QString &text = QString()) {
    QPixmap pixmap(32, 32);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);

    // Draw icon based on name
    if (name == "new") {
        painter.setPen(QPen(color, 2));
        painter.drawRect(8, 8, 16, 16);
        painter.drawLine(12, 16, 20, 16);
    } else if (name == "open") {
        painter.setPen(QPen(color, 2));
        painter.drawRect(6, 12, 20, 12);
        painter.drawLine(10, 12, 10, 8);
        painter.drawLine(10, 8, 22, 8);
        painter.drawLine(22, 8, 22, 12);
    } else if (name == "save") {
        painter.setPen(QPen(color, 2));
        painter.drawRect(6, 6, 20, 20);
        painter.drawRect(10, 6, 12, 8);
    } else if (name == "save-as") {
        painter.setPen(QPen(color, 2));
        painter.drawRect(4, 4, 16, 16);
        painter.drawRect(8, 4, 8, 6);
        painter.drawRect(12, 12, 16, 16);
    } else if (name == "cut") {
        painter.setPen(QPen(color, 2));
        painter.drawLine(8, 8, 24, 24);
        painter.drawEllipse(6, 18, 8, 8);
        painter.drawEllipse(18, 6, 8, 8);
    } else if (name == "copy") {
        painter.setPen(QPen(color, 2));
        painter.drawRect(8, 8, 12, 16);
        painter.drawRect(12, 4, 12, 16);
    } else if (name == "paste") {
        painter.setPen(QPen(color, 2));
        painter.drawRect(8, 12, 16, 12);
        painter.drawRect(12, 8, 8, 4);
    } else if (name == "undo") {
        painter.setPen(QPen(color, 2));
        painter.drawArc(8, 8, 16, 16, 0, -270 * 16);
        painter.drawLine(8, 16, 12, 12);
        painter.drawLine(8, 16, 12, 20);
    } else if (name == "redo") {
        painter.setPen(QPen(color, 2));
        painter.drawArc(8, 8, 16, 16, 0, 270 * 16);
        painter.drawLine(24, 16, 20, 12);
        painter.drawLine(24, 16, 20, 20);
    } else if (name == "find") {
        painter.setPen(QPen(color, 2));
        painter.drawEllipse(8, 8, 12, 12);
        painter.drawLine(18, 18, 24, 24);
    } else if (name == "settings") {
        painter.setPen(QPen(color, 2));
        painter.drawEllipse(8, 8, 16, 16);
        for (int i = 0; i < 8; ++i) {
            qreal angle = i * M_PI / 4;
            painter.drawLine(
                16 + cos(angle) * 12,
                16 + sin(angle) * 12,
                16 + cos(angle) * 14,
                16 + sin(angle) * 14
            );
        }
    }

    // Draw text if provided
    if (!text.isEmpty()) {
        QFont font;
        font.setPixelSize(10);
        painter.setFont(font);
        painter.drawText(pixmap.rect(), Qt::AlignCenter, text);
    }

    // Save the icon
    QDir().mkpath("resources/icons");
    pixmap.save("resources/icons/" + name + ".png");
}

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    // Create light theme icons
    QColor lightColor(0x2d, 0x2d, 0x2d);
    createIcon("new", lightColor);
    createIcon("open", lightColor);
    createIcon("save", lightColor);
    createIcon("save-as", lightColor);
    createIcon("cut", lightColor);
    createIcon("copy", lightColor);
    createIcon("paste", lightColor);
    createIcon("undo", lightColor);
    createIcon("redo", lightColor);
    createIcon("find", lightColor);
    createIcon("settings", lightColor);

    return 0;
} 