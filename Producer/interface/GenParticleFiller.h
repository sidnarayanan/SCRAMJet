#ifndef GenParticle_H
#define GenParticle_H

#include "SCRAMJet/Producer/interface/BaseFiller.h"
#include "SCRAMJet/Objects/interface/PGenParticle.h"

class GenParticleFiller : virtual public BaseFiller
{
    public:
        GenParticleFiller(TString n);
        ~GenParticleFiller();
        int analyze(const edm::Event& iEvent);
        virtual inline string name(){return "GenParticleFiller";};
        void init(TTree *t);

        edm::Handle<edm::View<pat::PackedGenParticle>> packed_handle;   
        edm::EDGetTokenT<edm::View<pat::PackedGenParticle> > packed_token;

        edm::Handle<edm::View<reco::GenParticle> > pruned_handle;
        edm::EDGetTokenT<edm::View<reco::GenParticle> > pruned_token;

        float minPt=5;

    private:
        scramjet::VGenParticle *data;
        TString treename;
};



#endif
