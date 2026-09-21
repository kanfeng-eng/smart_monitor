#include "captchagenerator.h"

#include <QColor>
#include <QFont>
#include <QPainter>
#include <QRandomGenerator>
#include <QSize>

CaptchaGenerator::CaptchaGenerator()
{
    regenerate();
}

void CaptchaGenerator::regenerate()
{
    static const QString characters = "23456789ABCDEFGHJKLMNPQRSTUVWXYZ";
    captchaText.clear();
    for (int i = 0; i < 4; ++i)
    {
        const int index = QRandomGenerator::global()->bounded(characters.size());
        captchaText.append(characters.at(index));
    }
}

bool CaptchaGenerator::matches(const QString &input) const
{
    return input.trimmed().compare(captchaText, Qt::CaseInsensitive) == 0;
}

QPixmap CaptchaGenerator::render(const QSize &size) const
{
    QPixmap image(size);
    image.fill(QColor("#17263A"));

    QPainter painter(&image);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);
    QRandomGenerator *random = QRandomGenerator::global();

    for (int i = 0; i < 7; ++i)
    {
        painter.setPen(QPen(QColor(70, 170, 165, 100), 1));
        painter.drawLine(random->bounded(size.width()), random->bounded(size.height()),
                         random->bounded(size.width()), random->bounded(size.height()));
    }
    for (int i = 0; i < 32; ++i)
    {
        painter.setPen(QColor(220, 165, 90, 120));
        painter.drawPoint(random->bounded(size.width()), random->bounded(size.height()));
    }

    QFont font("Consolas");
    font.setBold(true);
    font.setPixelSize(qMax(20, size.height() / 2));
    painter.setFont(font);

    const QColor colors[] = {
        QColor("#FFD486"), QColor("#78DED3"),
        QColor("#F6A9A1"), QColor("#B9C8FF")
    };
    const qreal cellWidth = size.width() / 4.0;
    for (int i = 0; i < captchaText.size(); ++i)
    {
        painter.save();
        painter.translate(cellWidth * i + cellWidth / 2.0, size.height() / 2.0);
        painter.rotate(random->bounded(17) - 8);
        painter.setPen(colors[i]);
        painter.drawText(QRectF(-cellWidth / 2.0, -size.height() / 2.0,
                                cellWidth, size.height()),
                         Qt::AlignCenter, QString(captchaText.at(i)));
        painter.restore();
    }
    return image;
}

QString CaptchaGenerator::text() const
{
    return captchaText;
}
