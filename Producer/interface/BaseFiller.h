#ifndef SCRAMJET_FILLER_H
#define SCRAMJET_FILLER_H

#include "SCRAMJet/Producer/interface/Includes.h"


#include <vector>
using namespace std;

class BaseFiller
{
    public:
        virtual int  analyze(const edm::Event &) = 0 ;
        virtual int  analyze(const edm::Event &iEvent,const edm::EventSetup& iSetup) { return analyze(iEvent) ; } ;
        virtual inline string name(){return "BaseFiller";};
        virtual void init(TTree *t) = 0;
};
#endif
