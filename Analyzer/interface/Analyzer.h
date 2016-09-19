#ifndef Analyzer_h
#define Analyzer_h

// STL
#include "vector"
#include "map"
#include <string>

// ROOT
#include <TTree.h>
#include <TFile.h>
#include <TMath.h>
#include <TH1D.h>
#include <TH2F.h>
#include <TLorentzVector.h>

// fastjet
#include "fastjet/PseudoJet.hh"
#include "fastjet/JetDefinition.hh"
#include "fastjet/GhostedAreaSpec.hh"
#include "fastjet/AreaDefinition.hh"
#include "fastjet/ClusterSequenceArea.hh"
#include "fastjet/contrib/SoftDrop.hh"
#include "fastjet/contrib/NjettinessPlugin.hh"
#include "fastjet/contrib/MeasureDefinition.hh"
#include "fastjet/contrib/EnergyCorrelator.hh"

// SCRAMJet Objects
#include "SCRAMJet/Objects/interface/PEvent.h"
#include "SCRAMJet/Objects/interface/PPFCand.h"
#include "SCRAMJet/Objects/interface/PJet.h"
#include "SCRAMJet/Objects/interface/PFatJet.h"
#include "SCRAMJet/Objects/interface/PGenParticle.h"

// PANDACore
#include "PandaCore/Tools/interface/Common.h"

// Analyzer-specific
#include "QjetsFunctions.h"
#include "EnergyCorrelations.h"
#include "HeatMap.h"
#include "KinFitFunction.h"
#include "PullCalcs.h"
#include "SDAlgorithm.h"

// JEC
//#include "CondFormats/JetMETObjects/interface/JetCorrectorParameters.h"
//#include "CondFormats/JetMETObjects/interface/FactorizedJetCorrector.h"
//#include "CondFormats/JetMETObjects/interface/JetCorrectionUncertainty.h"

/////////////////////////////////////////////////////////////////////////////
// some misc definitions
#define WMASS 80.4

std::vector<double> betas = {0.5, 1.0, 2.0};
std::vector<int> Ns = {1,2,3,4}; // only used for making branches and stuff
std::vector<int> orders = {1,2,3};

double clean(double x, double d=-1) {
  return (x==x) ? x : d;
}

class sjpair {
  public:
    sjpair(float dR2_, float mW_, float sumqg_) {
      dR2 = dR2_;
      mW = mW_;
      sumqg = sumqg_;
    }
    ~sjpair() {}
  float dR2=-1;
  float mW=-1;
  float sumqg=-1;
};

bool orderByQG(sjpair p1, sjpair p2) {
  return p1.sumqg > p2.sumqg;
}

bool orderByDR(sjpair p1, sjpair p2) {
  // order in increasing dR order
  return p1.dR2 < p2.dR2;
}

bool orderByMW(sjpair p1, sjpair p2) {
  // order in increasing order of |mjj-mW|
  return TMath::Abs(p1.mW-WMASS) < TMath::Abs(p2.mW-WMASS);
}

