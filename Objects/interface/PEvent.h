#ifndef SCRAMJET_PEVENTINFO
#define SCRAMJET_PEVENTINFO

#include <TObject.h>
#include <TClonesArray.h>


namespace scramjet
{
  class PEvent : public TObject
  {
    public:
      PEvent():
        runNumber(0),
        lumiNumber(0),
        eventNumber(0)
        {}
    ~PEvent(){}
    
    int runNumber, lumiNumber;
    ULong64_t eventNumber;
    ClassDef(PEvent,1)
  };
}
#endif
