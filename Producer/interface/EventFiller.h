#ifndef EVENT_H
#define EVENT_H

#include "SCRAMJet/Producer/interface/BaseFiller.h"
#include "SCRAMJet/Objects/interface/PEvent.h"

#include <map>
#include <string>

class EventFiller : virtual public BaseFiller
{
    public:
        EventFiller(TString n);
        ~EventFiller();
        int analyze(const edm::Event& iEvent);
        virtual inline string name(){return "EventFiller";};
        void init(TTree *t);

        edm::EDGetTokenT<GenEventInfoProduct> gen_token;
        edm::Handle<GenEventInfoProduct> gen_handle;
        

    private:
        scramjet::PEvent *data;
        TString treename;
};



#endif