TString makeECFString(int order, int N, double beta) {
  return TString::Format("%i_%i_%.2i",order,N,(int)(10*beta));
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

double Mjj(fastjet::PseudoJet &j1, fastjet::PseudoJet &j2) {
  TLorentzVector v1,v2;
  v1.SetPtEtaPhiM(j1.pt(),j1.eta(),j1.phi(),j1.m());
  v2.SetPtEtaPhiM(j2.pt(),j2.eta(),j2.phi(),j2.m());
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
      t->Branch("qmass",&qmass,"qmass/f");
      t->Branch("qpt",&qpt,"qpt/f");
      t->Branch("qtau32",&qtau32,"qtau32/f");
      t->Branch("qtau21",&qtau21,"qtau21/f");
      t->Branch("betapull1",&betapull1,"betapull1/f");
      t->Branch("betapull2",&betapull2,"betapull2/f");
      t->Branch("betapull3",&betapull3,"betapull3/f");
      t->Branch("alphapull1",&alphapull1,"alphapull1/f");
      t->Branch("alphapull2",&alphapull2,"alphapull2/f");
      t->Branch("alphapull3",&alphapull3,"alphapull3/f");
      t->Branch("mW_minalphapull",&mW_minalphapull,"mW_minalphapull/f");
      t->Branch("minpullalpha",&minpullangle,"minpullalpha/f");
      t->Branch("sumqg",&sumqg,"sumqg/f");
      t->Branch("minqg",&minqg,"minqg/f");
      t->Branch("avgqg",&avgqg,"avgqg/f");
      t->Branch("mW_qg",&mW_qg,"mW_qg/f");
      for (auto beta : betas) {
        for (auto N : Ns) {
          for (auto o : orders) {
            TString ecfname = "ecfN_"+makeECFString(o,N,beta);
            // accidentally works - accessing a missing element creates it and returns reference
            t->Branch(ecfname.Data(),&(ecfns[ecfname]),(ecfname+"/f").Data());
            t->Branch(("max"+ecfname).Data(),&(ecfns["max"+ecfname]),("max"+ecfname+"/f").Data());
            if (N==3) {
              t->Branch("min_s"+ecfname,&(subecfns["min_s"+ecfname]),"min_s"+ecfname+"/f");
              t->Branch("sum_s"+ecfname,&(subecfns["sum_s"+ecfname]),"sum_s"+ecfname+"/f");
              t->Branch("avg_s"+ecfname,&(subecfns["avg_s"+ecfname]),"avg_s"+ecfname+"/f");
            }
          }
        }
      }
    }

    void read(const scramjet::PFatJet *j) {
      // read some basic floats from j
      pt = j->pt; eta = j->eta; phi = j->phi; 
      m = j->m; rawpt = j->rawPt; mSD = j->mSD;
      tau32 = clean(j->tau3/j->tau2); tau21 = clean(j->tau2/j->tau1);

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
      tau32SD=-1; tau21SD=-1;
      //kinematic fit
      fitmass=-1; fitmassW=-1; fitprob=-1;
      fitchi2=-1; fitconv=-1;
      //subjet kinematics
      dR2_minDR=-1; mW_minDR=0; mW_best=0;
      nsubjets=-1;
      //qjets
      qmass=-1; qpt=-1;
      qtau32=-1; qtau21=-1;
      //pull angle
      betapull1=999;betapull2=999;betapull3=999;
      alphapull1=999;alphapull2=999;alphapull3=999;
      mW_minalphapull=-1;
      minpullangle=999;
      // QGL
      sumqg=-1; minqg=-1; avgqg=-1; mW_qg=-1;
      // ecf
      for (auto beta : betas) {
        for (auto N : Ns) {
          for (auto o : orders) {
            ecfns["ecfN_"+makeECFString(o,N,beta)] = -1;
            ecfns["maxecfN_"+makeECFString(o,N,beta)] = -1;
            if (N==3) {
              subecfns["min_secfN_"+makeECFString(o,N,beta)] = 999;
              subecfns["sum_secfN_"+makeECFString(o,N,beta)] = 0;
              subecfns["avg_secfN_"+makeECFString(o,N,beta)] = 0;
            }
          }
        }
      }
      if (hmap) {
        hmap->Delete(); hmap=0;
      }
    }

    float pt=0, eta=0, phi=0, m=0, rawpt=0, maxcsv=0, mincsv=0;
    float mSD=0, tau32=0, tau21=0, gensize=0, genpt=0;
    int idx=-1, matched=-1; 
    // custom substructure
    float tau32SD=0, tau21SD=0;
    //ecfs
    std::map<TString,float> ecfns;
    std::map<TString,float> subecfns;
    TH2F *hmap=0;
    //kinematic fit
    float fitmass=0, fitmassW=0, fitprob=0, fitchi2=0;
    int fitconv=0;
    //subjet kinematics
    float dR2_minDR=0, mW_minDR=0, mW_best=0;
    int nsubjets=0;
    //qjets
    float qmass=0,qpt=0;
    float qtau32=0,qtau21=0;
    //pull angle
    float betapull1=0,betapull2=0,betapull3=0;
    float alphapull1=0,alphapull2=0,alphapull3=0;
    float mW_minalphapull=0, minpullangle=0;
    // QGL
    float sumqg=0, minqg=0, avgqg=0, mW_qg=0;

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
  int firstEvent=-1;
  int lastEvent=-1;                          // max events to process; -1=>all
  int maxJets=-1;                            // max fat jets to process; -1=>all
  double minFatJetPt=250;                    // min fatjet pt
  ProcessType processType=kNone;             // determine what to do the jet matching to
  TString showerDecoConfig="";

  bool doECF=false;
  bool doQjets=false;
  bool doKinFit=false;
  bool doHeatMap=false;
  bool doShowerDeco=false;
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
  int npv;
  float mcWeight;

  // fastjet
  fastjet::AreaDefinition *areaDef=0;
  fastjet::GhostedAreaSpec *activeArea=0;

  // kin fit
  KinematicFitter *fitter=0;
  FitResults *fitresults=0;

  // qjets
  qjets::QjetsPlugin *qplugin=0;
  fastjet::JetDefinition *qdef=0;

  // shower deco
  AnalysisParameters *decoParams=0;
  Deconstruction::Deconstruct* decoNstruct{0};
  Deconstruction::TopGluonModel* decoTop{0};
  Deconstruction::BackgroundModel* decoBG{0};
  Deconstruction::ISRModel* decoISR{0};
};

typedef std::vector<fastjet::PseudoJet> VPseudoJet;
  


#endif

