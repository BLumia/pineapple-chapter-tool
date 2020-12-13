#pragma once

#include <QString>
#include <QMap>
#include <QVariant>

enum ElementProperty {
    P_INVALID,
    P_START_TIME_MS,
    P_END_TIME_MS,
    P_START_TIME_OFFSET,
    P_END_TIME_OFFSET,
    P_CHAP_TITLE,
    P_CHAP_URL,
    P_CHAP_COVER
};

class TOCElement;
class Element
{
public:
    TOCElement * parent() const;
    void setParent(TOCElement * parent);
    virtual bool isTOCElement() = 0;
    virtual int propertyCount() = 0;
    virtual enum ElementProperty propertyType(int nthProperty) = 0;
    virtual QVariant property(enum ElementProperty propertyType) const = 0;
    virtual void setProperty(enum ElementProperty propertyType, QVariant propertyValue) = 0;
private:
    TOCElement * m_parent = nullptr;
};

class TOCElement : public Element
{
public:
    void setSubElementsIds(QStringList ids);
    QString subElementAt(int index);
    int subElementAt(const QString & elementId);
    int subElementsCount() const;
    // Element interface
public:
    bool isTOCElement() override;
    int propertyCount() override;
    ElementProperty propertyType(int nthProperty) override;
    QVariant property(ElementProperty propertyType) const override;
    void setProperty(ElementProperty propertyType, QVariant propertyValue) override;
private:
    QStringList m_subElements = {};
};

class ChapterElement : public Element
{
    // Element interface
public:
    bool isTOCElement() override;
    int propertyCount() override;
    ElementProperty propertyType(int nthProperty) override;
    QVariant property(ElementProperty propertyType) const override;
    void setProperty(enum ElementProperty propertyType, QVariant propertyValue) override;
private:
    static constexpr int PROPERTIES_COUNT = 4;
    enum ElementProperty m_supportedProperties[PROPERTIES_COUNT] = {
        P_START_TIME_MS,
        P_END_TIME_MS,
        P_CHAP_TITLE,
        P_CHAP_URL
    };
    QString m_title;
    QString m_wwwUrl;
    uint64_t m_startTimeMs = 0;
    uint64_t m_endTimeMs = 0;
};

class ChapterTreeManager
{
public:
    ChapterTreeManager();
    ~ChapterTreeManager();

    template <typename Trait>
    Trait * registerElement(QString elementId) {
        Element * el = m_elementsIdMap.value(elementId, nullptr);
        if (el != nullptr) {
            return dynamic_cast<Trait *>(el);
        }

        Trait * trait = new Trait();
        el = trait;
        m_elements.append(el);
        m_elementsIdMap[elementId] = el;
        return trait;
    }

    void setTopLevelElementId(const QString & elementId);
    void * topLevelElement() const;
    TOCElement * topLevelTOCElement() const;
    Element * element(QString elementId) const;
    QString elementId(Element * elementPointer) const;
    int subElementsCount(QString elementId);

private:
    QString m_topLevelElementId;
    QMap<QString, Element *> m_elementsIdMap; // <element id, element>
    QList<Element *> m_elements;
};
