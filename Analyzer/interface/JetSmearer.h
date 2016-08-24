#ifndef JET_RESOLUTIONS
#define JET_RESOLUTIONS

#include "TF1.h"
#include "TLorentzVector.h"


class JetSmearer { 
public:
  JetSmearer() {
  //Smear in Pt
  fPtSmear = new TF1("JetPt","sqrt(((TMath::Sign(1,[0])*sq([0]/x))+(sq([1])*(x^([3]-1))))+sq([2]))");
  fPtPar0 = new double[10];
  fPtPar1 = new double[10];
  fPtPar2 = new double[10];
  fPtPar3 = new double[10];

  //9 Eta Bins
  fPtPar0[0] = 1.41584; fPtPar1[0] = 0.209477; fPtPar2[0] = 0; fPtPar3[0] = 0.588872;
  fPtPar0[1] = 1.41584; fPtPar1[1] = 0.209477; fPtPar2[1] = 0; fPtPar3[1] = 0.588872;
  fPtPar0[2] = 1.65966; fPtPar1[2] = 0.223683; fPtPar2[2] = 0; fPtPar3[2] = 0.60873;
  fPtPar0[3] = 2.81978; fPtPar1[3] = 0.272373; fPtPar2[3] = 0; fPtPar3[3] = 0.579396;
  fPtPar0[4] = 2.56933; fPtPar1[4] = 0.305802; fPtPar2[4] = 0; fPtPar3[4] = 0.398929;
  fPtPar0[5] = 1.04792; fPtPar1[5] = 0.466763; fPtPar2[5] = 0; fPtPar3[5] = 0.193137;
  fPtPar0[6] = -1.12329; fPtPar1[6] = 0.657891; fPtPar2[6] = 0; fPtPar3[6] = 0.139595;
  fPtPar0[7] = -0.561649; fPtPar1[7] = 0.420293; fPtPar2[7] = 0; fPtPar3[7] = 0.392398;
  fPtPar0[8] = -0.499735; fPtPar1[8] = 0.336391; fPtPar2[8] = 0; fPtPar3[8] = 0.430689;
  fPtPar0[9] = -0.349206; fPtPar1[9] = 0.297831; fPtPar2[9] = 0; fPtPar3[9] = 0.471121;
  //Smear in Eta
  fEtaSmear = new TF1("JetEta","(sqrt((sq([0]/x)+(sq([1])/x))+sq([2]))+([3]/x))+(([4]/x)/sqrt(x))");
  fEtaPar0 = new double[10];
  fEtaPar1 = new double[10];
  fEtaPar2 = new double[10];
  fEtaPar3 = new double[10];
  fEtaPar4 = new double[10];
  //9 bins again
  fEtaPar0[0] = 0.710478; fEtaPar1[0] = 0.143847; fEtaPar2[0] = -2.52411e-05; fEtaPar3[0] = 0.77394; fEtaPar4[0] = -1.89622;
  fEtaPar0[1] = 527.518; fEtaPar1[1] = -0.0143625; fEtaPar2[1] = 0.316441; fEtaPar3[1] =-526.599; fEtaPar4[1] = 0.244142;
  fEtaPar0[2] = 0.494977; fEtaPar1[2] = 1.48277e-06; fEtaPar2[2] = 0.0171135; fEtaPar3[2] = 0.354901; fEtaPar4[2] = 0.554065;
  fEtaPar0[3] = 2.88983; fEtaPar1[3] = -1.71576e-06; fEtaPar2[3] = 0.0402796; fEtaPar3[3] = -1.72649; fEtaPar4[3] = -0.124126;
  fEtaPar0[4] = 0.851656; fEtaPar1[4] = 4.56968e-07; fEtaPar2[4] = 0.0441276; fEtaPar3[4] =-0.101993; fEtaPar4[4] = 0.773812;
  fEtaPar0[5] = 9.64435; fEtaPar1[5] = 0.458594 ; fEtaPar2[5] = 1.92485e-08; fEtaPar3[5] = -8.67043; fEtaPar4[5] = -0.0541106;
  fEtaPar0[6] = 0.343262; fEtaPar1[6] = -3.39452e-07; fEtaPar2[6] = 0.00849674; fEtaPar3[6] = 1.05358; fEtaPar4[6] = -1.24367;
  fEtaPar0[7] = 0.572596; fEtaPar1[7] = -1.09687e-07; fEtaPar2[7] = 0.0094876; fEtaPar3[7] = 0.799819; fEtaPar4[7] = -1.23444;
  fEtaPar0[8] = 0.622981; fEtaPar1[8] = 0.100943; fEtaPar2[8] = 0.00744374; fEtaPar3[8] = 0.317454; fEtaPar4[8] = -0.324557;
  fEtaPar0[9] = 355.708; fEtaPar1[9] = 2.20794; fEtaPar2[9] = 0.032666; fEtaPar3[9] = -354.691; fEtaPar4[9] = -0.857295;
  //No I'm going to kill myself
  fPhiSmear = new TF1("JetPhi","(sqrt((sq([0]/x)+(sq([1])/x))+sq([2]))+([3]/x))+(([4]/x)/sqrt(x))");
  fPhiPar0 = new double[10];
  fPhiPar1 = new double[10];
  fPhiPar2 = new double[10];
  fPhiPar3 = new double[10];
  fPhiPar4 = new double[10];
  //10 bins again
  fPhiPar0[0] = 259.189 ; fPhiPar1[0] = 0.00132792; fPhiPar2[0] = -0.311411; fPhiPar3[0] = -258.647; fPhiPar4[0] = 0;
  fPhiPar0[1] = 0.765787; fPhiPar1[1] = -3.90638e-06; fPhiPar2[1] = -4.70224e-08; fPhiPar3[1] = 0.11831; fPhiPar4[1] = -1.4675;
  fPhiPar0[2] = 2.11446; fPhiPar1[2] = 0.203329; fPhiPar2[2] = -0.0175832; fPhiPar3[2] = -1.67946; fPhiPar4[2] = -0.00853474;
  fPhiPar0[3] = 1.9027; fPhiPar1[3] = -4.56986e-06; fPhiPar2[3] = 0.0304793; fPhiPar3[3] = -1.09124; fPhiPar4[3] = -0.136402;
  fPhiPar0[4] = 11.1957; fPhiPar1[4] = 0.643236; fPhiPar2[4] = 0.00711422; fPhiPar3[4] = -10.7613; fPhiPar4[4] = 0.280927;
  fPhiPar0[5] = 0.00336639; fPhiPar1[5] = 0.0880209; fPhiPar2[5] = -0.0023084; fPhiPar3[5] = 0.214304; fPhiPar4[5] = -0.416353;
  fPhiPar0[6] = 2.92001e-07; fPhiPar1[6] = 0.0718389; fPhiPar2[6] = -0.00385579; fPhiPar3[6] = 0.403668; fPhiPar4[6] = -0.62698;
  fPhiPar0[7] = 0.38469 ; fPhiPar1[7] = 0.0755727; fPhiPar2[7] = -0.0044353; fPhiPar3[7] = 0.453887; fPhiPar4[7] = -1.8947;
  fPhiPar0[8] = 3.32512e-06; fPhiPar1[8] = 0.063941; fPhiPar2[8] = -0.00387593; fPhiPar3[8] = 0.301932; fPhiPar4[8] = -0.825352;
  fPhiPar0[9] = 926.978; fPhiPar1[9] = 2.52747; fPhiPar2[9] = 0.0304001; fPhiPar3[9] = -926.224; fPhiPar4[9] = -1.94117;
  }
  ~JetSmearer() {
    delete fPtSmear;
    delete [] fPtPar0;
    delete [] fPtPar1;
    delete [] fPtPar2;
    delete [] fPtPar3;
    delete fEtaSmear;
    delete [] fEtaPar0;
    delete [] fEtaPar1;
    delete [] fEtaPar2;
    delete [] fEtaPar3;
    delete [] fEtaPar4;
    delete fPhiSmear;
    delete [] fPhiPar0;
    delete [] fPhiPar1;
    delete [] fPhiPar2;
    delete [] fPhiPar3;
    delete [] fPhiPar4;
  }

  void setSmear(int bin);
  void getUncertainties( TLorentzVector & v, double & upx, double & upy, double & upz, double & um );
  // double ptres(double x);
  // double etares(double x);
  // double phires(double x);

  TF1 * fPtSmear; 
  TF1 * fEtaSmear; 
  TF1 * fPhiSmear; 

  double * fPtPar0 ;
  double * fPtPar1 ;
  double * fPtPar2 ;
  double * fPtPar3 ;

  double * fEtaPar0 ;
  double * fEtaPar1 ;
  double * fEtaPar2 ;
  double * fEtaPar3 ;
  double * fEtaPar4 ;

  double * fPhiPar0 ;
  double * fPhiPar1 ;
  double * fPhiPar2 ;
  double * fPhiPar3 ;
  double * fPhiPar4 ;


};


#endif
