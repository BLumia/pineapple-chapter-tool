#include "cuefilehandler.h"

#include "chapteritem.h"

#include <QFile>
#include <QSaveFile>
#include <QTextStream>
#include <QTime>

CueFileHandler::CueFileHandler()
{

}

FileHandlerInterface::Status CueFileHandler::setFile(const QString filePath)
{
    m_file = filePath;

    return SUCCESS;
}

// https://en.wikipedia.org/wiki/Cue_sheet_(computing)#Cue_sheet_syntax
ChapterItem *CueFileHandler::createChapterTree() const
{
    QFile cueFile(m_file);
    cueFile.open(QIODevice::ReadOnly | QIODevice::Text);

    QMap<int, ChapterItem *> chapterMap; // track should in order, just in case.
    int currentTrackNum = -1;

    QTextStream in(&cueFile);
    while (!in.atEnd()) {
        QString newLine(in.readLine().trimmed());
        if (newLine.isEmpty()) continue;
        QStringList parts(newLine.split(' '));
        if (parts.count() < 2) continue;

        // TODO: REM maybe contain metadata, may need to parse and save.
        // eg: REM GENRE Musical
        //     REM DATE 1994
        //     REM DISCID F40C490F
        // these example from cue file exported with `yatoc2cue`
        if (parts[0] == QStringLiteral("REM")) continue;
        // TRACK 01 AUDIO
        if (parts[0] == QStringLiteral("TRACK") && parts.count() == 3) {
            ChapterItem * chapterItem = new ChapterItem(parts[1]);
            chapterItem->setColumnCount(3);
            currentTrackNum = parts[1].toInt();
            chapterMap[currentTrackNum] = chapterItem;
            continue;
        }
        // TITLE "Track01"
        if (parts[0] == QStringLiteral("TITLE")) {
            if (currentTrackNum < 0) {
                // it's the title of the file, not the current track/chapter title
                continue;
            }
            // just in case
            if (chapterMap.contains(currentTrackNum)) {
                // TODO: unescape, remove doube-quote in a halal way.
                QString unquote(newLine.mid(7));
                unquote = unquote.left(unquote.lastIndexOf('"'));
                chapterMap[currentTrackNum]->setItemProperty(ChapterTitle, unquote);
            }
            continue;
        }
        // INDEX 01 00:00:32
        // INDEX 00 is optional and denotes the pregap, so we can ignore it.
        // or maybe use 00 as end time of previous chapter, then it's a TODO.
        if (parts[0] == QStringLiteral("INDEX") && parts[1] == QStringLiteral("01") && parts.count() == 3) {
            QStringList msf(parts[2].split(':'));
            if (msf.count() != 3) continue;
            QTime trackTime(0, 0);
            trackTime = trackTime.addMSecs(msf[2].toInt() * (1000.0 / 75));
            trackTime = trackTime.addSecs(msf[1].toInt());
            trackTime = trackTime.addSecs(msf[0].toInt() * 60);
            // just in case
            if (chapterMap.contains(currentTrackNum)) {
                chapterMap[currentTrackNum]->setItemProperty(ChapterStartTimeMs, trackTime.msecsSinceStartOfDay());
            }
            continue;
        }
    }

    cueFile.close();

    ChapterItem * tocItem = new ChapterItem("pseudoTOC");
    tocItem->setColumnCount(3);

    for (int i = 1; i <= currentTrackNum; i++) {
        if (chapterMap.contains(i)) {
            tocItem->appendRow(chapterMap[i]);
        }
    }

    return tocItem;
}

FileHandlerInterface::ChapterFeatures CueFileHandler::chapterFeatures() const
{
    return ChapterFeatures(StartTimeMs | Title);
}

FileHandlerInterface::Status CueFileHandler::importFromFile()
{
    return SUCCESS;
}

QString cueTimeStr(int ms)
{
    QTime startTime(QTime::fromMSecsSinceStartOfDay(ms));
    // FIXME: mm cannot larger than 59, could be a bug when the time is very long.
    int minOnly = startTime.hour() * 60 + startTime.minute();
    int secOnly = startTime.second();
    int frameOnly = startTime.toString("zzz").toInt() * 75 / 1000.0;
    char timeCStr[11]; // "mmmm:ss:ff"
    snprintf(timeCStr, 11, "%02d:%02d:%02d", minOnly, secOnly, frameOnly);
    return QString(timeCStr);
}

FileHandlerInterface::Status CueFileHandler::writeToFile(ChapterItem *chapterRoot)
{
    QSaveFile sf(m_file);
    sf.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text);
    sf.fileName();

    QTextStream out(&sf);
    out.setCodec("UTF-8");

    // FIXME: some line have quote s

    QString titleLine(R"(TITLE "%1")");
    out << titleLine.arg(sf.fileName()) << Qt::endl;
    out << "FILE \"" << sf.fileName() << "\" WAVE" << Qt::endl;

    int currentTrackNumber = 1; // yeah it starts from 1.
    ChapterItem::forEach(chapterRoot, [&](const ChapterItem * currentItem) {
        if (currentItem->hasChildren()) return;

        out << "  TRACK " << currentTrackNumber << " AUDIO" << Qt::endl;
        out << "    TITLE \"" << currentItem->data(ChapterTitle).toString() << "\"" << Qt::endl;
        out << "    INDEX 01 " << cueTimeStr(currentItem->data(ChapterStartTimeMs).toInt()) << Qt::endl;

        currentTrackNumber++;
    });

    out.flush();

    sf.commit();

    return SUCCESS;
}
