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
        eventNumber(0),
        isData(false),
        npv(0),
        mcWeight(-1)
        {}
    ~PEvent(){}
    
    int runNumber, lumiNumber;
    ULong64_t eventNumber;
    bool isData;
    int npv;
    float mcWeight;
    ClassDef(PEvent,1)
  };
}
#endif
