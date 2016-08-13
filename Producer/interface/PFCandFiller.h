#ifndef PFCand_H
#define PFCand_H

#include "SCRAMJet/Producer/interface/BaseFiller.h"
#include "SCRAMJet/Objects/interface/PPFCand.h"

#include <map>
#include <string>

class PFCandFiller : virtual public BaseFiller
{
    public:
        PFCandFiller(TString n);
        ~PFCandFiller();
        int analyze(const edm::Event& iEvent);
        virtual inline string name(){return "PFCandFiller";};
        void init(TTree *t);

        edm::EDGetTokenT<pat::PackedCandidateCollection> pat_token;
        edm::Handle<pat::PackedCandidateCollection> pat_handle;

        edm::EDGetTokenT<reco::PFCandidateCollection> reco_token;
        edm::Handle<reco::PFCandidateCollection> reco_handle;
        
        bool useReco=true;

    private:
        TClonesArray *data;
        TString treename;
};



#endif
