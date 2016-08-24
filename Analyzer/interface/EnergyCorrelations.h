#include "fastjet/PseudoJet.hh"
#include <vector>
#include "TMath.h"

double DeltaR2(fastjet::PseudoJet j1, fastjet::PseudoJet j2) {
  return DeltaR2(j1.eta(),j1.phi(),j2.eta(),j2.phi());
}

void calcECF(double beta, std::vector<fastjet::PseudoJet> &constituents, double *n1=0, double *n2=0, double *n3=0, double *n4=0) {
  unsigned int nC = constituents.size();
  double halfBeta = beta/2.;

  // if only N=1,2, do not bother caching kinematics
  if (!n3 && !n4) {
    if (n1) { // N=1
      double val=0;
      for (unsigned int iC=0; iC!=nC; ++iC) {    
        val += constituents[iC].perp();
      }
      *n1 = val;
    }
    if (n2) { // N=2
      double val=0;
      for (unsigned int iC=0; iC!=nC; ++iC) {
        fastjet::PseudoJet iconst = constituents[iC];
        for (unsigned int jC=0; jC!=iC; ++jC) {
          fastjet::PseudoJet jconst = constituents[jC];
          val += iconst.perp() * jconst.perp() * pow(DeltaR2(iconst,jconst),halfBeta);
        }
      }
      *n2 = val;
    }
    return;
  }

  // cache kinematics
  double *pTs = new double[nC];
  double **dRs = new double*[nC];
  for (unsigned int iC=0; iC!=nC; ++iC) {
    dRs[iC] = new double[iC];
  }
  for (unsigned int iC=0; iC!=nC; ++iC) {
    fastjet::PseudoJet iconst = constituents[iC];
    pTs[iC] = iconst.perp();
    for (unsigned int jC=0; jC!=iC; ++jC) {
      fastjet::PseudoJet jconst = constituents[jC];
      dRs[iC][jC] = pow(DeltaR2(iconst,jconst),halfBeta);
    }
  }
  
  // now we calculate the real ECFs
  if (n1) { // N=1
    double val=0;
    for (unsigned int iC=0; iC!=nC; ++iC) {    
      val += pTs[iC];
    } // iC
    *n1 = val;
  }
  if (n2) { // N=2
    double val=0;
    for (unsigned int iC=0; iC!=nC; ++iC) {
      for (unsigned int jC=0; jC!=iC; ++jC) {
        val += pTs[iC] * pTs[jC] * dRs[iC][jC]; 
      } // jC
    } // iC
    *n2 = val;
  }
  if (n3) {
    double val=0;
    for (unsigned int iC=0; iC!=nC; ++iC) {
      for (unsigned int jC=0; jC!=iC; ++jC) {
        double val_ij = pTs[iC]*pTs[jC]*dRs[iC][jC];
        for (unsigned int kC=0; kC!=jC; ++kC) {
          val += val_ij * pTs[kC] * dRs[iC][kC] * dRs[jC][kC];
        } // kC
      } // jC
    } // iC
    *n3 = val;
  }
  if (n4) {
    double val=0;
    for (unsigned int iC=0; iC!=nC; ++iC) {
      for (unsigned int jC=0; jC!=iC; ++jC) {
        double val_ij = pTs[iC]*pTs[jC]*dRs[iC][jC];
        for (unsigned int kC=0; kC!=jC; ++kC) {
          double val_ijk = val_ij * pTs[kC] * dRs[iC][kC] * dRs[jC][kC];
          for (unsigned int lC=0; lC!=kC; ++lC) {
            val += val_ijk * pTs[lC] * dRs[iC][lC] * dRs[jC][lC] * dRs[kC][lC];
          } // lC
        } // kC
      } // jC
    } // iC
    *n4 = val;
  }

  // cleanup
  delete pTs;
  for (unsigned int iC=0; iC!=nC; ++iC) {
    delete dRs[iC];
  }
}

