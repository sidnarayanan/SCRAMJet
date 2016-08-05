#include "SCRAMJet/Objects/src/SCRAMJetObjectsLinkDef.h"
//#include "SCRAMJet/Objects/interface/PEvent.h"
//#include "SCRAMJet/Objects/interface/PEvent.h"
//#include "SCRAMJet/Objects/interface/PPFCand.h"
//#include "SCRAMJet/Objects/interface/PJet.h"
//#include "SCRAMJet/Objects/interface/PFatJet.h"
#include <TSystem.h>
#include "TFile.h"
#include "TTree.h"

void testTree() {
  
  gSystem->Load("libSCRAMJetObjects.so");

  using namespace scramjet;

  PEvent::Class()->IgnoreTObjectStreamer();
  PPFCand::Class()->IgnoreTObjectStreamer();
  PJet::Class()->IgnoreTObjectStreamer();
  PFatJet::Class()->IgnoreTObjectStreamer();
  
  TFile *fTest = new TFile("test.root","RECREATE");
  TTree *t = new TTree("events","events");
  PEvent *event = new PEvent();
  TClonesArray *pfArray = new TClonesArray("scramjet::PPFCand",5000);
  TClonesArray *subjets = new TClonesArray("scramjet::PJet",100);
  TClonesArray *fatjets = new TClonesArray("scramjet::PFatJet",100);

  int counter=0;
  t->Branch("counter",&counter,"counter/I");
  t->Branch("event",&event,99);
  t->Branch("pfcandidates",&pfArray,99);
  t->Branch("subjets",&subjets,99);
  t->Branch("fatjets",&fatjets,99);

  event->eventNumber = 2; event->runNumber = 2, event->lumiNumber = 2;

  //pf cands
  new((*pfArray)[0]) PPFCand();
  PPFCand *cand = (PPFCand*) pfArray->At(0);
  cand->pt = 100; cand->eta=0; cand->phi=0; cand->m=10;

  new((*pfArray)[1]) PPFCand();
  cand = (PPFCand*) pfArray->At(1);
  cand->pt = 200; cand->eta=0; cand->phi=0; cand->m=10;

  // subjets
  new((*subjets)[0]) PJet();
  PJet *subjet = (PJet*)subjets->At(0);
  subjet->pt = 100; 
  subjet->pfcands.push_back(1);

  new((*subjets)[1]) PJet();
  subjet = (PJet*)subjets->At(1);
  subjet->pt = 200; 
  subjet->pfcands.push_back(0);

  //fatjets
  new((*fatjets)[0]) PFatJet();
  PFatJet *fatjet = (PFatJet*)fatjets->At(0);
  fatjet->pt = 300;
  fatjet->pfcands.push_back(0);
  fatjet->pfcands.push_back(1);
  fatjet->subjets.push_back(0);
  fatjet->subjets.push_back(1);

  t->Fill();
  
  subjets->Clear();
  fatjets->Clear();
  pfArray->Clear();

  fTest->WriteTObject(t,"events");
  fTest->Close();
}

TTree *getTree() {
  gSystem->Load("libSCRAMJetObjects.so");
  TFile *fTest = TFile::Open("test.root");
  TTree *t = (TTree*)fTest->Get("events");
//  t->Print();
  t->Scan();
  printf("%i\n",(int)t->GetEntries());
  return t;
}
  


