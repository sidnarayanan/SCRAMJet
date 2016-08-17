#include "SCRAMJet/Producer/interface/PFCandFiller.h"
#include "SCRAMJet/Producer/interface/Producer.h"

using namespace scramjet;

PFCandFiller::PFCandFiller(TString n):
    BaseFiller()
{
  data = new VPFCand();
  treename = n;
}

PFCandFiller::~PFCandFiller(){
  delete data;
}

void PFCandFiller::init(TTree *t) {
//  PPFCand::Class()->IgnoreTObjectStreamer();
  t->Branch(treename.Data(),&data,2);
}

int PFCandFiller::analyze(const edm::Event& iEvent){
    for (auto d : *data)
      delete d;
    data->clear();

    candMap.clear();

    if (skipEvent!=0 && *skipEvent) {
      return 0;
    }

    if (useReco) {
      iEvent.getByToken(reco_token,reco_handle); 
      assert(reco_handle.isValid());

      const reco::PFCandidateCollection *pfCol = reco_handle.product();

      for (reco::PFCandidateCollection::const_iterator iPF=pfCol->begin();
            iPF!=pfCol->end(); ++iPF) {

        PPFCand *cand = new PPFCand();

        cand->pt = iPF->pt();
        cand->eta = iPF->eta();
        cand->phi = iPF->phi();
        cand->m = iPF->mass();
        cand->e = iPF->energy();
        cand->pftype = iPF->particleId();
        cand->q = iPF->charge();

        data->push_back(cand);

        candMap[&(*iPF)] = (UShort_t)(data->size()-1);
      }

    } else {
      iEvent.getByToken(pat_token,pat_handle);
      assert(pat_handle.isValid());

      const pat::PackedCandidateCollection *pfCol = pat_handle.product();

      for(pat::PackedCandidateCollection::const_iterator iPF = pfCol->begin(); 
            iPF!=pfCol->end(); ++iPF) {

        PPFCand *cand = new PPFCand();

        cand->pt = iPF->pt();
        cand->eta = iPF->eta();
        cand->phi = iPF->phi();
        cand->m = iPF->mass();
        cand->e = iPF->energy();
        cand->pftype = iPF->pdgId();
        cand->q = iPF->charge();

        data->push_back(cand);
      }
    }


    return 0;
}

