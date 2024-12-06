#include "parsingsummary.h"

#include "parsing.h"
#include "parsingstep.h"
#include "nodes/abstractnode.h"

using namespace ME;

ParsingSummary::ParsingSummary(const SummaryType & stemType, const WritingSystem & stemWritingSystem, const SummaryType & affixType, const WritingSystem & affixWritingSystem, const QString & beforeMorpheme, const QString & afterMorpheme, const QString & betweenMorphemes) :
    mStemType(stemType),
    mStemWritingSystem(stemWritingSystem),
    mAffixType(affixType),
    mAffixWritingSystem(affixWritingSystem),
    mBeforeMorpheme(beforeMorpheme),
    mAfterMorpheme(afterMorpheme),
    mBetweenMorphemes(betweenMorphemes)
{

}

ParsingSummary ParsingSummary::labelSummary()
{
    return ParsingSummary(MorphemeLabelType, WritingSystem(), MorphemeLabelType, WritingSystem(), "[", "]", "");
}

ParsingSummary ParsingSummary::labelSummary(const WritingSystem &stemWritingSystem)
{
    return ParsingSummary(MorphemeLabelType, stemWritingSystem, MorphemeLabelType, WritingSystem(), "[", "]", "");
}

ParsingSummary ParsingSummary::morphemeDelimitedSummary(const WritingSystem &ws)
{
    return ParsingSummary(MorphemeFormType, ws, MorphemeFormType, ws, "", "", "-");
}

ParsingSummary ParsingSummary::hypotheticalStyleSummary(const WritingSystem &ws)
{
    return ParsingSummary(MorphemeFormType, ws, MorphemeLabelType, WritingSystem(), "[", "]", "");
}

ParsingSummary ParsingSummary::interlinearStyleSummary(const WritingSystem &ws)
{
    return ParsingSummary(MorphemeGlossType, ws, MorphemeGlossType, ws, "", "", "-");
}

QString ParsingSummary::summarize(const Parsing &parsing) const
{
    QString string;
    QListIterator<ParsingStep> iter(parsing.steps());
    while( iter.hasNext() )
    {
        const ParsingStep step = iter.next();
        const bool hasNext = iter.hasNext();
        string += summarize( parsing, step, hasNext );
    }
    return string;
}

QString ParsingSummary::summarize(const Parsing & parsing, const ParsingStep &step, bool hasFollowingMorphemes) const
{
    if( step.isStem() )
        return summarizeStem(parsing,step,hasFollowingMorphemes);
    else
        return summarizeAffix(parsing,step,hasFollowingMorphemes);
}

QString ParsingSummary::format(const MorphemeSequence &sequence, bool hasFollowingMorphemes) const
{
    QString str;
    QListIterator<MorphemeLabel> i(sequence);
    while(i.hasNext())
        str += format( i.next().toString(), i.hasNext() );
    if( hasFollowingMorphemes )
        str += mBetweenMorphemes;
    return str;
}

QString ParsingSummary::formatAffixPortmanteauWithGlosses(const Portmanteau &portmanteau, bool hasFollowingMorphemes) const
{
    QString str;
    QListIterator<const AbstractNode*> i(portmanteau.nodes());
    while(i.hasNext())
    {
        const AbstractNode* node = i.next();
        const bool hasNext = i.hasNext();
        str += format( node->gloss( mAffixWritingSystem ).text() , hasNext );
    }
    if( hasFollowingMorphemes )
        str += mBetweenMorphemes;
    return str;
}

QString ParsingSummary::format(const QString &portion, bool hasFollowingMorphemes) const
{
    QString str;
    str += mBeforeMorpheme + portion + mAfterMorpheme;
    if( hasFollowingMorphemes )
        str += mBetweenMorphemes;
    return str;
}

QString ParsingSummary::summarizeStem(const Parsing &parsing, const ParsingStep &step, bool hasFollowingMorphemes) const
{
    switch(mStemType)
    {
    case MorphemeFormType:
        return format( step.allomorph().form( mStemWritingSystem ).text(), hasFollowingMorphemes);
    case MorphemeGlossType:
        return format( step.lexicalStem().gloss( mStemWritingSystem ).text(), hasFollowingMorphemes);
    case MorphemeLabelType:
        if( step.allomorph().hasPortmanteau( parsing.writingSystem() ) )
            return format( step.allomorph().portmanteau().morphemes(), hasFollowingMorphemes );
        else
            return format(step.node()->label().toString(), hasFollowingMorphemes);
    }
    return QString();
}

QString ParsingSummary::summarizeAffix(const Parsing &parsing, const ParsingStep &step, bool hasFollowingMorphemes) const
{
    switch(mAffixType)
    {
    case MorphemeFormType:
        return format( step.allomorph().form( mAffixWritingSystem  ).text(), hasFollowingMorphemes );
    case MorphemeGlossType:
        if( step.allomorph().hasPortmanteau( parsing.writingSystem() ) )
            return formatAffixPortmanteauWithGlosses( step.allomorph().portmanteau(), hasFollowingMorphemes);
        else
            return format( step.node()->gloss( mAffixWritingSystem ).text(), hasFollowingMorphemes );
    case MorphemeLabelType:
        if( step.allomorph().hasPortmanteau( parsing.writingSystem() ) )
            return format( step.allomorph().portmanteau().morphemes(), hasFollowingMorphemes );
        else
            return format( step.node()->label().toString(), hasFollowingMorphemes);
    }
    return QString();
}
