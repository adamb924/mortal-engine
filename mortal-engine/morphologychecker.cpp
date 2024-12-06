#include "morphologychecker.h"

#include "morphology.h"
#include "nodes/morphologicalmodel.h"
#include "nodes/morphemenode.h"

using namespace ME;

MorphologyChecker::MorphologyChecker(const Morphology *morphology) : mMorphology(morphology)
{

}

void MorphologyChecker::printCheck(QTextStream &out) const
{
    originalDuplicateCheck(out);
    allDuplicateCheck(out);
    originalConstraintCheck(out);
    allConstraintCheck(out);
    missingGlossCheck(out);
}

void MorphologyChecker::duplicateAllomorphFormCheck(QTextStream &out, bool originalsOnly) const
{
    int overallCount = 0;
    /// iterate over all morpheme nodes for this test
    foreach(MorphemeNode * node, mMorphology->mMorphemeNodes)
    {
        QList<Form> allForms;

        /// add all forms in the morpheme to the list
        QList<Allomorph> allomorphs = node->allomorphs();
        QListIterator<Allomorph> aIter(allomorphs);
        while( aIter.hasNext() )
        {
            const Allomorph a = aIter.next();
            if( !originalsOnly || a.type() == Allomorph::Original )
            {
                allForms.append( a.forms().values() );
            }
        }

        /// iterate over all of the unique forms of the list
        QSet<Form> uniqueForms( allForms.begin(), allForms.end() );
        QSetIterator<Form> list( uniqueForms );
        while( list.hasNext() )
        {
            Form f = list.next();
            int ct = allForms.count(f);
            if( ct > 1 )
            {
                out << node->label().toString() << ":\t" << ct << " duplicate allomorph forms: " << f.summary() << Qt::endl;
                overallCount++;
                continue;
            }
        }

    }
    if( overallCount == 0 )
    {
        out << "No problems!" << Qt::endl;
    }
    out << Qt::endl;
}

void MorphologyChecker::originalDuplicateCheck(QTextStream &out) const
{
    out << "===Duplicate allomorph form test (original allomorphs)===" << Qt::endl;
    duplicateAllomorphFormCheck(out, true);
}

void MorphologyChecker::allDuplicateCheck(QTextStream &out) const
{
    out << "===Duplicate allomorph form test (all allomorphs)===" << Qt::endl;
    duplicateAllomorphFormCheck(out, false);
}

void MorphologyChecker::allomorphConstraintCheck(QTextStream &out, bool originalsOnly) const
{
    int overallCount = 0;
    /// iterate over all morpheme nodes for this test
    foreach(MorphemeNode * node, mMorphology->mMorphemeNodes)
    {
        QList<Allomorph> allomorphs = node->allomorphs();
        for(int i=0; i < allomorphs.count(); i++)
        {
            for(int j=0; j < allomorphs.count(); j++)
            {
                if( i != j )
                {
                    if( !originalsOnly || ( allomorphs.at(i).type() == Allomorph::Original && allomorphs.at(j).type() == Allomorph::Original ) )
                    {
                        /// the obvious check is whether the constraints are a subset
                        bool isSubset = allomorphs.at(i).constraints().contains( allomorphs.at(j).constraints() );
                        /// if one form is a portmanteau, it doesn't matter if they have the same constraints; that could be legitimate
                        bool hasSamePortmanteau = allomorphs.at(i).portmanteau() == allomorphs.at(j).portmanteau();
                        /// we're also only interested if the allomorphs have forms for the same writing system; it might be that different writing systems have different allomorphic patterns
                        bool haveOverlappingWritingSystems = allomorphs.at(i).writingSystems().intersects( allomorphs.at(j).writingSystems() );
                        if( isSubset && hasSamePortmanteau && haveOverlappingWritingSystems)
                        {
                            out << node->label().toString() << ":\t" << " The constraints on the first allomorph below have the constraints of the second allomorph as a subset." << Qt::endl;
                            out << "\t" << allomorphs.at(i).summary() << Qt::endl;
                            out << "\t" << allomorphs.at(j).summary() << Qt::endl;

                            overallCount++;
                        }
                    }
                }
            }
        }
    }
    if( overallCount == 0 )
    {
        out << "No problems!" << Qt::endl;
    }
    out << Qt::endl;
}

void MorphologyChecker::originalConstraintCheck(QTextStream &out) const
{
    out << "===Allomorph constraint check (originals)===" << Qt::endl;
    allomorphConstraintCheck(out, true);
}

void MorphologyChecker::allConstraintCheck(QTextStream &out) const
{
    out << "===Allomorph constraint check (all)===" << Qt::endl;
    allomorphConstraintCheck(out, false);
}

void MorphologyChecker::missingGlossCheck(QTextStream &out) const
{
    out << "===Missing gloss check===" << Qt::endl;
    foreach( MorphemeNode * n, mMorphology->mMorphemeNodes )
    {
        if( n->glossCount() == 0 )
        {
            out << QObject::tr("No glosses. Label: %1, ID: %2, Model: %3").arg( n->label().toString(), n->id(), n->model()->label().toString() ) << Qt::endl;
        }
    }
}
