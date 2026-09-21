#ifndef CAPTCHAGENERATOR_H
#define CAPTCHAGENERATOR_H

#include <QPixmap>
#include <QString>

class QSize;

class CaptchaGenerator
{
public:
    CaptchaGenerator();

    void regenerate();
    bool matches(const QString &input) const;
    QPixmap render(const QSize &size) const;
    QString text() const;

private:
    QString captchaText;
};

#endif
