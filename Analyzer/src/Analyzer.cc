#include "../interface/Analyzer.h"
#include "TVector2.h"
#include "TMath.h"
#include <algorithm>
#include <vector>

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
  npv=-1;
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
  tout->Branch("npv",&npv,"npv/I");
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
  unsigned int nZero = 0;
  if (lastEvent>=0 && lastEvent<(int)nEvents)
    nEvents = lastEvent;
  if (firstEvent>=0)
    nZero = firstEvent;

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
  ECFNManager *subecfnmanager = new ECFNManager(); 
      subecfnmanager->doN1=false;
      subecfnmanager->doN2=false;
      subecfnmanager->doN4=false;

  // initialize qjets
  if (doQjets) {
    qplugin = new qjets::QjetsPlugin(0.1,0.5,0.,0.,0.1,0.);
    qdef = new fastjet::JetDefinition(qplugin);
  }
  int qcounter=0;

  // initialize HTT
  if (doHTT) {
    bool optimalR=true; bool doHTTQ=false;
    double minSJPt=0.; double minCandPt=0.;
    double sjmass=30.; double mucut=0.8;
    double filtR=0.3; int filtN=5;
    int mode=4; double minCandMass=0.;
    double maxCandMass=9999999.; double massRatioWidth=9999999.;
    double minM23Cut=0.; double minM13Cut=0.;
    double maxM13Cut=9999999.;  bool rejectMinR=false;
    htt = new fastjet::HEPTopTaggerV2(optimalR,doHTTQ,
                             minSJPt,minCandPt,
                             sjmass,mucut,
                             filtR,filtN,
                             mode,minCandMass,
                             maxCandMass,massRatioWidth,
                             minM23Cut,minM13Cut,
                             maxM13Cut,rejectMinR);
  }

  // initialize shower deco
  if (doShowerDeco) {
    decoParams = new AnalysisParameters(showerDecoConfig.Data());
    decoTop = new Deconstruction::TopGluonModel(*decoParams);
    decoBG = new Deconstruction::BackgroundModel(*decoParams);
    decoISR = new Deconstruction::ISRModel(*decoParams);
    decoNstruct = new Deconstruction::Deconstruct(*decoParams,*decoTop,*decoBG,*decoISR);
  }

  unsigned int iE=0;
  ProgressReporter pr("SCRAMJetAnalyzer::Run",&iE,&nEvents,10);
  TimeReporter tr("SCRAMJetAnalyzer::Run",DEBUG);
  hDTotalMCWeight->Reset();

  // EVENTLOOP --------------------------------------------------------------------------
  for (iE=nZero; iE!=nEvents; ++iE) {
    tr.Start();
    pr.Report();
    ResetBranches();
    tIn->GetEntry(iE);
    tr.TriggerEvent("GetEntry");

    // event info
    mcWeight = (event->mcWeight>0) ? 1 : -1;
    hDTotalMCWeight->Fill(mcWeight,mcWeight);

    runNumber = event->runNumber;
    lumiNumber = event->lumiNumber;
    eventNumber = event->eventNumber;
    npv = event->npv;

    tr.TriggerEvent("initialize");

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

    tr.TriggerEvent("gen matching");

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

    tr.TriggerEvent("jets");

    // these are fat jets stored in the tree
    // much more complicated!
    for (auto *anafatjet : anafatjets) {
      FatJetWriter *outjet = anafatjet->outjet;
      VFatJet *injets = anafatjet->injets;

      int nJets = injets->size();
      for (int iJ=0; iJ!=nJets; ++iJ) {
        outjet->reset();
        if (maxJets>=0 && iJ==maxJets)
          break;
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

        tr.TriggerSubEvent("gen matching");

        /////// fastjet ////////
        VPseudoJet vpj = ConvertFatJet(pfatjet,anafatjet->pfcands,0.01);

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
        
        tr.TriggerSubEvent("clustering");

        if (leadingJetCA!=NULL || leadingJetAK!=NULL) {
          fastjet::PseudoJet sdJetCA = (*anafatjet->sd)(*leadingJetCA);
          fastjet::PseudoJet sdJetAK = (*anafatjet->sd)(*leadingJetAK);
          
          VPseudoJet sdsubjets = fastjet::sorted_by_pt(sdJetCA.exclusive_subjets_up_to(3));

          // get the constituents and sort them
          VPseudoJet sdConstituentsCA = sdJetCA.constituents();
          std::sort(sdConstituentsCA.begin(),sdConstituentsCA.end(),orderPseudoJet);

          // VPseudoJet sdConstituentsAK = sdJetAK.constituents();
          // std::sort(sdConstituentsAK.begin(),sdConstituentsAK.end(),orderPseudoJet);

          tr.TriggerSubEvent("soft drop");

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
            /*
            for (auto beta : betas) {
              calcECFN(beta,sdConstituentsCAFiltered,ecfnmanager,false);
              for (auto N : Ns) {
                for (auto o : orders) {
                  outjet->ecfns["maxecfN_"+makeECFString(o,N,beta)] = ecfnmanager->ecfns[TString::Format("%i_%i",N,o)];
                }
              }
            }
            */

            tr.TriggerSubEvent("ecfns");

            // now we calculate ECFs for the subjets
            unsigned int nS = sdsubjets.size();
            float subbeta=1.;
            /*
            for (unsigned int iS=0; iS!=nS; ++iS) {
              VPseudoJet subconstituents = sdsubjets[iS].constituents();
              nFilter = TMath::Min(80,(int)subconstituents.size());
              std::sort(subconstituents.begin(),subconstituents.end(),orderPseudoJet);
              VPseudoJet subconstituentsFiltered(subconstituents.begin(),subconstituents.begin()+nFilter);

              calcECFN(subbeta,subconstituentsFiltered,subecfnmanager);
              outjet->subecfns["min_secfN_"+makeECFString(1,3,subbeta)] = TMath::Min(
                  (double)subecfnmanager->ecfns[TString::Format("%i_%i",3,1)],
                  (double)outjet->subecfns["min_secfN_"+makeECFString(1,3,subbeta)]);
              outjet->subecfns["min_secfN_"+makeECFString(2,3,subbeta)] = TMath::Min(
                  (double)subecfnmanager->ecfns[TString::Format("%i_%i",3,2)],
                  (double)outjet->subecfns["min_secfN_"+makeECFString(2,3,subbeta)]);
              outjet->subecfns["min_secfN_"+makeECFString(3,3,subbeta)] = TMath::Min(
                  (double)subecfnmanager->ecfns[TString::Format("%i_%i",3,3)],
                  (double)outjet->subecfns["min_secfN_"+makeECFString(3,3,subbeta)]);
              outjet->subecfns["sum_secfN_"+makeECFString(1,3,subbeta)] += subecfnmanager->ecfns[TString::Format("%i_%i",3,1)];
              outjet->subecfns["sum_secfN_"+makeECFString(2,3,subbeta)] += subecfnmanager->ecfns[TString::Format("%i_%i",3,2)];
              outjet->subecfns["sum_secfN_"+makeECFString(3,3,subbeta)] += subecfnmanager->ecfns[TString::Format("%i_%i",3,3)];
            }
            outjet->subecfns["avg_secfN_"+makeECFString(1,3,subbeta)] = outjet->subecfns["sum_secfN_"+makeECFString(1,3,subbeta)]/nS;
            outjet->subecfns["avg_secfN_"+makeECFString(2,3,subbeta)] = outjet->subecfns["sum_secfN_"+makeECFString(2,3,subbeta)]/nS;
            outjet->subecfns["avg_secfN_"+makeECFString(3,3,subbeta)] = outjet->subecfns["sum_secfN_"+makeECFString(3,3,subbeta)]/nS;

            tr.TriggerSubEvent("subecfns");
            */

          }

          //////////// now let's do groomed tauN! /////////////
          double tau3 = anafatjet->tau->getTau(3,sdConstituentsCA);
          double tau2 = anafatjet->tau->getTau(2,sdConstituentsCA);
          double tau1 = anafatjet->tau->getTau(1,sdConstituentsCA);
          outjet->tau32SD = clean(tau3/tau2);
          outjet->tau21SD = clean(tau2/tau1);
          outjet->tau3SD = tau3;
          outjet->tau2SD = tau2;
          outjet->tau1SD = tau1;

          tr.TriggerSubEvent("tauSD");

          //////////// Q-jet quantities ////////////////
          if (doQjets) {
            std::vector<qjetwrapper> q_jets = getQjets(vpj,qplugin,qdef,qcounter++,15,anafatjet->tau);
            //std::vector<qjetwrapper> q_jets = getQjets(vpj,qplugin,qdef,qcounter++,10);

            JetQuantity getmass = [](qjetwrapper w) { return w.jet.m(); };
            outjet->qmass = clean(qVolQuantity(q_jets,getmass));

            JetQuantity getpt= [](qjetwrapper w) { return w.jet.pt(); };
            outjet->qpt = clean(qVolQuantity(q_jets,getpt));

            JetQuantity gettau32 = [](qjetwrapper w) { return w.tau32; };
            outjet->qtau32 = clean(qVolQuantity(q_jets,gettau32));

            JetQuantity gettau21 = [](qjetwrapper w) { return w.tau21; };
            outjet->qtau21 = clean(qVolQuantity(q_jets,gettau21));

            tr.TriggerSubEvent("qvol");
          }

          //////////// heat map! ///////////////
          if (doHeatMap) {
            outjet->hmap = HeatMap(sdJetCA.eta(),sdJetCA.phi(),sdConstituentsCA,1.5,20,20);

            tr.TriggerSubEvent("heat map");
          }

          //////////// subjet kinematics! /////////
          VJet *subjets = pfatjet->subjets;
          std::sort(subjets->begin(),subjets->end(),orderByPT);
          outjet->nsubjets=subjets->size();
          for (unsigned int iS=0; iS!=TMath::Min(3,outjet->nsubjets); ++iS) {
            outjet->subpts[iS] = subjets->at(iS)->pt;
            outjet->subetas[iS] = subjets->at(iS)->eta;
            outjet->subphis[iS] = subjets->at(iS)->phi;
            outjet->subms[iS] = subjets->at(iS)->m;
            outjet->subcsvs[iS] = subjets->at(iS)->csv;
          }

          std::vector<sjpair> sjpairs;
          if (outjet->nsubjets>1) {
            // first set up the pairs
            double dR2 = DeltaR2(subjets->at(0),subjets->at(1));
            double mW = Mjj(subjets->at(0),subjets->at(1));
            double sumqg = subjets->at(0)->qgl + subjets->at(1)->qgl;
            sjpairs.emplace_back(dR2,mW,sumqg);
            outjet->sumqg=sumqg;
            outjet->minqg = TMath::Min(subjets->at(0)->qgl,subjets->at(1)->qgl);
            if (outjet->nsubjets>2) {
              dR2 = DeltaR2(subjets->at(0),subjets->at(2));
              mW = Mjj(subjets->at(0),subjets->at(2));
              sumqg = subjets->at(0)->qgl + subjets->at(2)->qgl;
              sjpairs.emplace_back(dR2,mW,sumqg);

              dR2 = DeltaR2(subjets->at(1),subjets->at(2));
              mW = Mjj(subjets->at(1),subjets->at(2));
              sumqg = subjets->at(1)->qgl + subjets->at(2)->qgl;
              sjpairs.emplace_back(dR2,mW,sumqg);

              // now order by dR
              std::sort(sjpairs.begin(),sjpairs.end(),orderByDR);
              outjet->dR2_minDR=sjpairs[0].dR2;
              outjet->mW_minDR=sjpairs[0].mW;

              // now by mW
              std::sort(sjpairs.begin(),sjpairs.end(),orderByMW);
              outjet->mW_best=sjpairs[0].mW;
              
              // now sumqg
              std::sort(sjpairs.begin(),sjpairs.end(),orderByQG);
              outjet->mW_qg=sjpairs[0].mW;
              outjet->sumqg = subjets->at(0)->qgl+subjets->at(1)->qgl+subjets->at(2)->qgl;
              outjet->minqg = TMath::Min(TMath::Min(subjets->at(0)->qgl,subjets->at(1)->qgl),subjets->at(2)->qgl);
            } else {
              outjet->dR2_minDR=sjpairs[0].dR2;
              outjet->mW_minDR=subjets->at(0)->m;
              outjet->mW_best = (TMath::Abs(subjets->at(0)->m-WMASS)<TMath::Abs(subjets->at(1)->m-WMASS)) ? subjets->at(0)->m : subjets->at(1)->m;
              outjet->mW_qg = (subjets->at(0)->qgl<subjets->at(1)->qgl) ? subjets->at(0)->m : subjets->at(1)->m;
            }
            outjet->avgqg = outjet->sumqg/outjet->nsubjets;
          }

          tr.TriggerSubEvent("subjet kinematics");

          //////////// kinematic fit! ///////////
          if (doKinFit) {
            PJet *sj1=0, *sj2=0, *sjb=0;
            if (subjets->size()>=3) {
              VJet leadingSubjets(subjets->begin(),subjets->begin()+3);
              if (leadingSubjets.at(2)->pt>2) {
                std::sort(leadingSubjets.begin(),leadingSubjets.end(),orderByCSV);
                PerformKinFit(fitter,fitresults,leadingSubjets[1],leadingSubjets[2],leadingSubjets[0]); 
                outjet->fitconv = (fitresults->converged) ? 1 : 0;
                if (fitresults->converged) {
                  outjet->fitprob = fitresults->prob;
                  outjet->fitchi2 = fitresults->chisq;
                  outjet->fitmass = fitresults->fitmass;
                  outjet->fitmassW = fitresults->fitmassW;
                }
              } else {
                outjet->fitconv = 0;
              }
            } else {
              outjet->fitconv = 0;
            }
            tr.TriggerSubEvent("kinematic fit");

          }
          
          ////// pull angles! ///////
          unsigned int nS=sdsubjets.size();
          std::vector<TVector2> pulls; pulls.reserve(3);
          if (nS>0) {
            pulls.push_back(GetPull(sdsubjets[0]));
            outjet->betapull1 = GetPullBeta(pulls[0]);
            if (nS>1) {
              pulls.push_back(GetPull(sdsubjets[1]));
              outjet->betapull2 = GetPullBeta(pulls[1]);
              // 0:01, 1:02, 2:12
              outjet->alphapull1 = GetPullAlpha(sdsubjets[0],sdsubjets[1],pulls[0]);
              if (nS>2) {
                pulls.push_back(GetPull(sdsubjets[2]));
                outjet->betapull3 = GetPullBeta(pulls[2]);
                outjet->alphapull2 = GetPullAlpha(sdsubjets[0],sdsubjets[2],pulls[0]);
                outjet->alphapull3 = GetPullAlpha(sdsubjets[1],sdsubjets[2],pulls[1]);

                outjet->minpullangle = TMath::Abs(outjet->alphapull1); outjet->mW_minalphapull = Mjj(sdsubjets[0],sdsubjets[1]);
                if (TMath::Abs(outjet->alphapull2) < outjet->minpullangle) {
                  outjet->minpullangle = TMath::Abs(outjet->alphapull2);
                  outjet->mW_minalphapull = Mjj(sdsubjets[0],sdsubjets[2]);
                }
                if (TMath::Abs(outjet->alphapull3) < outjet->minpullangle) {
                  outjet->minpullangle = TMath::Abs(outjet->alphapull3);
                  outjet->mW_minalphapull = Mjj(sdsubjets[1],sdsubjets[2]);
                }
              }
              outjet->mW_minalphapull = sdsubjets[0].m();
            }
          }
          tr.TriggerSubEvent("pulls");

          /////// HTT ///////
          if (doHTT) {
            fastjet::PseudoJet taggedJet = htt->result(*leadingJetCA);
            if (taggedJet!=0) {
              fastjet::HEPTopTaggerV2Structure *s = (fastjet::HEPTopTaggerV2Structure*)taggedJet.structure_non_const_ptr();
              outjet->htt_mass = s->top_mass();
              outjet->htt_frec = s->fRec();
            }
          }
          tr.TriggerSubEvent("HTT");

          /////// shower deco ///////
          // TODO 

          /////// fill ////////
          anafatjet->outtree->Fill();
        } else {
          //???
          PError("SCRAMJetAnalyzer::Run","No jet was clustered???");
        }

        tr.TriggerEvent("fat jet",false);
      } // loop over jets
    } // loop over jet collections


  } // entry loop

  if (DEBUG) { PDebug("SCRAMJetAnalyzer::Run","Done with entry loop"); }

} // Run()

