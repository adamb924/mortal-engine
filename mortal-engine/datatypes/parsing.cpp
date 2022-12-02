#include "parsing.h"

#include <QXmlStreamWriter>
#include <QRegularExpression>
#include <QDomElement>

#include "nodes/morphologicalmodel.h"
#include "allomorph.h"
#include "constraints/abstractconstraint.h"
#include "constraints/abstractlongdistanceconstraint.h"
#include "morphology.h"

int Parsing::MAXIMUM_JUMPS = 1;

Parsing::Parsing() :
    mForm( WritingSystem(), "" ),
    mPosition(0),
    mStatus(Parsing::Null),
    mMorphologicalModel(nullptr),
    mNextNodeRequired(false)
{

}

Parsing::Parsing(const Form &form, const MorphologicalModel *morphologicalModel) :
    mForm(form),
    mPosition(0),
    mStatus(Parsing::Null),
    mMorphologicalModel(morphologicalModel),
    mNextNodeRequired(false)
{

}

Parsing::~Parsing()
{

}

Parsing::Parsing(const Parsing &other) :
    /// possibly bad to share pointers?
      mSteps(other.mSteps),
      mForm(other.mForm),
      mPosition(other.mPosition),
      mLocalConstraints(other.mLocalConstraints),
      mLongDistanceConstraints(other.mLongDistanceConstraints),
      mStatus(other.mStatus),
      mMorphologicalModel(other.mMorphologicalModel),
      mJumpCounts(other.mJumpCounts),
      mNextNodeRequired(other.mNextNodeRequired),
      mStackTrace(other.mStackTrace),
      mHash(other.mHash)
{
}

Parsing &Parsing::operator=(const Parsing &other)
{
    mSteps = other.mSteps;
    mForm = other.mForm;
    mPosition = other.mPosition;
    mLocalConstraints = other.mLocalConstraints;
    mLongDistanceConstraints = other.mLongDistanceConstraints;
    mStatus = other.mStatus;
    mMorphologicalModel = other.mMorphologicalModel;
    mJumpCounts = other.mJumpCounts;
    mNextNodeRequired = other.mNextNodeRequired;
    mStackTrace = other.mStackTrace;
    mHash = other.mHash;

    return *this;
}

bool Parsing::operator==(const Parsing &other) const
{
    if( labelSummary() != other.labelSummary() )
        return false;
    const QList<LexicalStem> myStems = lexicalStems();
    const QList<LexicalStem> otherStems = other.lexicalStems(); 
    if( myStems.count() != otherStems.count() )
        return false;
    for(int i=0; i<myStems.count(); i++)
    {
        if( !( myStems.at(i) == otherStems.at(i) ) )
            return false;
    }
    return true;
}

Form Parsing::parsedSoFar() const
{
    return Form( mForm.writingSystem(), mForm.text().mid(0,mPosition) );
}

Form Parsing::remainder() const
{
    return Form( mForm.writingSystem(), mForm.text().mid(mPosition) );
}

Form Parsing::formFromPosition(int pos) const
{
    return Form( mForm.writingSystem(), mForm.text().mid(pos) );
}

QString Parsing::intermediateSummary() const
{
    if( isCompleted() )
    {
        return QString("'%1' parsed as %2.")
                .arg( parsedSoFar().text(),
                      labelSummary() );
    }
    else
    {
        return QString("'%1' parsed as %2, with '%3' remaining.")
                .arg( parsedSoFar().text(),
                      labelSummary(),
                      remainder().text() );
    }
}

Parsing::Status Parsing::status() const
{
    return mStatus;
}

QString Parsing::statusToString(Parsing::Status status)
{
    switch(status)
    {
    case Parsing::Null:
        return "Null";
    case Parsing::Ongoing:
        return "Ongoing";
    case Parsing::Failed:
        return "Failed";
    case Parsing::Completed:
        return "Completed";
    }
    return "Null";
}

