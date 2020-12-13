#include "chaptertreemanager.h"

TOCElement *Element::parent() const
{
    return m_parent;
}

void Element::setParent(TOCElement *parent)
{
    m_parent = parent;
}

void TOCElement::setSubElementsIds(QStringList ids)
{
    m_subElements = ids;
}

QString TOCElement::subElementAt(int index)
{
    return m_subElements.value(index);
}

int TOCElement::subElementAt(const QString &elementId)
{
    return m_subElements.indexOf(elementId);
}

int TOCElement::subElementsCount() const
{
    return m_subElements.count();
}

bool ChapterElement::isTOCElement()
{
    return false;
}

int ChapterElement::propertyCount()
{
    return PROPERTIES_COUNT;
}

// start at 0, < count.
ElementProperty ChapterElement::propertyType(int nthProperty)
{
    return m_supportedProperties[nthProperty];
}

QVariant ChapterElement::property(ElementProperty propertyType) const
{
    switch (propertyType) {
    case P_START_TIME_MS:
        return m_startTimeMs;
    case P_END_TIME_MS:
        return m_endTimeMs;
    case P_CHAP_TITLE:
        return m_title;
    case P_CHAP_URL:
        return m_wwwUrl;
    default:
        return QVariant();
    }
}

void ChapterElement::setProperty(enum ElementProperty propertyType, QVariant propertyValue)
{
    switch (propertyType) {
    case P_START_TIME_MS:
        m_startTimeMs = propertyValue.toUInt();
        break;
    case P_END_TIME_MS:
        m_endTimeMs = propertyValue.toUInt();
        break;
    case P_CHAP_TITLE:
        m_title = propertyValue.toString();
        break;
    case P_CHAP_URL:
        m_wwwUrl = propertyValue.toString();
        break;
    default:
        break;
    }

    return;
}

bool TOCElement::isTOCElement()
{
    return true;
}

int TOCElement::propertyCount()
{
    return 0;
}

ElementProperty TOCElement::propertyType(int nthProperty)
{
    Q_UNUSED(nthProperty);
    return P_INVALID;
}

QVariant TOCElement::property(ElementProperty propertyType) const
{
    Q_UNUSED(propertyType);
    return QVariant();
}

void TOCElement::setProperty(ElementProperty propertyType, QVariant propertyValue)
{
    Q_UNUSED(propertyType);
    Q_UNUSED(propertyValue);
    return;
}

ChapterTreeManager::ChapterTreeManager()
{

}

ChapterTreeManager::~ChapterTreeManager()
{
    qDeleteAll(m_elements);
}

void ChapterTreeManager::setTopLevelElementId(const QString &elementId)
{
    m_topLevelElementId = elementId;
}

void * ChapterTreeManager::topLevelElement() const
{
    return m_elementsIdMap.value(m_topLevelElementId, nullptr);
}

TOCElement *ChapterTreeManager::topLevelTOCElement() const
{
    return reinterpret_cast<TOCElement *>(topLevelElement());
}

//template <typename Trait>
//Trait *ChapterTreeManager::registerElement(QString elementId)
//{
//    Element * el = m_elementsIdMap.value(elementId, nullptr);
//    if (el != nullptr) {
//        return dynamic_cast<Trait *>(el);
//    }
//
//    Trait * trait = new Trait();
//    el = trait;
//    m_elements.append(el);
//    m_elementsIdMap[elementId] = el;
//    return trait;
//}

Element *ChapterTreeManager::element(QString elementId) const
{
    return m_elementsIdMap.value(elementId, nullptr);
}

QString ChapterTreeManager::elementId(Element *elementPointer) const
{
    QMap<QString, Element *>::const_iterator i;
    for (i = m_elementsIdMap.begin(); i != m_elementsIdMap.end(); ++i) {
        if (i.value() == elementPointer) return i.key();
    }

    return QString();
}

int ChapterTreeManager::subElementsCount(QString elementId)
{
    Element * el = element(elementId);
    if (!el) return 0;
    if (!el->isTOCElement()) return 0;

    TOCElement * tocEl = dynamic_cast<TOCElement *>(el);
    return tocEl->subElementsCount();
}
