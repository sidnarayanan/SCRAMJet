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
  // construct bare minumum objects 
  event = new PEvent();
  gen = new VGenParticle();

  // set up fastjet
  int activeAreaRepeats = 1;
  double ghostArea = 0.01;
  double ghostEtaMax = 7.0;
  activeArea = new fastjet::GhostedAreaSpec(ghostEtaMax,activeAreaRepeats,ghostArea);
  areaDef = new fastjet::AreaDefinition(fastjet::active_area_explicit_ghosts,*activeArea);

  // everything else is created at the beginning of Run(), after configuration is done
}

Analyzer::~Analyzer() {
  delete event;
  delete gen;

  delete activeArea;
  delete areaDef;

  for (auto *a : anajets) {
    delete a;
  }
  anajets.clear();

  for (auto *a : anafatjets) {
    delete a;
  }
  anafatjets.clear();

  delete chs;
  delete puppi;

  delete fitter;
  delete fitresults;

  delete qplugin;
  delete qdef;
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
  hDTotalMCWeight = new TH1F("hDTotalMCWeight","hDTotalMCWeight",10,-2,2);
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
  t->SetBranchAddress("puppicands",&puppi);
  t->SetBranchAddress("pfcands",&chs);
  if (!isData)
    t->SetBranchAddress("gen",&gen); 
}

void Analyzer::AddJetFromTree(TString inName, TString outName) {
  if (!tIn) {
    PError("SCRAMJetAnalyzer::AddJetFromTree","Input tree not set!");
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
    PError("SCRAMJetAnalyzer::AddFatJetFromTree","Input tree not set!");
  }
  AnaFatJet *anafatjet = new AnaFatJet();
  
  // get from tree
  anafatjet->injets =0; //= new VFatJet();
  tIn->SetBranchAddress(inName,&anafatjet->injets);

  // set up output
  fOut->cd();
  TTree *outtree = new TTree(outName.Data(),outName.Data());
  anafatjet->outtree = outtree;
  AddEventBranches(outtree);
  FatJetWriter *outjet = new FatJetWriter(outtree);
  anafatjet->outjet = outjet;

  // configure
  anafatjet->radius = radius;
  anafatjet->algo = algo;
  anafatjet->pfcands = (pu==kPuppi) ? puppi : chs;

  // set up fastjet
  anafatjet->jetDefCA = new fastjet::JetDefinition(fastjet::cambridge_algorithm, radius);
  anafatjet->jetDefAK = new fastjet::JetDefinition(fastjet::antikt_algorithm, radius);
  
  double sdZcut, sdBeta;
  if (radius<1) {
    sdZcut=0.1; sdBeta=0.;
  } else {
    sdZcut=0.15; sdBeta=1.;
  }
  anafatjet->sd = new fastjet::contrib::SoftDrop(sdBeta,sdZcut,radius);

  fastjet::contrib::OnePass_KT_Axes onepass;
  anafatjet->tau = new fastjet::contrib::Njettiness(onepass, fastjet::contrib::NormalizedMeasure(1., radius));

  // save fatjet
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
  fOut->WriteTObject(hDTotalMCWeight);
  fOut->Close();
//  delete ak8jec;
//  delete ak8unc;
}

VPseudoJet Analyzer::ConvertFatJet(PFatJet *pfatjet, VPFCand *pfcands, double minPt) {
  VPseudoJet vpj;
  unsigned int nPF = pfatjet->constituents->size();
  for (unsigned int iPF=0; iPF!=nPF; ++iPF) {
    PPFCand *thispf = pfatjet->getPFCand(iPF,pfcands);
    if (thispf->pt<minPt)
      continue;
    TLorentzVector mom; mom.SetPtEtaPhiM(thispf->pt,thispf->eta,thispf->phi,thispf->m);
    vpj.emplace_back(mom.Px(),mom.Py(),mom.Pz(),mom.E());
  }
  return vpj;
}

