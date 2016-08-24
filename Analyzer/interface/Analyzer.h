#ifndef Analyzer_h
#define Analyzer_h

#include <TTree.h>
#include <TFile.h>
#include <TMath.h>
#include <TH1D.h>
#include <TH2F.h>
#include <TLorentzVector.h>

#include "SCRAMJet/Objects/interface/PEvent.h"
#include "SCRAMJet/Objects/interface/PPFCand.h"
#include "SCRAMJet/Objects/interface/PJet.h"
#include "SCRAMJet/Objects/interface/PFatJet.h"
#include "SCRAMJet/Objects/interface/PGenParticle.h"

#include "PandaCore/Tools/interface/Common.h"

#include "EnergyCorrelations.h"
#include "HeatMap.h"
#include "KinFitFunction.h"

#include "vector"
#include "map"
#include <string>

#include "fastjet/PseudoJet.hh"
#include "fastjet/JetDefinition.hh"
#include "fastjet/GhostedAreaSpec.hh"
#include "fastjet/AreaDefinition.hh"
#include "fastjet/ClusterSequenceArea.hh"
#include "fastjet/contrib/SoftDrop.hh"
#include "fastjet/contrib/NjettinessPlugin.hh"
#include "fastjet/contrib/MeasureDefinition.hh"
//#include "fastjet/contrib/EnergyCorrelator.hh"

//#include "CondFormats/JetMETObjects/interface/JetCorrectorParameters.h"
//#include "CondFormats/JetMETObjects/interface/FactorizedJetCorrector.h"
//#include "CondFormats/JetMETObjects/interface/JetCorrectionUncertainty.h"

/////////////////////////////////////////////////////////////////////////////
// some misc definitions
#define WMASS 80.4

std::vector<double> betas = {0.1, 1.0, 2.0};
std::vector<int> Ns = {1,2,3,4}; // only used for making branches and stuff

class sjpair {
  public:
    sjpair(float dR2_, float mW_) {
      dR2 = dR2_;
      mW = mW_;
    }
    ~sjpair() {}
  float dR2=-1;
  float mW=-1;
};

bool orderByDR(sjpair p1, sjpair p2) {
  // order in increasing dR order
  return p1.dR2 < p2.dR2;
}

bool orderByMW(sjpair p1, sjpair p2) {
  // order in increasing order of |mjj-mW|
  return TMath::Abs(p1.mW-WMASS) < TMath::Abs(p2.mW-WMASS);
}

TString makeECFString(int N, double beta) {
  return TString::Format("%i_%.2i",N,(int)(10*beta));
}

bool orderPseudoJet(fastjet::PseudoJet j1, fastjet::PseudoJet j2) {
  // to be used to order pseudojets in decreasing pT order
  return j1.perp2() > j2.perp2();
}

bool orderByCSV(scramjet::PJet *j1, scramjet::PJet *j2) {
  // order PJets by decreasing csv
  return j1->csv > j2->csv;
}

double DeltaR2(scramjet::PJet *j1, scramjet::PJet *j2) {
  return DeltaR2(j1->eta,j1->phi,j2->eta,j2->phi);
}

double Mjj(scramjet::PJet *j1, scramjet::PJet *j2) {
  TLorentzVector v1,v2;
  v1.SetPtEtaPhiM(j1->pt,j1->eta,j1->phi,j1->m);
  v2.SetPtEtaPhiM(j2->pt,j2->eta,j2->phi,j2->m);
  return (v1+v2).M();
}

/////////////////////////////////////////////////////////////////////////////
// Analyzer definition
class Analyzer {
public :
  // configuration enums
  enum ProcessType { 
    kNone,
    kQCD,
    kTop,
    kV,
    kH
  };

  enum PileupAlgo {
    kCHS,
    kPuppi
  };

  enum ClusterAlgo {
    kCA,
    kAK
  };

  ///////////////////////////////////////////////////////////////////////////////////
  // object classes used for writing the tree 
  // only defined for objects that will have many flavors
  // e.g. puppi/chs, radius, algorithm, etc

