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

#include "../interface/JetSmearer.h"
#include "../interface/KinematicFitter.h"

// Top candidate ---------------------------------

bool TopCandidate::matched() { 
  if( label == std::string("Wd2:Wd1:B") || 
      label == std::string("Wd1:Wd2:B") ) {
    return true;
  }
  return false;
};  

int TopCandidate::nunmatched() { 
  int n=0;
  if( particles[0].bitrep > 0x4 ) n++;
  if( particles[1].bitrep > 0x4 ) n++;
  if( particles[2].bitrep > 0x4 ) n++;
  return n;
};  


void TopCandidate::reset() { 
  particles_fit = particles;
  topvec = particles[0].vec + particles[1].vec + particles[2].vec;
  Wvec   = particles[0].vec + particles[1].vec;
};


bool TopCandidate::compare(TopCandidateParticle first, TopCandidateParticle second ) { 
  if (first.vec.Pt() > second.vec.Pt()) return true;
  return false;
}


// Kinematic fitter ------------------------------

bool KinematicFitter::fit( TopCandidate & topcand, FitResults & fitres) 
{ 
  fitres.mass       = topcand.topvec.M();	
  fitres.massW      = topcand.Wvec.M();	
  fitres.matched    = topcand.matched();
  fitres.nunmatched = topcand.nunmatched();
  fitres.label      = topcand.label;
  fitres.bitrep     = topcand.bitrep;
  
  topcand.reset();       // what follows will change the vectors so take a copy first	
  
  //
  // initialize 
  //
  set_y(y,topcand);
  zero_dy(dy);
  setV(V, jetres, topcand);
  TMatrixD Vinv = TMatrixD(V).Invert();
  setB(B, topcand);
  
  TMatrixD VsubBtmp = B*V*TMatrixD(TMatrixD::kTransposed,B);
  TMatrixD VsubB = TMatrixD(VsubBtmp).Invert();

  //
  // start fit loop ...
  //
  
  fitres.converged = false;
  fitres.cost      = 999999;
  fitres.nsteps    = 0;
  double cost_last=99999, S=0, lastS=0;

  while( !fitres.converged && fitres.nsteps<100 ) { 
    
    dy_last = dy;
    
    TVectorD f(2), c(2);
    f(0) = topcand.topvec.M() - MTop;
    f(1) = topcand.Wvec.M() - MW;

    c = B*dy - f;
    dy = V*TMatrixD(TMatrixD::kTransposed,B)*VsubB * c;
    
    cost_last = fitres.cost;
    fitres.cost = fabs(f(0)) + fabs(f(1));	  
    
    //
    // if we've overshot
    //
    if( fabs(fitres.cost) > fabs(cost_last) ) { 
      
      fitres.cost  = cost_last;
      cost_last -= 0.01;
      
      while( fabs(fitres.cost) > fabs(cost_last) ) { 
	//restore ...
	dy = dy_last;
	dy *= -1;
	updateVectors(topcand,dy);
	// decrease corrections
	dy = dy_last;
	dy *= 0.5;
	updateVectors(topcand,dy);
	// recalc
	f(0) = topcand.topvec.M() - MTop;
	f(1) = topcand.Wvec.M() - MW;
	
	cost_last = fitres.cost;
	fitres.cost=fabs(f(0)) + fabs(f(1));	  	      
	//cost=fabs(f(0));	  	      
	  TVectorD tmpV = Vinv*dy;
	  S = dy*tmpV;
      }
      fitres.chisq     = S;
      fitres.fitmass   = topcand.topvec.M();
      fitres.fitmassW  = topcand.Wvec.M();
      fitres.converged = true;
      break; // from outer while loop
      } // fabs(cost) > fabs(cost_last) 
    
    
	// chisq
    lastS = S;
    TVectorD tmpV = Vinv*dy;
    S = dy*tmpV;
    
    
    if ( fitres.nsteps > 1 && (lastS-S)/2. < 0.5 && fabs(fitres.cost) <  5) {
      fitres.converged=true;
      break;
    }
    
    // update
    VsubBtmp = B*V*TMatrixD(TMatrixD::kTransposed,B);
    VsubB = TMatrixD(VsubBtmp).Invert();	  
    updateVectors(topcand,dy);	  
    
    setB(B, topcand);
    
    fitres.nsteps++;
    
  } // while not converged
  
  fitres.chisq     = S;
  fitres.prob      = TMath::Prob(fitres.chisq,2);
  fitres.fitmass   = topcand.topvec.M();	
  fitres.fitmassW  = topcand.Wvec.M();	
  
  return fitres.converged;
};

  
void KinematicFitter::set_y(TVectorD & y, TopCandidate & combo ) 
{ 
  y(0) =  combo.particles[0].vec.Px();
  y(1) =  combo.particles[0].vec.Py();
  y(2) =  combo.particles[0].vec.Pz();
  y(3) = -combo.particles[0].vec.E();
  
  y(4) =  combo.particles[1].vec.Px();
  y(5) =  combo.particles[1].vec.Py();
  y(6) =  combo.particles[1].vec.Pz();
  y(7) = -combo.particles[1].vec.E();
  
  y(8) =  combo.particles[2].vec.Px();
  y(9) =  combo.particles[2].vec.Py();
  y(10) =  combo.particles[2].vec.Pz();
  y(11) = -combo.particles[2].vec.E();

  //  y.Print();

};

