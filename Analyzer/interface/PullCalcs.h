
double DeltaPhi(double phi1, double phi2) {
  double dPhi = phi1-phi2;
  if (dPhi<-TMath::Pi())
    dPhi = 2*TMath::Pi()+dPhi;
  else if (dPhi>TMath::Pi())
    dPhi = -2*TMath::Pi()+dPhi;
  return dPhi;
}


TVector2 GetPull(fastjet::PseudoJet const &jet)
{
  double dEtaSum   = 0;
  double dPhiSum = 0;
  std::vector<fastjet::PseudoJet> constituents = jet.constituents();

  double jetEta=jet.eta(), jetPhi=jet.phi(), jetPt=jet.pt();

  for (auto&& constituent : constituents) {
    double dEta = constituent.eta()-jetEta;

    double dPhi = DeltaPhi(constituent.phi(),jetPhi);

    double weight = constituent.pt()*TMath::Sqrt(dEta*dEta + dPhi*dPhi);

    dEtaSum   += weight*dEta;
    dPhiSum += weight*dPhi;
  }

  return TVector2(dEtaSum/jet.pt(), dPhiSum/jet.pt());
}

//--------------------------------------------------------------------------------------------------
double GetPullAngle(fastjet::PseudoJet const &j1, fastjet::PseudoJet const &j2, TVector2 vPull)
{
  TVector2 vDir(j1.eta()-j2.eta(),DeltaPhi(j1.phi(),j2.phi()));
  double phiPull = vPull.DeltaPhi(vDir);
  return phiPull;
}
