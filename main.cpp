#include "flasher.h"

#include <QApplication>
#include <QTranslator>
#include <QLocale>
#include <QErrorMessage>

int main(int argc, char *argv[])
{
    QApplication application(argc, argv);

    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages) {
        const QString baseName = "Flasher_" + QLocale(locale).name();
        if (translator.load(":/i18n/" + baseName)) {
            application.installTranslator(&translator);
            break;
        }
    }

    QErrorMessage::qtHandler();

    Flasher flasher;
    flasher.show();
    return application.exec();
}
