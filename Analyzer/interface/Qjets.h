#ifndef _QJETS_
#define _QJETS_
// modified from code found at http://jets.physics.harvard.edu/Qjets/html/Welcome.html
#include <queue>
#include <vector>
#include <list>
#include <algorithm>
#include "fastjet/JetDefinition.hh"
#include "fastjet/PseudoJet.hh"
#include "fastjet/ClusterSequence.hh"

namespace qjets {

  struct jet_distance{
    double dij;
    int j1;
    int j2;
  };

  class JetDistanceCompare{
   public:
    JetDistanceCompare(){};
    bool operator() (const jet_distance& lhs, const jet_distance&rhs) const {return lhs.dij > rhs.dij;};
  };

  class Qjets{
   private:
    unsigned int _seed;
    double _zcut, _dcut, _dcut_fctr, _exp_min, _exp_max, _rigidity, _truncation_fctr;
    bool _rand_seed_set;
    std::map<int,bool> _merged_jets;
    std::priority_queue <jet_distance, std::vector<jet_distance>, JetDistanceCompare> _distances;

    double d_ij(const fastjet::PseudoJet& v1, const fastjet::PseudoJet& v2) const; 
    void ComputeDCut(fastjet::ClusterSequence & cs);

    double Rand();
    bool Prune(jet_distance& jd,fastjet::ClusterSequence & cs);
    bool JetsUnmerged(const jet_distance& jd) const;
    bool JetUnmerged(int num) const;
    void ComputeNewDistanceMeasures(fastjet::ClusterSequence & cs, int new_jet);
    void ComputeAllDistances(const std::vector<fastjet::PseudoJet>& inp);  
    double ComputeMinimumDistance();
    double ComputeNormalization(double dmin);
    jet_distance GetNextDistance();
    bool Same(const jet_distance& lhs, const jet_distance& rhs);

   public:
    Qjets(double zcut, double dcut_fctr, double exp_min, double exp_max, double rigidity, double truncation_fctr);
    void Cluster(fastjet::ClusterSequence & cs);
    void SetRandSeed(unsigned int seed); /* In case you want reproducible behavior */
  };

}
#endif
