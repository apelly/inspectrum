/*
 *  Copyright (C) 2015, Mike Walters <mike@flomp.net>
 *
 *  This file is part of inspectrum.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <QApplication>
#include <QCommandLineParser>
#include <QTranslator>
#include <QLibraryInfo>
#include <QTextStream>
#include <QList>
#include <QFileInfo>

#include "mainwindow.h"

int main(int argc, char *argv[])
{
    //TODO: does the coding policy want this stuff in a header?
    QString     appName = "inspectrum";
    QString     translationDir = "/i18n/";           // Qt takes care of path slashes
    QString     translationPrefix = "lang";
    QString     translationPath;
    QString     systemLocale;
    QString     translationFile;
    QFileInfo   translationFileInfo;
    bool        translationAvailable = false;
    QTranslator translator;

    QApplication a(argc, argv);
    a.setApplicationName(appName);

// there is much debugging code here :)
    QTextStream sout(stderr);

// translation

    // the translation files are stored here
    // TODO: but where will they be when the application is installed?
    translationPath = QApplication::applicationDirPath();
    translationPath.append(translationDir);

sout << "translation path " << translationPath << endl;

    // https://doc.qt.io/qt-5/qtranslator.html#load
    // Usually, it is better to use the QTranslator::load(const QLocale &, const QString &, const QString &, const
    // QString &, const QString &) function instead, because it uses QLocale::uiLanguages() and not simply the locale
    // name, which refers to the formatting of dates and numbers and not necessarily the UI language.
    // firstly try to match the whole language

    // https://doc.qt.io/qt-5/qtranslator.html#load-1

QLocale here = QLocale();
QStringList lang = here.uiLanguages();
sout << "system langs: " << lang.join(",") << endl;

    // install the translator
    if (translator.load(QLocale(), translationPath + translationPrefix, "_", translationPath, ".qm")){
sout << "loaded xlation " << endl;
        a.installTranslator(&translator);
    }

    // widgets must load after translator
    MainWindow  mainWin;

    // parse the command line
    QCommandLineParser parser;
    parser.setApplicationDescription(QCoreApplication::translate("main", "Spectrum viewer"));
    parser.addHelpOption();

    // Add options
    // file selection
    parser.addPositionalArgument(QCoreApplication::translate("main", "file"),
                                 QCoreApplication::translate("main", "File to view."));

    // passed sample rate
    QCommandLineOption rateOption(QStringList()
                                    << QCoreApplication::translate("main", "r")
                                    << QCoreApplication::translate("main", "rate"),
                                    QCoreApplication::translate("main", "Set sample rate."),
                                    "integer, in Hz");
    parser.addOption(rateOption);

    // locale override: useful for testing?
    QCommandLineOption localeOption(QStringList()
                                    << QCoreApplication::translate("main", "l")
                                    << QCoreApplication::translate("main", "locale")
                                    << QCoreApplication::translate("main", "language"),
                                    QCoreApplication::translate("main", "Override system locale."),
                                    "language, e.g. en_GB.");
    parser.addOption(localeOption);

    // Process the actual command line
    parser.process(a);

    const QStringList args = parser.positionalArguments();
    if (args.size()>=1)
        mainWin.openFile(args.at(0));

    if (parser.isSet(rateOption)) {
        bool ok;
        // Use toDouble just for scientific notation support
        int rate = parser.value(rateOption).toDouble(&ok);
        if(!ok) {
            QTextStream out(stderr);
            out << QCoreApplication::translate("main", "ERROR: could not parse rate") << endl;
            return 1;
        }
        mainWin.setSampleRate(rate);
    }

    if (parser.isSet(localeOption)) {
        // https://wiki.qt.io/How_to_create_a_multi_language_application
        // TODO: haven't tested this at all. i suspect the translator should be replaced on mainwin too
        a.removeTranslator(&translator);
        QString locale = parser.value(localeOption);
        translationFile = translationPath + "lang_" + locale + ".qm";
        if(translator.load(translationFile))
            a.installTranslator(&translator);
sout << "trying override locale: " << translationFile << endl;

    }
    mainWin.show();
    return a.exec();
}
