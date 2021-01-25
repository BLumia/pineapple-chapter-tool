#ifndef TAGLIBUTILS_H
#define TAGLIBUTILS_H

class ChapterItem;

namespace TagLib{
namespace Ogg{
class XiphComment;
}
}

class TagLibUtils
{
public:
    static ChapterItem * loadFromXiphComment(TagLib::Ogg::XiphComment * tags);
    static bool saveToXiphComment(ChapterItem * rootItem, TagLib::Ogg::XiphComment * xiphComment);
};

#endif // TAGLIBUTILS_H
