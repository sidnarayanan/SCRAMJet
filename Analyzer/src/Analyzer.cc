#include "../interface/Analyzer.h"
#include "TVector2.h"
#include "TMath.h"
#include <algorithm>
#include <vector>
#include "TStopwatch.h"

#define DEBUG 0
using namespace scramjet;
using namespace std;

Analyzer::Analyzer() {
  // load bare minumum objects to read
  event = new PEvent();
  gen = new VGenParticle();
}

Analyzer::~Analyzer() {
  delete event;
  delete gen;

  for (auto *a : anajets) {
    delete a->injets;
    delete a->outjet;
    delete a;
  }
  anajets.clear();

  for (auto *a : anafatjets) {
    delete a->injets;
    delete a->outjet;
    delete a;
  }
  anafatjets.clear();

  delete chs;
  delete puppi;
}

void Analyzer::ResetBranches() {
  genObjects.clear();
  mcWeight=-1; 
  runNumber=-1; 
  eventNumber=-1; 
  lumiNumber=-1; 

  for (auto *a : anajets) {
    a->outjet->reset();
  }

  for (auto *a : anafatjets) {
    a->outjet->reset();
  }
}

void Analyzer::SetOutputFile(TString fOutName) {
  fOut = new TFile(fOutName,"RECREATE");
}

void Analyzer::AddEventBranches(TTree *tout) {
  tout->Branch("mcWeight",&mcWeight,"mcWeight/F");
  tout->Branch("runNumber",&runNumber,"runNumber/I");
  tout->Branch("eventNumber",&eventNumber,"eventNumber/l");
  tout->Branch("lumiNumber",&lumiNumber,"lumiNumber/I");
}


void Analyzer::Init(TTree *t)
{
  if (!t) return;
  tIn = t;
  t->SetBranchAddress("event",&event);
  if (!isData)
    t->SetBranchAddress("gen",&gen); 
}

void Analyzer::AddJetFromTree(TString inName, TString outName) {
  if (!tIn) {
    PError("[SCRAMJetAnalyzer::AddJetFromTree]","Input tree not set!");
  }
  AnaJet *anajet = new AnaJet();
  
  anajet->injets = 0; // new VJet();
  tIn->SetBranchAddress(inName,&anajet->injets);

  fOut->cd();
  TTree *outtree = new TTree(outName.Data(),outName.Data());
  anajet->outtree = outtree;
  AddEventBranches(outtree);

  JetWriter *outjet = new JetWriter(outtree);
  anajet->outjet = outjet;

  anajets.push_back(anajet);
}

void Analyzer::AddFatJetFromTree(TString inName, TString outName,PileupAlgo pu, double radius, ClusterAlgo algo) {
  if (!tIn) {
    PError("[SCRAMJetAnalyzer::AddFatJetFromTree]","Input tree not set!");
  }
  AnaFatJet *anafatjet = new AnaFatJet();
  
  anafatjet->injets =0; //= new VFatJet();
  tIn->SetBranchAddress(inName,&anafatjet->injets);

  fOut->cd();
  TTree *outtree = new TTree(outName.Data(),outName.Data());
  anafatjet->outtree = outtree;
  AddEventBranches(outtree);

  FatJetWriter *outjet = new FatJetWriter(outtree);
  anafatjet->outjet = outjet;

  anafatjet->radius = radius;
  anafatjet->algo = algo;
  anafatjet->pfcands = puppi;

  anafatjets.push_back(anafatjet);
}

PGenParticle *Analyzer::Match(double eta, double phi, double radius) {
  PGenParticle *found=NULL;
  double r2 = radius*radius;

  unsigned int counter=0;
  for (map<PGenParticle*,float>::iterator iG=genObjects.begin();
        iG!=genObjects.end(); ++iG) {
    if (found!=NULL)
      break;
    if (DeltaR2(eta,phi,iG->first->eta,iG->first->phi)<r2) 
      found = iG->first;
  }

  return found;
}

