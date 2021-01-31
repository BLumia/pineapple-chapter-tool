#ifndef TAGLIBUTILS_H
#define TAGLIBUTILS_H

#include <string>

class ChapterItem;

namespace TagLib{
namespace Ogg{
class XiphComment;
}
}

class TagLibUtils
{
public:
    enum OgmKeyType {
        OgmChapterTime,
        OgmChapterName,
        OgmChapterUrl
    };

    static std::string ogmChapterKey(int chapterNumber, OgmKeyType type = OgmChapterTime);
    static std::string ogmTimeStr(int ms);
    static ChapterItem * loadFromXiphComment(TagLib::Ogg::XiphComment * tags);
    static bool saveToXiphComment(ChapterItem * rootItem, TagLib::Ogg::XiphComment * xiphComment);
};

#endif // TAGLIBUTILS_H