Parsing::Status Parsing::statusFromString(const QString &status)
{
    if( status == "Ongoing" )
    {
        return Parsing::Ongoing;
    }
    else if( status == "Failed" )
    {
        return Parsing::Failed;
    }
    else if( status == "Completed" )
    {
        return Parsing::Completed;
    }
    else
    {
        return Parsing::Null;
    }
}

void Parsing::setStatus(const Status &status)
{
    mStatus = status;
}

bool Parsing::allConstraintsSatisfied() const
{
    /// this function should only be called at the end of a derivation, so the localConstraintsSatisfied
    /// should be called with an empty Allomorph (i.e., no Allomorph), so that any local constraints
    /// can voice their objections to not being satisfied
    bool localConstraintsResolved = constraintsSetSatisfied( mLocalConstraints, mSteps.last().node(), Allomorph(Allomorph::Null) );
    bool finalAllomorphConstraintsResolved = constraintsSetSatisfied( mSteps.last().allomorph().localConstraints(), mSteps.last().node(), Allomorph(Allomorph::Null) );
    bool longDistanceConstraintsResolved = longDistanceConstraintsSatisfied();

    if( Morphology::DebugOutput )
    {
        if( localConstraintsResolved )
        {
            qInfo() << qPrintable("\t") << "Local constraints satisfied";
            qInfo() << qPrintable("\t\t") << constraintsSetSatisfactionSummary( mLocalConstraints, mSteps.last().node(), Allomorph(Allomorph::Null) );
        }
        else
        {
            qInfo() << qPrintable("\t") << "Local constraints failed";
            qInfo() << qPrintable("\t\t") << constraintsSetSatisfactionSummary( mLocalConstraints, mSteps.last().node(), Allomorph(Allomorph::Null) );
        }
        if( finalAllomorphConstraintsResolved )
        {
            qInfo() << qPrintable("\t") << "Final allomorph's constraints satisfied";
            qInfo() << qPrintable("\t\t") << constraintsSetSatisfactionSummary( mSteps.last().allomorph().localConstraints(), mSteps.last().node(), Allomorph(Allomorph::Null) );
        }
        else
        {
            qInfo() << qPrintable("\t") << "Final allomorph's constraints failed";
            qInfo() << qPrintable("\t\t") << constraintsSetSatisfactionSummary( mSteps.last().allomorph().localConstraints(), mSteps.last().node(), Allomorph(Allomorph::Null) );
        }
        if( longDistanceConstraintsResolved )
        {
            qInfo() << qPrintable("\t") << "Long distance constraints satisfied";
            qInfo() << qPrintable("\t\t") << longDistanceConstraintsSatisfactionSummary();
        }
        else
        {
            qInfo() << qPrintable("\t") << "Long distance constraints failed";
            qInfo() << qPrintable("\t\t") << longDistanceConstraintsSatisfactionSummary();
        }
    }

    return localConstraintsResolved && finalAllomorphConstraintsResolved && longDistanceConstraintsResolved;
}

void Parsing::serialize(QXmlStreamWriter &out, bool includeGlosses) const
{
    out.writeStartElement("parsing");
    out.writeAttribute("lang",mForm.writingSystem().abbreviation());
    out.writeAttribute("text",mForm.text());
    out.writeAttribute("status", statusToString(mStatus));
    out.writeAttribute("position", QString("%1").arg( mPosition ) );
    out.writeAttribute("morphological-model", mMorphologicalModel->label() );
    if( isCompleted() )
        /// only write further information if the parsing is actually completed
        /// this function should not be used for any other parsing stages
    {
        QListIterator<ParsingStep> i(mSteps);
        while(i.hasNext())
        {
            i.next().serialize(out, includeGlosses);
        }
    }
    out.writeEndElement(); /// parsing
}