void Analyzer::Terminate() {
  for (auto *a : anajets) {
    fOut->WriteTObject(a->outtree);
  }
  for (auto *a : anafatjets) {
    fOut->WriteTObject(a->outtree);
  }

  fOut->Close();
//  delete ak8jec;
//  delete ak8unc;
}

void Analyzer::Run() {

  unsigned int nEvents = tIn->GetEntries();
  if (maxEvents>=0 && maxEvents<(int)nEvents)
    nEvents = maxEvents;

  if (!fOut || !tIn) {
    PError("SCRAMJetAnalyzer::Run","NOT SETUP CORRECTLY");
    exit(1);
  }

  TStopwatch *sw = 0;
  if (DEBUG) sw = new TStopwatch();
  unsigned int iE=0;
  ProgressReporter pr("SCRAMJetAnalyzer::Run",&iE,&nEvents,10);

  for (iE=0; iE!=nEvents; ++iE) {
    if (DEBUG) sw->Start(true);
    pr.Report();
    ResetBranches();
    tIn->GetEntry(iE);
    if (DEBUG) { PDebug("SCRAMJetAnalyzer::Run",TString::Format("-1: %f",sw->RealTime()*1000)); sw->Start(); }

    // event info
    mcWeight = event->mcWeight;
    runNumber = event->runNumber;
    lumiNumber = event->lumiNumber;
    eventNumber = event->eventNumber;

    if (DEBUG) { PDebug("SCRAMJetAnalyzer::Run",TString::Format(" 0: %f",sw->RealTime()*1000)); sw->Start(); }

    // identify interesting gen particles
    if (processType!=kQCD && processType!=kNone) {
      std::vector<int> targets;

      int nGen = gen->size();
      for (int iG=0; iG!=nGen; ++iG) {
        PGenParticle *part = gen->at(iG);
        int pdgid = part->pdgid;
        unsigned int abspdgid = TMath::Abs(pdgid);
        bool good= (  (processType==kTop && abspdgid==6) ||
                      (processType==kV && (abspdgid==23 || abspdgid==24)) ||
                      (processType==kH && (abspdgid==25))
                    );
        if (good)
          targets.push_back(iG);
      } //looking for targets

      PDebug("SCRAMJetAnalyzer::Run",TString::Format("Found %lu tops",targets.size()));

      for (int iG : targets) {
        PGenParticle *part = gen->at(iG);

        // check there is no further copy:
        bool isLastCopy=true;
        for (int jG : targets) {
          if (gen->at(jG)->parent==iG) {
            isLastCopy=false;
            break;
          }
        }
        if (!isLastCopy)
          continue;
        
        // (a) check it is a hadronic decay and if so, (b) calculate the size
        if (processType==kTop) {
          
          // first look for a W whose parent is the top at iG, or a W further down the chain
          int iW=-1;
          for (int jG=0; jG!=nGen; ++jG) {
            PGenParticle *partW = gen->at(jG);
            if (TMath::Abs(partW->pdgid)==24 && partW->pdgid*part->pdgid>0) {
              // it's a W and has the same sign as the top
              if (iW<0 && partW->parent==iG) {
                iW=jG;
              } else if (iW>=0 && partW->parent==iW) {
                iW=jG;
              }
            }
          } // looking for W
          // PGenParticle *genW = gen->at(iW);

          // now look for b or W->qq
          int iB=-1, iQ1=-1, iQ2=-1;
          if (iW<0) // ???
            continue;
          double size=0;
          for (int jG=0; jG!=nGen; ++jG) {
            PGenParticle *partQ = gen->at(jG);
            int pdgidQ = partQ->pdgid;
            unsigned int abspdgidQ = TMath::Abs(pdgidQ);
            if (abspdgidQ>5)
              continue;
            if (abspdgidQ==5 && iB<0 && partQ->parent==iG) {
              // only keep first copy
              iB = jG;
              size = TMath::Max(DeltaR2(part->eta,part->phi,partQ->eta,partQ->phi),size);
            } else if (abspdgidQ<5 && partQ->parent==iW) {
              if (iQ1<0) {
                iQ1 = jG;
                size = TMath::Max(DeltaR2(part->eta,part->phi,partQ->eta,partQ->phi),size);
              } else if (iQ2<0) {
                iQ2 = jG;
                size = TMath::Max(DeltaR2(part->eta,part->phi,partQ->eta,partQ->phi),size);
              }
            }
            if (iB>=0 && iQ1>=0 && iQ2>=0)
              break;
          } // looking for quarks

          bool isHadronic = (iB>=0 && iQ1>=0 && iQ2>=0); // all 3 quarks were found

          // add to collection
          if (isHadronic)
            genObjects[part] = size;

        } else { // these are W,Z,H - 2 prong decays

          int iQ1=-1, iQ2=-1;
          double size=0;
          for (int jG=0; jG!=nGen; ++jG) {
            PGenParticle *partQ = gen->at(jG);
            int pdgidQ = partQ->pdgid;
            unsigned int abspdgidQ = TMath::Abs(pdgidQ);
            if (abspdgidQ>5)
              continue; 
            if (partQ->parent==iG) {
              if (iQ1<0) {
                iQ1=jG;
                size = TMath::Max(DeltaR2(part->eta,part->phi,partQ->eta,partQ->phi),size);
              } else if (iQ2<0) {
                iQ2=jG;
                size = TMath::Max(DeltaR2(part->eta,part->phi,partQ->eta,partQ->phi),size);
              }
            }
            if (iQ1>=0 && iQ2>=0)
              break;
          } // looking for quarks

          bool isHadronic = (iQ1>=0 && iQ2>=0); // both quarks were found

          // add to coll0ection
          if (isHadronic)
            genObjects[part] = size;
        }

      } // loop over targets
    } // process is not QCD or undef

    if (DEBUG) { PDebug("SCRAMJetAnalyzer::Run",TString::Format(" 1: %f",sw->RealTime()*1000)); sw->Start(); }

    // these are ak4 jets stored in the tree
    for (auto *anajet : anajets) {
      JetWriter *outjet = anajet->outjet;
      VJet *injets = anajet->injets;

      int nJets = injets->size();
      for (int iJ=0; iJ!=nJets; ++iJ) {
        PJet *pjet = injets->at(iJ);
        outjet->read(pjet);
        outjet->idx = iJ;
      } // loop over jets

      anajet->outtree->Fill();
    } // loop over jet collections

    if (DEBUG) { PDebug("SCRAMJetAnalyzer::Run",TString::Format(" 2: %f",sw->RealTime()*1000)); sw->Start(); }

    // these are fat jets stored in the tree
    // much more complicated!
    for (auto *anafatjet : anafatjets) {
      FatJetWriter *outjet = anafatjet->outjet;
      VFatJet *injets = anafatjet->injets;

      int nJets = injets->size();
      for (int iJ=0; iJ!=nJets; ++iJ) {
        PFatJet *pfatjet = injets->at(iJ);
        outjet->read(pfatjet);
        outjet->idx = iJ;

        PGenParticle *matched = Match(pfatjet->eta,pfatjet->phi,anafatjet->radius);
        if (matched!=NULL) {
          outjet->matched = 1;
          outjet->genpt = matched->pt;
          outjet->gensize = genObjects[matched];
        } else { 
          outjet->matched = 0; 
        }
      } // loop over jets

      anafatjet->outtree->Fill();
    } // loop over jet collections

    if (DEBUG) { PDebug("SCRAMJetAnalyzer::Run",TString::Format(" 3: %f",sw->RealTime()*1000)); sw->Start(); }

  } // entry loop

  if (DEBUG) { delete sw; sw=0; }
  if (DEBUG) { PDebug("SCRAMJetAnalyzer::Run","Done with entry loop"); }

} // Run()

