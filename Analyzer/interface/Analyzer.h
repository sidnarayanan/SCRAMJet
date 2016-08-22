#ifndef Analyzer_h
#define Analyzer_h

#include <TTree.h>
#include <TFile.h>
#include <TMath.h>
#include <TH1D.h>
#include <TH2D.h>
#include <TLorentzVector.h>

#include "SCRAMJet/Objects/interface/PEvent.h"
#include "SCRAMJet/Objects/interface/PPFCand.h"
#include "SCRAMJet/Objects/interface/PJet.h"
#include "SCRAMJet/Objects/interface/PFatJet.h"
#include "SCRAMJet/Objects/interface/PGenParticle.h"

#include "Common.h"

#include "vector"
#include "map"
#include <string>

//#include "CondFormats/JetMETObjects/interface/JetCorrectorParameters.h"
//#include "CondFormats/JetMETObjects/interface/FactorizedJetCorrector.h"
//#include "CondFormats/JetMETObjects/interface/JetCorrectionUncertainty.h"


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
  // e.g. puppi/chs, radius, etc

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
    }
    float pt=0, eta=0, phi=0, m=0, rawpt=0, maxcsv=0, mincsv=0;
    float mSD=0, tau32=0, tau21=0, gensize=0, genpt=0;
    int idx=-1, matched=-1; 
  };

  //////////////////////////////////////////////////////////////////////////////////////

  //////////////////////////////////////////////////////////////////////////////////////
  // classes to connect VXJet* to XJetWriter (if reading from tree)
  // or to connect VPFCand* to XJetWriter (if reclustering from scratch)
  // these classes DO NOT own the objects

  class AnaJet {
  public:
    AnaJet () {}
    ~AnaJet() {}
    JetWriter *outjet=0;
    scramjet::VJet *injets=0;
    TTree *outtree;
  };

  class AnaFatJet {
  public:
    AnaFatJet () {}
    ~AnaFatJet() {}
    FatJetWriter *outjet=0;
    scramjet::VFatJet *injets=0;
    scramjet::VPFCand *pfcands=0;
    double radius=0;
    ClusterAlgo algo=kCA;
    TTree *outtree;
  };

  //////////////////////////////////////////////////////////////////////////////////////

  Analyzer();
  ~Analyzer();
  // void SetDataDir(const char *s);
  void Init(TTree *tree);
  void SetOutputFile(TString fOutName);
  void ResetBranches();
  void Run();
  void Terminate();
  void AddJetFromTree(TString inName, TString outName);
  void AddFatJetFromTree(TString inName, TString outName,PileupAlgo pu, double radius, ClusterAlgo algo);

  // public configuration
  bool isData=false;                         // to do gen matching, etc
  ProcessType processType=kNone;             // determine what to do the jet matching to
  int maxEvents=-1;                          // max events to process; -1=>all
  double minFatJetPt=250;                    // min fatjet pt

private:

//  JetCorrectorParameters *ak8jec=0;
//  JetCorrectionUncertainty *ak8unc=0;

  std::map<scramjet::PGenParticle*,float> genObjects;                      // particles we want to match the jets to, and the 'size' of the daughters
  scramjet::PGenParticle *Match(double eta, double phi, double radius);    // private function to match a jet; returns NULL if not found
  void AddEventBranches(TTree *t);
  
  TFile *fOut;   // output file is owned by Analyzer
  TTree *tIn=0;  // input tree to read


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

  

};

#endif

