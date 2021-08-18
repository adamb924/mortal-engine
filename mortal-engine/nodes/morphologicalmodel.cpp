#include "morphologicalmodel.h"

#include "nodes/abstractnode.h"

#include "datatypes/generation.h"

#include "morphologyxmlreader.h"
#include <QXmlStreamReader>
#include "debug.h"

MorphologicalModel::MorphologicalModel() : AbstractPath(nullptr), mHasZeroLengthForms(false)
{

}

MorphologicalModel *MorphologicalModel::copy(MorphologyXmlReader *morphologyReader, const QString & idSuffix) const
{
    Q_UNUSED(morphologyReader)
    Q_UNUSED(idSuffix)
    qCritical() << "Don't try to copy an entire morphological model. The program is probably going to crash now.";
    return nullptr;
}

QString MorphologicalModel::elementName()
{
    return "model";
}

AbstractNode *MorphologicalModel::readFromXml(QXmlStreamReader &in, MorphologyXmlReader *morphologyReader, const MorphologicalModel * model)
{
    Q_UNUSED(model)
    Q_ASSERT( in.isStartElement() );
    MorphologicalModel * node = new MorphologicalModel();
    node->readInitialNodeAttributes(in, morphologyReader);

    /// move past the initial node
    in.readNextStartElement();

    node->setInitialNode( morphologyReader->readNodes( in, elementName(), node ) );

    Q_ASSERT( in.isEndElement() && in.name() == elementName() );
    return node;
}

bool MorphologicalModel::matchesElement(QXmlStreamReader &in)
{
    return in.isStartElement() && in.name() == elementName();
}

bool MorphologicalModel::checkHasOptionalCompletionPath() const
{
    return true;
}

bool MorphologicalModel::isModel() const
{
    return true;
}

bool MorphologicalModel::hasZeroLengthForms() const
{
    return mHasZeroLengthForms;
}

void MorphologicalModel::setHasZeroLengthForms(bool hasNullForms)
{
    mHasZeroLengthForms = hasNullForms;
}

QString MorphologicalModel::summary(const AbstractNode *doNotFollow) const
{
    QString dbgString;
    Debug dbg(&dbgString);
    Debug::atBeginning = false;

    dbg << QString("MorphologicalModel[%1] (").arg(label()) << Debug::endl << Debug::endl;
    dbg.indent();
    if( mInitialNode == nullptr )
    {
        dbg << "No initial node.";
    }
    else
    {
        const QString debugString = mInitialNode->summary(doNotFollow);
        dbg << debugString << newline;
    }
    dbg.unindent();
    dbg << ")" << newline;

    Debug::atBeginning = true;

    return dbgString;
}
