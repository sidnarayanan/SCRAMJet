#ifndef FatJet_H
#define FatJet_H

#include "SCRAMJet/Producer/interface/BaseFiller.h"
#include "SCRAMJet/Producer/interface/PFCandFiller.h"
#include "SCRAMJet/Objects/interface/PFatJet.h"

#include "CondFormats/JetMETObjects/interface/JetCorrectorParameters.h"
#include "CondFormats/JetMETObjects/interface/FactorizedJetCorrector.h"
#include "CondFormats/JetMETObjects/interface/JetCorrectionUncertainty.h"

#include <map>
#include <string>

class FatJetFiller : virtual public BaseFiller
{
    public:
        FatJetFiller(TString n);
        ~FatJetFiller();
        int analyze(const edm::Event& iEvent);
        virtual inline string name(){return "FatJetFiller";};
        void init(TTree *t);
        TString get_treename() { return treename; }

        edm::Handle<pat::JetCollection> jet_handle; 
        edm::EDGetTokenT<pat::JetCollection> jet_token;

        edm::Handle<double> rho_handle;
        edm::EDGetTokenT<double> rho_token;

        edm::Handle<reco::PFJetCollection> subjets_handle;
        edm::EDGetTokenT<reco::PFJetCollection> subjets_token;

        edm::Handle<reco::JetTagCollection> btags_handle;
        edm::EDGetTokenT<reco::JetTagCollection> btags_token;

        edm::Handle<edm::ValueMap<float>> qgl_handle;
        edm::EDGetTokenT<edm::ValueMap<float>> qgl_token;

        float minPt=180, maxEta=2.5;
        float jetRadius;

        PFCandFiller *pfcands=0; // pointer to the relevant pf cand filler, used to get a map

    private:
        // TClonesArray *data;
        scramjet::VFatJet *data;
        //scramjet::VJet    *subjet_data;
        TString treename;

        FactorizedJetCorrector *mMCJetCorrector;   // needed for puppi fat jets
        FactorizedJetCorrector *mDataJetCorrector; 

};



#endif
