#ifndef SCRAMJET_PJET
#define SCRAMJET_PJET

#include "PObject.h"
#include "PPFCand.h"

namespace scramjet
{
  class PJet : public PObject
  {
    public:
      PJet():
        PObject(),
        rawPt(0),
        csv(-1),
        qgl(-1),
        pfcands(0),
        jetid(-1)
      {}
    ~PJet(){}
    
    float rawPt,csv,qgl;
    int *pfcands;
    int jetid;

    PPFCand *GetPFCand(unsigned int iPF, TClonesArray *cands) { return (PPFCand*) cands->At(pfcands[iPF]); }
    ClassDef(PJet,0)
  };
}
#endif