void Parsing::serialize(QDomElement &out, bool includeGlosses) const
{
    /// clear nodes and attributes
    QDomNamedNodeMap attr = out.attributes();
    for(int i=0; i<attr.count();i++)
        out.removeAttribute( attr.item(i).nodeName() );
    QDomNodeList nodes = out.childNodes();
    for(int i=0; i<nodes.count();i++)
        out.removeChild( nodes.at(i) );

    out.setAttribute("lang", mForm.writingSystem().abbreviation());
    out.setAttribute("text",mForm.text());
    out.setAttribute("status", statusToString(mStatus));
    out.setAttribute("position", QString("%1").arg( mPosition ) );
    out.setAttribute("morphological-model", mMorphologicalModel->label() );

    if( isCompleted() )
        /// only write further information if the parsing is actually completed
        /// this function should not be used for any other parsing stages
    {
        QListIterator<ParsingStep> i(mSteps);
        while(i.hasNext())
        {
            QDomElement psEl = out.ownerDocument().createElement("parsing-step");
            i.next().serialize(psEl, includeGlosses);
            out.appendChild(psEl);
        }
    }
}

Parsing Parsing::readFromXml(QXmlStreamReader &in, const Morphology *morphology, const QString &elementName)
{
    Q_ASSERT( in.isStartElement() && in.name() == elementName );

    WritingSystem ws = morphology->writingSystems().value( in.attributes().value("lang").toString() );
    QString text = in.attributes().value("text").toString();
    QString status = in.attributes().value("status").toString();
    int position = in.attributes().value("position").toInt();
    QString modelLabel = in.attributes().value("morphological-model").toString();
    MorphologicalModel * model = morphology->getMorphologicalModelFromLabel(modelLabel);
    Q_ASSERT(model != nullptr);

    Parsing p( Form(ws, text), model );
    p.setStatus( Parsing::statusFromString(status) );
    p.setPosition(position);

    bool stepsOk = true;
    const QList<ParsingStep> steps = ParsingStep::readListFromXml(in, morphology, elementName, stepsOk );
    if( stepsOk )
    {
        p.setSteps( steps );
    }
    else
    {
        p = Parsing();
    }

    Q_ASSERT( in.isEndElement() && in.name() == elementName );
    return p;
}

QList<Parsing> Parsing::readListFromXml(QXmlStreamReader &in, const Morphology * morphology, const QString &untilElement, const QString &elementName)
{
    Q_ASSERT( in.isStartElement() && in.name() == untilElement );

    QList<Parsing> parsings;
    while( !in.atEnd() && !(in.tokenType() == QXmlStreamReader::EndElement && in.name() == untilElement ) )
    {
        in.readNext();

        if( in.isStartElement() && in.name() == elementName )
        {
            parsings << Parsing::readFromXml(in, morphology, elementName);
        }
    }

    Q_ASSERT( in.isEndElement() && in.name() == untilElement );
    return parsings;
}

Parsing Parsing::readFromXml(QDomElement parsing, const Morphology *morphology)
{
    bool incomplete = ! parsing.hasAttribute("morphological-model")
                        || ! parsing.hasAttribute("lang")
                        || ! parsing.hasAttribute("status")
                        || ! parsing.hasAttribute("position");

    if( incomplete )
    {
        return Parsing();
    }

    WritingSystem ws = morphology->writingSystems().value( parsing.attribute("lang") );
    QString status = parsing.attribute("status");
    int position = parsing.attribute("position").toInt();
    QString modelLabel = parsing.attribute("morphological-model");
    MorphologicalModel * model = morphology->getMorphologicalModelFromLabel(modelLabel);
    Q_ASSERT(model != nullptr);

    Parsing p( Form(ws, parsing.attribute("text") ), model );
    p.setStatus( Parsing::statusFromString(status) );
    p.setPosition(position);

    bool stepsOk = true;
    const QList<ParsingStep> steps = ParsingStep::readListFromXml(parsing, morphology, stepsOk );
    if( stepsOk )
    {
        p.setSteps( steps );
    }
    else
    {
        p = Parsing();
    }

    return p;
}

