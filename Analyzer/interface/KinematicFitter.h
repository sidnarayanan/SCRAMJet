#ifndef KINEMATIC_FITTER
#define KINEMATIC_FITTER

/*

single top mass constrain to get teh right combinations

S term
-------
3 jet 4 vectors
deltas to those componenents
covariance matrix for measured params

constraint
-----------
mass constraint, here only depends on measured params f(y)
need derivative matrix 'A', 1x12 for 1 constraint eqn



 */

#include <iostream>
#include <iomanip>
#include <vector>
#include <algorithm>
#include <string>

#include "TClonesArray.h"
#include "TLorentzVector.h"
#include "TMatrixD.h"
#include "TVectorD.h"

#include "JetSmearer.h"

// Top candidate ----------------------------------------- 

class TopCandidate {
public:

  class TopCandidateParticle { 
  public:
    TopCandidateParticle () {;}
    TopCandidateParticle (TLorentzVector v, int num):vec(v),topnum(num) {;}
    TopCandidateParticle (TLorentzVector v, std::string l, int num):vec(v),label(l),topnum(num) {;}
    TopCandidateParticle (TLorentzVector v, std::string l, int num, unsigned b):vec(v),label(l),topnum(num),bitrep(b) {;}
    ~TopCandidateParticle () {;}

    static const unsigned BitB=0x1;
    static const unsigned BitWd1=0x2;
    static const unsigned BitWd2=0x4;
    
    
    TLorentzVector vec;
    std::string label;
    int topnum;
    unsigned bitrep;
  };
  

  TLorentzVector topvec;
  TLorentzVector Wvec;
  std::vector<TopCandidateParticle> particles;
  std::vector<TopCandidateParticle> particles_fit;
  std::string label;
  std::pair< unsigned, unsigned > bitrep; // 1st "W", 2nd "B"

  TopCandidate(TopCandidateParticle p0,TopCandidateParticle p1,TopCandidateParticle p2) {
    particles.push_back(p0);
    particles.push_back(p1);
    particles.push_back(p2);
    topvec = p0.vec + p1.vec + p2.vec;
    Wvec   = p0.vec + p1.vec;    
    label = p0.label + std::string(":") + p1.label + std::string(":") + p2.label;
    bitrep.first = p0.bitrep | p1.bitrep;
    bitrep.second = p2.bitrep;
  };
  ~TopCandidate() {};

  void updateLabeling ();
  bool matched();
  int  nunmatched();
  void reset();
  static bool compare(TopCandidateParticle first, TopCandidateParticle second );

};


// Fit results  ----------------------------------------- 

class FitResults { 
public:

  FitResults():
    mass(0.), massW(0.),
    fitmass(0.), fitmassW(0.),
    prob(0.), chisq(999.), cost(999.),
    nsteps(0), nunmatched(0),
    matched(false), converged(false)
  {}
  ~FitResults(){}

  std::string label;
  std::pair<unsigned,unsigned> bitrep; // 1st "W", 2nd "B"
  double mass, massW;
  double fitmass, fitmassW;
  double prob, chisq, cost;
  int nsteps, nunmatched;
  bool matched, converged;

  int topnum;

};

// Kinematic fitter ----------------------------------------- 

class KinematicFitter { 

public:

  JetSmearer     jetres;
  FitResults     fitstat;  

  const double MTop = 173.2;
  //const double MTop = 175.;
  const double MW   = 81.385;

  KinematicFitter () {
    y.ResizeTo(12);
    dy.ResizeTo(12);
    dy_last.ResizeTo(12);
    V.ResizeTo(12,12);
    B.ResizeTo(2,12);
  };
  ~KinematicFitter () {};

  bool fit( TopCandidate & topcand, FitResults & fitstat); 

  
  private:

  TVectorD y;
  TVectorD dy, dy_last;
  TMatrixD V;
  TMatrixD B;
    
  //  std::pair<unsigned, unsigned> bitrep;
  //  std::string label;
  std::vector<TopCandidate::TopCandidateParticle> the4vecs;

  void set_y( TVectorD & y, TopCandidate & combo );
  void zero_dy( TVectorD & dy );
  void setB( TMatrixD & B, TopCandidate & combo );
  void setV( TMatrixD & V, JetSmearer & myres, TopCandidate & combo );
  void updateVectors( TopCandidate & combo, TVectorD & dy );
};
  
  
#endif
