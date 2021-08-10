#include "createallomorphscase.h"

#include "datatypes/allomorph.h"
#include "datatypes/writingsystem.h"

#include "morphologyxmlreader.h"
#include "debug.h"

#include <QXmlStreamReader>

QString CreateAllomorphsCase::XML_MATCH_TAG = "match-tag";
QString CreateAllomorphsCase::XML_WITHOUT_TAG = "without-tag";
QString CreateAllomorphsCase::XML_WHEN = "when";
QString CreateAllomorphsCase::XML_THEN = "then";
QString CreateAllomorphsCase::XML_REPLACE_THIS = "replace-this";
QString CreateAllomorphsCase::XML_WITH_THIS = "with-this";
QString CreateAllomorphsCase::XML_ADD_TAG = "add-tag";
QString CreateAllomorphsCase::XML_REMOVE_TAG = "remove-tag";

CreateAllomorphsCase::CreateAllomorphsCase() : mFormsMode(IncludeAllForms)
{

}

Allomorph CreateAllomorphsCase::createAllomorph(const Allomorph & input, const QSet<Allomorph> *allomorphs) const
{
    /// Counterintuitively, you try to create the allomorphs first
    /// If new forms are generated, then you return the form
    /// Otherwise, return the nullptr
    ///

    /// if the allomorph doesn't match the tags for this case, return nullptr (i.e., don't apply)
    if( ! input.tags().contains( mMatchTags ) || ( !mNotMatchTags.isEmpty() && input.tags().contains( mNotMatchTags ) ) )
    {
        return Allomorph(Allomorph::Null);
    }

    Allomorph newAllomorph( input );
    newAllomorph.clearForms(); /// this will be replaced or not, according to mFormsMode
    newAllomorph.setType(Allomorph::Derived);
    bool allomorphChanged = false;

    newAllomorph.addConstraints( mConstraints );

    foreach( Form f, input.forms() )
    {
        Form original = f;
        bool formChanged = false;

        /// perform all the replacements
        for(int i=0; i<mReplacements.count(); i++)
        {
            bool changedThisTime = false;
            f = mReplacements.at(i).perform(f, changedThisTime);
            formChanged = formChanged || changedThisTime;
        }

        /// this checks whether this form is available somewhere else in the set of allomorphs
        bool notADuplicate = ! Allomorph::hasForm(allomorphs, f);

        /// if it's not a duplicate
        if( notADuplicate )
        {
            switch(mFormsMode)
            {
            case IncludeAllForms:
                /// remember the form always
                newAllomorph.setForm( f );
                break;
            case IncludeOnlyChangedForms:
                /// remember it only if it's different
                if( formChanged && notADuplicate )
                {
                    newAllomorph.setForm( f );
                }
                break;
            }
        }

        /// remember if a form's changed
        allomorphChanged = allomorphChanged || formChanged;
    }

    /// if any of them have changed
    if( allomorphChanged )
    {
        newAllomorph.addTags( mAddTags );
        newAllomorph.removeTags( mRemoveTags );

        return newAllomorph;
    }
    else
    {
        return Allomorph(Allomorph::Null);
    }
}

QString CreateAllomorphsCase::elementName()
{
    return "case";
}

