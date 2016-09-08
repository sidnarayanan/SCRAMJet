#include "fastjet/PseudoJet.hh"
#include "fastjet/JetDefinition.hh"
#include "fastjet/ClusterSequence.hh"
#include "fastjet/contrib/NjettinessPlugin.hh" 
#include <vector>
#include <map>
#include "TMath.h"

#include "QjetsPlugin.h"
#include "Qjets.h"

struct qjetwrapper {
  fastjet::PseudoJet jet;
  double tau32;
  double tau21;
};

qjetwrapper getQjet(std::vector<fastjet::PseudoJet> &constituents, qjets::QjetsPlugin *qplugin, fastjet::JetDefinition *qdef, int seed, fastjet::contrib::Njettiness *tau=0) {
  qplugin->SetRandSeed(seed);
  fastjet::ClusterSequence qseq(constituents,*qdef);
  fastjet::PseudoJet dummyJet(0,0,0,0);
  
  qjetwrapper wrapper;
  wrapper.tau32=0; wrapper.tau21=0;

  std::vector<fastjet::PseudoJet> qoutjets = fastjet::sorted_by_pt(qseq.inclusive_jets(10.));
  if (qoutjets.size()==0)
    wrapper.jet=dummyJet;
  else if (qoutjets.size()>1 && qoutjets[1].pt() > 0.1*qoutjets[0].pt())
    wrapper.jet=dummyJet;
  else {
    wrapper.jet=qoutjets[0];
    if (tau) {
      double tau3=tau->getTau(3,wrapper.jet.constituents());
      double tau2=tau->getTau(2,wrapper.jet.constituents());
      double tau1=tau->getTau(1,wrapper.jet.constituents());
//      fprintf(stderr,"%f %f %f\n",tau3,tau2,tau1);
      wrapper.tau32=tau3/tau2;
      wrapper.tau21=tau2/tau1;
    }
  }

  return wrapper;
}

std::vector<qjetwrapper> getQjets(std::vector<fastjet::PseudoJet> &constituents, qjets::QjetsPlugin *qplugin, fastjet::JetDefinition *qdef, int seed, unsigned int nQ, fastjet::contrib::Njettiness *tau=0) {
  std::vector<qjetwrapper> q_jets; q_jets.reserve(nQ);
  for (unsigned int iQ=0; iQ!=nQ; ++iQ) {
    q_jets.push_back( getQjet(constituents,qplugin,qdef,seed*10000+iQ,tau) );
  }
  return q_jets;
}

double getMean(std::vector<double> xs) {
  double mean=0;
  for (auto x : xs) 
    mean += x;
  mean /= xs.size();
  return mean;
}

double getRMS(std::vector<double> xs,double mean=0) {
  if (mean==0) 
    mean = getMean(xs);
  double rms=0;
  for (auto x : xs) {
    rms += (x-mean)*(x-mean);
  }
  rms /= xs.size();
  return TMath::Sqrt(rms);
}

typedef double (*JetQuantity)(qjetwrapper);

double qVolQuantity(std::vector<qjetwrapper> &q_jets, JetQuantity eval) {
  std::vector<double> xs; xs.reserve(q_jets.size());
  for (auto jet : q_jets) {
    if (jet.jet.pt()<0.1)
      continue;
    xs.push_back( (*eval)(jet) );
  }
  
  double mean = getMean(xs);
  double rms = getRMS(xs,mean);

  return rms/mean;
}

double qVolTau(std::vector<fastjet::PseudoJet> &q_jets,fastjet::contrib::Njettiness *tau,int n,int m) {
  std::vector<double> xs; xs.reserve(q_jets.size());
  for (auto jet : q_jets) {
    if (jet.pt()<0.1)
      continue;
    xs.push_back( tau->getTau(n,jet.constituents())/tau->getTau(m,jet.constituents()) );
  }
  
  double mean = getMean(xs);
  double rms = getRMS(xs,mean);

  return rms/mean;
}
