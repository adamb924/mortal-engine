#ifndef LEXICALSTEM_H
#define LEXICALSTEM_H

#include "mortal-engine_global.h"
#include "allomorph.h"

#include <QSet>

namespace ME {

class CreateAllomorphs;

class MORTAL_ENGINE_EXPORT LexicalStem
{
public:
    LexicalStem();

    explicit LexicalStem(const Allomorph &allomorph);
    LexicalStem(const LexicalStem & other);
    LexicalStem &operator=(const LexicalStem &other);

    bool operator==(const LexicalStem &other) const;
    bool operator==(qlonglong stemId) const;

    /**
     * @brief Returns a string representation of the Form for logging purposes.
     * 
     * @return QString The logging output.
     */
    QString summary() const;
    QString oneLineSummary() const;

    void insert(const Allomorph &allomorph);
    void remove(const Allomorph &allomorph);

    QListIterator<Allomorph> allomorphIterator() const;

    void generateAllomorphs(const CreateAllomorphs & ca);
    void generateAllomorphs(const QList<CreateAllomorphs> & cas);

    bool hasAllomorph(const Allomorph &allomorph, bool matchConstraints = true ) const;
    bool hasAllomorph(const Form & form, const QSet<Tag> containingTags = QSet<Tag>(), const QSet<Tag> withoutTags = QSet<Tag>(), bool includeDerivedAllomorphs = false) const;
    bool hasAllomorphWithForm(const Form & form ) const;

    QHash<WritingSystem, Form> glosses() const;
    bool hasGloss(const WritingSystem & ws) const;
    Form gloss(const WritingSystem & ws) const;
    void setGloss(const Form & gloss);

    qlonglong id() const;
    void setId(const qlonglong &id);

    bool isEmpty() const;

    int allomorphCount() const;

    QListIterator<Allomorph> allomorphs() const;

    QString liftGuid() const;
    void setLiftGuid(const QString &liftGuid);

    /// I don't like this, but I need a quick way to get a display form
    Allomorph displayAllomorph() const;

    void initializePortmanteaux(const AbstractNode * parent);
    QList<MorphemeSequence> portmanteaux(const WritingSystem & ws);

private:
    QList<Allomorph> mAllomorphs;
    QHash<WritingSystem,Form> mGlosses;
    qlonglong mId;
    QString mLiftGuid;
    Allomorph mOriginalAllomorph;
    QMultiHash<WritingSystem,MorphemeSequence> mPortmanteaux;
};

Q_DECL_EXPORT uint qHash(const ME::LexicalStem & key);

} // namespace ME

#endif // LEXICALSTEM_H
