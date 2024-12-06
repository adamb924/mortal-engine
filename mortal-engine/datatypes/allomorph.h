/**
 * @file allomorph.h
 * @author Adam Baker (adam@adambaker.org)
 * @brief A class representing the allomorph of a morpheme. Instantiated in classes such as LexicalStem and MorphemeNode.
 * @version 0.1
 * @date 2020-10-26
 * 
 * @copyright Copyright (c) 2020
 * 
 */
#ifndef ALLOMORPH_H
#define ALLOMORPH_H

#include <QHash>
#include <QSet>

#include "form.h"
#include "writingsystem.h"
#include "tag.h"
#include "constraints/abstractconstraint.h"
#include "portmanteau.h"

#include "mortal-engine_global.h"

namespace ME {

class MORTAL_ENGINE_EXPORT Allomorph
{
public:
    /// Original allomorphs are created by user input, (e.g., database)
    /// Derived allomorphs are created by rules
    /// Hypothetical allomorphs are created by hypothesis (e.g., guessing a parsing)
    /// Null allomorphs are just that
    enum Type { Original, Derived, Hypothetical, Null };

    /**
     * @brief Construct a new Allomorph empty object of the given \a type
     * 
     * @param type The type of the Allomorph
     */
    explicit Allomorph(Allomorph::Type type);

    /**
     * @brief Construct a new Allomorph object of the given \a type, and add form \f to it.
     * 
     * @param f A form to be added to the Allomorph
     * @param type The type of the Allomorph
     */
    Allomorph(const Form & f, Allomorph::Type type);

    /**
     * @brief Construct a new Allomorph object from \a other; all data structures are copied.
     * 
     * @param other The Allomorph to be copied
     */
    Allomorph(const Allomorph & other);

    Allomorph &operator=(const Allomorph & other);

    /**
     * @brief Returns true if the other Allomorph has the same forms, tags, constraints, and type; otherwise returns false.
     * 
     * @param other the other Allomorph
     * @return true the other Allomorph has the same forms, tags, constraints, and type
     * @return false the other Allomorph does not have the same forms, tags, constraints, and type
     */
    bool operator==(const Allomorph & other) const;

    /**
    * @brief Sets the Form associated with \a f's WritingSystem
    * 
    * @param f the new Form
    */
    void setForm( const Form & f );

    /**
     * @brief Adds the given \a constraint to the allomorph. The Allomorph will never delete the \a constraint.
     * 
     * @param constraint 
     */
    void addConstraint(const AbstractConstraint * constraint);
    void addConstraints(const QSet<const AbstractConstraint *> &constraints);

    /**
     * @brief Adds the given \a tag to teh allomorph
     * 
     * @param tag the tag to add
     */
    void addTag( const QString & tag );
    void addTags(const QSet<Tag> tags);

    /**
     * @brief Removes the tags from the Allomorph.
     * 
     * @param tags the tags to remove
     */
    void removeTags( const QSet<Tag> & tags);


    /**
     * @brief Replace any tags with \a tags.
     * 
     * @param tags the new tags
     */
    void setTags(const QSet<Tag> tags);

    QSet<Tag> tags() const;

    QHash<WritingSystem, Form> forms() const;

    QSet<WritingSystem> writingSystems() const;

    /**
     * @brief Return the Form for the given WritingSystem. If no Form exists for the given WritingSystem,
     * an empty Form (with \a ws) is returned, and \a ok is set to false.
     * 
     * @param ws 
     * @param ok Set to false if the Allomorph does not have a form with the given WritingSystem
     * @return Form The requested form (see above)
     */
    Form form( const WritingSystem & ws, bool * ok = nullptr ) const;

    /**
     * @brief Remove all forms from the Allomorph.
     * 
     */
    void clearForms();

    /**
     * @brief Returns true if the Allomorph has a Form with the given WritingSystem.
     * 
     * @param ws the WritingSystem
     * @return true The Allomorph has a form for \a ws
     * @return false The Allomorph does not have a form for \a ws
     */
    bool hasForm( const WritingSystem & ws ) const;

