#include "SCRAMJet/Producer/interface/FatJetFiller.h"
#include "SCRAMJet/Producer/interface/Producer.h"

using namespace scramjet;

FatJetFiller::FatJetFiller(TString n):
    BaseFiller()
{
  data = new VFatJet();
  subjet_data = new VJet(); 
  // data = new TClonesArray("scramjet::PFatJet",100);
  treename = n;
}

FatJetFiller::~FatJetFiller(){
  delete data;
  delete subjet_data;
}

void FatJetFiller::init(TTree *t) {
  // PFatJet::Class()->IgnoreTObjectStreamer();
  t->Branch(treename.Data(),&data,99);
  t->Branch((treename+"Subjets").Data(),&subjet_data,99);
  std::string jecDir = "jec/";
 
  std::vector<JetCorrectorParameters> mcParams;
  mcParams.push_back(JetCorrectorParameters(jecDir + "Spring16_25nsV6_MC_L1FastJet_AK8PFPuppi.txt"));
  mcParams.push_back(JetCorrectorParameters(jecDir + "Spring16_25nsV6_MC_L2Relative_AK8PFPuppi.txt"));
  mcParams.push_back(JetCorrectorParameters(jecDir + "Spring16_25nsV6_MC_L3Absolute_AK8PFPuppi.txt"));
  mcParams.push_back(JetCorrectorParameters(jecDir + "Spring16_25nsV6_MC_L2L3Residual_AK8PFPuppi.txt"));
  mMCJetCorrector = new FactorizedJetCorrector(mcParams);
 
  std::vector<JetCorrectorParameters> dataParams;
  dataParams.push_back(JetCorrectorParameters(jecDir + "Spring16_25nsV6_DATA_L1FastJet_AK8PFPuppi.txt"));
  dataParams.push_back(JetCorrectorParameters(jecDir + "Spring16_25nsV6_DATA_L2Relative_AK8PFPuppi.txt"));
  dataParams.push_back(JetCorrectorParameters(jecDir + "Spring16_25nsV6_DATA_L3Absolute_AK8PFPuppi.txt"));
  dataParams.push_back(JetCorrectorParameters(jecDir + "Spring16_25nsV6_DATA_L2L3Residual_AK8PFPuppi.txt"));
  mDataJetCorrector = new FactorizedJetCorrector(dataParams);

}

int FatJetFiller::analyze(const edm::Event& iEvent){
    for (auto d : *data)
      delete d;
    data->clear();

    for (auto d : *subjet_data)
      delete d;
    subjet_data->clear();

    iEvent.getByToken(jet_token, jet_handle);
    iEvent.getByToken(rho_token,rho_handle);
    iEvent.getByToken(subjets_token,subjets_handle);
    iEvent.getByToken(btags_token,btags_handle);

    FactorizedJetCorrector *corrector=0;
    corrector = ( iEvent.isRealData() ) ? mDataJetCorrector : mMCJetCorrector;

    const reco::PFJetCollection *subjetCol = subjets_handle.product();

    int ijetRef=-1;
    for (const pat::Jet &j : *jet_handle) {

      ijetRef++;

      if (fabs(j.eta())>maxEta) continue;

      double this_pt = j.pt(), this_rawpt=j.pt(), jecFactor=1;
      if (fabs(j.eta())<5.191) {
        corrector->setJetPt(j.pt());
        corrector->setJetEta(j.eta());
        corrector->setJetPhi(j.phi());
        corrector->setJetE(j.energy());
        corrector->setRho(*rho_handle);
        corrector->setJetA(j.jetArea());
        corrector->setJetEMF(-99.0);
        jecFactor = corrector->getCorrection();
        this_pt *= jecFactor;
      }

      if (this_pt < minPt || this_rawpt < minPt) continue;

      // const int idx = data->GetEntries();
      // assert(idx<data->GetSize());

      // new((*data)[idx]) PFatJet();
      // PFatJet *jet = (PFatJet*)data->At(idx);

      PFatJet *jet = new PFatJet();

      jet->pt = this_pt;
      jet->rawPt = this_rawpt;
      jet->eta = j.eta();
      jet->phi = j.phi();
      jet->m = j.mass();
      
      jet->tau1 = j.userFloat(treename+"Njettiness:tau1");
      jet->tau2 = j.userFloat(treename+"Njettiness:tau2");
      jet->tau3 = j.userFloat(treename+"Njettiness:tau3");
      jet->mSD  = j.userFloat(treename+"SDKinematics:Mass");

      jet->firstSubjet = subjet_data->size();
      jet->nSubjets = 0;

      // jet->subjets = new TClonesArray("scramjet::PJet",5);
      // TClonesArray *subjets = jet->subjets;

      for (reco::PFJetCollection::const_iterator i = subjetCol->begin(); i!=subjetCol->end(); ++i) {

        if (reco::deltaR(i->eta(),i->phi(),j.eta(),j.phi())>jetRadius) 
          continue;

        jet->nSubjets++;

        // const int jdx = subjets->GetEntries();
        // new ((*subjets)[jdx]) PJet();
        // PJet *subjet = (PJet*)subjets->At(jdx);

        PJet *subjet = new PJet();

        subjet->pt = i->pt();
        subjet->eta = i->eta();
        subjet->phi = i->phi();
        subjet->m = i->mass();

        reco::JetBaseRef sjBaseRef(reco::PFJetRef(subjets_handle,i-subjetCol->begin()));
        subjet->csv = (float)(*(btags_handle.product()))[sjBaseRef];

        // std::vector<reco::PFCandidatePtr> constituentPtrs = i->getPFConstituents();
        // for (auto ptr : constituentPtrs) 
        // {
        //   const reco::PFCandidate *constituent = ptr.get();

        // subjet->constituents = new TClonesArray("scramjet::PPFCand",1000);
        // TClonesArray *constituents = subjet->constituents;

        subjet->constituents = new VPFCand();
        VPFCand *constituents = subjet->constituents;

        reco::Jet thisSubjet(*i);
        for (unsigned k=0; k < thisSubjet.getJetConstituents().size(); k++){
          const edm::Ptr<reco::Candidate> constituent = thisSubjet.getJetConstituents().at(k);

          // const int kdx = subjet->constituents->GetEntries();
          // if (kdx>=1000)
          //   break; // ???

          // new ((*constituents)[kdx]) PPFCand();
          // PPFCand *cand = (PPFCand*)constituents->At(kdx);

          PPFCand *cand = new PPFCand();

          cand->pt = constituent->pt();
          cand->eta = constituent->eta();
          cand->phi = constituent->phi();
          cand->m = constituent->mass();

          constituents->push_back(cand);

        }

        subjet_data->push_back(subjet);
        
      }

      data->push_back(jet);
  
    }


    return 0;
}