QList<Parsing> Parsing::readListFromXml(QDomElement list, const Morphology *morphology)
{
    QList<Parsing> parsings;
    QDomNodeList parsingElements = list.elementsByTagName("parsing");
    for(int i=0; i< parsingElements.count(); i++)
    {
        parsings << Parsing::readFromXml( parsingElements.at(i).toElement(), morphology);
    }
    return parsings;
}

WritingSystem Parsing::writingSystem() const
{
    return mForm.writingSystem();
}

void Parsing::append(const AbstractNode *node, const Allomorph &allomorph, const LexicalStem & lexicalStem, bool isStem)
{
    if( constraintsSetSatisfied( mLocalConstraints, node, allomorph) )
    {
        if( Morphology::DebugOutput )
        {
            /// add a plus to any node that appends
            if( mStackTrace.count() > 0 )
            {
                mStackTrace.replace( mStackTrace.count() - 1, mStackTrace.last() + " +" );
            }
        }

        /// the local constraints are kept in the parsing object,
        /// so they need to be cleared if the allomorph satisfies them
        mLocalConstraints.clear();

        /// update the position of the parsing
        Form allomorphForm = allomorph.form( writingSystem() );
        mPosition += allomorphForm.text().length();

        ParsingStep ps(node, allomorph, lexicalStem);
        ps.setIsStem(isStem);
        mSteps.append( ps );

        addLocalConstraints( allomorph.localConstraints() );
        addLongDistanceConstraints( allomorph.longDistanceConstraints() );

        if( atEnd() )
        {
            if( allConstraintsSatisfied() )
            {
                setStatus( Parsing::Completed );
                if( Morphology::DebugOutput )
                {
                    qInfo().noquote() << "Parse completed:" << intermediateSummary();
                    qInfo() << "------ Stack Trace ------\n" << qPrintable( mStackTrace.join("\n") ) << "\n-------------------------";
                }
            }
            else
            {
                if( Morphology::DebugOutput )
                {
                    qInfo().noquote() << "Parse failed because not all constraints were satisfied:" << intermediateSummary() << "Trying to append:" << allomorph.focusedSummary( writingSystem() );
                }
                setStatus( Parsing::Failed );
            }
        }
        else
        {
            if( Morphology::DebugOutput )
            {
                qInfo().noquote() << "Parse continuing:" << intermediateSummary();
            }
            setStatus( Parsing::Ongoing );
        }
    }
    else
    {
        setStatus( Parsing::Failed );
        if( Morphology::DebugOutput )
        {
            qInfo().noquote() << "Parse failed because local constraints were not satisfied:" << intermediateSummary() << "Trying to append:" << allomorph.focusedSummary( writingSystem() );
            qInfo().noquote() << qPrintable("\t\t") << constraintsSetSatisfactionSummary( mLocalConstraints, node, allomorph);
        }
    }
    calculateHash();
}

int Parsing::position() const
{
    return mPosition;
}

bool Parsing::constraintsSetSatisfied(const QSet<const AbstractConstraint *> &set, const AbstractNode *node, const Allomorph &allomorph) const
{
    QSetIterator<const AbstractConstraint *> i( set );
    while( i.hasNext() )
    {
        const AbstractConstraint * c = i.next();
        if( ! c->matches(this, node, allomorph) )
        {
            return false;
        }
    }
    return true;
}

QString Parsing::constraintsSetSatisfactionSummary(const QSet<const AbstractConstraint *> &set, const AbstractNode *node, const Allomorph &allomorph) const
{
    QString summary;
    QSetIterator<const AbstractConstraint *> i( set );
    while( i.hasNext() )
    {
        const AbstractConstraint * c = i.next();
        if( c->matches(this, node, allomorph) )
        {
            summary += "Condition satisfied: " + c->satisfactionSummary(this,node,allomorph) + "\n";
        }
        else
        {
            summary += "Condition failed: " + c->satisfactionSummary(this,node,allomorph) + "\n";
        }

    }
    return summary;
}

