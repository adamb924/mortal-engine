#include "allomorph.h"

#include "form.h"
#include "tag.h"
#include "parsing.h"
#include "portmanteau.h"
#include "debug.h"
#include "hashseed.h"

#include "constraints/abstractconstraint.h"

#include <QXmlStreamWriter>
#include <QtDebug>
#include <QRegularExpression>
#include <QDomElement>

using namespace ME;

QString Allomorph::XML_ALLOMORPH = "allomorph";
QString Allomorph::XML_FORM = "form";
QString Allomorph::XML_TAG = "tag";
QString Allomorph::XML_STEM = "stem";
QString Allomorph::XML_USE_IN_GENERATIONS = "use-in-generations";
QString Allomorph::XML_TRUE = "true";
QString Allomorph::XML_PORTMANTEAU = "portmanteau";

Allomorph::Allomorph(Allomorph::Type type) : mType(type), mId(-1), mUseInGenerations(true)
{
    calculateHash();
}

Allomorph::Allomorph(const Form &f, Type type) : mType(type), mId(-1), mUseInGenerations(true)
{
    mForms.insert( f.writingSystem(), f );
    calculateHash();
}

Allomorph::Allomorph(const Allomorph &other) :
    mForms(other.mForms),
    mConstraints(other.mConstraints),
    mTags(other.mTags),
    mType(other.mType),
    mPortmanteau(other.mPortmanteau),
    mId(other.mId),
    mHash(other.mHash),
    mUseInGenerations(other.mUseInGenerations)
{
}

Allomorph &Allomorph::operator=(const Allomorph & other)
{
    mForms = other.mForms;
    mConstraints = other.mConstraints;
    mTags = other.mTags;
    mType = other.mType;
    mPortmanteau = other.mPortmanteau;
    mId = other.mId;
    mHash = other.mHash;
    mUseInGenerations = other.mUseInGenerations;
    return *this;
}

bool Allomorph::operator==(const Allomorph &other) const
{
    return mForms == other.mForms
            && mTags == other.mTags
            && mConstraints == other.mConstraints
            && mType == other.mType;
}

void Allomorph::setForm(const Form &f)
{
    mForms.insert( f.writingSystem(), f );
    calculateHash();
}

void Allomorph::addConstraint(const AbstractConstraint *constraint)
{
    /// convenience
    if( constraint != nullptr )
        mConstraints.insert(constraint);
    calculateHash();
}

void Allomorph::addConstraints(const QSet<const AbstractConstraint *> &constraints)
{
    mConstraints.unite(constraints);
    calculateHash();
}

void Allomorph::addTag(const QString &tag)
{
    mTags.insert( Tag(tag) );
    calculateHash();
}

void Allomorph::addTags(const QSet<Tag> tags)
{
    mTags.unite(tags);
    calculateHash();
}

void Allomorph::removeTags(const QSet<Tag> &tags)
{
    mTags.subtract(tags);
    calculateHash();
}

void Allomorph::setTags(const QSet<Tag> tags)
{
    mTags = tags;
    calculateHash();
}

QSet<Tag> Allomorph::tags() const
{
    return mTags;
}

QHash<WritingSystem, Form> Allomorph::forms() const
{
    return mForms;
}

QSet<WritingSystem> Allomorph::writingSystems() const
{
    QList<WritingSystem> list = mForms.keys();
    return QSet<WritingSystem>(list.begin(), list.end());
}

Form Allomorph::form(const WritingSystem &ws, bool *ok) const
{
    if( ok != nullptr )
    {
        *ok = mForms.contains(ws);
    }
    return mForms.value( ws, Form(ws, "") );
}

void Allomorph::clearForms()
{
    mForms.clear();
    calculateHash();
}

bool Allomorph::hasForm(const WritingSystem &ws) const
{
    return mForms.contains(ws);
}

bool Allomorph::hasForm(const Form &form) const
{
    return mForms.value( form.writingSystem() ) == form;
}

QSet<const AbstractConstraint *> Allomorph::matchConditions() const
{
    QSet<const AbstractConstraint *> set;
    foreach( const AbstractConstraint * c, mConstraints )
    {
        if( c->isMatchCondition() )
        {
            set << c;
        }
    }
    return set;
}

QSet<const AbstractConstraint *> Allomorph::localConstraints() const
{
    QSet<const AbstractConstraint *> set;
    foreach( const AbstractConstraint * c, mConstraints )
    {
        if( c->isLocalConstraint() )
        {
            set << c;
        }
    }
    return set;
}

QSet<const AbstractConstraint *> Allomorph::longDistanceConstraints() const
{
    QSet<const AbstractConstraint *> set;
    foreach( const AbstractConstraint * c, mConstraints )
    {
        if( c->isLongDistanceConstraint() )
        {
            set << c;
        }
    }
    return set;
}

