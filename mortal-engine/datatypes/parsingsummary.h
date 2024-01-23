#ifndef PARSINGSUMMARY_H
#define PARSINGSUMMARY_H

#include <QString>
#include "datatypes/portmanteau.h"
#include "datatypes/writingsystem.h"
#include "datatypes/morphemesequence.h"

class Parsing;
class ParsingStep;

#include "mortal-engine_global.h"

struct MORTAL_ENGINE_EXPORT ParsingSummary
{
public:
    enum SummaryType { MorphemeGlossType, MorphemeFormType, MorphemeLabelType };

    ParsingSummary(const SummaryType & stemType, const WritingSystem & stemWritingSystem, const SummaryType & affixType, const WritingSystem & affixWritingSystem, const QString & beforeMorpheme, const QString & afterMorpheme, const QString & betweenMorphemes);

    QString summarize(const Parsing & parsing) const;

    /// Some preset formats
    static ParsingSummary labelSummary();
    static ParsingSummary labelSummary(const WritingSystem & stemWritingSystem);
    static ParsingSummary morphemeDelimitedSummary( const WritingSystem & ws );
    static ParsingSummary hypotheticalStyleSummary( const WritingSystem & ws );
    static ParsingSummary interlinearStyleSummary( const WritingSystem & ws = WritingSystem() );
    static ParsingSummary glossSummary( const WritingSystem & ws );

private:
    QString summarize(const Parsing &parsing, const ParsingStep & step, bool hasFollowingMorphemes) const;
    QString format(const QString & portion, bool hasFollowingMorphemes) const;
    QString format(const MorphemeSequence & portmanteau, bool hasFollowingMorphemes) const;
    QString formatAffixPortmanteauWithGlosses(const Portmanteau & portmanteau, bool hasFollowingMorphemes) const;
    QString summarizeStem(const Parsing & parsing, const ParsingStep &step, bool hasFollowingMorphemes) const;
    QString summarizeAffix(const Parsing & parsing, const ParsingStep &step, bool hasFollowingMorphemes) const;

    SummaryType mStemType;
    WritingSystem mStemWritingSystem;
    SummaryType mAffixType;
    WritingSystem mAffixWritingSystem;
    QString mBeforeMorpheme;
    QString mAfterMorpheme;
    QString mBetweenMorphemes;
};

#endif // PARSINGSUMMARY_H