CreateAllomorphsCase CreateAllomorphsCase::readFromXml(QXmlStreamReader &in, MorphologyXmlReader *morphologyReader)
{
    Q_UNUSED(morphologyReader)
    Q_ASSERT( in.isStartElement() );

    CreateAllomorphsCase c;

    if( in.attributes().hasAttribute("mode") )
    {
        c.setFormsMode( formsModeFromString( in.attributes().value("mode").toString() ) );
    }

    while( !in.atEnd() && !(in.tokenType() == QXmlStreamReader::EndElement && in.name() == elementName() ) )
    {
        in.readNext();

        if( in.tokenType() == QXmlStreamReader::StartElement )
        {
            if( in.name() == XML_MATCH_TAG )
            {
                c.addMatchTag( Tag( in.readElementText() ) );
            }
            if( in.name() == XML_WITHOUT_TAG )
            {
                c.addNotMatchTag( Tag( in.readElementText() ) );
            }
            else if( in.name() == XML_WHEN )
            {
                in.readNextStartElement();
                QSet<const AbstractConstraint *> constraints = morphologyReader->readConstraints("when", in);
                c.addConstraints( constraints );
            }
            else if( in.name() == XML_THEN )
            {
                /// create an empty list of replace-this
                QList<Form> replaceThis;

                while( !in.atEnd() && !(in.tokenType() == QXmlStreamReader::EndElement && in.name() == XML_THEN ) )
                {
                    in.readNext();

                    if( in.tokenType() == QXmlStreamReader::StartElement )
                    {
                        if( in.name() == XML_REPLACE_THIS)
                        {
                            replaceThis = Form::readListFromXml( in, morphologyReader->morphology(), "replace-this", "form" );
                        }
                        else if( in.name() == XML_WITH_THIS)
                        {
                            c.addReplacement( CreateAllomorphsReplacement( replaceThis, Form::readListFromXml( in, morphologyReader->morphology(), "with-this", "form" ) ) );
                        }
                        else if( in.name() == XML_ADD_TAG)
                        {
                            c.addAddTag( Tag(in.readElementText()) );
                        }
                        else if( in.name() == XML_REMOVE_TAG)
                        {
                            c.addRemoveTag( Tag(in.readElementText()) );
                        }
                    }
                }
            }
        }
    }

    Q_ASSERT( in.isEndElement() && in.name() == elementName() );
    return c;
}

bool CreateAllomorphsCase::matchesElement(QXmlStreamReader &in)
{
    return in.isStartElement() && in.name() == elementName();
}

void CreateAllomorphsCase::addMatchTag(const Tag &t)
{
    mMatchTags << t;
}

void CreateAllomorphsCase::addNotMatchTag(const Tag &t)
{
    mNotMatchTags << t;
}

void CreateAllomorphsCase::addConstraints(const QSet<const AbstractConstraint *> c)
{
    mConstraints.unite(c);
}

void CreateAllomorphsCase::addReplacement(const CreateAllomorphsReplacement &r)
{
    mReplacements << r;
}

QSet<const AbstractConstraint *> CreateAllomorphsCase::constraints() const
{
    return mConstraints;
}

void CreateAllomorphsCase::setFormsMode(const FormsMode &formsMode)
{
    mFormsMode = formsMode;
}

CreateAllomorphsCase::FormsMode CreateAllomorphsCase::formsModeFromString(const QString &mode)
{
    if( mode == "include-only-changed-forms" )
    {
        return CreateAllomorphsCase::IncludeOnlyChangedForms;
    }
    else
    {
        return CreateAllomorphsCase::IncludeAllForms;
    }
}

void CreateAllomorphsCase::addAddTag(const Tag &t)
{
    mAddTags << t;
}

void CreateAllomorphsCase::addRemoveTag(const Tag &t)
{
    mRemoveTags << t;
}

QString CreateAllomorphsCase::summary() const
{
    QString dbgString;
    Debug dbg(&dbgString);
    dbg << "CreateAllomorphsCase(" << newline;
    dbg.indent();
    dbg << "Match tags(";
    QSetIterator<Tag> ti(mMatchTags);
    while(ti.hasNext())
    {
        dbg << ti.next().summary();
        if( ti.hasNext() )
        {
            dbg << ", ";
        }
    }
    dbg << ")" << newline;
    dbg << "Not Match tags(";
    QSetIterator<Tag> nti(mNotMatchTags);
    while(nti.hasNext())
    {
        dbg << nti.next().summary();
        if( nti.hasNext() )
        {
            dbg << ", ";
        }
    }
    dbg << ")" << newline;

    QSetIterator<const AbstractConstraint *> ci(mConstraints);
    while(ci.hasNext())
    {
        dbg << ci.next()->summary() << newline;
    }

    QListIterator<CreateAllomorphsReplacement> ri(mReplacements);
    while(ri.hasNext())
    {
        dbg << ri.next().summary() << newline;
    }

    dbg.unindent();
    dbg << ")";

    return dbgString;
}
