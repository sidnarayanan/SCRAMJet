#include "fastjet/PseudoJet.hh"
#include <vector>
#include <map>
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

class ECFNManager {
public:
  // just a bunch of floats and bools to hold different values of normalized ECFs
  ECFNManager() {
    flags["3_1"]=true; 
    flags["3_2"]=true; 
    flags["3_3"]=true; 
    flags["4_1"]=true; 
    flags["4_2"]=true; 
    flags["4_3"]=false; 
  }
  ~ECFNManager() {}

  std::map<TString,double> ecfns; // maps "N_I" to ECFN
  std::map<TString,bool>   flags; // maps "N_I" to flag

  bool doN1=true, doN2=true, doN3=true, doN4=true;

};

void calcECFN(double beta, std::vector<fastjet::PseudoJet> &constituents, ECFNManager *manager) {
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
  if (manager->doN1) { // N=1
    manager->ecfns["1_1"] = 1;
    manager->ecfns["1_2"] = 1;
    manager->ecfns["1_3"] = 1;
  }
  if (manager->doN2) { // N=2
    double norm = pow(baseNorm,2);
    double val=0;
    for (unsigned int iC=0; iC!=nC; ++iC) {
      for (unsigned int jC=0; jC!=iC; ++jC) {
        val += pTs[iC] * pTs[jC] * dRs[iC][jC] / norm; 
      } // jC
    } // iC
    manager->ecfns["2_1"] = val;
    manager->ecfns["2_2"] = val;
    manager->ecfns["2_3"] = val;
  }

  bool doI1=manager->flags["3_1"];
  bool doI2=manager->flags["3_2"];
  bool doI3=manager->flags["3_3"];
  if (manager->doN3 && (doI1||doI2||doI3)) {
    double norm = pow(baseNorm,3);
    double val1=0,val2=0,val3=0;
    unsigned int nAngles=3;
    std::vector<double> angles(nAngles);

    for (unsigned int iC=0; iC!=nC; ++iC) {
      for (unsigned int jC=0; jC!=iC; ++jC) {
        double val_ij = pTs[iC]*pTs[jC];
        angles[0] = dRs[iC][jC];

        for (unsigned int kC=0; kC!=jC; ++kC) {
          angles[1] = dRs[iC][kC];
          angles[2] = dRs[jC][kC];

          if (doI1||doI2||doI3) {
            double angle_1=999; unsigned int index_1=999;
            for (unsigned int iA=0; iA!=nAngles; ++iA) {
              if (angles[iA]<angle_1) {
                angle_1 = angles[iA];
                index_1 = iA;
              }
            }
            if (doI2||doI3) {
              double angle_2=999; 
              for (unsigned int jA=0; jA!=nAngles; ++jA) {
                if (jA==index_1) continue;
                if (angles[jA]<angle_2) {
                  angle_2 = angles[jA];
                }
              }
              if (doI3) {
                val3 += val_ij * pTs[kC] * angles[0] * angles[1] * angles[2] / norm;
              }
              if (doI2)
                val2 += val_ij * pTs[kC] * angle_1 * angle_2 / norm;
            }
            if (doI1)
              val1 += val_ij * pTs[kC] * angle_1 / norm;
          }

        } // kC
      } // jC
    } // iC
    manager->ecfns["3_1"] = val1;
    manager->ecfns["3_2"] = val2;
    manager->ecfns["3_3"] = val3;
  }

  doI1=manager->flags["4_1"];
  doI2=manager->flags["4_2"];
  if (manager->doN4 && (doI1||doI2)) {
    double norm = pow(baseNorm,4);
    double val1=0,val2=0;
    unsigned int nAngles=6;
    std::vector<double> angles(nAngles);

    for (unsigned int iC=0; iC!=nC; ++iC) {
      for (unsigned int jC=0; jC!=iC; ++jC) {
        double val_ij = pTs[iC]*pTs[jC];
        angles[0] = dRs[iC][jC];

        for (unsigned int kC=0; kC!=jC; ++kC) {
          double val_ijk = val_ij * pTs[kC];
          angles[1] = dRs[iC][kC];
          angles[2] = dRs[jC][kC];

          for (unsigned int lC=0; lC!=kC; ++lC) {
            angles[3] = dRs[jC][kC];
            angles[4] = dRs[jC][lC];
            angles[5] = dRs[kC][lC];

            if (doI1||doI2) {
              double angle_1=999; unsigned int index_1=999;
              for (unsigned int iA=0; iA!=nAngles; ++iA) {
                if (angles[iA]<angle_1) {
                  angle_1 = angles[iA];
                  index_1 = iA;
                }
              }
              if (doI2) {
                double angle_2=999; 
                for (unsigned int jA=0; jA!=nAngles; ++jA) {
                  if (jA==index_1) continue;
                  if (angles[jA]<angle_2) {
                    angle_2 = angles[jA];
                  }
                }
                val2 += val_ijk * pTs[lC] * angle_1 * angle_2 / norm;
              }
              if (doI1)
                val1 += val_ijk * pTs[lC] * angle_1 / norm;
            }
          } // lC
        } // kC
      } // jC
    } // iC
    manager->ecfns["4_1"] = val1;
    manager->ecfns["4_2"] = val2;
    manager->ecfns["4_3"] = 0;
  }

  // cleanup
  delete pTs;
  for (unsigned int iC=0; iC!=nC; ++iC) {
    delete dRs[iC];
  }
}

/*

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
*/
