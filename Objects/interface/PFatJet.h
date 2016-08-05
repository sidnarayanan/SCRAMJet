#ifndef SCRAMJET_PFATJET
#define SCRAMJET_PFATJET

#include "PObject.h"
#include "PPFCand.h"
#include "PJet.h"
#include <vector>

namespace scramjet
{
  class PFatJet : public PJet
  {
    public:
      PFatJet():
        PJet(),
        tau1(-1),
        tau2(-1),
        tau3(-1),
        mSD(-1),
        subjets(0)
      {}
    ~PFatJet() { }

    float tau1, tau2, tau3, mSD;
    std::vector<int> subjets;

    PJet *GetSubjet(unsigned int iSJ, TClonesArray *sjs) { return (PJet*) sjs->At(subjets[iSJ]); }

    ClassDef(PFatJet,1)
    
  };
}
#endif
