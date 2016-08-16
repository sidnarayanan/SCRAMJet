#include "SCRAMJet/Producer/interface/GenParticleFiller.h"
#include "SCRAMJet/Producer/interface/Producer.h"

using namespace scramjet;

GenParticleFiller::GenParticleFiller(TString n):
    BaseFiller()
{
  // data = new TClonesArray("scramjet::PGenParticle",8000);
  data = new VGenParticle();
  treename = n;
}

GenParticleFiller::~GenParticleFiller(){
  delete data;
}

void GenParticleFiller::init(TTree *t) {
  // PGenParticle::Class()->IgnoreTObjectStreamer();
  t->Branch(treename.Data(),&data,99);
}

int GenParticleFiller::analyze(const edm::Event& iEvent){
    // data->Clear();
    for (auto d : *data)
      delete d;
    data->clear(); 

    if (iEvent.isRealData()) return 0;

    iEvent.getByToken(packed_token, packed_handle);
    iEvent.getByToken(pruned_token, pruned_handle);

    std::map<const reco::Candidate *,unsigned int> genIndices;
    std::vector<const reco::Candidate *> savedParticles;

    for ( unsigned int i=0;i < packed_handle->size() ;++i)
    {
        const auto gen  = & (*packed_handle)[i];
        if (gen->pt() < minPt ) continue;
        int pdg = gen->pdgId();
        int apdg = abs(pdg);

        //FILL
        if ( apdg == 11 or apdg == 13 or apdg == 22  // e - mu - gamma
                or (apdg >=12 and apdg<=16) // neutrinos
                or apdg > 1000000  // DM particles
            )
        {
            // const int idx = data->GetEntries();
            // assert(idx<data->GetSize());

            // new ((*data)[idx]) PGenParticle();
            // PGenParticle *particle = (PGenParticle*)(data->At(idx));

            PGenParticle *particle = new PGenParticle();

            particle->pt = gen->pt();
            particle->eta = gen->eta();
            particle->phi = gen->phi();
            particle->m = gen->mass();
            particle->pdgid = pdg;

            data->push_back(particle);

            genIndices[gen] = data->size()-1; // save gen particle pointers
            savedParticles.push_back(gen);     // so we can associate mothers later

        }

    }

    for (unsigned int i=0;i<pruned_handle->size() ;++i)
    {
        const auto gen = &(*pruned_handle)[i];
        if (gen->pt()  < minPt ) continue;
        int pdg = gen->pdgId();
        int apdg = abs(pdg);
        if (gen->status() == 1) continue; //packed
        if (!gen->isLastCopy()) continue;

        if ( apdg == 15 or  // tau (15)
                (apdg >= 23 and apdg <26 ) or   // Z(23) W(24) H(25)
                apdg <= 6 or // quarks 
                apdg == 21 or // gluons (21)
                apdg > 1000000 or // BSM
                apdg == 11 or apdg == 13 // leps
            )
        {
            // const int idx = data->GetEntries();
            // assert(idx<data->GetSize());

            // new ((*data)[idx]) PGenParticle();
            // PGenParticle *particle = (PGenParticle*)(data->At(idx));

            PGenParticle *particle = new PGenParticle();

            particle->pt = gen->pt();
            particle->eta = gen->eta();
            particle->phi = gen->phi();
            particle->m = gen->mass();
            particle->pdgid = pdg;

            data->push_back(particle);

            genIndices[gen] = data->size()-1; // save gen particle pointers
            savedParticles.push_back(gen);     // so we can associate mothers later

        }
    }

    for ( unsigned int i=0;i < savedParticles.size() ;++i) // repeat loop to associate parents
                                                           // this loop is O(N*M*logN) where N is the number of saved particles and M the average number of mothers per particle
    {
        const auto gen = savedParticles[i];
        PGenParticle *particle = data->at(genIndices[gen]);
        if (gen->numberOfMothers()==0) {
            particle->parent = -1;
            continue;
        }         
        int motherIdx=-1;
        const reco::Candidate *gen_parent = gen;
        while (gen_parent->numberOfMothers()>0) {
            gen_parent = gen_parent->mother(0);
            auto gen_parent_ = genIndices.find(gen_parent);
            if (gen_parent_==genIndices.end())
                continue;
            motherIdx=gen_parent_->second;
            break;
        }
        particle->parent = motherIdx;
    }

    return 0;
}