  class JetWriter {
  public:
    JetWriter(TTree *t) {
      t->Branch("pt",&pt,"pt/F");
      t->Branch("eta",&eta,"eta/F");
      t->Branch("phi",&phi,"phi/F");
      t->Branch("m",&m,"m/F");
      t->Branch("rawpt",&rawpt,"rawpt/F");
      t->Branch("csv",&csv,"csv/F");
      t->Branch("idx",&idx,"idx/I");
    }
    void read(const scramjet::PJet *j) {
      // read some basic floats from j
      pt = j->pt; eta = j->eta; phi = j->phi; 
      m = j->m; rawpt = j->rawPt; csv = j->csv;
    }
    void reset() {
        pt=-1; eta=999; phi=999; m=-1; rawpt=-1; csv=-1;
        idx=-1; 
    }
    float pt=0, eta=0, phi=0, m=0, rawpt=0, csv=0;
    int idx=-1; 
  };

  class FatJetWriter {
  public:
    FatJetWriter(TTree *t) {
      t->Branch("pt",&pt,"pt/f");
      t->Branch("eta",&eta,"eta/f");
      t->Branch("phi",&phi,"phi/f");
      t->Branch("m",&m,"m/f");
      t->Branch("rawpt",&rawpt,"rawpt/f");
      t->Branch("maxcsv",&maxcsv,"maxcsv/f");
      t->Branch("mincsv",&mincsv,"mincsv/f");
      t->Branch("mSD",&mSD,"mSD/f");
      t->Branch("tau32",&tau32,"tau32/f");
      t->Branch("tau21",&tau21,"tau21/f");
      t->Branch("gensize",&gensize,"gensize/f");
      t->Branch("genpt",&genpt,"genpt/f");
      t->Branch("idx",&idx,"idx/I");
      t->Branch("matched",&matched,"matched/I");
      t->Branch("tau32SD",&tau32SD,"tau32SD/f");
      t->Branch("tau21SD",&tau21SD,"tau21SD/f");
      for (auto beta : betas) {
        for (auto N : Ns) {
          TString ecfname;
          ecfname = "ecfNCA_"+makeECFString(N,beta);
          t->Branch(ecfname.Data(),&(ecfns[ecfname]),(ecfname+"/f").Data());
          ecfname = "ecfNAK_"+makeECFString(N,beta);
          t->Branch(ecfname.Data(),&(ecfns[ecfname]),(ecfname+"/f").Data());
        }
      }
      t->Branch("heatmap",&hmap);
      t->Branch("fitmass",&fitmass,"fitmass/f");
      t->Branch("fitmassW",&fitmassW,"fitmassW/f");
      t->Branch("fitprob",&fitprob,"fitprob/f");
      t->Branch("fitchi2",&fitchi2,"fitchi2/f");
      t->Branch("fitconv",&fitconv,"fitconv/I");
      t->Branch("dR2_minDR",&dR2_minDR,"dR2_minDR/f");
      t->Branch("mW_minDR",&mW_minDR,"mW_minDR/f");
      t->Branch("mW_best",&mW_best,"mW_best/f");
      t->Branch("nsubjets",&nsubjets,"nsubjets/I");
    }
    void read(const scramjet::PFatJet *j) {
      // read some basic floats from j
      pt = j->pt; eta = j->eta; phi = j->phi; 
      m = j->m; rawpt = j->rawPt; mSD = j->mSD;
      tau32 = j->tau3/j->tau2; tau21 = j->tau2/j->tau1;

      mincsv=999; maxcsv=-999;
      for (auto *sj : *(j->subjets)) {
        mincsv = TMath::Min(mincsv,sj->csv);
        maxcsv = TMath::Max(maxcsv,sj->csv);
      }
    }
    void reset() {
      pt=-1; eta=999; phi=999; m=-1; rawpt=-1; maxcsv=-1; mincsv=-1;
      mSD=-1; tau32=-1; tau21=-1; gensize=-1; genpt=-1;
      idx=-1; matched=-1; 
      //substructure
      tau32SD=-1; tau21SD=-1;
      for (auto beta : betas) {
        for (auto N : Ns) {
          ecfns["ecfNCA_"+makeECFString(N,beta)] = -1;
          ecfns["ecfNAK_"+makeECFString(N,beta)] = -1;
        }
      }
      if (hmap) {
        hmap->Delete(); hmap=0;
      }
      //kinematic fit
      fitmass=-1; fitmassW=-1; fitprob=-1;
      fitchi2=-1; fitconv=-1;
      //subjet kinematics
      dR2_minDR=-1; mW_minDR=0; mW_best=0;
      nsubjets=-1;
    }
    float pt=0, eta=0, phi=0, m=0, rawpt=0, maxcsv=0, mincsv=0;
    float mSD=0, tau32=0, tau21=0, gensize=0, genpt=0;
    int idx=-1, matched=-1; 
    // custom substructure
    float tau32SD=0, tau21SD=0;
    std::map<TString,float> ecfns;
    TH2F *hmap=0;
    //kinematic fit
    float fitmass=0, fitmassW=0, fitprob=0, fitchi2=0;
    int fitconv=0;
    //subjet kinematics
    float dR2_minDR=0, mW_minDR=0, mW_best=0;
    int nsubjets=0;

  };

