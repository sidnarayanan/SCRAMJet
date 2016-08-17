#ifndef Skimmer_H
#define Skimmer_H

#include "SCRAMJet/Producer/interface/BaseFiller.h"
#include "SCRAMJet/Objects/interface/PFatJet.h"

#include <map>
#include <string>

class Skimmer : virtual public BaseFiller
{
    public:
        Skimmer(TString n);
        ~Skimmer();
        int analyze(const edm::Event& iEvent);
        virtual inline string name(){return "Skimmer";};
        void init(TTree *t);

        edm::Handle<pat::JetCollection> chsAK8_handle; 
        edm::EDGetTokenT<pat::JetCollection> chsAK8_token;
        edm::Handle<pat::JetCollection> puppiAK8_handle; 
        edm::EDGetTokenT<pat::JetCollection> puppiAK8_token;
        edm::Handle<pat::JetCollection> chsCA15_handle; 
        edm::EDGetTokenT<pat::JetCollection> chsCA15_token;
        edm::Handle<pat::JetCollection> puppiCA15_handle; 
        edm::EDGetTokenT<pat::JetCollection> puppiCA15_token;

        float minPt=180, maxEta=2.5, minMass=0;

    private:
        TString treename;

};



#endif
