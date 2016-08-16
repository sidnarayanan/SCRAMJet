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
        pdgid(0),
        parent(-1)
      {}
    ~PGenParticle(){}
    
    int pdgid, parent;
    ClassDef(PGenParticle,1)
  };

  typedef std::vector<PGenParticle*> VGenParticle;
}
#endif