void KinematicFitter::zero_dy(TVectorD & dy ) 
{ 
  dy(0) = 0.;
  dy(1) = 0.;
  dy(2) = 0.;
  dy(3) = 0.;
  dy(4) = 0.;
  dy(5) = 0.;
  dy(6) = 0.;
  dy(7) = 0.;
  dy(8) = 0.;
  dy(9) = 0.;
  dy(10) = 0.;
  dy(11) = 0.;

  //  dy.Print();

};

void KinematicFitter::setB(TMatrixD & B, TopCandidate & combo ) 
{ 
  // top constraint
  B(0,0)  = combo.topvec.P()/combo.topvec.M();
  B(0,1)  = combo.topvec.P()/combo.topvec.M();
  B(0,2)  = combo.topvec.P()/combo.topvec.M();
  B(0,3)  = -combo.topvec.E()/combo.topvec.M();
  B(0,4)  = combo.topvec.P()/combo.topvec.M();
  B(0,5)  = combo.topvec.P()/combo.topvec.M();
  B(0,6)  = combo.topvec.P()/combo.topvec.M();
  B(0,7)  = -combo.topvec.E()/combo.topvec.M();
  B(0,8)  = combo.topvec.P()/combo.topvec.M();
  B(0,9)  = combo.topvec.P()/combo.topvec.M();
  B(0,10) = combo.topvec.P()/combo.topvec.M();
  B(0,11) = -combo.topvec.E()/combo.topvec.M();
  // W constraint
  B(1,0)  = combo.Wvec.P()/combo.Wvec.M();
  B(1,1)  = combo.Wvec.P()/combo.Wvec.M();
  B(1,2)  = combo.Wvec.P()/combo.Wvec.M();
  B(1,3)  = -combo.Wvec.E()/combo.Wvec.M();
  B(1,4)  = combo.Wvec.P()/combo.Wvec.M();
  B(1,5)  = combo.Wvec.P()/combo.Wvec.M();
  B(1,6)  = combo.Wvec.P()/combo.Wvec.M();
  B(1,7)  = -combo.Wvec.E()/combo.Wvec.M();
  B(1,8)  = 0.;
  B(1,9)  = 0.;
  B(1,10) = 0.;
  B(1,11) = 0.;
  B *= -1;

  //  B.Print();

};


void KinematicFitter::setV( TMatrixD & V, JetSmearer & myres, TopCandidate & combo ) 
{ 

  double uncert_px_0, uncert_py_0 ,uncert_pz_0, uncert_e_0;
  myres.getUncertainties( combo.particles[0].vec, uncert_px_0, uncert_py_0, uncert_pz_0, uncert_e_0 ); 
  double uncert_px_1, uncert_py_1 ,uncert_pz_1, uncert_e_1;
  myres.getUncertainties( combo.particles[1].vec, uncert_px_1, uncert_py_1, uncert_pz_1, uncert_e_1 ); 
  double uncert_px_2, uncert_py_2 ,uncert_pz_2, uncert_e_2;
  myres.getUncertainties( combo.particles[2].vec, uncert_px_2, uncert_py_2, uncert_pz_2, uncert_e_2 ); 
  
  
  V(0,0)     = uncert_px_0*uncert_px_0;
  V(1,1)     = uncert_py_0*uncert_py_0;
  V(2,2)     = uncert_pz_0*uncert_pz_0;
  V(3,3)     = uncert_e_0*uncert_e_0;
  V(4,4)     = uncert_px_1*uncert_px_1;
  V(5,5)     = uncert_py_1*uncert_py_1;
  V(6,6)     = uncert_pz_1*uncert_pz_1;
  V(7,7)     = uncert_e_1*uncert_e_1;
  V(8,8)     = uncert_px_2*uncert_px_2;
  V(9,9)     = uncert_py_2*uncert_py_2;
  V(10,10)   = uncert_pz_2*uncert_pz_2;
  V(11,11)   = uncert_e_2*uncert_e_2;

//  V.Print();

};


void KinematicFitter::updateVectors( TopCandidate & combo, 
		    TVectorD & dy ) { 

  TLorentzVector dyv0(dy(0), dy(1), dy(2), dy(3));
  combo.particles_fit[0].vec += dyv0;
  TLorentzVector dyv1(dy(4), dy(5), dy(6), dy(7));
  combo.particles_fit[1].vec += dyv1;
  TLorentzVector dyv2(dy(8), dy(9), dy(10), dy(11));
  combo.particles_fit[2].vec += dyv2;

  combo.topvec = combo.particles_fit[0].vec + combo.particles_fit[1].vec + combo.particles_fit[2].vec;
  combo.Wvec   = combo.particles_fit[0].vec + combo.particles_fit[1].vec;
}
