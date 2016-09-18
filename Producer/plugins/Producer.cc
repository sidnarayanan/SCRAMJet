#include "SCRAMJet/Producer/interface/Producer.h"
#include "SCRAMJet/Producer/interface/BaseFiller.h"
#include "SCRAMJet/Producer/interface/Skimmer.h"
#include "SCRAMJet/Producer/interface/EventFiller.h"
#include "SCRAMJet/Producer/interface/PFCandFiller.h"
#include "SCRAMJet/Producer/interface/JetFiller.h"
#include "SCRAMJet/Producer/interface/FatJetFiller.h"
#include "SCRAMJet/Producer/interface/GenParticleFiller.h"

using namespace scramjet;

Producer::Producer(const edm::ParameterSet& iConfig) 

{

    EventFiller *event = new EventFiller("event");
    event->gen_token   = consumes<GenEventInfoProduct>(iConfig.getParameter<edm::InputTag>("generator"));
    event->vtx_token   = mayConsume<reco::VertexCollection>(edm::InputTag("offlineSlimmedPrimaryVertices"));
    obj.push_back(event);

    Skimmer *skim         = new Skimmer("skimmer");
    skim->chsAK8_token    = consumes<pat::JetCollection>(iConfig.getParameter<edm::InputTag>("chsAK8"));
    skim->puppiAK8_token  = consumes<pat::JetCollection>(iConfig.getParameter<edm::InputTag>("puppiAK8"));
    skim->chsCA15_token   = consumes<pat::JetCollection>(iConfig.getParameter<edm::InputTag>("chsCA15"));
    skim->puppiCA15_token = consumes<pat::JetCollection>(iConfig.getParameter<edm::InputTag>("puppiCA15"));
    skim->skipEvent       = new bool(false);
    skim->minPt           = 200;
    skim->minMass         = 0;
    skim->maxEta          = 2.5;
    obj.push_back(skim);

    PFCandFiller *puppicands = new PFCandFiller("puppicands");
    puppicands->which_cand   = PFCandFiller::kRecoPF;
    puppicands->recopf_token = consumes<reco::PFCandidateCollection>(iConfig.getParameter<edm::InputTag>("puppiPFCands"));
    puppicands->skipEvent    = skim->skipEvent;
    obj.push_back(puppicands);
  
    PFCandFiller *pfcands = new PFCandFiller("pfcands");
    pfcands->pat_token    = consumes<pat::PackedCandidateCollection>(iConfig.getParameter<edm::InputTag>("chsPFCands"));
    //pfcands->reco_token   = consumes<reco::CandidateCollection>(iConfig.getParameter<edm::InputTag>("chsPFCands"));
    pfcands->which_cand   = PFCandFiller::kPat;
    pfcands->skipEvent    = skim->skipEvent;
    obj.push_back(pfcands);

    JetFiller *chsAK4     = new JetFiller("chsAK4");
    chsAK4->rho_token     = consumes<double>(iConfig.getParameter<edm::InputTag>("rho"));
    chsAK4->jet_token     = consumes<pat::JetCollection>(iConfig.getParameter<edm::InputTag>("chsAK4"));
    chsAK4->applyJEC      = false;
    chsAK4->minPt         = 15;
    chsAK4->skipEvent     = skim->skipEvent;
    obj.push_back(chsAK4);

    JetFiller *puppiAK4     = new JetFiller("puppiAK4");
    puppiAK4->rho_token     = consumes<double>(iConfig.getParameter<edm::InputTag>("rho"));
    puppiAK4->jet_token     = consumes<pat::JetCollection>(iConfig.getParameter<edm::InputTag>("puppiAK4"));
    puppiAK4->applyJEC      = true;
    puppiAK4->minPt         = 15;
    puppiAK4->skipEvent     = skim->skipEvent;
    obj.push_back(puppiAK4);

    FatJetFiller *chsAK8  = new FatJetFiller("chsAK8");
    chsAK8->rho_token     = consumes<double>(iConfig.getParameter<edm::InputTag>("rho"));
    chsAK8->jet_token     = consumes<pat::JetCollection>(iConfig.getParameter<edm::InputTag>("chsAK8"));
    chsAK8->subjets_token = mayConsume<reco::PFJetCollection>(edm::InputTag("PFJetsSoftDropchsAK8","SubJets"));
    chsAK8->btags_token   = mayConsume<reco::JetTagCollection>(edm::InputTag("chsAK8PFCombinedInclusiveSecondaryVertexV2BJetTags") ) ;
    chsAK8->qgl_token     = mayConsume<edm::ValueMap<float>>(edm::InputTag("chsAK8SubQGTag","qgLikelihood") ) ;
    chsAK8->jetRadius     = 0.8;
    chsAK8->skipEvent     = skim->skipEvent;
    chsAK8->pfcands       = pfcands;
    obj.push_back(chsAK8);

    FatJetFiller *puppiAK8  = new FatJetFiller("puppiAK8");
    puppiAK8->rho_token     = consumes<double>(iConfig.getParameter<edm::InputTag>("rho"));
    puppiAK8->jet_token     = consumes<pat::JetCollection>(iConfig.getParameter<edm::InputTag>("puppiAK8"));
    puppiAK8->subjets_token = mayConsume<reco::PFJetCollection>(edm::InputTag("PFJetsSoftDroppuppiAK8","SubJets"));
    puppiAK8->btags_token   = mayConsume<reco::JetTagCollection>(edm::InputTag("puppiAK8PFCombinedInclusiveSecondaryVertexV2BJetTags") ) ;
    puppiAK8->qgl_token     = mayConsume<edm::ValueMap<float>>(edm::InputTag("puppiAK8SubQGTag","qgLikelihood") ) ;
    puppiAK8->jetRadius     = 0.8;
    puppiAK8->skipEvent     = skim->skipEvent;
    puppiAK8->pfcands       = puppicands;
    obj.push_back(puppiAK8);

    FatJetFiller *chsCA15  = new FatJetFiller("chsCA15");
    chsCA15->rho_token     = consumes<double>(iConfig.getParameter<edm::InputTag>("rho"));
    chsCA15->jet_token     = consumes<pat::JetCollection>(iConfig.getParameter<edm::InputTag>("chsCA15"));
    chsCA15->subjets_token = mayConsume<reco::PFJetCollection>(edm::InputTag("PFJetsSoftDropchsCA15","SubJets"));
    chsCA15->btags_token   = mayConsume<reco::JetTagCollection>(edm::InputTag("chsCA15PFCombinedInclusiveSecondaryVertexV2BJetTags") ) ;
    chsCA15->qgl_token     = mayConsume<edm::ValueMap<float>>(edm::InputTag("chsCA15SubQGTag","qgLikelihood") ) ;
    chsCA15->jetRadius     = 1.5;
    chsCA15->skipEvent     = skim->skipEvent;
    chsCA15->pfcands       = pfcands;
    obj.push_back(chsCA15);

    FatJetFiller *puppiCA15  = new FatJetFiller("puppiCA15");
    puppiCA15->rho_token     = consumes<double>(iConfig.getParameter<edm::InputTag>("rho"));
    puppiCA15->jet_token     = consumes<pat::JetCollection>(iConfig.getParameter<edm::InputTag>("puppiCA15"));
    puppiCA15->subjets_token = mayConsume<reco::PFJetCollection>(edm::InputTag("PFJetsSoftDroppuppiCA15","SubJets"));
    puppiCA15->btags_token   = mayConsume<reco::JetTagCollection>(edm::InputTag("puppiCA15PFCombinedInclusiveSecondaryVertexV2BJetTags") ) ;
    puppiCA15->qgl_token     = mayConsume<edm::ValueMap<float>>(edm::InputTag("puppiCA15SubQGTag","qgLikelihood") ) ;
    puppiCA15->jetRadius     = 1.5;
    puppiCA15->skipEvent     = skim->skipEvent;
    puppiCA15->pfcands       = puppicands;
    obj.push_back(puppiCA15);

    GenParticleFiller *gen   = new GenParticleFiller("gen");
    gen->packed_token        = consumes<edm::View<pat::PackedGenParticle> >(iConfig.getParameter<edm::InputTag>("packedgen"));
    gen->pruned_token        = consumes<edm::View<reco::GenParticle> >(iConfig.getParameter<edm::InputTag>("prunedgen")) ;
    gen->skipEvent           = skim->skipEvent;
    obj.push_back(gen);

}


Producer::~Producer()
{
}


void Producer::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup)
{
    using namespace edm;

    for(auto o : obj)
    {
        if (o->analyze(iEvent, iSetup) ) return; 

    }


    tree_->Fill();

}


void Producer::beginJob()
{
    tree_    = fileService_ -> make<TTree>("events", "events");

    for(auto o : obj)
    {
        o->init(tree_);
    }

} 


void Producer::endJob() 
{
}


void Producer::beginRun(edm::Run const&iRun, edm::EventSetup const&)
{
}



void Producer::endRun(edm::Run const&iRun, edm::EventSetup const&iSetup)
{ 
}



void Producer::beginLuminosityBlock(edm::LuminosityBlock const&, edm::EventSetup const&)
{
}


void Producer::endLuminosityBlock(edm::LuminosityBlock const&iLumi, edm::EventSetup const&)
{
}


void Producer::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
    //The following says we do not know what parameters are allowed so do no validation
    // Please change this to state exactly what you do use, even if it is no parameters
    edm::ParameterSetDescription desc;
    desc.setUnknown();
    descriptions.addDefault(desc);
}

//define this as a plug-in
DEFINE_FWK_MODULE(Producer);
