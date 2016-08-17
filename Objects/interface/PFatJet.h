#ifndef SCRAMJET_PFATJET
#define SCRAMJET_PFATJET

#include "PObject.h"
#include "PPFCand.h"
#include "PJet.h"

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
        //firstSubjet(-1),
        //nSubjets(-1)
      { }
    ~PFatJet() { }

    float tau1, tau2, tau3, mSD;
    //std::vector<int> subjets;
    // TClonesArray *subjets=0;
    VJet *subjets;

    //int firstSubjet, nSubjets;
    //PJet *getSubjet(unsigned int iSJ, VJet *subjets) { return subjets->at(firstSubjet+iSJ); }

    ClassDef(PFatJet,1)
    
  };

  typedef std::vector<PFatJet*> VFatJet;
}
#endif
