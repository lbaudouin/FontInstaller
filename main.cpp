#include <QApplication>

#include "mainwindow.h"
#include <QTranslator>
#include <QLibraryInfo>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QString lang = QLocale::system().name().section('_', 0, 0);
    lang.replace("EN","en");
    lang.replace("FR","fr");

    QTranslator *translator = new QTranslator();
    translator->load(QString(":/lang/lang_") + lang);
    qApp->installTranslator( translator );

    QTranslator *translatorQt = new QTranslator();
    translatorQt->load(QString("qt_") + lang, QLibraryInfo::location(QLibraryInfo::TranslationsPath));
    qApp->installTranslator( translatorQt );

    MainWindow window;
    window.show();

    return app.exec();
}
