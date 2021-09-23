#include "form.h"

#include <QXmlStreamWriter>
#include <QRegularExpression>
#include <QDomElement>
#include <stdexcept>

#include "morphologyxmlreader.h"
#include "morphology.h"

QRegularExpression Form::whitespaceAndNonWordExpression("^[\\W\\s\\d]+$", QRegularExpression::UseUnicodePropertiesOption);

Form::Form() : mId(-1)
{
    calculateHash();
}

Form::Form(const WritingSystem &ws, const QString &text)
    : mWritingSystem(ws),
      mText(text),
      mId(-1)
{
    calculateHash();
}

Form::Form(const WritingSystem &ws, const QString &text, qlonglong id)
    : mWritingSystem(ws),
      mText(text),
      mId(id)
{
    calculateHash();
}

bool Form::operator==(const Form &other) const
{
    return mWritingSystem == other.writingSystem() && mText == other.text();
}

bool Form::operator!=(const Form &other) const
{
    return mWritingSystem != other.writingSystem() || mText != other.text();
}

Form Form::operator+=(const Form &other)
{
    if( other.writingSystem() == mWritingSystem )
        mText += other.mText;
    calculateHash();
    return *this;
}

Form Form::operator+=(const QString &other)
{
    mText += other;
    calculateHash();
    return *this;
}

WritingSystem Form::writingSystem() const
{
    return mWritingSystem;
}

QString Form::text() const
{
    return mText;
}

void Form::setText(const QString &text)
{
    mText = text;
    calculateHash();
}

Form Form::mid(int position, int n) const
{
    return Form(mWritingSystem, mText.mid(position, n) );
}

int Form::length() const
{
    return mText.length();
}

void Form::serialize(QXmlStreamWriter &out, const QString &elementName) const
{
    out.writeStartElement(elementName);
    out.writeAttribute("lang", mWritingSystem.abbreviation() );
    if( mId != -1 )
        out.writeAttribute("id", QString("%1").arg(mId) );
    out.writeCharacters( mText );
    out.writeEndElement(); /// \a elementName
}

void Form::serialize(QDomElement &out) const
{
    out.setAttribute("lang", mWritingSystem.abbreviation() );
    if( mId != -1 )
        out.setAttribute("id", QString("%1").arg(mId) );
    QDomText textNode = out.ownerDocument().createTextNode( mText );
    out.appendChild(textNode);
}

QDomElement Form::toElement(QDomDocument * document, const QString &elementName)
{
    QDomElement form = document->createElement(elementName);
    form.setAttribute("lang", mWritingSystem.abbreviation() );
    if( mId != -1 )
        form.setAttribute("id", QString("%1").arg(mId) );

    QDomText innerText = document->createTextNode(mText);
    form.appendChild(innerText);

    return form;
}

qlonglong Form::id() const
{
    return mId;
}

void Form::setId(const qlonglong &id)
{
    mId = id;
}

bool Form::isWhitespaceAndNonWordCharacters() const
{
    return whitespaceAndNonWordExpression.match(mText).hasMatch();
}

Form Form::readFromXml(QXmlStreamReader &in, const Morphology *morphology, const QString &elementName)
{
    Q_ASSERT( in.isStartElement() && in.name() == elementName );

    WritingSystem ws = morphology->writingSystems().value( in.attributes().value("lang").toString() );
    if( ws.isNull() )
    {
        std::string ln = std::to_string( in.lineNumber() );
        std::string lang = in.attributes().value("lang").toString().toUtf8().constData();
        throw std::runtime_error( "Null writing system on line" + ln + ": " + lang );
    }
    QString text = in.readElementText();

    /// get to the end element
    while( !in.isEndElement() )
        in.readNext();

    Q_ASSERT( in.isEndElement() && in.name() == elementName );
    return Form(ws, text);
}

Form Form::readFromXml(QDomElement element, const Morphology *morphology)
{
    WritingSystem ws = morphology->writingSystems().value( element.attribute("lang") );
    if( ws.isNull() )
    {
        qWarning() << "Form::readFromXml: Null writing system in DOM element: " << element.attribute("lang") << " Node name: " << element.nodeName() << ", Inner text: " << element.text();
    }
    return Form(ws, element.text());
}

QList<Form> Form::readListFromXml(QXmlStreamReader &in, const Morphology *morphology, const QString &untilElement, const QString &elementName)
{
    Q_ASSERT( in.isStartElement() && in.name() == untilElement );

    QList<Form> forms;
    while( !in.atEnd() && !(in.tokenType() == QXmlStreamReader::EndElement && in.name() == untilElement ) )
    {
        in.readNext();

        if( in.isStartElement() && in.name() == elementName )
        {
            forms << Form::readFromXml(in, morphology, elementName);
        }
    }

    Q_ASSERT( in.isEndElement() && in.name() == untilElement );
    return forms;
}

uint Form::hash() const
{
    return mHash;
}

void Form::calculateHash()
{
    mHash = mWritingSystem.hash() ^ qHash(mText);
}

QString Form::summary(const QString &label) const
{
    QString dbgString;
    QTextStream dbg(&dbgString);
    
    dbg << label + "(";
    dbg << "'" << mText << "\', " << mWritingSystem.abbreviation();
    dbg << ")";

    return dbgString;
}

uint qHash(const Form &key)
{
    return key.hash();
}
