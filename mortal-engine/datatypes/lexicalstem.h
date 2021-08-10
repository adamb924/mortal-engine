#ifndef LEXICALSTEM_H
#define LEXICALSTEM_H

#include <QSet>
#include "allomorph.h"

class CreateAllomorphs;

#include "mortal-engine_global.h"

class MORTAL_ENGINE_EXPORT LexicalStem
{
public:
    LexicalStem();
    ~LexicalStem();

    explicit LexicalStem(const Allomorph &allomorph);
    LexicalStem(const LexicalStem & other);

    bool operator==(const LexicalStem &other) const;

    /**
     * @brief Returns a string representation of the Form for logging purposes.
     * 
     * @return QString The logging output.
     */
    QString summary() const;
    QString oneLineSummary() const;

    void insert(const Allomorph &allomorph);
    void remove(const Allomorph &allomorph);

    QSetIterator<Allomorph> allomorphIterator() const;

    void generateAllomorphs(const CreateAllomorphs & ca);
    void generateAllomorphs(const QList<CreateAllomorphs> & cas);

    bool hasAllomorph(const Allomorph &allomorph, bool matchConstraints = true ) const;
    bool hasAllomorph(const Form & form, const QSet<Tag> containing = QSet<Tag>() , bool includeDerivedAllomorphs = false) const;
    bool hasAllomorphWithForm(const Form & form ) const;

    QHash<WritingSystem, Form> glosses() const;
    bool hasGloss(const WritingSystem & ws) const;
    Form gloss(const WritingSystem & ws) const;
    void setGloss(const Form & gloss);

    qlonglong id() const;
    void setId(const qlonglong &id);

    bool isEmpty() const;

    int allomorphCount() const;

    QSet<Allomorph> allomorphs() const;

    QString liftGuid() const;
    void setLiftGuid(const QString &liftGuid);

private:
    QSet<Allomorph> mAllomorphs;
    QHash<WritingSystem,Form> mGlosses;
    qlonglong mId;
    QString mLiftGuid;
};

Q_DECL_EXPORT uint qHash(const LexicalStem & key);

#endif // LEXICALSTEM_H
