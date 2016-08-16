#include "SCRAMJet/Producer/interface/EventFiller.h"
#include "SCRAMJet/Producer/interface/Producer.h"

using namespace scramjet;

EventFiller::EventFiller(TString n):
    BaseFiller()
{
  data = new PEvent();
  treename = n;
}

EventFiller::~EventFiller(){
  delete data;
}

void EventFiller::init(TTree *t) {
//  PEvent::Class()->IgnoreTObjectStreamer();
  t->Branch(treename.Data(),&data,99);
}

int EventFiller::analyze(const edm::Event& iEvent){
    data->runNumber     = iEvent.id().run();
    data->lumiNumber    = iEvent.luminosityBlock();
    data->eventNumber   = iEvent.id().event();
    data->isData        = iEvent.isRealData();

    if (!(data->isData)) {
      iEvent.getByToken(gen_token,gen_handle); 
      data->mcWeight = gen_handle->weight();
    }

    return 0;
}