bool Parsing::longDistanceConstraintsSatisfied() const
{
    QSetIterator<const AbstractConstraint *> i( mLongDistanceConstraints );
    while( i.hasNext() )
    {
        const AbstractLongDistanceConstraint * c = i.next()->toLongDistanceConstraint();
        if( ! c->satisfied(this) )
        {
            return false;
        }
    }
    return true;
}

QString Parsing::longDistanceConstraintsSatisfactionSummary() const
{
    QString summary;
    QSetIterator<const AbstractConstraint *> i( mLongDistanceConstraints );
    while( i.hasNext() )
    {
        const AbstractLongDistanceConstraint * c = i.next()->toLongDistanceConstraint();
        if( ! c->satisfied(this) )
        {
            summary += "Condition satisfied: " + c->summary() + "\n";
        }
        else
        {
            summary += "Condition failed: " + c->summary() + "\n";
        }
    }
    return summary;
}

void Parsing::calculateHash()
{
    mHash = qHash(labelSummary(), static_cast<uint>(qGlobalQHashSeed()));
}

uint Parsing::hash() const
{
    return mHash;
}

void Parsing::setSteps(const QList<ParsingStep> &steps)
{
    mSteps = steps;
    calculateHash();
}

bool Parsing::hasHypotheticalStem() const
{
    QListIterator<ParsingStep> i(mSteps);
    while(i.hasNext())
    {
        if( i.next().allomorph().isHypothetical() )
            return true;
    }
    return false;
}

void Parsing::setPosition(int position)
{
    mPosition = position;
}

bool Parsing::nextNodeRequired() const
{
    return mNextNodeRequired;
}

void Parsing::setNextNodeRequired(bool nextNodeRequired)
{
    mNextNodeRequired = nextNodeRequired;
}

void Parsing::addToStackTrace(const QString &str)
{
    mStackTrace << str;
}

const MorphologicalModel *Parsing::morphologicalModel() const
{
    return mMorphologicalModel;
}

bool Parsing::isGeneration() const
{
    return false;
}

bool Parsing::hasPortmanteauClash(const QStringList &morphemes, const WritingSystem & ws) const
{
    for(int i=0; i <= (mSteps.count() - morphemes.count()); i++)
    {
        bool matches = true;
        for(int j=0; j < morphemes.count(); j++)
        {
            if( !mSteps.at(i+j).allomorph().hasPortmanteau(ws)
                    && mSteps.at(i+j).label(ws) == morphemes.at(j) )
            {
                continue;
            }
            else
            {
                matches = false;
                break;
            }
        }
        if( matches )
        {
            return true;
        }
    }
    return false;
}

bool Parsing::hasPortmanteauClash(const QMultiHash<WritingSystem, QStringList> &portmanteaux, const WritingSystem & ws) const
{

    QListIterator<QStringList> i( portmanteaux.values(ws) );
    while(i.hasNext())
    {
        QStringList p = i.next();
        if( hasPortmanteauClash( p, ws ) )
        {
            return true;
        }
    }
    return false;
}

void Parsing::incrementJumpCounter(const Jump *jump)
{
    int previous = mJumpCounts.value(jump, 0);
    mJumpCounts.insert(jump, previous + 1);
}

bool Parsing::jumpPermitted(const Jump *jump) const
{
    return mJumpCounts.value(jump, 0) < MAXIMUM_JUMPS;
}

int Parsing::jumpCounter(const Jump *jump) const
{
    return mJumpCounts.value(jump, 0);
}

LexicalStem Parsing::firstLexicalStem() const
{
    QListIterator<ParsingStep> i(mSteps);
    while(i.hasNext())
    {
        ParsingStep ps = i.next();
        if( ! ps.lexicalStem().isEmpty() )
        {
            return ps.lexicalStem();
        }
    }
    return LexicalStem();
}

