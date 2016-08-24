#include "fastjet/PseudoJet.hh"
#include <vector>
#include "TMath.h"
#include "TH2F.h"

TH2F *HeatMap(double jetEta, double jetPhi, std::vector<fastjet::PseudoJet> constituents, double radius, int nPhi=20, int nEta=20) {
  static int histo_number=0;
  TH2F *hmap = new TH2F(TString::Format("hmap_%i",histo_number),"hmap",nEta,-radius,radius,nPhi,-radius,radius);
  for (auto iconst : constituents) {
    float this_eta = iconst.eta()-jetEta; float this_phi = iconst.phi()-jetPhi;
    float this_temp = iconst.perp(); // e?
    hmap->Fill(this_eta,this_phi,this_temp);
  }
  histo_number++;
  return hmap;
}