    /**
     * @brief Returns true if the Allomorph has the specified Form; otherwise returns false.
     * 
     * @param form The Form
     * @return true The Allomorph has \a form
     * @return false The Allomorph does not have \a form
     */
    bool hasForm( const Form & form ) const; /// this override seems a little shady; different meanings...?

    /**
     * @brief Returns a string representation of the Form for logging purposes.
     * 
     * @return QString The logging output.
     */
    QString summary() const;
    QString oneLineSummary() const;

    /**
     * @brief Returns a brief summary of the Allomorph, showing the form only from the given WritingSystem. Cf. the QString operator.
     * 
     * @param ws The WritingSystem for which to show the form
     * @return QString A summary
     */
    QString focusedSummary(const WritingSystem & ws) const;

    /**
     * @brief Returns the Allomorphs's match condition constraints (i.e., AbstractConstraint::MatchCondition)
     * 
     * @return QSet<const AbstractConstraint *> the set of constraints
     */
    QSet<const AbstractConstraint *> matchConditions() const;

    /**
     * @brief Returns the Allomorphs's local constraints (i.e., AbstractConstraint::LocalConstraint)
     * 
     * @return QSet<const AbstractConstraint *> the set of constraints
     */
    QSet<const AbstractConstraint *> localConstraints() const;

    /**
     * @brief Returns the Allomorphs's long distance constraints (i.e., AbstractConstraint::LongDistanceConstraint)
     * 
     * @return QSet<const AbstractConstraint *> the set of constraints
     */
    QSet<const AbstractConstraint *> longDistanceConstraints() const;

    /**
     * @brief Returns all of the Allomorphs's constraints (of all types)
     * 
     * @return QSet<const AbstractConstraint *> the set of constraints
     */
    QSet<const AbstractConstraint *> constraints() const;

    /**
     * @brief Writes an XML representation of the object to the specified writer.
     * 
     * @param out The XML stream to be written to
     */
    void serialize(QXmlStreamWriter & out) const;
    void serialize(QDomElement & out) const;

    static Allomorph readFromXml(QXmlStreamReader &in, const Morphology *morphology, const QString & elementName = "allomorph");

    static Allomorph readFromXml(QDomElement element, const Morphology *morphology);

    /**
     * @brief Returns true if this is a hypothetical Allomorph rather than an actual allomorph.
     * 
     * @return true the Allomorph is hypothetical
     * @return false the Allomorph is not hypothetical
     */
    bool isHypothetical() const;

    bool isOriginal() const;

    /**
     * @brief Returns true if the Allomorph has no Form objects; otherwise returns false.
     * 
     * @return true the Allomorph has no Form objects
     * @return false the Allomorph has at least one Fork object
     */
    bool isEmpty() const;

    Allomorph::Type type() const;
    void setType(const Allomorph::Type &type);
    static QString typeToString(Allomorph::Type type);
    static Allomorph::Type typeFromString(const QString & type);

    Portmanteau portmanteau() const;
    Portmanteau &portmanteau();
    bool hasPortmanteau(const WritingSystem &ws) const;
    void setPortmanteau(const Portmanteau &portmanteau);

    static bool hasForm(const QSet<Allomorph> * allomorphs, const Form & f);

    qlonglong id() const;
    void setId(const qlonglong &id);

    bool hasZeroLengthForms() const;

    static QString XML_ALLOMORPH;
    static QString XML_FORM;
    static QString XML_TAG;
    static QString XML_STEM;
    static QString XML_USE_IN_GENERATIONS;
    static QString XML_TRUE;
    static QString XML_PORTMANTEAU;

    uint hash() const;

    bool useInGenerations() const;
    void setUseInGenerations(bool useInGenerations);

private:
    void calculateHash();

private:
    QHash<WritingSystem,Form> mForms;
    QSet<const AbstractConstraint *> mConstraints;
    QSet<Tag> mTags;
    Allomorph:: Type mType;
    Portmanteau mPortmanteau;
    qlonglong mId;
    uint mHash;
    bool mUseInGenerations;
};

Q_DECL_EXPORT uint qHash(const ME::Allomorph &key);

} // namespace ME

#endif // ALLOMORPH_H
