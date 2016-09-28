#include "../interface/JetSmearer.h"

void JetSmearer::setSmear(int iBin) {
  fPtSmear->SetParameter(0,fPtPar0[iBin]);
  fPtSmear->SetParameter(1,fPtPar1[iBin]);
  fPtSmear->SetParameter(2,fPtPar2[iBin]);
  fPtSmear->SetParameter(3,fPtPar3[iBin]);
  fPhiSmear->SetParameter(0,fPhiPar0[iBin]);
  fPhiSmear->SetParameter(1,fPhiPar1[iBin]);
  fPhiSmear->SetParameter(2,fPhiPar2[iBin]);
  fPhiSmear->SetParameter(3,fPhiPar3[iBin]);
  fPhiSmear->SetParameter(4,fPhiPar4[iBin]);
  fEtaSmear->SetParameter(0,fEtaPar0[iBin]);
  fEtaSmear->SetParameter(1,fEtaPar1[iBin]);
  fEtaSmear->SetParameter(2,fEtaPar2[iBin]);
  fEtaSmear->SetParameter(3,fEtaPar3[iBin]);
  fEtaSmear->SetParameter(4,fEtaPar4[iBin]);
};




void JetSmearer::getUncertainties( TLorentzVector & v, double & upx, double & upy, double & upz, double & um ) 
{
  double pt  = v.Pt();
  double eta = v.Eta();
  double phi = v.Phi();
  
  int eta_bin = 9-int(fabs(eta*2));
  if( eta_bin < 0 ) eta_bin = 0.;
  setSmear(eta_bin);
  
  double uncert_pt  = fPtSmear->Eval(pt)*pt;
  double uncert_eta = fEtaSmear->Eval(pt);
  double uncert_phi = fPhiSmear->Eval(pt);
//  double uncert_m = uncert_pt;

  
  upx 
    = sqrt(TMath::Cos(phi)*uncert_pt*TMath::Cos(phi)*uncert_pt 
	   + TMath::Sin(phi)*pt*uncert_phi*TMath::Sin(phi)*pt*uncert_phi);
  upy 
    = sqrt(TMath::Sin(phi)*uncert_pt *TMath::Sin(phi)*uncert_pt 
	   + TMath::Cos(phi)*pt*uncert_phi*TMath::Cos(phi)*pt*uncert_phi);
  upz 
    = sqrt(TMath::SinH(eta)*uncert_pt*TMath::SinH(eta)*uncert_pt 
	   + TMath::CosH(eta)*pt*uncert_eta*TMath::CosH(eta)*pt*uncert_eta);
  um  = uncert_pt;
  //um  = sqrt(uncert_pt*uncert_pt + upz*upz);

  float inflation=2.0;
  upx *= inflation;
  upy *= inflation;
  upz *= inflation;
  um *= inflation;

};
