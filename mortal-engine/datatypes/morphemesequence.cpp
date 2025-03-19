#include "morphemesequence.h"

#include <QRegularExpression>

using namespace ME;

QString MorphemeSequence::STEM_LABEL = "Stem";
QRegularExpression MorphemeSequence::morphemeStringFormat = QRegularExpression("^(\\[.*\\])+$");

MorphemeSequence::MorphemeSequence()
{

}

MorphemeSequence::MorphemeSequence(const QList<MorphemeLabel> &list) : QList<MorphemeLabel>(list)
{
}

MorphemeSequence::MorphemeSequence(const MorphemeLabel &label)
{
    *this << label;
}

MorphemeSequence MorphemeSequence::operator<<(const MorphemeLabel &newLabel)
{
    append(newLabel);
    return *this;
}

MorphemeSequence MorphemeSequence::operator<<(const MorphemeSequence & newSequence)
{
    append(newSequence);
    return *this;
}

MorphemeSequence MorphemeSequence::operator+(const MorphemeLabel &newLabel) const
{
    return MorphemeSequence( *this ) << newLabel;
}

bool MorphemeSequence::isEmpty() const
{
    return length() == 0;
}

bool MorphemeSequence::contains(const MorphemeSequence &needle, int *at) const
{
    if( length() < needle.length() )
    {
        return false;
    }
    else if( length() == needle.length() )
    {
        for(int i=0; i<length(); i++)
        {
            if( this->at(i) != needle.at(i) )
                return false;
        }
        if( at != nullptr )
        {
            *at = 0;
        }
        return true;
    }
    else
    {
        for(int i=0; i< length() - needle.length() + 1; i++)
        {
            bool matches = true;
            for(int j=0; j<needle.length(); j++)
            {
                if( this->at(i+j) != needle.at(j) )
                {
                    matches = false;
                    break;
                }
            }
            if( matches )
            {
                if( at != nullptr )
                {
                    *at = i;
                }
                return true;
            }
        }
        return false;
    }
}
/*
qDebug() << "True:" << MorphemeSequence::fromString("[a][b][c]").contains( MorphemeSequence::fromString("[a][b][c]") );
qDebug() << "True:" << MorphemeSequence::fromString("[a][b][c]").contains( MorphemeSequence::fromString("[a][b]") );
qDebug() << "True:" << MorphemeSequence::fromString("[a][b][c]").contains( MorphemeSequence::fromString("[b][c]") );
qDebug() << "True:" << MorphemeSequence::fromString("[a][b][c]").contains( MorphemeSequence::fromString("[a]") );
qDebug() << "True:" << MorphemeSequence::fromString("[a][b][c]").contains( MorphemeSequence::fromString("[b]") );
qDebug() << "True:" << MorphemeSequence::fromString("[a][b][c]").contains( MorphemeSequence::fromString("[c]") );
qDebug() << "False:" << MorphemeSequence::fromString("[a][b][c]").contains( MorphemeSequence::fromString("[d][e][f]") );
qDebug() << "False:" << MorphemeSequence::fromString("[a][b][c]").contains( MorphemeSequence::fromString("[d][e]") );
qDebug() << "False:" << MorphemeSequence::fromString("[a][b][c]").contains( MorphemeSequence::fromString("[e][f]") );
qDebug() << "False:" << MorphemeSequence::fromString("[a][b][c]").contains( MorphemeSequence::fromString("[d]") );
qDebug() << "False:" << MorphemeSequence::fromString("[a][b][c]").contains( MorphemeSequence::fromString("[e]") );
qDebug() << "False:" << MorphemeSequence::fromString("[a][b][c]").contains( MorphemeSequence::fromString("[f]") );
*/

MorphemeSequence MorphemeSequence::replace(const MorphemeSequence &replaceThis, const MorphemeSequence &withThis) const
{
    int index = -1;
    if( contains(replaceThis, &index) )
    {
        Q_ASSERT( index != -1 );
        MorphemeSequence retVal;
        /// what comes before
        if( index > 0 )
        {
            retVal.append( mid(0, index) );
        }
        /// the replacement
        retVal.append( withThis );
        if( index + replaceThis.length() < length() )
        {
            retVal.append( mid( index + replaceThis.length() ) );
        }
        return retVal;
    }
    else
    {
        return *this;
    }
}

MorphemeSequence MorphemeSequence::mid(int pos, int length) const
{
    return MorphemeSequence( QList::mid(pos,length) );
}

bool MorphemeSequence::beginsWith(const MorphemeSequence &needle) const
{
    return length() <= needle.length()
           && needle == mid(0,needle.length());
}

/*
qDebug() << "Should be xyz" << MorphemeSequence::fromString("[a][b][c]").replace( MorphemeSequence::fromString("[a][b][c]"), MorphemeSequence::fromString("[x][y][z]") ).toString();
qDebug() << "Should be xyzdef" << MorphemeSequence::fromString("[a][b][c][d][e][f]").replace( MorphemeSequence::fromString("[a][b][c]"), MorphemeSequence::fromString("[x][y][z]") ).toString();
qDebug() << "Should be abcxyz" << MorphemeSequence::fromString("[a][b][c][d][e][f]").replace( MorphemeSequence::fromString("[d][e][f]"), MorphemeSequence::fromString("[x][y][z]") ).toString();
qDebug() << "Should be x" << MorphemeSequence::fromString("[a][b][c]").replace( MorphemeSequence::fromString("[a][b][c]"), MorphemeSequence::fromString("[x]") ).toString();
qDebug() << "Should be xdef" << MorphemeSequence::fromString("[a][b][c][d][e][f]").replace( MorphemeSequence::fromString("[a][b][c]"), MorphemeSequence::fromString("[x]") ).toString();
qDebug() << "Should be abcx" << MorphemeSequence::fromString("[a][b][c][d][e][f]").replace( MorphemeSequence::fromString("[d][e][f]"), MorphemeSequence::fromString("[x]") ).toString();
qDebug() << "Should be xy" << MorphemeSequence::fromString("[a][b][c]").replace( MorphemeSequence::fromString("[a][b][c]"), MorphemeSequence::fromString("[x][y]") ).toString();
qDebug() << "Should be xydef" << MorphemeSequence::fromString("[a][b][c][d][e][f]").replace( MorphemeSequence::fromString("[a][b][c]"), MorphemeSequence::fromString("[x][y]") ).toString();
qDebug() << "Should be abcxy" << MorphemeSequence::fromString("[a][b][c][d][e][f]").replace( MorphemeSequence::fromString("[d][e][f]"), MorphemeSequence::fromString("[x][y]") ).toString();
qDebug() << "Should be axyzef" << MorphemeSequence::fromString("[a][b][c][d][e][f]").replace( MorphemeSequence::fromString("[b][c][d]"), MorphemeSequence::fromString("[x][y][z]") ).toString();
qDebug() << "Should be abxyzf" << MorphemeSequence::fromString("[a][b][c][d][e][f]").replace( MorphemeSequence::fromString("[c][d][e]"), MorphemeSequence::fromString("[x][y][z]") ).toString();
 */
QString MorphemeSequence::toString() const
{
    QString retval;
    foreach(MorphemeLabel l, *this)
        retval += QString("[%1]").arg( l.toString() );
    return retval;
}

MorphemeSequence MorphemeSequence::fromString(const QString &str)
{
    Q_ASSERT(str.length() > 2);
    MorphemeSequence retval;
    /// ignore the opening and closing bracket
    QStringList labels = str.mid(1, str.length() - 2).split("][");
    foreach(QString label, labels)
        retval << MorphemeLabel(label);
    return retval;
}
