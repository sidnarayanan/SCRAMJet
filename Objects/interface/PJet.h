#ifndef SCRAMJET_PJET
#define SCRAMJET_PJET

#include "PObject.h"
#include "PPFCand.h"
#include <vector>

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
        constituents(0),
        jetid(-1)
      {  }
    ~PJet(){ delete constituents; }
    
    float rawPt,csv,qgl;
    // std::vector<PPFCand> constituents;
    // TClonesArray *constituents=0;
    //VPFCand *constituents;
    std::vector<UShort_t> *constituents;
    int jetid;

    PPFCand *getPFCand(UShort_t ipf, VPFCand *vpf) { return vpf->at(ipf); }

    ClassDef(PJet,1)
  };

  typedef std::vector<PJet*> VJet;
}
#endif
