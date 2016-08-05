#ifndef SCRAMJET_POBJECT
#define SCRAMJET_POBJECT

#include <TObject.h>
#include <TClonesArray.h>


namespace scramjet
{
  class PObject : public TObject
  {
    public:
      PObject():
        pt(0),
        eta(0),
        phi(0),
        m(0)
        {}
    ~PObject(){}
    
    float pt,eta, phi, m;
    ClassDef(PObject,1)
  };
}
#endif
