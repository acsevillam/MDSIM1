#ifndef MD1_STAT_ACCUMULABLE_HH
#define MD1_STAT_ACCUMULABLE_HH

#include "G4StatDouble.hh"
#include "G4VAccumulable.hh"

class MD1StatAccumulable : public G4VAccumulable
{
  public:
    explicit MD1StatAccumulable(const G4String& name = G4String())
      : G4VAccumulable(name)
    {
      fStat.reset();
    }

    void Fill(G4double value)
    {
      fStat.fill(value);
    }

    const G4StatDouble& GetStat() const
    {
      return fStat;
    }

    void Merge(const G4VAccumulable& other) override
    {
      const auto& otherStatAccumulable = static_cast<const MD1StatAccumulable&>(other);
      fStat.add(&otherStatAccumulable.fStat);
    }

    void Reset() override
    {
      fStat.reset();
    }

  private:
    G4StatDouble fStat;
};

#endif