void Allomorph::serialize(QXmlStreamWriter &out) const
{
    out.writeStartElement("allomorph");
    out.writeAttribute("type", typeToString(mType));
    if( mPortmanteau.isValid() )
    {
        out.writeAttribute(XML_PORTMANTEAU, mPortmanteau.morphemes().toString() );
    }
    if( mId != -1 )
    {
        out.writeAttribute("id", QString("%1").arg(mId) );
    }
    QHashIterator<WritingSystem,Form> fi(mForms);
    while( fi.hasNext() )
    {
        fi.next();
        fi.value().serialize(out);
    }
    QSetIterator<Tag> ti(mTags);
    while( ti.hasNext() )
    {
        ti.next().serialize(out);
    }
    out.writeEndElement(); /// allomorph
}

void Allomorph::serialize(QDomElement &out) const
{
    out.setAttribute("type", typeToString(mType));
    if( mId != -1 )
    {
        out.setAttribute("id", mId );
    }
    if( mPortmanteau.isValid() )
    {
        out.setAttribute(XML_PORTMANTEAU, mPortmanteau.morphemes().toString());
    }
    QHashIterator<WritingSystem,Form> fi(mForms);
    while( fi.hasNext() )
    {
        fi.next();
        QDomElement fEl = out.ownerDocument().createElement("form");
        fi.value().serialize(fEl);
        out.appendChild(fEl);
    }
    QSetIterator<Tag> ti(mTags);
    while( ti.hasNext() )
    {
        QDomElement tEl = out.ownerDocument().createElement("tag");
        ti.next().serialize(tEl);
        out.appendChild(tEl);
    }
}

Allomorph Allomorph::readFromXml(QXmlStreamReader &in, const Morphology *morphology, const QString &elementName)
{
    Q_ASSERT( in.isStartElement() && in.name() == elementName );

    Allomorph::Type type = typeFromString( in.attributes().value("type").toString() );

    Allomorph allomorph(type);

    if( in.attributes().hasAttribute(XML_USE_IN_GENERATIONS) )
    {
        allomorph.setUseInGenerations( in.attributes().value(XML_USE_IN_GENERATIONS) == XML_TRUE );
    }
    if( in.attributes().hasAttribute(XML_PORTMANTEAU) )
    {
        allomorph.setPortmanteau( Portmanteau( in.attributes().value(XML_PORTMANTEAU).toString() ) );
    }

    while(!in.atEnd() && !(in.tokenType() == QXmlStreamReader::EndElement && in.name() == elementName ) )
    {
        in.readNext();

        if( in.tokenType() == QXmlStreamReader::StartElement )
        {
            if( in.name() == XML_FORM )
            {
                allomorph.setForm( Form::readFromXml(in, morphology) );
            }
            else if( in.name() == XML_TAG )
            {
                allomorph.addTag( in.readElementText() );
            }
            else
            {
                qWarning() << "Unexpected tag when reading Allomorph" << in.name() << "on line" << in.lineNumber();
            }
        }
    }

    Q_ASSERT( in.isEndElement() && in.name() == elementName );
    return allomorph;
}

Allomorph Allomorph::readFromXml(QDomElement element, const Morphology *morphology)
{
    Allomorph::Type type = typeFromString( element.attribute("type") );

    Allomorph allomorph(type);

    if( element.hasAttribute(XML_USE_IN_GENERATIONS) )
    {
        allomorph.setUseInGenerations( element.attribute(XML_USE_IN_GENERATIONS) == XML_TRUE );
    }

    QDomNodeList forms = element.elementsByTagName("form");
    for(int i=0; i < forms.count(); i++)
    {
        allomorph.setForm( Form::readFromXml(forms.at(i).toElement(), morphology) );
    }

    QDomNodeList tags = element.elementsByTagName("tag");
    for(int i=0; i < tags.count(); i++)
    {
        allomorph.addTag( tags.at(i).toElement().text() );
    }

    return allomorph;
}

bool Allomorph::isHypothetical() const
{
    return mType == Allomorph::Hypothetical;
}

bool Allomorph::isOriginal() const
{
    return mType == Allomorph::Original;
}

QSet<const AbstractConstraint *> Allomorph::constraints() const
{
    return mConstraints;
}

bool Allomorph::isEmpty() const
{
    return mForms.count() == 0;
}

Allomorph::Type Allomorph::type() const
{
    return mType;
}

void Allomorph::setType(const Allomorph::Type &type)
{
    mType = type;
    calculateHash();
}

