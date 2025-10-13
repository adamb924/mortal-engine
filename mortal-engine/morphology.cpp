#include "morphology.h"

#include "morphologyxmlreader.h"
#include "datatypes/generation.h"
#include "nodes/abstractstemlist.h"
#include "nodes/morphemenode.h"
#include "returns/lexicalsteminsertresult.h"
#include "datatypes/lexicalstem.h"
#include <stdexcept>

#include <QDir>

#include "morphologychecker.h"

#include "debug.h"
#include "logging/xmlparsinglog.h"

using namespace ME;

QRegularExpression Morphology::endingStemId("##(\\d+)$", QRegularExpression::UseUnicodePropertiesOption);
ParsingLog Morphology::NULL_PARSING_LOG;

Morphology::Morphology() : mIsOk(true), mDebugOutput(false), mStemDebugOutput(false)
{
    /// TODO this isn't quite right b/c maybe we don't want to initialize it at all
    mParsingLog = new XmlParsingLog;
}

Morphology::~Morphology()
{
    clearData();
    delete mParsingLog;
}

void Morphology::readXmlFile(const QString &path)
{
    /// there's no guarantee that this method hasn't been called previously
    /// so clear any data that might exist
    clearData();

    MorphologyXmlReader reader(this);
    try {
        reader.readXmlFile(path);
    }  catch (const std::runtime_error& e) {
        mIsOk = false;
        qCritical() << e.what() << "(" << path << ")";
    }
}

bool Morphology::isWellFormed(const Form & form) const
{
    QListIterator<MorphologicalModel*> i(mMorphologicalModels);
    while (i.hasNext())
    {
        MorphologicalModel * model = i.next();
        Parsing p( form, model );
        int count = model->possibleParsings(p, Parsing::OnlyOneResult).count();
        if( count > 0 )
        {
            return true;
        }
    }
    return false;
}

void Morphology::clearData()
{
    qDeleteAll( mNodes );

    mMorphologicalModels.clear();
    mWritingSystems.clear();
    mNodesById.clear();
    mNodesByLabel.clear();
    mMorphemeNodesByLabel.clear();
    mMorphologicalModelsByLabel.clear();
    mNodes.clear();
    mStemAcceptingStemLists.clear();
    mStemLists.clear();
    mMorphemeNodes.clear();
    mNormalizationFunctions.clear();
}

QList<MorphologicalModel *> Morphology::morphologicalModels() const
{
    return mMorphologicalModels;
}

WritingSystem Morphology::writingSystem(const QString &lang) const
{
    return mWritingSystems.value(lang);
}

QList<Parsing> Morphology::possibleParsings(const Form &form, Parsing::Flags flags) const
{
    QList<Parsing> candidates;

    parsingLog()->beginParse(form);

    foreach(MorphologicalModel *model,  mMorphologicalModels)
    {
        parsingLog()->beginModel(model);

        Parsing p( normalize(form), model );
        QList<Parsing> parsings = model->possibleParsings(p, flags);
        candidates.append( parsings );

        parsingLog()->end(); /// beginModel
    }

    parsingLog()->end(); /// beginParse

    return candidates;
}

QSet<Parsing> Morphology::uniqueParsings(const Form &form, Parsing::Flags flags) const
{
    QList<Parsing> parsings = possibleParsings(normalize(form), flags);
    return QSet<Parsing>(parsings.begin(),parsings.end());
}

QList<Parsing> Morphology::guessStem(const Form &form) const
{
    QList<Parsing> candidates;

    foreach(MorphologicalModel *model,  mMorphologicalModels)
    {
        Parsing p( normalize(form), model );
        candidates.append( model->possibleParsings(p, Parsing::GuessStem) );
    }

    return candidates;
}