QList<LexicalStem> Parsing::lexicalStems() const
{
    QList<LexicalStem> stems;
    QListIterator<ParsingStep> i(mSteps);
    while(i.hasNext())
    {
        ParsingStep ps = i.next();
        if( ! ps.lexicalStem().isEmpty() )
        {
            stems << ps.lexicalStem();
        }
    }
    return stems;
}

bool Parsing::containsStem(const LexicalStem &stem) const
{
    QListIterator<ParsingStep> i(mSteps);
    while(i.hasNext())
    {
        if( i.next().lexicalStem() == stem )
        {
            return true;
        }
    }
    return false;
}

bool Parsing::containsStem(qlonglong stemId) const
{
    QListIterator<ParsingStep> i(mSteps);
    while(i.hasNext())
    {
        if( i.next().lexicalStem() == stemId )
        {
            return true;
        }
    }
    return false;
}

int Parsing::numberOfInstancesOfStem(qlonglong stemId) const
{
    int ct = 0;
    QListIterator<ParsingStep> i(mSteps);
    while(i.hasNext())
    {
        if( i.next().lexicalStem() == stemId )
        {
            ct++;
        }
    }
    return ct;
}

Allomorph Parsing::firstLexicalStemAllomorph() const
{
    QListIterator<ParsingStep> i(mSteps);
    while(i.hasNext())
    {
        ParsingStep ps = i.next();
        if( ! ps.lexicalStem().isEmpty() )
        {
            return ps.allomorph();
        }
    }
    return Allomorph(Allomorph::Null);
}

Allomorph Parsing::firstHypotheticalAllomorph() const
{
    QListIterator<ParsingStep> i(mSteps);
    while(i.hasNext())
    {
        ParsingStep ps = i.next();
        if( ps.allomorph().isHypothetical() )
        {
            return ps.allomorph();
        }
    }
    return Allomorph(Allomorph::Null);
}

Form Parsing::form() const
{
    return mForm;
}

Form Parsing::stem(const WritingSystem &ws) const
{
    QListIterator<ParsingStep> i(mSteps);
    while(i.hasNext())
    {
        ParsingStep ps = i.next();
        if( ps.node()->type() == AbstractNode::StemNodeType )
        {
            return ps.allomorph().form( ws );
        }
    }
    return Form(ws, "");
}

QStringList Parsing::morphemeNames() const
{
    QStringList names;
    QListIterator<ParsingStep> i(mSteps);
    while(i.hasNext())
    {
        names << i.next().label( writingSystem() );
    }
    return names;
}

void Parsing::addLocalConstraints(const QSet<const AbstractConstraint *> &newConstraints)
{
    mLocalConstraints.unite(newConstraints);
}

void Parsing::addLongDistanceConstraints(const QSet<const AbstractConstraint *> &newConstraints)
{
    mLongDistanceConstraints.unite(newConstraints);
}

QList<ParsingStep> Parsing::steps() const
{
    return mSteps;
}

bool Parsing::allomorphMatches(const Allomorph &allomorph) const
{
    bool segmentalMatch = allomorphMatchesSegmentally(allomorph);
    if( !segmentalMatch )
    {
        if( Morphology::DebugOutput )
        {
            qInfo().noquote() << qPrintable("\t") << allomorph.form( writingSystem() ).text() << "Segmental Fail: " << allomorph.oneLineSummary();
        }
        return false;
    }
    bool conditionMatch = allomorphMatchConditionsSatisfied(allomorph);
    if( Morphology::DebugOutput )
    {
        if( segmentalMatch && conditionMatch )
        {
            qInfo().noquote() << qPrintable("\t") << allomorph.form( writingSystem() ).text() << "Segmental and Condition Match:" << allomorph.oneLineSummary();
        }
        else if( segmentalMatch && !conditionMatch )
        {
            qInfo().noquote() << qPrintable("\t") << allomorph.form( writingSystem() ).text() << "Segmental Match but Conditional Fail:" << allomorph.oneLineSummary();
            allomorphConditionMatchStringSummary(allomorph);
        }
        else if( !segmentalMatch && conditionMatch )
        {
            qInfo().noquote() << qPrintable("\t") << allomorph.form( writingSystem() ).text() << "Conditional Match but Segmental Fail:" << allomorph.oneLineSummary();
        }
        else if( !segmentalMatch && !conditionMatch )
        {
            qInfo().noquote() << qPrintable("\t") << allomorph.form( writingSystem() ).text() << "Segmental Fail and Conditional Fail:" << allomorph.oneLineSummary();
            allomorphConditionMatchStringSummary(allomorph);
        }
    }
    return segmentalMatch && conditionMatch;
}