QString Allomorph::typeToString(Allomorph::Type type)
{
    switch( type )
    {
    case Allomorph::Original:
        return "Original";
    case Allomorph::Derived:
        return "Derived";
    case Allomorph::Hypothetical:
        return "Hypothetical";
    case Allomorph::Null:
        return "Null";
    }
    return "Null";
}

Allomorph::Type Allomorph::typeFromString(const QString &type)
{
    if( type == "Original" )
    {
        return Allomorph::Original;
    }
    else if( type == "Derived" )
    {
        return Allomorph::Derived;
    }
    else if( type == "Hypothetical" )
    {
        return Allomorph::Hypothetical;
    }
    else
    {
        return Allomorph::Null;
    }
}

Portmanteau Allomorph::portmanteau() const
{
    return mPortmanteau;
}

Portmanteau &Allomorph::portmanteau()
{
    return mPortmanteau;
}

bool Allomorph::hasPortmanteau(const WritingSystem & ws) const
{
    return mPortmanteau.isValid() && hasForm( ws );
}

void Allomorph::setPortmanteau(const Portmanteau &portmanteau)
{
    /// 2021: TODO question: should the portmanteau be included in the hash?
    mPortmanteau = portmanteau;
}

bool Allomorph::hasForm(const QSet<Allomorph> *allomorphs, const Form &f)
{
    QSetIterator<Allomorph> i(*allomorphs);
    while( i.hasNext() )
    {
        if( i.next().hasForm(f) )
        {
            return true;
        }
    }
    return false;
}

qlonglong Allomorph::id() const
{
    return mId;
}

void Allomorph::setId(const qlonglong &id)
{
    mId = id;
    calculateHash();
}

bool Allomorph::hasZeroLengthForms() const
{
    QHashIterator<WritingSystem, Form> i(mForms);
    while(i.hasNext())
    {
        i.next();
        if( i.value().length() == 0 )
        {
            return true;
        }
    }
    return false;
}

uint Allomorph::hash() const
{
    return mHash;
}

void Allomorph::calculateHash()
{
    mHash = mType ^ qHash( mForms, HASH_SEED ) ^ qHash( mConstraints, HASH_SEED ) ^ qHash( mTags, HASH_SEED );
}

bool Allomorph::useInGenerations() const
{
    return mUseInGenerations;
}

void Allomorph::setUseInGenerations(bool useInGenerations)
{
    mUseInGenerations = useInGenerations;
}

QString Allomorph::summary() const
{
    QString dbgString;
    Debug dbg(&dbgString);
    
    dbg << "Allomorph(ID: " << mId << ", "  << typeToString( mType ) << ", " << mForms.count() << " form(s), Use in generations: " << (mUseInGenerations ? "true" : "false") << newline;
    dbg.indent();
    if( mPortmanteau.isValid() )
    {
        dbg << " " << portmanteau().summary() << "," << newline;
    }
    QHashIterator<WritingSystem, Form> i(mForms);
    while(i.hasNext())
    {
        i.next();
        dbg << "(" << i.key().abbreviation() << ", " << i.value().text() << ")" << newline;
    }
    if( ! mTags.isEmpty() )
    {
        dbg << "Tags: ";
        QSetIterator<Tag> ti(mTags);
        while( ti.hasNext() )
        {
            dbg << ti.next().summary();
            if( ti.hasNext() )
            {
                dbg << ", ";
            }
        }
        dbg << newline;
    }
    if( mConstraints.count() > 0 )
    {
        dbg << "Constraints (" << mConstraints.count() << ":" << newline;
        dbg.indent();
        QSetIterator<const AbstractConstraint *> mi(mConstraints);
        while( mi.hasNext() )
        {
            dbg << mi.next()->summary() << newline;
        }
        dbg.unindent();
        dbg << ")" << newline; // constraints
    }
    dbg.unindent();
    dbg << ")"; // constraints, allomorph
    return dbgString;
}

QString Allomorph::oneLineSummary() const
{
    return summary().replace(QRegularExpression("\\s+")," ").trimmed();
}

QString Allomorph::focusedSummary(const WritingSystem &ws) const
{
    QString dbgString;
    QTextStream dbg(&dbgString);

    dbg << "Allomorph(" << typeToString( mType ) << ", ";

    Form f = mForms.value(ws, Form(ws,"ERROR") );
    dbg << f.text() << ", ";

    dbg << "Tags: ";
    QSetIterator<Tag> ti(mTags);
    while( ti.hasNext() )
    {
        dbg << ti.next().summary();
        if( ti.hasNext() )
        {
            dbg << ", ";
        }
    }
    dbg << ")";
    return dbgString;
}

uint ME::qHash(const Allomorph &key)
{
    return key.hash();
}