// run
void Analyzer::Run() {

  // INITIALIZE --------------------------------------------------------------------------
  
  unsigned int nEvents = tIn->GetEntries();
  if (maxEvents>=0 && maxEvents<(int)nEvents)
    nEvents = maxEvents;

  if (!fOut || !tIn) {
    PError("SCRAMJetAnalyzer::Run","NOT SETUP CORRECTLY");
    exit(1);
  }

  // initialize kinematic fit
  if (doKinFit) {
    fitter = new KinematicFitter();
    fitresults = new FitResults();
  }

  // initialize ECFN calculation
  ECFNManager *ecfnmanager = new ECFNManager(); // default is everything is on

  // initialize qjets
  if (doQjets) {
    qplugin = new qjets::QjetsPlugin(0.1,0.5,0.,0.,0.1,0.);
    qdef = new fastjet::JetDefinition(qplugin);
  }
  int qcounter=0;

  TStopwatch *sw = 0;
  if (DEBUG) sw = new TStopwatch();
  unsigned int iE=0;
  ProgressReporter pr("SCRAMJetAnalyzer::Run",&iE,&nEvents,10);
  hDTotalMCWeight->Reset();

  // EVENTLOOP --------------------------------------------------------------------------
  for (iE=0; iE!=nEvents; ++iE) {
    if (DEBUG) sw->Start(true);
    pr.Report();
    ResetBranches();
    tIn->GetEntry(iE);
    if (DEBUG) { PDebug("SCRAMJetAnalyzer::Run",TString::Format("-1: %f",sw->RealTime()*1000)); sw->Start(); }

    // event info
    mcWeight = (event->mcWeight>0) ? 1 : -1;
    hDTotalMCWeight->Fill(mcWeight,mcWeight);

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
        outjet->reset();
        PJet *pjet = injets->at(iJ);
        outjet->read(pjet);
        outjet->idx = iJ;
        anajet->outtree->Fill();
      } // loop over jets
    } // loop over jet collections

    if (DEBUG) { PDebug("SCRAMJetAnalyzer::Run",TString::Format(" 2: %f",sw->RealTime()*1000)); sw->Start(); }

    // these are fat jets stored in the tree
    // much more complicated!
    for (auto *anafatjet : anafatjets) {
      FatJetWriter *outjet = anafatjet->outjet;
      VFatJet *injets = anafatjet->injets;

      int nJets = injets->size();
      for (int iJ=0; iJ!=nJets; ++iJ) {
        outjet->reset();
        PFatJet *pfatjet = injets->at(iJ);
        if (pfatjet->pt<minFatJetPt)
          continue;
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

        if (DEBUG) { PDebug("SCRAMJetAnalyzer::Run",TString::Format(" 2.1: %f",sw->RealTime()*1000)); sw->Start(); }

        /////// fastjet ////////
        VPseudoJet vpj = ConvertFatJet(pfatjet,anafatjet->pfcands,0.1);

        fastjet::ClusterSequenceArea seqCA(vpj, *(anafatjet->jetDefCA), *areaDef);
        fastjet::ClusterSequenceArea seqAK(vpj, *(anafatjet->jetDefAK), *areaDef);

        VPseudoJet alljetsCA(seqCA.inclusive_jets(0.));
        fastjet::PseudoJet *leadingJetCA=0; 
        for (auto &jet : alljetsCA) {
          if (!leadingJetCA || jet.perp2()>leadingJetCA->perp2())
            leadingJetCA = &jet;
        }

        VPseudoJet alljetsAK(seqAK.inclusive_jets(0.));
        fastjet::PseudoJet *leadingJetAK=0;
        for (auto &jet : alljetsAK) {
          if (!leadingJetAK || jet.perp2()>leadingJetAK->perp2())
            leadingJetAK = &jet;
        }
        
        if (DEBUG) { PDebug("SCRAMJetAnalyzer::Run",TString::Format(" 2.2: %f",sw->RealTime()*1000)); sw->Start(); }

        if (leadingJetCA!=NULL || leadingJetAK!=NULL) {
          fastjet::PseudoJet sdJetCA = (*anafatjet->sd)(*leadingJetCA);
          fastjet::PseudoJet sdJetAK = (*anafatjet->sd)(*leadingJetAK);
          
          // get the constituents and sort them
          VPseudoJet sdConstituentsCA = sdJetCA.constituents();
          std::sort(sdConstituentsCA.begin(),sdConstituentsCA.end(),orderPseudoJet);

          VPseudoJet sdConstituentsAK = sdJetAK.constituents();
          std::sort(sdConstituentsAK.begin(),sdConstituentsAK.end(),orderPseudoJet);

          if (DEBUG) { PDebug("SCRAMJetAnalyzer::Run",TString::Format(" 2.3: %f",sw->RealTime()*1000)); sw->Start(); }

          /////////// let's calculate ECFs! ///////////

          // filter the constituents
          int nFilter;
          nFilter = TMath::Min(100,(int)sdConstituentsCA.size());
          VPseudoJet sdConstituentsCAFiltered(sdConstituentsCA.begin(),sdConstituentsCA.begin()+nFilter);
          
          if (doECF) {
            for (auto beta : betas) {
              calcECFN(beta,sdConstituentsCAFiltered,ecfnmanager);
              for (auto N : Ns) {
                for (auto o : orders) {
                  outjet->ecfns["ecfN_"+makeECFString(o,N,beta)] = ecfnmanager->ecfns[TString::Format("%i_%i",N,o)];
                }
              }
            }
            if (DEBUG) { PDebug("SCRAMJetAnalyzer::Run",TString::Format(" 2.4: %f",sw->RealTime()*1000)); sw->Start(); }
          }


          //////////// now let's do groomed tauN! /////////////
          double tau3 = anafatjet->tau->getTau(3,sdConstituentsCA);
          double tau2 = anafatjet->tau->getTau(2,sdConstituentsCA);
          double tau1 = anafatjet->tau->getTau(1,sdConstituentsCA);
          outjet->tau32SD = tau3/tau2;
          outjet->tau21SD = tau2/tau1;

          if (DEBUG) { PDebug("SCRAMJetAnalyzer::Run",TString::Format(" 2.5: %f",sw->RealTime()*1000)); sw->Start(); }

          //////////// Q-jet quantities ////////////////
          if (doQjets) {
            std::vector<qjetwrapper> q_jets = getQjets(vpj,qplugin,qdef,qcounter++,15,anafatjet->tau);

            JetQuantity getmass = [](qjetwrapper w) { return w.jet.m(); };
            outjet->qmass = qVolQuantity(q_jets,getmass);

            JetQuantity getpt= [](qjetwrapper w) { return w.jet.pt(); };
            outjet->qpt = qVolQuantity(q_jets,getpt);

            JetQuantity gettau32 = [](qjetwrapper w) { return w.tau32; };
            outjet->qtau32 = qVolQuantity(q_jets,gettau32);

            JetQuantity gettau21 = [](qjetwrapper w) { return w.tau21; };
            outjet->qtau21 = qVolQuantity(q_jets,gettau21);

            if (DEBUG) { PDebug("SCRAMJetAnalyzer::Run",TString::Format(" 2.6: %f",sw->RealTime()*1000)); sw->Start(); }
          }

          //////////// heat map! ///////////////
          if (doHeatMap) {
            outjet->hmap = HeatMap(sdJetCA.eta(),sdJetCA.phi(),sdConstituentsCA,1.5,20,20);

            if (DEBUG) { PDebug("SCRAMJetAnalyzer::Run",TString::Format(" 2.7: %f",sw->RealTime()*1000)); sw->Start(); }
          }

          //////////// subjet kinematics! /////////
          VJet *subjets = pfatjet->subjets;
          outjet->nsubjets=subjets->size();
          std::vector<sjpair> sjpairs;
          if (outjet->nsubjets>1) {
            // first set up the pairs
            double dR2 = DeltaR2(subjets->at(0),subjets->at(1));
            double mW = Mjj(subjets->at(0),subjets->at(1));
            sjpairs.emplace_back(dR2,mW);
            if (outjet->nsubjets>2) {
              dR2 = DeltaR2(subjets->at(0),subjets->at(2));
              mW = Mjj(subjets->at(0),subjets->at(2));
              sjpairs.emplace_back(dR2,mW);
              dR2 = DeltaR2(subjets->at(1),subjets->at(2));
              mW = Mjj(subjets->at(1),subjets->at(2));
              sjpairs.emplace_back(dR2,mW);
            }

            // now order by dR
            std::sort(sjpairs.begin(),sjpairs.end(),orderByDR);
            outjet->dR2_minDR=sjpairs[0].dR2;
            outjet->mW_minDR=sjpairs[0].mW;

            // now by mW
            std::sort(sjpairs.begin(),sjpairs.end(),orderByMW);
            outjet->mW_best=sjpairs[0].mW;
          }

          if (DEBUG) { PDebug("SCRAMJetAnalyzer::Run",TString::Format(" 2.8: %f",sw->RealTime()*1000)); sw->Start(); }

          //////////// kinematic fit! ///////////
          if (doKinFit) {
            PJet *sj1=0, *sj2=0, *sjb=0;
            if (subjets->size()>=3) {
              VJet leadingSubjets(subjets->begin(),subjets->begin()+3);
              std::sort(leadingSubjets.begin(),leadingSubjets.end(),orderByCSV);
              PerformKinFit(fitter,fitresults,leadingSubjets[1],leadingSubjets[2],leadingSubjets[0]); 
              outjet->fitconv = (fitresults->converged) ? 1 : 0;
              if (fitresults->converged) {
                outjet->fitmass = fitresults->fitmass;
                outjet->fitmassW = fitresults->fitmassW;
                outjet->fitprob = fitresults->prob;
                outjet->fitchi2 = fitresults->chisq;
              }
            }
            if (DEBUG) { PDebug("SCRAMJetAnalyzer::Run",TString::Format(" 2.9: %f",sw->RealTime()*1000)); sw->Start(); }

          }

          /////// fill ////////
          anafatjet->outtree->Fill();
        } else {
          //???
          PError("SCRAMJetAnalyzer::Run","No jet was clustered???");
        }

      } // loop over jets
    } // loop over jet collections

    if (DEBUG) { PDebug("SCRAMJetAnalyzer::Run",TString::Format(" 3: %f",sw->RealTime()*1000)); sw->Start(); }

  } // entry loop

  if (DEBUG) { delete sw; sw=0; }
  if (DEBUG) { PDebug("SCRAMJetAnalyzer::Run","Done with entry loop"); }

} // Run()