bool Parsing::allomorphMatchConditionsSatisfied(const Allomorph &allomorph) const
{
    foreach(const AbstractConstraint * c, allomorph.matchConditions() )
    {
        bool satisfied = c->matches(this, nullptr, allomorph );
        if( ! satisfied )
        {
            if( Morphology::DebugOutput )
            {
                qInfo() << qPrintable("\t") << allomorph.form(writingSystem()).text() << "Match conditions failed:";
                allomorphConditionMatchStringSummary(allomorph);
            }
            return false;
        }
    }
    if( Morphology::DebugOutput )
    {
        qInfo() << qPrintable("\t") << allomorph.form(writingSystem()).text() << "Match conditions satisfied:";
        allomorphConditionMatchStringSummary(allomorph);
    }
    return true;
}

void Parsing::allomorphConditionMatchStringSummary(const Allomorph &allomorph) const
{
    foreach(const AbstractConstraint * c, allomorph.matchConditions() )
    {
        if( c->matches(this, nullptr, allomorph ) )
        {
            qInfo() << qPrintable("\t\t") << "Condition satisfied: " + c->satisfactionSummary(this, nullptr, allomorph);
        }
        else
        {
            qInfo() << qPrintable("\t\t") << "Condition failed: " + c->satisfactionSummary(this, nullptr, allomorph);
        }
    }
}

bool Parsing::allomorphMatchesSegmentally(const Allomorph &allomorph) const
{
    bool ok;
    Form allomorphForm = allomorph.form( writingSystem(), &ok );
    /// 10/22/2020: previously this checked for the string no be of non-zero length
    /// but this probhibits zero-length morphemes, which are legit in some contexts
    /// unfortunately I can't remember what this was meant to fix
    if( ok )
    {
        return mForm.text().mid( mPosition, allomorphForm.text().length() ) == allomorphForm.text();
    }
    else
    {
        return false;
    }
}

bool Parsing::atEnd() const
{
    return mPosition == mForm.text().length()
            && mSteps.last().lastNode( writingSystem() )->hasPathToEnd();
}

bool Parsing::isNull() const
{
    return mStatus == Parsing::Null;
}

bool Parsing::isCompleted() const
{
    return mStatus == Parsing::Completed;
}

bool Parsing::isOngoing() const
{
    return mStatus == Parsing::Ongoing;
}

bool Parsing::hasNotFailed() const
{
    return mStatus != Parsing::Failed;
}

QString Parsing::labelSummary( const WritingSystem & ws ) const
{
    if( ws.isNull() )
    {
        return stringSummary( ParsingStep::MorphemeLabel, WritingSystem(), ParsingStep::MorphemeLabel, WritingSystem(), "[", "]", "" );
    }
    else
    {
        return stringSummary( ParsingStep::MorphemeForm, ws, ParsingStep::MorphemeLabel, WritingSystem(), "[", "]", "" );
    }
}

QString Parsing::morphemeDelimitedSummary(const WritingSystem &ws) const
{
    return stringSummary( ParsingStep::MorphemeForm, ws, ParsingStep::MorphemeForm, ws, "", "", "-" );
}

QString Parsing::hypotheticalStyleSummary(const WritingSystem &ws) const
{
    return stringSummary( ParsingStep::MorphemeForm, ws, ParsingStep::MorphemeLabel, WritingSystem(), "[", "]", "" );
}

