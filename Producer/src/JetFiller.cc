#include "SCRAMJet/Producer/interface/JetFiller.h"
#include "SCRAMJet/Producer/interface/Producer.h"

using namespace scramjet;

JetFiller::JetFiller(TString n):
    BaseFiller()
{
  // data = new TClonesArray("scramjet::PJet",100);
  data = new VJet();
  treename = n;
}

JetFiller::~JetFiller(){
  delete data;
}

void JetFiller::init(TTree *t) {
//  PJet::Class()->IgnoreTObjectStreamer();
  t->Branch(treename.Data(),&data);
  
  if (applyJEC) {
   std::string jecDir = "jec/";

   std::vector<JetCorrectorParameters> mcParams;
   mcParams.push_back(JetCorrectorParameters(jecDir + "Spring16_25nsV6_MC_L1FastJet_AK4PFPuppi.txt"));
   mcParams.push_back(JetCorrectorParameters(jecDir + "Spring16_25nsV6_MC_L2Relative_AK4PFPuppi.txt"));
   mcParams.push_back(JetCorrectorParameters(jecDir + "Spring16_25nsV6_MC_L3Absolute_AK4PFPuppi.txt"));
   mcParams.push_back(JetCorrectorParameters(jecDir + "Spring16_25nsV6_MC_L2L3Residual_AK4PFPuppi.txt"));
   mMCJetCorrector = new FactorizedJetCorrector(mcParams);
 
   std::vector<JetCorrectorParameters> dataParams;
   dataParams.push_back(JetCorrectorParameters(jecDir + "Spring16_25nsV6_DATA_L1FastJet_AK4PFPuppi.txt"));
   dataParams.push_back(JetCorrectorParameters(jecDir + "Spring16_25nsV6_DATA_L2Relative_AK4PFPuppi.txt"));
   dataParams.push_back(JetCorrectorParameters(jecDir + "Spring16_25nsV6_DATA_L3Absolute_AK4PFPuppi.txt"));
   dataParams.push_back(JetCorrectorParameters(jecDir + "Spring16_25nsV6_DATA_L2L3Residual_AK4PFPuppi.txt"));
   mDataJetCorrector = new FactorizedJetCorrector(dataParams);

  }
}

int JetFiller::analyze(const edm::Event& iEvent){
    // data->Clear();
    for (auto d : *data)
      delete d;
    data->clear(); 

    iEvent.getByToken(jet_token, jet_handle);
    if (applyJEC) 
      iEvent.getByToken(rho_token,rho_handle);

    FactorizedJetCorrector *corrector=0;
    if (applyJEC) 
      corrector = ( iEvent.isRealData() ) ? mDataJetCorrector : mMCJetCorrector;

    for (const pat::Jet& j : *jet_handle) {
      if (fabs(j.eta())>maxEta) continue;

      double this_pt = j.pt(), this_rawpt=0, jecFactor=1;
      if (applyJEC) {
        this_rawpt = this_pt;
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
      } else {
          this_rawpt = j.pt()*j.jecFactor("Uncorrected");
      }

      if (this_pt < minPt || this_rawpt < minPt) continue;

      // const int idx = data->GetEntries();
      // assert(idx<data->GetSize());

      // new((*data)[idx]) PJet();
      // PJet *jet = (PJet*)data->At(idx);

      PJet *jet = new PJet();

      jet->pt = this_pt;
      jet->rawPt = this_rawpt;
      jet->eta = j.eta();
      jet->phi = j.phi();
      jet->m = j.mass();
      jet->csv = j.bDiscriminator("pfCombinedInclusiveSecondaryVertexV2BJetTags");

      data->push_back(jet);

    }


    return 0;
}

