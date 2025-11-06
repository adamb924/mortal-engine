#include "createallomorphs.h"
#include "createallomorphscase.h"
#include "datatypes/allomorph.h"

#include "constraints/orcondition.h"
#include "constraints/notcondition.h"
#include "constraints/andcondition.h"

#include <QXmlStreamReader>

#include "morphologyxmlreader.h"
#include "debug.h"

using namespace ME;

QString CreateAllomorphs::XML_NO_OTHERWISE = "no-otherwise";
QString CreateAllomorphs::XML_OVERRIDE_OTHERWISE = "override-otherwise";

CreateAllomorphs::CreateAllomorphs() : mOtherwiseMode(Default)
{

}

QSet<Allomorph> CreateAllomorphs::generateAllomorphs(const Allomorph & original) const
{
    QSet<Allomorph> allomorphs;

    /// generate an allomorph for each case
    for(int i=0; i<mCases.count(); i++)
    {
        /// try to create a new allomorph for with the case
        Allomorph newAllomorph = mCases.at(i).createAllomorph( original, &allomorphs );
        /// if no new allomorph is created, an empty Allomorph is returned
        if( newAllomorph.isEmpty() )
        {
            continue;
        }
        else
        {
            allomorphs << newAllomorph;
        }
    }

    /// if new allomorphs have been generated
    if( allomorphs.count() > 0 )
    {
        /// the default is to automatically create an otherwise morpheme
        if( mOtherwiseMode == CreateAllomorphs::Default )
        {
            /// then add the otherwise allomorph
            allomorphs << createOtherwiseAllomorph( original, allomorphs );
        }
        /// the user may have specifed his/her own list of conditions to add to the original morpheme instead
        else if( mOtherwiseMode == CreateAllomorphs::Override )
        {
            Allomorph newAllomorph = original;
            newAllomorph.addConstraints( mOtherwiseOverride.constraints() );
            allomorphs << newAllomorph;
        }
        /// CreateAllomorphs::None means don't add an otherwise morpheme at all
    }
    /// if no new allomorphs have been generated
    else
    {
        /// then the original allomorph is returned as the singleton member of the set
        allomorphs << original;
    }

    return allomorphs;
}

void CreateAllomorphs::setOtherwiseMode(CreateAllomorphs::OtherwiseMode mode)
{
    mOtherwiseMode = mode;
}

void CreateAllomorphs::addCase(CreateAllomorphsCase c)
{
    mCases << c;
}

QString CreateAllomorphs::elementName()
{
    return "create-allomorphs";
}

CreateAllomorphs CreateAllomorphs::readFromXml(QXmlStreamReader &in, MorphologyXmlReader *morphologyReader)
{
    Q_UNUSED(morphologyReader)
    Q_ASSERT( in.isStartElement() );

    CreateAllomorphs ca;

    if( in.attributes().hasAttribute("id") )
    {
        ca.setId( in.attributes().value("id").toString() );
    }

    while( !in.atEnd() && !(in.tokenType() == QXmlStreamReader::EndElement && in.name() == elementName() ) )
    {
        in.readNext();

        if( CreateAllomorphsCase::matchesElement(in) )
        {
            ca.addCase( CreateAllomorphsCase::readFromXml(in, morphologyReader) );
        }
        else if( in.isStartElement() && in.name() == XML_NO_OTHERWISE )
        {
            ca.setOtherwiseMode(None);
        }
        else if( in.isStartElement() && in.name() == XML_OVERRIDE_OTHERWISE )
        {
            in.readNextStartElement();
            QSet<const AbstractConstraint *> constraints = morphologyReader->readConstraints(XML_OVERRIDE_OTHERWISE, in);
            CreateAllomorphsCase otherwiseOverride;
            otherwiseOverride.addConstraints( constraints );
            ca.setOtherwiseOverride(otherwiseOverride);
        }
    }

    Q_ASSERT( in.isEndElement() && in.name() == elementName() );
    return ca;
}

bool CreateAllomorphs::matchesElement(QXmlStreamReader &in)
{
    return in.isStartElement() && in.name() == elementName();
}