QString Parsing::interlinearStyleSummary(const WritingSystem &ws) const
{
    return stringSummary( ParsingStep::MorphemeGloss, ws, ParsingStep::MorphemeGloss, ws, "", "", "-" );
}

QString Parsing::stringSummary(ParsingStep::SummaryType stemType, const WritingSystem & stemWs, ParsingStep::SummaryType affixType, const WritingSystem & affixWs, const QString & beforeMorpheme, const QString & afterMorpheme, const QString & betweenMorphemes) const
{
    QString string;
    QListIterator<ParsingStep> iter(mSteps);
    while( iter.hasNext() )
    {
        ParsingStep s = iter.next();

        /// e.g., [
        string += beforeMorpheme;

        if( s.isStem() )
        {
            string += s.summaryPortion(stemType, stemWs, writingSystem(), betweenMorphemes );
        }
        else
        {
            string += s.summaryPortion(affixType, affixWs, writingSystem(), betweenMorphemes );
        }

        /// e.g., ]
        string += afterMorpheme;
        if( iter.hasNext() )
        {
            /// e.g., -
            string += betweenMorphemes;
        }
    }

    return string;
}

QString Parsing::parsingListSummary(const QList<Parsing> &list, const WritingSystem &ws)
{
    QString summary;
    QListIterator<Parsing> iter(list);
    while( iter.hasNext() )
    {
        summary += iter.next().labelSummary(ws);
        if( iter.hasNext() )
            summary += ", ";
    }
    return summary;
}

bool Parsing::parsingsHaveSameStems(const QList<Parsing> &list)
{
    if( list.count() < 2 ) return true;
    QList<LexicalStem> firstStems = list.first().lexicalStems();
    for( int i=1; i<list.count(); i++ )
    {
        if( firstStems != list.at(i).lexicalStems() )
        {
            return false;
        }
    }
    return true;
}

bool Parsing::parsingsHaveSameMorphologicalSequence(const QList<Parsing> &list)
{
    if( list.count() < 2 ) return true;
    QString firstSequence = list.first().labelSummary();
    for( int i=1; i<list.count(); i++ )
    {
        if( firstSequence != list.at(i).labelSummary() )
        {
            return false;
        }
    }
    return true;
}

QString Parsing::summary() const
{
    QString dbgString;
    QTextStream dbg(&dbgString);

    dbg << "Parsing(\n";
    dbg << "Form: " << mForm.summary() << "\n";
    dbg << "Summary: " << labelSummary() << "\n";
    dbg << "Status: " << statusToString(mStatus) << "\n";
    dbg << "Position: " << mPosition << "\n";
    dbg << "Remainder: " << mForm.text().mid(mPosition) << "\n";
    dbg << "Model: " << (mMorphologicalModel == nullptr ? "null" : mMorphologicalModel->label()) << "\n";

    dbg << "Steps(s) (n=" << mSteps.length() << ") (\n";

    QListIterator<ParsingStep> i(mSteps);
    while(i.hasNext())
    {
        ParsingStep ps = i.next();
        dbg << ps.node()->label() << ", " << ps.allomorph().focusedSummary( writingSystem() ) << "\n";
    }
    dbg << "),\n";
    dbg << "Local Constraint(s) (n=" << mLocalConstraints.size() << ") (\n";
    QSetIterator<const AbstractConstraint*> lci(mLocalConstraints);
    while(lci.hasNext())
    {
        dbg << lci.next()->summary() << "\n";
    }

    dbg << ")";

    return dbgString;
}

QString Parsing::oneLineSummary() const
{
    return summary().replace(QRegularExpression("\\s+")," ").trimmed();
}

bool parsingLessThanStepwise(const Parsing &p1, const Parsing &p2)
{
    return p1.steps().length() > p2.steps().length();
}

uint qHash(const Parsing & key)
{
    return key.hash();
}