QList<Generation> Morphology::generateForms( const WritingSystem & ws, StemIdentityConstraint sic, MorphemeSequenceConstraint msc, const MorphologicalModel *model ) const
{
    QList<Generation> forms;

    parsingLog()->beginGenerate(ws, sic, msc, model);

    /// if no model is specified, try it with every model
    if( model == nullptr )
    {
        foreach(MorphologicalModel *mm,  mMorphologicalModels)
        {
            parsingLog()->beginModel(mm);

            Generation p( ws, mm );
            p.setStemIdentityConstraint( sic );
            p.setMorphemeSequenceConstraint( msc );
            forms.append( mm->generateForms(p) );

            parsingLog()->end(); /// beginModel
        }
    }
    /// otherwise use the model that was specified
    else
    {
        Generation p( ws, model );
        p.setStemIdentityConstraint( sic );
        p.setMorphemeSequenceConstraint( msc );
        forms.append( model->generateForms(p) );
    }

    parsingLog()->end(); /// beginGenerate

    return forms;
}

QList<Generation> Morphology::generateForms(const WritingSystem &ws, const LexicalStem &stem, const MorphemeSequence &morphemeSequence, const MorphologicalModel *model) const
{
    return generateForms( ws, StemIdentityConstraint(QList<LexicalStem>() << stem), MorphemeSequenceConstraint(morphemeSequence), model );
}

QList<Generation> Morphology::generateForms(const WritingSystem & ws, const Parsing &parsing) const
{
    QList<Generation> generations;
    if( parsing.isNull() )
    {
        return generations;
    }

    MorphemeSequenceConstraint msc;
    msc.setMorphemeSequence( parsing.morphemeSequence() );

    QList<LexicalStem> lss = parsing.lexicalStems();
    if( !lss.isEmpty() )
    {
        StemIdentityConstraint sic(lss);
        generations.append( generateForms( ws, sic, msc, parsing.morphologicalModel() ) );
    }

    return generations;
}

QList<Generation> Morphology::replaceStemInto(const Form &husk, const LexicalStem kernel, const WritingSystem & outputWs ) const
{
    QList<Generation> result;

    QList<Parsing> parsings = possibleParsings( husk );

    QListIterator<Parsing> oldParsingIterator(parsings);
    while( oldParsingIterator.hasNext() )
    {
        Parsing oldParsing = oldParsingIterator.next();

        MorphemeSequenceConstraint msc;
        msc.setMorphemeSequence( oldParsing.morphemeSequence() );

        StemIdentityConstraint sic;
        sic.addLexicalStem( kernel );

        result.append( generateForms( outputWs, sic, msc, oldParsing.morphologicalModel() ) );
    }

    return result;
}

QList<Generation> Morphology::transduceInto(const Form &form, const WritingSystem &newWs) const
{
    QList<Generation> result;

    QList<Parsing> parsings = possibleParsings( form );

    QListIterator<Parsing> oldParsingIterator(parsings);
    while( oldParsingIterator.hasNext() )
    {
        Parsing oldParsing = oldParsingIterator.next();

        MorphemeSequenceConstraint msc;
        msc.setMorphemeSequence( oldParsing.morphemeSequence() );

        QList<LexicalStem> lss = oldParsing.lexicalStems();
        if( !lss.isEmpty() )
        {
            StemIdentityConstraint sic(lss);
            result.append( generateForms( newWs, sic, msc, oldParsing.morphologicalModel() ) );
        }
    }

    return result;
}

Generation Morphology::getFirstTransduction(const Form &form, const WritingSystem &newWs) const
{
    QList<Parsing> parsings = possibleParsings( form, Parsing::OnlyOneResult );
    if( parsings.count() > 0 )
    {
        Parsing oldParsing = parsings.first();

        MorphemeSequenceConstraint msc;
        msc.setMorphemeSequence( oldParsing.morphemeSequence() );

        QList<LexicalStem> lss = oldParsing.lexicalStems();
        if( !lss.isEmpty() )
        {
            StemIdentityConstraint sic(lss);

            QList<Generation> gs = generateForms( newWs, sic, msc, oldParsing.morphologicalModel() );
            if( ! gs.isEmpty() )
            {
                return gs.first();
            }
        }
    }
    return Generation(newWs, nullptr);
}