QHash<QString, CreateAllomorphs> CreateAllomorphs::readListFromXml(QXmlStreamReader &in, MorphologyXmlReader *morphologyReader, const QString &untilElement)
{
    Q_ASSERT( in.isStartElement() && in.name() == untilElement );

    QHash<QString, CreateAllomorphs> cas;
    while( !in.atEnd() && !(in.tokenType() == QXmlStreamReader::EndElement && in.name() == untilElement ) )
    {
        in.readNext();

        if( CreateAllomorphs::matchesElement(in) )
        {
            CreateAllomorphs ca = CreateAllomorphs::readFromXml(in, morphologyReader);
            cas.insert( ca.id(), ca );
        }
    }

    Q_ASSERT( in.isEndElement() && in.name() == untilElement );
    return cas;
}

QString CreateAllomorphs::summary() const
{
    QString dbgString;
    Debug dbg(&dbgString);
    dbg << "CreateAllomorphs(" << newline;
    dbg.indent();
    dbg << "no-otherwise: " << overrideModeAsString() << newline;

    dbg << mCases.count() << " case(s)," << newline;

    foreach(CreateAllomorphsCase c, mCases)
    {
        dbg << c.summary() << newline;
    }
    dbg.unindent();
    dbg << ")";

    return dbgString;
}

Allomorph CreateAllomorphs::createOtherwiseAllomorph(const Allomorph &original, QSet<Allomorph> allomorphs) const
{
    Allomorph otherwise(original);

    /// the otherwise allomorph is the one that satisfies none of the other conditions
    /// i.e. not( or( condition1, condition2, condition3 ) )
    /// where condition1 is, e.g., and(constraint1, constraint2, constraint3) etc.

    NotCondition * nc = new NotCondition;
    OrCondition * oc = new OrCondition;
    int count = 0;

    QSetIterator<Allomorph> i(allomorphs);
    while(i.hasNext())
    {
        Allomorph a = i.next();
        if( a.constraints().count() > 1 )
        {
            AndCondition * ac = new AndCondition;
            ac->setConstraints( a.constraints() );
            count += a.constraints().count();
            oc->addConstraint(ac);
        }
        else if( a.constraints().count() == 1 )
        {
            oc->addConstraint( a.constraints().values().first() );
            count++;
        }
    }

    if( count > 0 )
    {
        /// if there is only one constraint in the OrConstraint,
        /// just add that one constraint instead of the Or
        /// this avoids an OrConstraint with a single constraint in it
        if( oc->count() == 1 )
        {
            const AbstractConstraint * singleton = oc->children().values().first();
            /// one final improvement: if this happens to be a NotConstraint, add
            /// the children of the NotConstraint rather than the NotConstraint,
            /// which would create a Not( Not( ... ) ) structure
            if( singleton->isNot() )
            {
                otherwise.addConstraints( singleton->toNestedConstraint()->children() );
                /// and in that case we don't need nc anymore
                delete nc;
            }
            /// otherwise place the singleton into a NotConstraint
            else
            {
                nc->addConstraint( singleton );
                otherwise.addConstraint( nc );
            }
            /// by now we're done with oc
            delete oc;
        }
        /// otherwise there must be more than one constraint in the
        /// OrConstraint, so go ahead and add the whole thing
        else
        {
            nc->addConstraint( oc );
            otherwise.addConstraint( nc );
        }
    }
    else
    {
        delete oc;
        delete nc;
    }

    return otherwise;
}

void CreateAllomorphs::setOtherwiseOverride(const CreateAllomorphsCase &otherwiseOverride)
{
    mOtherwiseOverride = otherwiseOverride;
    setOtherwiseMode(Override);
}

QString CreateAllomorphs::overrideModeAsString() const
{
    switch( mOtherwiseMode )
    {
    case Default:
        return "Default";
    case Override:
        return "Override";
    case None:
        return "None";
    }
    return "impossible";
}

QString CreateAllomorphs::id() const
{
    return mId;
}

void CreateAllomorphs::setId(const QString &id)
{
    mId = id;
}
