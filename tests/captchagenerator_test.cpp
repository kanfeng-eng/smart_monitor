#include "../src/utils/captchagenerator.h"

#include <QApplication>
#include <QRegularExpression>
#include <QSize>

#include <cassert>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    CaptchaGenerator generator;

    const QRegularExpression allowed("^[23456789ABCDEFGHJKLMNPQRSTUVWXYZ]{4}$");
    assert(allowed.match(generator.text()).hasMatch());
    assert(generator.matches(generator.text().toLower()));
    assert(!generator.matches("WRONG"));
    assert(!generator.render(QSize(108, 38)).isNull());

    const QString original = generator.text();
    bool changed = false;
    for (int i = 0; i < 8 && !changed; ++i)
    {
        generator.regenerate();
        changed = generator.text() != original;
    }
    assert(changed);
    return 0;
}
