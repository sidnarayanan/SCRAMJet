

TVector2 GetPull(fastjet::PseudoJet const &jet)
{
  double dYSum   = 0;
  double dPhiSum = 0;
  std::vector<fastjet::PseudoJet> constituents = jet.constituents();

  double jetY=jet.rapidity(), jetPhi=jet.phi(), jetPt=jet.pt();

  for (auto&& constituent : constituents) {
    double dY = constituent.rapidity()-jetY;

    double dPhi = SignedDeltaPhi(constituent.phi(),jetPhi);

    double weight = constituent.pt()*TMath::Sqrt(dY*dY + dPhi*dPhi);

    dYSum   += weight*dY;
    dPhiSum += weight*dPhi;
  }

  return TVector2(dYSum, dPhiSum);
}

double GetPullAlpha(fastjet::PseudoJet const &j1, fastjet::PseudoJet const &j2, TVector2 vPull)
{
  TVector2 vDir(j1.rapidity()-j2.rapidity(),SignedDeltaPhi(j1.phi(),j2.phi()));
  double phiPull = vPull.DeltaPhi(vDir);
  return phiPull;
}

double GetPullBeta(TVector2 vPull)
{
  if (vPull.X()>0) {
    TVector2 vBeam(1,0);
    return vPull.DeltaPhi(vBeam);
  } else {
    TVector2 vBeam(-1,0);
    return vPull.DeltaPhi(vBeam);
  }
}
