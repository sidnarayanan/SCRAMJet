#include "SCRAMJet/Producer/interface/Skimmer.h"
#include "SCRAMJet/Producer/interface/Producer.h"

using namespace scramjet;

Skimmer::Skimmer(TString n):
    BaseFiller()
{
  treename = n;
}

Skimmer::~Skimmer(){
}

void Skimmer::init(TTree *t) {
}

int Skimmer::analyze(const edm::Event& iEvent){
    bool skip=true;

    iEvent.getByToken(chsAK8_token, chsAK8_handle);
    iEvent.getByToken(puppiAK8_token, puppiAK8_handle);
    iEvent.getByToken(chsCA15_token, chsCA15_handle);
    iEvent.getByToken(puppiCA15_token, puppiCA15_handle);

    std::vector<edm::Handle<pat::JetCollection>> handles = {
                                                              chsAK8_handle,
                                                              puppiAK8_handle,
                                                              chsCA15_handle,
                                                              puppiCA15_handle
                                                            };
    std::vector<TString> names = {
                                    "chsAK8",
                                    "puppiAK8",
                                    "chsCA15",
                                    "puppiCA15"
                                  };

    unsigned int nTest = handles.size();

    for (unsigned int iT=0; iT!=nTest; ++iT) {
      if (!skip)
        break;
      edm::Handle<pat::JetCollection> jet_handle = handles.at(iT);
      TString name = names.at(iT);
      for (const pat::Jet &j : *jet_handle) {
        if (fabs(j.eta())<maxEta && j.pt()>minPt && j.userFloat(name+"SDKinematics:Mass")>minMass) {
          //fprintf(stderr,"found %f %f\n",j.pt(),j.eta());
          skip=false;
          break;
        }
      }
    }

    /*
    if (skip) {
      fprintf(stderr,"would have skipped this event!\n");
      skip=false;
    }
    */

    *skipEvent = skip;
  
    return 0;
}

