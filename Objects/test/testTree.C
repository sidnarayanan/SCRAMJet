{
  gROOT->LoadMacro("${CMSSW_BASE}/src/SCRAMJet/Objects/interface/PPFCand.h");
  gROOT->LoadMacro("${CMSSW_BASE}/src/SCRAMJet/Objects/interface/PJet.h");
  gROOT->LoadMacro("${CMSSW_BASE}/src/SCRAMJet/Objects/interface/PFatJet.h");
  
  gSystem->Load("libSCRAMJetObjects.so");

}