Form Morphology::normalize(const Form &f) const
{
    InputNormalizer n = mNormalizationFunctions.value(f.writingSystem(), nullptr);
    if( n == nullptr )
        return f;
    else
        return Form( f.writingSystem(), n(f.text()) );
}

void Morphology::setNormalizationFunction(const WritingSystem &forWs, InputNormalizer n)
{
    mNormalizationFunctions[forWs] = n;
}

QSet<const AbstractStemList *> Morphology::getMatchingStemLists(const LexicalStem &stem) const
{
    QSet<const AbstractStemList *> result;
    QSetIterator<AbstractStemList*> iter(mStemAcceptingStemLists);
    while( iter.hasNext() )
    {
        AbstractStemList* asl = iter.next();
        if( asl->matchesForInsert(stem) )
        {
            result << asl;
        }
    }
    return result;
}

LexicalStemInsertResult Morphology::addLexicalStem(const LexicalStem &stem)
{
    LexicalStemInsertResult result;
    QSetIterator<AbstractStemList*> iter(mStemAcceptingStemLists);
    while( iter.hasNext() )
    {
        AbstractStemList* asl = iter.next();
        bool thisResult = asl->addStem( new LexicalStem(stem) );
        result.recordResult( asl, thisResult );
    }
    return result;
}

LexicalStemInsertResult Morphology::replaceLexicalStem(const LexicalStem &stem)
{
    LexicalStemInsertResult result;
    QSetIterator<AbstractStemList*> iter(mStemAcceptingStemLists);
    while( iter.hasNext() )
    {
        AbstractStemList* asl = iter.next();
        bool thisResult = asl->replaceStem( stem );
        result.recordResult( asl, thisResult );
    }
    return result;
}

LexicalStem * Morphology::getLexicalStem(qlonglong id) const
{
    LexicalStemInsertResult result;
    QSetIterator<AbstractStemList*> iter( mStemLists );
    while( iter.hasNext() )
    {
        AbstractStemList* asl = iter.next();
        LexicalStem * ls = asl->getStem( id );
        if( ls != nullptr )
        {
            return ls;
        }
    }
    return nullptr;
}

void Morphology::removeLexicalStem(qlonglong id)
{
    LexicalStemInsertResult result;
    QSetIterator<AbstractStemList*> iter(mStemAcceptingStemLists);
    while( iter.hasNext() )
    {
        AbstractStemList* asl = iter.next();
        asl->removeLexicalStem(id);
    }
}

void Morphology::printModelCheck(QTextStream &out) const
{
    MorphologyChecker checker(this);
    checker.printCheck(out);
}

AbstractNode *Morphology::getNodeFromId(const NodeId &id) const
{
    return mNodesById.value(id, nullptr);
}

QList<AbstractNode *> Morphology::getNodesFromLabel(const MorphemeLabel &label) const
{
    return mNodesByLabel.values(label);
}

QList<MorphemeNode *> Morphology::getMorphemeNodeFromLabel(const MorphemeLabel &label) const
{
    return mMorphemeNodesByLabel.values(label);
}

MorphologicalModel *Morphology::getMorphologicalModelFromLabel(const MorphemeLabel &label) const
{
    return mMorphologicalModelsByLabel.value(label);
}

QHash<QString, WritingSystem> Morphology::writingSystems() const
{
    return mWritingSystems;
}

void Morphology::setNodeId(const NodeId &id, AbstractNode *node)
{
    mNodesById.insert( id, node );
}