void calcECFN(double beta, std::vector<fastjet::PseudoJet> &constituents, double *n1=0, double *n2=0, double *n3=0, double *n4=0, bool useAllAngles=false) {
  unsigned int nC = constituents.size();
  double halfBeta = beta/2.;

  // get the normalization factor
  double baseNorm=0; 
  calcECF(beta,constituents,&baseNorm,0,0,0);

  // cache kinematics
  double *pTs = new double[nC];
  double **dRs = new double*[nC];
  for (unsigned int iC=0; iC!=nC; ++iC) {
    dRs[iC] = new double[iC];
  }
  for (unsigned int iC=0; iC!=nC; ++iC) {
    fastjet::PseudoJet iconst = constituents[iC];
    pTs[iC] = iconst.perp();
    for (unsigned int jC=0; jC!=iC; ++jC) {
      fastjet::PseudoJet jconst = constituents[jC];
      dRs[iC][jC] = pow(DeltaR2(iconst,jconst),halfBeta);
    }
  }
  
  // now we calculate the ECFNs
  if (n1) { // N=1
    *n1 = 1;
  }
  if (n2) { // N=2
    double norm = pow(baseNorm,2);
    double val=0;
    for (unsigned int iC=0; iC!=nC; ++iC) {
      for (unsigned int jC=0; jC!=iC; ++jC) {
        val += pTs[iC] * pTs[jC] * dRs[iC][jC] / norm; 
      } // jC
    } // iC
    *n2 = val;
  }
  if (n3) {
    double norm = pow(baseNorm,3);
    double val=0;
    unsigned int nAngles=3;
    double *angles = new double[nAngles];

    for (unsigned int iC=0; iC!=nC; ++iC) {
      for (unsigned int jC=0; jC!=iC; ++jC) {
        double val_ij = pTs[iC]*pTs[jC];
        angles[0] = dRs[iC][jC];

        for (unsigned int kC=0; kC!=jC; ++kC) {
          angles[1] = dRs[iC][kC];
          angles[2] = dRs[jC][kC];

          double finalAngle=999;
          if (useAllAngles) {
            finalAngle = angles[0]*angles[1]*angles[2];
          } else {
            for (unsigned int iA=0; iA!=nAngles; ++iA) {
              finalAngle = TMath::Min(finalAngle,angles[iA]);
            }
          }

          val += val_ij * pTs[kC] * finalAngle / norm;
        } // kC
      } // jC
    } // iC
    *n3 = val;
    delete angles;
  }
  if (n4) {
    double norm = pow(baseNorm,4);
    double val=0;
    unsigned int nAngles=6;
    double *angles = new double[nAngles];

    for (unsigned int iC=0; iC!=nC; ++iC) {
      for (unsigned int jC=0; jC!=iC; ++jC) {
        double val_ij = pTs[iC]*pTs[jC];
        angles[0] = dRs[iC][jC];

        for (unsigned int kC=0; kC!=jC; ++kC) {
          double val_ijk = val_ij * pTs[kC];
          angles[1] = dRs[iC][kC];
          angles[2] = dRs[jC][kC];
          double angle_ijk=0;
          if (useAllAngles) {
            angle_ijk = angles[0]*angles[1]*angles[2];
          } else {
            angle_ijk = TMath::Min(angles[0],TMath::Min(angles[1],angles[2]));
          }

          for (unsigned int lC=0; lC!=kC; ++lC) {
            angles[3] = dRs[jC][kC];
            angles[4] = dRs[jC][lC];
            angles[5] = dRs[kC][lC];
            double finalAngle=0;
            if (useAllAngles) {
              finalAngle = angle_ijk * angles[3]*angles[4]*angles[4];
            } else {
              finalAngle = TMath::Min( angle_ijk,TMath::Min( angles[3],TMath::Min( angles[4],angles[5] ) ) );
            }
            val += val_ijk * pTs[lC] * finalAngle / norm;
          } // lC
        } // kC
      } // jC
    } // iC
    *n4 = val;
    delete angles;
  }

  // cleanup
  delete pTs;
  for (unsigned int iC=0; iC!=nC; ++iC) {
    delete dRs[iC];
  }
}

/*
double calcECFN(double N, double beta, fastjet::PseudoJet &jet) {
  double ecfn=0;
  Vfastjet::PseudoJet constituents = jet.constituents();
  unsigned int nC = constituents.size();

  // normalization
  double norm = pow( contrib::EnergyCorrelator(1,beta,contrib::EnergyCorrelator::pt_R).result(jet), N);
  // cache kinematics
  double *pTs = new double[nC];
  double **dRs = new double*[nC];
  for (unsigned int iC=0; iC!=nC; ++iC) {
    dRs[iC] = new double[iC];
  }
  for (unsigned int iC=0; iC!=nC; ++iC) {
    fastjet::PseudoJet iconst = constituents[iC];
    pTs[iC] = iconst.perp();
    for (unsigned int jC=0; jC!=iC; ++jC) {
      fastjet::PseudoJet jconst = constituents[jC];
      dRs[iC][jC] = pow(DeltaR2(iconst.eta(),iconst.phi(),jconst.eta(),jconst.phi()),beta/2);
    }
  }

  N=4; // for testing

  if (N==4) {
    unsigned int nAngles=6;
    double *angles = new double[6];
    for (unsigned int iC=0; iC!=nC; ++iC) {
      for (unsigned int jC=0; jC!=iC; ++jC) {
        for (unsigned int kC=0; kC!=jC; ++kC) {
          for (unsigned int lC=0; lC!=kC; ++lC) {
            angles[0] = dRs[iC][jC];
            angles[1] = dRs[iC][kC];
            angles[2] = dRs[iC][lC];
            angles[3] = dRs[jC][kC];
            angles[4] = dRs[jC][lC];
            angles[5] = dRs[kC][lC];

            double minAngle=999;
            for (unsigned int iA=0; iA!=nAngles; ++iA) {
              minAngle = TMath::Min(minAngle,angles[iA]);
            }

            ecfn += pTs[iC] * pTs[jC] * pTs[kC] * pTs[lC] * minAngle / norm;
          }
        }
      }
    }      
  }

  delete pTs;
  for (unsigned int iC=0; iC!=nC; ++iC) {
    delete dRs[iC];
  }

  return ecfn;
}
*/
