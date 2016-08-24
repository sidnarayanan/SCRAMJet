#include "KinematicFitter.h"
#include "SCRAMJet/Objects/interface/PJet.h"

#include <TLorentzVector.h>
#include <string>

void PerformKinFit(KinematicFitter *fitter, FitResults *results, scramjet::PJet *j1, scramjet::PJet *j2, scramjet::PJet *jb) {
  TLorentzVector v1, v2, vb;
  v1.SetPtEtaPhiM(j1->pt,j1->eta,j1->phi,j1->m);
  v2.SetPtEtaPhiM(j2->pt,j2->eta,j2->phi,j2->m);
  vb.SetPtEtaPhiM(jb->pt,jb->eta,jb->phi,jb->m);

  TopCandidate::TopCandidateParticle j1_(v1, std::string("unmatched"), 3, 0);
  TopCandidate::TopCandidateParticle j2_(v2, std::string("unmatched"), 3, 0);
  TopCandidate::TopCandidateParticle jb_(vb, std::string("unmatched"), 3, 0);

  TopCandidate combo(j1_,j2_,jb_);
  combo.reset();

  fitter->fit(combo,*results);
}

