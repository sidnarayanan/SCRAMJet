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
//        constituents(0),
        jetid(-1)
      {}
    ~PJet(){ }
    
    float rawPt,csv,qgl;
    std::vector<PPFCand> constituents;
    //TClonesArray *constituents;
    int jetid;

    ClassDef(PJet,1)
  };
}
#endif
