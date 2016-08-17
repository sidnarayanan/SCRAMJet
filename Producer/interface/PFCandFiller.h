#ifndef PFCand_H
#define PFCand_H

#include "SCRAMJet/Producer/interface/BaseFiller.h"
#include "SCRAMJet/Objects/interface/PPFCand.h"

#include <map>

class PFCandFiller : virtual public BaseFiller
{
    public:
        enum pfcandtype {
          kPat,
          kRecoPF,
          kReco
        };

        PFCandFiller(TString n);
        ~PFCandFiller();
        int analyze(const edm::Event& iEvent);
        virtual inline string name(){return "PFCandFiller";};
        void init(TTree *t);

        edm::EDGetTokenT<pat::PackedCandidateCollection> pat_token;
        edm::Handle<pat::PackedCandidateCollection> pat_handle;

        edm::EDGetTokenT<reco::PFCandidateCollection> recopf_token;
        edm::Handle<reco::PFCandidateCollection> recopf_handle;
        
        edm::EDGetTokenT<reco::CandidateCollection> reco_token;
        edm::Handle<reco::CandidateCollection> reco_handle;
        
        pfcandtype which_cand=kRecoPF;

        const std::map<const reco::Candidate*,UShort_t>& get_map() const { return candMap; }

    private:
        // TClonesArray *data;
        scramjet::VPFCand *data;
        TString treename;
        std::map<const reco::Candidate*,UShort_t> candMap; 

        void fillCand(const reco::Candidate*);

};



#endif
