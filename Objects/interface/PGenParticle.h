#ifndef SCRAMJET_PGENPARTICLE
#define SCRAMJET_PGENPARTICLE

#include "PObject.h"


namespace scramjet
{
  class PGenParticle : public PObject
  {
    public:
      PGenParticle():
        PObject(),
        pdgid(0)
      {}
    ~PGenParticle(){}
    
    int pdgid;
    ClassDef(PGenParticle,1)
  };
}
#endif