QList<LexicalStem *> Morphology::searchLexicalStems(const Form &formSearchString) const
{
    QRegularExpressionMatch finalIdMatch = endingStemId.match(formSearchString.text());
    if( finalIdMatch.hasMatch() )
    {
        qlonglong id = finalIdMatch.captured(1).toLongLong();
        LexicalStem * stem = getLexicalStem(id);
        if( stem != nullptr )
        {
            return QList<LexicalStem *>() << stem;
        }
    }

    /// handle tags: stem#tag1#tag2#tag3 etc.
    QStringList elements = formSearchString.text().split('#');

    Form stemForm = Form( formSearchString.writingSystem(), elements.first() ); /// if there is no #, this contains the whole original string
    QSet<Tag> containingTags;
    QSet<Tag> withoutTags;
    for(int i=1; i<elements.count(); i++)
    {
        if( elements.at(i).at(0) == '!' )
            withoutTags << Tag( elements.at(i).right( elements.at(i).length() - 1 ) );
        else
            containingTags << Tag( elements.at(i) );
    }

    return lexicalStems( stemForm, containingTags, withoutTags );
}

QList<LexicalStem *> Morphology::lexicalStems(const Form &form, const QSet<Tag> containingTags, const QSet<Tag> withoutTags) const
{
    QList<LexicalStem *> stems;

    QSetIterator<AbstractStemList*> iter(mStemLists);
    while( iter.hasNext() )
    {
        AbstractStemList* asl = iter.next();
        stems.append(  asl->stemsFromAllomorph( form, containingTags, withoutTags ) );
    }

    return stems;
}

QList<LexicalStem *> Morphology::lexicalStems(const Allomorph &allomorph, bool matchConstraints) const
{
    QList<LexicalStem *> stems;

    QSetIterator<AbstractStemList*> iter(mStemLists);
    while( iter.hasNext() )
    {
        AbstractStemList* asl = iter.next();
        stems.append(  asl->stemsFromAllomorph( allomorph, matchConstraints ) );
    }

    return stems;
}

LexicalStem *Morphology::uniqueLexicalStem(const Form &formSearchString) const
{
    QList<LexicalStem *> stems = searchLexicalStems(formSearchString);
    if( stems.count() == 0 )
    {
        const QString message =  "No match for search string: " + formSearchString.text();
        throw std::runtime_error( message.toUtf8() );
    }
    else if( stems.count() > 1 )
    {
        const QString message =  "Multiple stems match the string " + formSearchString.text() + ". Use tags to differentiate them.";
        throw std::runtime_error( message.toUtf8() );
    }
    else
    {
        return stems.first();
    }
}

QSet<AbstractStemList *> Morphology::stemLists() const
{
    return mStemLists;
}

bool Morphology::isOk() const
{
    return mIsOk;
}

void Morphology::setPath(const QString &path)
{
    QDir::setCurrent(path);
}

QSet<const AbstractNode *> Morphology::nodes() const
{
    QSet<const AbstractNode *> constNodeSet;
    for (AbstractNode *node : mNodes) {
        constNodeSet.insert(static_cast<const AbstractNode *>(node));
    }
    return constNodeSet;
}

QString Morphology::morphologyPath() const
{
    return mMorphologyPath;
}

const ParsingLog *Morphology::parsingLog() const
{
    if( mDebugOutput )
    {
        return mParsingLog;
    }
    else
    {
        return &NULL_PARSING_LOG;
    }
}

void Morphology::setDebugOutput(bool newDebugOutput)
{
    mDebugOutput = newDebugOutput;
}

bool Morphology::debugOutput() const
{
    return mDebugOutput;
}

bool Morphology::stemDebugOutput() const
{
    return mStemDebugOutput;
}

void Morphology::setStemDebugOutput(bool newStemDebugOutput)
{
    mStemDebugOutput = newStemDebugOutput;
}

QString Morphology::summary() const
{
    QString dbgString;
    Debug dbg(&dbgString);

    dbg << QString("Morphology (%1 model(s))").arg(morphologicalModels().count()) << Debug::endl << Debug::endl;
    dbg.indent();
    foreach( MorphologicalModel * mm, morphologicalModels() )
    {
        dbg << mm->summary(nullptr) << newline;
    }
    dbg.unindent();

    return dbgString;
}
