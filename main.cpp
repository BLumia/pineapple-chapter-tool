/*
    SPDX-FileCopyrightText: 2021 Gary Wang <wzc782970009@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only
*/

#include "mainwindow.h"

#include <QApplication>
#include <QCommandLineParser>
#include <QUrl>

QList<QUrl> convertToUrlList(const QStringList &files)
{
    QList<QUrl> urlList;
    for (const QString & str : qAsConst(files)) {
        QUrl url = QUrl::fromLocalFile(str);
        if (url.isValid()) {
            urlList.append(url);
        }
    }

    return urlList;
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setApplicationName(QStringLiteral("Pineapple Chapter Tool"));
    a.setApplicationDisplayName(QCoreApplication::translate("main", "Pineapple Chapter Tool"));

    // parse commandline arguments
    QCommandLineParser parser;
    parser.addPositionalArgument("File list", QCoreApplication::translate("main", "File list."));
    parser.addHelpOption();

    parser.process(a);

    QStringList urlStrList = parser.positionalArguments();
    QList<QUrl> && urlList = convertToUrlList(urlStrList);

    MainWindow w;
    w.show();

    if (!urlList.isEmpty()) {
        w.loadFile(urlList.first());
    }

    return a.exec();
}