  //////////////////////////////////////////////////////////////////////////////////////

  //////////////////////////////////////////////////////////////////////////////////////
  // classes to connect VXJet* to XJetWriter (if reading from tree)
  // or to connect VPFCand* to XJetWriter (if reclustering from scratch)
  // these classes DO NOT own the objects

  class AnaJet {
  public:
    AnaJet () {}
    ~AnaJet() {
      delete outjet;
      delete injets;
    }
    JetWriter *outjet=0;
    scramjet::VJet *injets=0;
    TTree *outtree;
  };

  class AnaFatJet {
  public:
    AnaFatJet () {}
    ~AnaFatJet() {
      delete outjet;
      delete injets;
      delete jetDefCA;
      delete jetDefAK;
      delete sd;
      delete tau;
    }
    FatJetWriter *outjet=0;
    scramjet::VFatJet *injets=0;
    scramjet::VPFCand *pfcands=0;
    double radius=0;
    ClusterAlgo algo=kCA;
    TTree *outtree;

    // fastjet stuff
    fastjet::JetDefinition *jetDefCA=0;
    fastjet::JetDefinition *jetDefAK=0;
    fastjet::contrib::SoftDrop *sd=0;
    fastjet::contrib::Njettiness *tau=0;
  };

  //////////////////////////////////////////////////////////////////////////////////////

  Analyzer();
  ~Analyzer();
  void Init(TTree *tree);
  void SetOutputFile(TString fOutName);
  void ResetBranches();
  void Run();
  void Terminate();
  void AddJetFromTree(TString inName, TString outName);
  void AddFatJetFromTree(TString inName, TString outName,PileupAlgo pu, double radius, ClusterAlgo algo);

  // public configuration
  bool isData=false;                         // to do gen matching, etc
  int maxEvents=-1;                          // max events to process; -1=>all
  double minFatJetPt=250;                    // min fatjet pt
  ProcessType processType=kNone;             // determine what to do the jet matching to

  bool doECF=false;
  bool doQjets=false;
  bool doKinFit=false;
  bool doHeatMap=false;
  bool doAKSubstructure=false;

private:

  std::vector<fastjet::PseudoJet> ConvertFatJet(scramjet::PFatJet*,scramjet::VPFCand*, double minPt=0.01);

//  JetCorrectorParameters *ak8jec=0;
//  JetCorrectionUncertainty *ak8unc=0;

  std::map<scramjet::PGenParticle*,float> genObjects;                      // particles we want to match the jets to, and the 'size' of the daughters
  scramjet::PGenParticle *Match(double eta, double phi, double radius);    // private function to match a jet; returns NULL if not found
  void AddEventBranches(TTree *t);
  
  TFile *fOut;   // output file is owned by Analyzer
  TTree *tIn=0;  // input tree to read
  TH1F *hDTotalMCWeight=0;


  scramjet::PEvent *event;                        // event object
  scramjet::VGenParticle *gen;                    // gen particle objects
  scramjet::VPFCand *chs=0;                       // CHS PF cands
  scramjet::VPFCand *puppi=0;                     // Puppi PF cands
  std::vector<AnaJet*> anajets;                   // narrow jets
  std::vector<AnaFatJet*> anafatjets;             // fat jets

  int runNumber;
  int lumiNumber;
  ULong64_t eventNumber; 
  float mcWeight;

  // fastjet
  fastjet::AreaDefinition *areaDef=0;
  fastjet::GhostedAreaSpec *activeArea=0;

  // kin fit
  KinematicFitter *fitter=0;
  FitResults *fitresults=0;

};

typedef std::vector<fastjet::PseudoJet> VPseudoJet;
  


#endif

