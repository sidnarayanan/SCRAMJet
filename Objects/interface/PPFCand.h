#ifndef SCRAMJET_PPFCAND
#define SCRAMJET_PPFCAND

#include "PObject.h"


namespace scramjet
{
  class PPFCand : public PObject
  {
    public:
      PPFCand():
        PObject(),
        e(0),
        q(-2),
        weight(-1),
        pftype(-1)
      {}
    ~PPFCand(){}
    
    float e, q, weight;
    int pftype;
    ClassDef(PPFCand,0)
  };
}
#endif
