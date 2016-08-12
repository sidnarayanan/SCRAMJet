#include "SCRAMJet/Producer/interface/Producer.h"
#include "SCRAMJet/Producer/interface/BaseFiller.h"
#include "SCRAMJet/Producer/interface/EventFiller.h"

using namespace scramjet;

Producer::Producer(const edm::ParameterSet& iConfig) 

{

    EventFiller *event = new EventFiller("event");
    event->gen_token   = consumes<GenEventInfoProduct>(iConfig.getParameter<edm::InputTag>("generator"));
    obj.push_back(event);

    /*
    // -- Before Leptons (mu uses Vtx)
    ProducerVertex *vtx = new ProducerVertex();
    vtx -> mOnlyMc = onlyMc;
    vtx -> token = consumes<reco::VertexCollection>(iConfig.getParameter<edm::InputTag>("vertices"));
    obj.push_back(vtx);

    // 
    ProducerPF *pf = new ProducerPF();
    pf -> token = consumes<pat::PackedCandidateCollection>(iConfig.getParameter<edm::InputTag>("pfCands"));
    obj.push_back (pf);


    //now do what ever initialization is needed
    ProducerJets *jets = new ProducerJets();
    jets -> mOnlyMc = onlyMc;
    jets -> token = consumes<pat::JetCollection>(iConfig.getParameter<edm::InputTag>("jets"));
    jets -> qg_token = consumes<edm::ValueMap<float>>(edm::InputTag("QGTagger", "qgLikelihood"));
    jets -> mMinPt = iConfig.getParameter<double>("minJetPt");
    jets -> mMinNjets = iConfig.getParameter<int>("minJetN");
    jets -> mMinEta = iConfig.getParameter<double>("minJetEta");
    jets -> mMinId = iConfig.getParameter<string>("minJetId");
    jets -> SetMatch( iConfig.getParameter<bool>("matchJet") );
    jets -> pf = pf;
    jets -> vtx = vtx;
    jets -> evt = evt;
    jets -> cachedPrefix = "";

    jets -> qg_token_Mult = consumes<edm::ValueMap<int>>(edm::InputTag("QGTagger", "mult"));
    jets -> qg_token_PtD = consumes<edm::ValueMap<float>>(edm::InputTag("QGTagger", "ptD"));
    jets -> qg_token_Axis2 = consumes<edm::ValueMap<float>>(edm::InputTag("QGTagger", "axis2"));

    obj.push_back(jets);

    bool doReclustering= iConfig.getParameter<bool>("doReclustering");
    
    ProducerPuppiJets *puppijets = new ProducerPuppiJets();
    puppijets -> mOnlyMc = onlyMc;
    puppijets -> token = consumes<pat::JetCollection>(iConfig.getParameter<edm::InputTag>("puppijets"));
    puppijets -> mMinPt = iConfig.getParameter<double>("minPuppiJetPt");
    puppijets -> mMinEta = iConfig.getParameter<double>("minPuppiJetEta");
    puppijets -> mMinNjets = iConfig.getParameter<int>("minPuppiJetN");
    puppijets -> mMinId = iConfig.getParameter<string>("minPuppiJetId");
    puppijets -> pf = pf;
    puppijets -> cachedPrefix = "";
    puppijets -> reclustered = doReclustering;
    puppijets -> rho_token = evt->rho_token;
    obj.push_back(puppijets);
    
    //--
    bool doAK8 = iConfig.getParameter<bool>("doAK8");
    bool doCA15 = iConfig.getParameter<bool>("doCA15");
    bool doPuppi = iConfig.getParameter<bool>("doPuppi");

    ProducerFatJets *chsAK8 = new ProducerFatJets();
    chsAK8 -> mRunJEC = false; // these jets are already corrected in MiniAOD
    chsAK8 -> mOnlyMc = onlyMc;
    chsAK8 -> token = consumes<pat::JetCollection>(iConfig.getParameter<edm::InputTag>("chsAK8"));
    chsAK8 -> rho_token = consumes<double>(iConfig.getParameter<edm::InputTag>("rho"));
    chsAK8 -> vertex_token = consumes<reco::VertexCollection>(iConfig.getParameter<edm::InputTag>("vertices"));
    chsAK8 -> mMinPt = iConfig.getParameter<double>("minAK8CHSPt");
    chsAK8 -> mMaxEta = iConfig.getParameter<double>("minAK8CHSEta");
    chsAK8 -> mMinId = iConfig.getParameter<string>("minAK8CHSId");
    chsAK8 -> cachedPrefix = iConfig.getParameter<string>("AK8CHSName");
    chsAK8 -> jetRadius = 0.8;
    chsAK8 -> subjets_token = mayConsume<reco::PFJetCollection>(edm::InputTag("PFJetsSoftDrop"+chsAK8 -> cachedPrefix ,"SubJets"));
    chsAK8 -> btags_token = mayConsume<reco::JetTagCollection>(edm::InputTag(chsAK8->cachedPrefix + "PFCombinedInclusiveSecondaryVertexV2BJetTags") ) ;
    obj.push_back(chsAK8);


    //--
    if (doReclustering){
        if (doAK8) {
           if (doPuppi) {
               ProducerPuppiFatJets *puppiAK8= new ProducerPuppiFatJets();
               puppiAK8 -> mOnlyMc = onlyMc;
               puppiAK8 -> token = consumes<pat::JetCollection>(iConfig.getParameter<edm::InputTag>("puppiAK8"));
               puppiAK8 -> rho_token = evt->rho_token;
               puppiAK8 -> mMinPt = iConfig.getParameter<double>("minAK8PuppiPt");
               puppiAK8 -> mMaxEta = iConfig.getParameter<double>("minAK8PuppiEta");
               puppiAK8 -> mMinId = iConfig.getParameter<string>("minAK8PuppiId");
               puppiAK8 -> cachedPrefix = iConfig.getParameter<string>("AK8PuppiName");
               puppiAK8 -> jetRadius = 0.8;
               puppiAK8 -> subjets_token = mayConsume<reco::PFJetCollection>(edm::InputTag("PFJetsSoftDrop"+puppiAK8 -> cachedPrefix ,"SubJets"));
               puppiAK8 -> btags_token = mayConsume<reco::JetTagCollection>(edm::InputTag(puppiAK8->cachedPrefix + "PFCombinedInclusiveSecondaryVertexV2BJetTags") ) ;
               obj.push_back(puppiAK8);
           }
        }

        if (doCA15) {

           if (doPuppi) {
               ProducerPuppiFatJets *puppiCA15= new ProducerPuppiFatJets();
               puppiCA15 -> mOnlyMc = onlyMc;
               puppiCA15 -> token = consumes<pat::JetCollection>(iConfig.getParameter<edm::InputTag>("puppiCA15"));
               puppiCA15 -> rho_token = evt->rho_token;
               puppiCA15 -> mMinPt = iConfig.getParameter<double>("minCA15PuppiPt");
               puppiCA15 -> mMaxEta = iConfig.getParameter<double>("minCA15PuppiEta");
               puppiCA15 -> mMinId = iConfig.getParameter<string>("minCA15PuppiId");
               puppiCA15 -> cachedPrefix = iConfig.getParameter<string>("CA15PuppiName");
               puppiCA15 -> jetRadius = 1.5;
               puppiCA15 -> subjets_token = mayConsume<reco::PFJetCollection>(edm::InputTag("PFJetsSoftDrop"+puppiCA15 -> cachedPrefix ,"SubJets"));
               puppiCA15 -> btags_token = mayConsume<reco::JetTagCollection>(edm::InputTag(puppiCA15->cachedPrefix + "PFCombinedInclusiveSecondaryVertexV2BJetTags") ) ;
               obj.push_back(puppiCA15);
           } else {
               ProducerFatJets *chsCA15 = new ProducerFatJets();
               chsCA15 -> mRunJEC = true; 
               chsCA15 -> mOnlyMc = onlyMc;
               chsCA15 -> token = consumes<pat::JetCollection>(iConfig.getParameter<edm::InputTag>("chsCA15"));
               chsCA15 -> rho_token = evt->rho_token;
               chsCA15 -> mMinPt = iConfig.getParameter<double>("minCA15CHSPt");
               chsCA15 -> mMaxEta = iConfig.getParameter<double>("minCA15CHSEta");
               chsCA15 -> mMinId = iConfig.getParameter<string>("minCA15CHSId");
               chsCA15 -> cachedPrefix = iConfig.getParameter<string>("CA15CHSName");
               chsCA15 -> jetRadius = 1.5;
               chsCA15 -> subjets_token = mayConsume<reco::PFJetCollection>(edm::InputTag("PFJetsSoftDrop"+chsCA15 -> cachedPrefix ,"SubJets"));
               chsCA15 -> btags_token = mayConsume<reco::JetTagCollection>(edm::InputTag(chsCA15->cachedPrefix + "PFCombinedInclusiveSecondaryVertexV2BJetTags") ) ;
               obj.push_back(chsCA15);

           }
        }
    }

    // --- 
    ProducerTaus *taus = new ProducerTaus();
    taus -> mOnlyMc = onlyMc;
    taus -> token = consumes<pat::TauCollection>(iConfig.getParameter<edm::InputTag>("taus"));
    taus -> mMinPt = iConfig.getParameter<double>("minTauPt");
    taus -> mMinNtaus = iConfig.getParameter<int>("minTauN");
    taus -> mMinEta = iConfig.getParameter<double>("minTauEta");
    taus -> mMinId = iConfig.getParameter<string>("minTauId");
    taus -> mMaxIso = iConfig.getParameter<double>("maxTauIso");
    taus -> SetExtend ( iConfig.getParameter<bool>("extendTau") );
    taus -> SetMatch( iConfig.getParameter<bool>("matchTau") );
    obj.push_back(taus);

    //--
    ProducerLeptons *leps = new ProducerLeptons();
    leps -> mOnlyMc = onlyMc;
    leps -> vtx_ = vtx; // Set the Vertex class
    leps -> evt_ = evt; // Set the Event class
    leps -> mu_token = consumes<pat::MuonCollection>(iConfig.getParameter<edm::InputTag>("muons"));
    leps -> el_token = consumes<pat::ElectronCollection>(iConfig.getParameter<edm::InputTag>("electrons"));
    leps -> el_vetoid_token = consumes<edm::ValueMap<bool> >(iConfig.getParameter<edm::InputTag>("eleVetoIdMap"));
    leps -> el_looseid_token = consumes<edm::ValueMap<bool> >(iConfig.getParameter<edm::InputTag>("eleLooseIdMap"));
    leps -> el_mediumid_token = consumes<edm::ValueMap<bool> >(iConfig.getParameter<edm::InputTag>("eleMediumIdMap"));
    leps -> el_tightid_token = consumes<edm::ValueMap<bool> >(iConfig.getParameter<edm::InputTag>("eleTightIdMap"));
    leps -> el_mva_token = consumes<edm::ValueMap<float> > (iConfig.getParameter<edm::InputTag>("eleMvaMap"));
    leps -> SetMatch( iConfig.getParameter<bool>("matchLep") );

    //leps -> el_iso_ch_token  = consumes<edm::ValueMap<float> >(iConfig.getParameter<edm::InputTag>("eleChargedIsolation") );
    //leps -> el_iso_nh_token  = consumes<edm::ValueMap<float> >(iConfig.getParameter<edm::InputTag>("eleNeutralHadronIsolation") );
    //leps -> el_iso_pho_token = consumes<edm::ValueMap<float> >(iConfig.getParameter<edm::InputTag>("elePhotonIsolation") );
    //
    leps -> mMinPt_mu = iConfig.getParameter<double>("minMuPt");
    leps -> mMinEta_mu = iConfig.getParameter<double>("minMuEta");
    leps -> mMaxIso_mu = iConfig.getParameter<double>("maxMuIso");

    leps -> mMinPt_el = iConfig.getParameter<double>("minElePt");
    leps -> mMinEta_el = iConfig.getParameter<double>("minEleEta");
    leps -> mMaxIso_el = iConfig.getParameter<double>("maxEleIso");

    leps -> mMinNleptons = iConfig.getParameter<int>("minLepN");
    leps -> SetMatch( iConfig.getParameter<bool>("matchLep") );


    obj. push_back(leps);

    //--

    ProducerMet *met = new ProducerMet();
    met -> mOnlyMc = onlyMc;
    met -> token = consumes<pat::METCollection>(iConfig.getParameter<edm::InputTag>("mets"));
    met -> pf = pf;
    if (doReclustering && doPuppi) {
        met -> rerunPuppi = true;
        met -> token_puppiRerun = consumes<reco::PFMETCollection>(iConfig.getParameter<edm::InputTag>("metsPuppi"));
        met -> token_puppiRerunUncorr = consumes<reco::PFMETCollection>(iConfig.getParameter<edm::InputTag>("metsPuppiUncorrected"));
    } else {
        met -> rerunPuppi = false;
        met -> token_puppi = consumes<pat::METCollection>(iConfig.getParameter<edm::InputTag>("metsPuppi"));
    }
    met -> SetExtend (iConfig.getParameter<bool>("extendMet"));
    obj.push_back(met);


    // --
    ProducerPhotons *phos = new ProducerPhotons();
    phos -> mOnlyMc = onlyMc;
    phos -> token = consumes<pat::PhotonCollection>(iConfig.getParameter<edm::InputTag>("photons"));
    phos -> loose_id_token = consumes<edm::ValueMap<bool>>(iConfig.getParameter<edm::InputTag>("phoLooseIdMap"));
    phos -> medium_id_token = consumes<edm::ValueMap<bool>>(iConfig.getParameter<edm::InputTag>("phoMediumIdMap"));
    phos -> tight_id_token = consumes<edm::ValueMap<bool>>(iConfig.getParameter<edm::InputTag>("phoTightIdMap"));
    phos -> iso_ch_token = consumes<edm::ValueMap<float>>(iConfig.getParameter<edm::InputTag>("phoChargedIsolation"));
    phos -> iso_nh_token = consumes<edm::ValueMap<float>>(iConfig.getParameter<edm::InputTag>("phoNeutralHadronIsolation"));
    phos -> iso_pho_token = consumes<edm::ValueMap<float>>(iConfig.getParameter<edm::InputTag>("phoPhotonIsolation"));
    phos -> iso_wch_token = consumes<edm::ValueMap<float>>(iConfig.getParameter<edm::InputTag>("phoWorstChargedIsolation"));
    phos -> ebRecHits_token = mayConsume<EcalRecHitCollection>(iConfig.getParameter<edm::InputTag>("ebRecHits"));
    phos -> eeRecHits_token = mayConsume<EcalRecHitCollection>(iConfig.getParameter<edm::InputTag>("eeRecHits"));
    phos -> mMinPt = iConfig.getParameter<double>("minPhoPt");
    phos -> mMaxIso = iConfig.getParameter<double>("maxPhoIso");
    phos -> mMinNpho = iConfig.getParameter<int>("minPhoN");
    phos -> mMaxEta = iConfig.getParameter<double>("minPhoEta");
    phos -> SetMatch( iConfig.getParameter<bool>("matchPho") );
    phos -> pf = pf;
    phos -> jets = jets;
    phos -> evt = evt;
    phos -> vtx = vtx;
    phos -> leps = leps;
    phos -> SetExtend (iConfig.getParameter<bool>("extendPhotons"));
    obj.push_back(phos);

    ProducerMonteCarlo *mc = new ProducerMonteCarlo();
    mc -> mOnlyMc = onlyMc;
    mc -> packed_token = consumes<edm::View<pat::PackedGenParticle> >(iConfig.getParameter<edm::InputTag>("packedgen"));
    mc -> pruned_token = consumes<edm::View<reco::GenParticle> >(iConfig.getParameter<edm::InputTag>("prunedgen")) ;
    mc -> info_token   = consumes<GenEventInfoProduct>(iConfig.getParameter<edm::InputTag>("generator"));
    mc -> lhe_token   = mayConsume<LHEEventProduct>(iConfig.getParameter<edm::InputTag>("lhe"));//LHEEventProduct_externalLHEProducer__LHE
    mc -> pu_token     = consumes<std::vector<PileupSummaryInfo> >(iConfig.getParameter<edm::InputTag>("pileup"));
    mc -> jet_token    = consumes<reco::GenJetCollection>(iConfig.getParameter<edm::InputTag>("genjets"));
    mc -> runinfo_token = consumes<GenRunInfoProduct,edm::InRun>(iConfig.getParameter<edm::InputTag>("genruninfo") );
    mc -> mMinGenParticlePt = iConfig.getParameter<double>("minGenParticlePt");
    mc -> mMinGenJetPt = iConfig.getParameter<double>("minGenJetPt");
    mc -> mParticleGun = iConfig.getUntrackedParameter<bool>("particleGun",false);

    obj.push_back(mc);
    runObj.push_back(mc);

    ProducerMatching *match = new ProducerMatching();
        match -> jets_ = jets;
        match -> leps_ = leps;
        match -> phos_ = phos;
        match -> taus_ = taus;
        match -> mc_ = mc;
        match -> mTauDr = iConfig.getParameter<double>("matchTauDr");
        match -> mJetDr = iConfig.getParameter<double>("matchJetDr");
        match -> mLepDr = iConfig.getParameter<double>("matchLepDr");
        match -> mPhoDr = iConfig.getParameter<double>("matchPhoDr");
    obj.push_back(match);

    ProducerTrigger *tr = new ProducerTrigger();
    tr -> mOnlyMc = onlyMc;
    tr -> token = consumes< edm::TriggerResults >( iConfig.getParameter<edm::InputTag>("trigger"));
    tr -> prescale_token = consumes<pat::PackedTriggerPrescales>( iConfig.getParameter<edm::InputTag>("prescales") );
    tr -> object_token = consumes< pat::TriggerObjectStandAloneCollection > ( iConfig.getParameter<edm::InputTag> ("objects") );
    // set the collection he needs to be awared of
    tr -> leps_ = leps;
    tr -> jets_ = jets;
    tr -> taus_ = taus;
    tr -> photons_ = phos;
    //
    *(tr -> triggerNames) =  iConfig.getParameter<std::vector<std::string> > ("triggerNames");
    obj.push_back(tr);

    // ----------------- Collection to be run at the Lumi Block ----
    ProducerAll *info = new ProducerAll();
    info -> mOnlyMc = onlyMc;
    info -> isSkim_ = 1;
    //info -> pu_token = consumes<std::vector<PileupSummaryInfo> >(edm::InputTag("addPileupInfo"));
    info -> info_token = consumes<GenEventInfoProduct>(edm::InputTag("generator"));
    info -> lhe_token   = mayConsume<LHEEventProduct>(iConfig.getParameter<edm::InputTag>("lhe"));//LHEEventProduct_externalLHEProducer__LHE
    info -> events_token = consumes<std::vector<long>,edm::InLumi>( edm::InputTag("InfoProducer","vecEvents") ) ;
    info -> weights_token = consumes<std::vector<float>,edm::InLumi>( edm::InputTag("InfoProducer","vecMcWeights") ) ;
    info -> putrue_token = consumes<std::vector<int>,edm::InLumi>( edm::InputTag("InfoProducer","vecPuTrueInt") ) ;
    obj.insert(obj.begin(),info);
    lumiObj.push_back(info);

    */

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
