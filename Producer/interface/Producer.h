#ifndef SCRAMJET_H
#define SCRAMJET_H

#include "SCRAMJet/Producer/interface/Includes.h"
#include "SCRAMJet/Producer/interface/BaseFiller.h"

class Producer : public edm::EDAnalyzer {
    public:
        explicit Producer(const edm::ParameterSet&);
        ~Producer();
        static void fillDescriptions(edm::ConfigurationDescriptions& descriptions);

    private:
        virtual void beginJob() override;
        virtual void analyze(const edm::Event&, const edm::EventSetup&) override;
        virtual void endJob() override;

        virtual void beginRun(edm::Run const&, edm::EventSetup const&) override;
        virtual void endRun(edm::Run const&, edm::EventSetup const&) override;
        virtual void beginLuminosityBlock(edm::LuminosityBlock const&, edm::EventSetup const&) override;
        virtual void endLuminosityBlock(edm::LuminosityBlock const&, edm::EventSetup const&) override;

        TTree *tree_;

        edm::Service<TFileService> fileService_;

        std::vector<BaseFiller*> obj;
};



#endif
