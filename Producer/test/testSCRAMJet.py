import FWCore.ParameterSet.Config as cms
import FWCore.ParameterSet.VarParsing as VarParsing
import re
import os

process = cms.Process("SCRAMJet")
cmssw_base = os.environ['CMSSW_BASE']


options = VarParsing.VarParsing ('analysis')
options.register('isData',
        False,
        VarParsing.VarParsing.multiplicity.singleton,
        VarParsing.VarParsing.varType.bool,
        "True if running on Data, False if running on MC")

options.register('isGrid', False, VarParsing.VarParsing.multiplicity.singleton,VarParsing.VarParsing.varType.bool,"Set it to true if running on Grid")

options.parseArguments()
isData = options.isData


process.load("FWCore.MessageService.MessageLogger_cfi")
# If you run over many samples and you save the log, remember to reduce
# the size of the output by prescaling the report of the event number
process.MessageLogger.cerr.FwkReport.reportEvery = 1

process.maxEvents = cms.untracked.PSet( input = cms.untracked.int32(10) )

if isData:
   fileList = [
       '/store/data/Run2016B/MET/MINIAOD/PromptReco-v2/000/273/150/00000/2CF02CDC-D819-E611-AA68-02163E011A52.root'
       ]
else:
   fileList = [
       '/store/mc/RunIISpring16MiniAODv2/ZprimeToWW_narrow_M-1000_13TeV-madgraph/MINIAODSIM/PUSpring16RAWAODSIM_80X_mcRun2_asymptotic_2016_miniAODv2_v0-v1/00000/54EEF262-7B27-E611-9468-A0369F7FC070.root'
       ]
### do not remove the line below!
###FILELIST###

process.source = cms.Source("PoolSource",
	skipEvents = cms.untracked.uint32(0),
    	fileNames = cms.untracked.vstring(fileList)
        )

# ---- define the output file -------------------------------------------
process.TFileService = cms.Service("TFileService",
        closeFileFast = cms.untracked.bool(True),
        fileName = cms.string("scramjet.root"),
        )

##----------------GLOBAL TAG ---------------------------
# used by photon id and jets
process.load("Configuration.Geometry.GeometryIdeal_cff") 
process.load('Configuration.StandardSequences.Services_cff')
process.load("Configuration.StandardSequences.MagneticField_cff")

#mc https://twiki.cern.ch/twiki/bin/view/CMSPublic/SWGuideFrontierConditions#Global_Tags_for_Run2_MC_Producti
process.load('Configuration.StandardSequences.FrontierConditions_GlobalTag_condDBv2_cff')
if (isData):
        process.GlobalTag.globaltag = '80X_dataRun2_Prompt_v8'
else:
        process.GlobalTag.globaltag = '80X_mcRun2_asymptotic_2016_miniAODv2'

### LOAD DATABASE
from CondCore.DBCommon.CondDBSetup_cfi import *
#from CondCore.CondDB.CondDB_cfi import *

######## LUMI MASK
if isData and not options.isGrid and False: ## dont load the lumiMaks, will be called by crab
    #pass
    import FWCore.PythonUtilities.LumiList as LumiList
    ## SILVER
    process.source.lumisToProcess = LumiList.LumiList(filename='/afs/cern.ch/cms/CAF/CMSCOMM/COMM_DQM/certification/Collisions15/13TeV/Cert_246908-260627_13TeV_PromptReco_Collisions15_25ns_JSON_Silver_v2.txt').getVLuminosityBlockRange()
    print "FIX JSON"

process.load('SCRAMJet.Producer.SCRAMJet_cfi')

#### RECOMPUTE JEC From GT ###
from PhysicsTools.PatAlgos.tools.jetTools import updateJetCollection
 
jecLevels= ['L1FastJet',  'L2Relative', 'L3Absolute']
if options.isData:
        jecLevels =['L1FastJet',  'L2Relative', 'L3Absolute', 'L2L3Residual']
 
updateJetCollection(
    process,
    jetSource = process.SCRAMJet.chsAK4,
    labelName = 'UpdatedJEC',
    jetCorrections = ('AK4PFchs', cms.vstring(jecLevels), 'None')  # Do not forget 'L2L3Residual' on data!
)

'''
updateJetCollection(
    process,
    jetSource = process.SCRAMJet.chsAK8,
    labelName = 'UpdatedJECAK8',
    jetCorrections = ('AK8PFchs', cms.vstring(jecLevels), 'None')  # Do not forget 'L2L3Residual' on data!
)
'''

process.SCRAMJet.chsAK4=cms.InputTag('updatedPatJetsUpdatedJEC')
#process.SCRAMJet.chsAK8=cms.InputTag('updatedPatJetsUpdatedJECAK8')
process.jecSequence = cms.Sequence( process.patJetCorrFactorsUpdatedJEC* process.updatedPatJetsUpdatedJEC)
#process.jecSequence = cms.Sequence( process.patJetCorrFactorsUpdatedJEC* process.updatedPatJetsUpdatedJEC* process.patJetCorrFactorsUpdatedJECAK8* process.updatedPatJetsUpdatedJECAK8)

############ RECOMPUTE MET #######################
from PhysicsTools.PatUtils.tools.runMETCorrectionsAndUncertainties import runMetCorAndUncFromMiniAOD
runMetCorAndUncFromMiniAOD(process,
           isData=isData,
           )

############ RUN CLUSTERING ##########################
process.puppiSequence = cms.Sequence()
process.puppiJetMETSequence = cms.Sequence()
process.jetSequence = cms.Sequence()

# run puppi algo
process.load('CommonTools.PileupAlgos.Puppi_cff')

process.puppi.candName   = cms.InputTag('packedPFCandidates')
process.puppi.vertexName = cms.InputTag('offlineSlimmedPrimaryVertices')

process.puppiNoLep = process.puppi.clone()
process.puppi.useExistingWeights = False
process.puppiNoLep.useExistingWeights = False
process.puppiNoLep.useWeightsNoLep = True

process.load('CommonTools/PileupAlgos/PhotonPuppi_cff')
process.puppiPhoton.weightsName = 'puppiNoLep'
process.puppiForMET = cms.EDProducer("CandViewMerger",src = cms.VInputTag( 'puppiPhoton'))

process.puppiSequence += process.puppi
process.puppiSequence += process.puppiNoLep
process.puppiSequence += process.puppiPhoton
process.puppiSequence += process.puppiForMET

# recompute ak4 jets
from RecoJets.JetProducers.ak4PFJets_cfi import ak4PFJets
from RecoJets.JetProducers.ak4GenJets_cfi import ak4GenJets
from PhysicsTools.PatAlgos.tools.pfTools import *

process.ak4PFJetsPuppi = ak4PFJets.clone(src=cms.InputTag('puppiNoLep'))
process.puppiJetMETSequence += process.ak4PFJetsPuppi
if not isData:
    process.packedGenParticlesForJetsNoNu = cms.EDFilter("CandPtrSelector", 
      src = cms.InputTag("packedGenParticles"), 
      cut = cms.string("abs(pdgId) != 12 && abs(pdgId) != 14 && abs(pdgId) != 16")
    )
    process.ak4GenJetsNoNu = ak4GenJets.clone(src = 'packedGenParticlesForJetsNoNu')
    process.puppiJetMETSequence += process.packedGenParticlesForJetsNoNu
    process.puppiJetMETSequence += process.ak4GenJetsNoNu

# btag and patify jets for access later
addJetCollection(
  process,
  labelName = 'PFAK4Puppi',
  jetSource=cms.InputTag('ak4PFJetsPuppi'),
  algo='AK4',
  rParam=0.4,
  pfCandidates = cms.InputTag('packedPFCandidates'),
  pvSource = cms.InputTag('offlineSlimmedPrimaryVertices'),
  svSource = cms.InputTag('slimmedSecondaryVertices'),
  muSource = cms.InputTag('slimmedMuons'),
  elSource = cms.InputTag('slimmedElectrons'),
  btagInfos = [
      'pfImpactParameterTagInfos'
     ,'pfInclusiveSecondaryVertexFinderTagInfos'
  ],
  btagDiscriminators = [
     'pfCombinedInclusiveSecondaryVertexV2BJetTags'
  ],
  genJetCollection = cms.InputTag('ak4GenJetsNoNu'),
  genParticles = cms.InputTag('prunedGenParticles'),
  getJetMCFlavour = False, # jet flavor disabled
)

if not isData:
  process.puppiJetMETSequence += process.patJetPartonMatchPFAK4Puppi
  process.puppiJetMETSequence += process.patJetGenJetMatchPFAK4Puppi
process.puppiJetMETSequence += process.pfImpactParameterTagInfosPFAK4Puppi
process.puppiJetMETSequence += process.pfInclusiveSecondaryVertexFinderTagInfosPFAK4Puppi
process.puppiJetMETSequence += process.pfCombinedInclusiveSecondaryVertexV2BJetTagsPFAK4Puppi
process.puppiJetMETSequence += process.patJetsPFAK4Puppi

# compute puppi MET
from RecoMET.METProducers.PFMET_cfi import pfMet
process.pfMETPuppi = pfMet.clone()
process.pfMETPuppi.src = cms.InputTag('puppiForMET')
process.pfMETPuppi.calculateSignificance = False
process.puppiJetMETSequence += process.pfMETPuppi

# correct puppi jets
jeclabel = 'DATA' if isData else 'MC'
process.jec =  cms.ESSource("PoolDBESSource",
                    CondDBSetup,
                    toGet = cms.VPSet(
              cms.PSet(record  = cms.string('JetCorrectionsRecord'),
                       tag     = cms.string('JetCorrectorParametersCollection_Spring16_25nsV6_'+jeclabel+'_AK4PFPuppi'),
                       label   = cms.untracked.string('AK4Puppi')
                       ),
               cms.PSet(record  = cms.string('JetCorrectionsRecord'),
                        tag     = cms.string('JetCorrectorParametersCollection_Spring16_25nsV6_'+jeclabel+'_AK8PFPuppi'),
                        label   = cms.untracked.string('AK8Puppi')
                        ),
              cms.PSet(record  = cms.string('JetCorrectionsRecord'),
                       tag     = cms.string('JetCorrectorParametersCollection_Spring16_25nsV6_'+jeclabel+'_AK4PFchs'),
                       label   = cms.untracked.string('AK4chs')
                       ),
              cms.PSet(record  = cms.string('JetCorrectionsRecord'),
                       tag     = cms.string('JetCorrectorParametersCollection_Spring16_25nsV6_'+jeclabel+'_AK8PFchs'),
                       label   = cms.untracked.string('AK8chs')
                       ),
              cms.PSet(record  = cms.string('JetCorrectionsRecord'),
                       tag     = cms.string('JetCorrectorParametersCollection_Spring16_25nsV6_'+jeclabel+'_AK4PF'),
                       label   = cms.untracked.string('AK4')
                       ),
               cms.PSet(record  = cms.string('JetCorrectionsRecord'),
                        tag     = cms.string('JetCorrectorParametersCollection_Spring16_25nsV6_'+jeclabel+'_AK8PF'),
                        label   = cms.untracked.string('AK8')
                        )
               ),

        )  
if isData:
  #process.jec.connect = cms.string('sqlite:////'+cmssw_base+'/src/NeroProducer/Nero/test/jec/Spring16_25nsV6_DATA.db')
  process.jec.connect = cms.string('sqlite:jec/Spring16_25nsV6_DATA.db')
else:
  process.jec.connect = cms.string('sqlite:jec/Spring16_25nsV6_MC.db')
process.es_prefer_jec = cms.ESPrefer('PoolDBESSource', 'jec')

from JetMETCorrections.Configuration.JetCorrectorsAllAlgos_cff  import *
jetlabel='AK4PFPuppi'
process.ak4PuppiL1  = ak4PFCHSL1FastjetCorrector.clone (algorithm = cms.string(jetlabel))
process.ak4PuppiL2  = ak4PFCHSL2RelativeCorrector.clone(algorithm = cms.string(jetlabel))
process.ak4PuppiL3  = ak4PFCHSL3AbsoluteCorrector.clone(algorithm = cms.string(jetlabel))
process.ak4PuppiRes = ak4PFCHSResidualCorrector.clone  (algorithm = cms.string(jetlabel))
process.puppiJetMETSequence += process.ak4PuppiL1
process.puppiJetMETSequence += process.ak4PuppiL2
process.puppiJetMETSequence += process.ak4PuppiL3

process.ak4PuppiCorrector = ak4PFL1FastL2L3Corrector.clone(
        correctors = cms.VInputTag("ak4PuppiL1", 
                                    "ak4PuppiL2",
                                    "ak4PuppiL3")
    )
process.ak4PuppiCorrectorRes = ak4PFL1FastL2L3Corrector.clone(
        correctors = cms.VInputTag("ak4PuppiL1", 
                                    "ak4PuppiL2",
                                    "ak4PuppiL3",
                                    'ak4PuppiRes')
    )
if isData:
    process.puppiJetMETSequence += process.ak4PuppiRes
    process.puppiJetMETSequence += process.ak4PuppiCorrectorRes
    correctorLabel = 'ak4PuppiCorrectorRes'
else:
    process.puppiJetMETSequence += process.ak4PuppiCorrector
    correctorLabel = 'ak4PuppiCorrector'

# correct puppi MET
process.puppiMETcorr = cms.EDProducer("PFJetMETcorrInputProducer",
    src = cms.InputTag('ak4PFJetsPuppi'),
    offsetCorrLabel = cms.InputTag('ak4PuppiL1'),
    jetCorrLabel = cms.InputTag(correctorLabel),
    jetCorrLabelRes = cms.InputTag('ak4PuppiCorrectorRes'),
    jetCorrEtaMax = cms.double(9.9),
    type1JetPtThreshold = cms.double(15.0),
    skipEM = cms.bool(True),
    skipEMfractionThreshold = cms.double(0.90),
    skipMuons = cms.bool(True),
    skipMuonSelection = cms.string("isGlobalMuon | isStandAloneMuon")
)
process.type1PuppiMET = cms.EDProducer("CorrectedPFMETProducer",
    src = cms.InputTag('pfMETPuppi'),
    applyType0Corrections = cms.bool(False),
    applyType1Corrections = cms.bool(True),
    srcCorrections = cms.VInputTag(cms.InputTag('puppiMETcorr', 'type1')),
    applyType2Corrections = cms.bool(False)
)   
process.puppiJetMETSequence += process.puppiMETcorr
process.puppiJetMETSequence += process.type1PuppiMET

from SCRAMJet.Producer.makeFatJets_cff import *
fatjetInitSequence = initFatJets(process,isData)
process.jetSequence += fatjetInitSequence

ak8PuppiSequence  = makeFatJets(process,isData=isData,pfCandidates='puppi',algoLabel='AK',jetRadius=0.8)
ak8CHSSequence    = makeFatJets(process,isData=isData,pfCandidates='pfCHS',algoLabel='AK',jetRadius=0.8)
ca15PuppiSequence = makeFatJets(process,isData=isData,pfCandidates='puppi',algoLabel='CA',jetRadius=1.5)
ca15CHSSequence   = makeFatJets(process,isData=isData,pfCandidates='pfCHS',algoLabel='CA',jetRadius=1.5)

process.jetSequence += ak8PuppiSequence
process.jetSequence += ak8CHSSequence
process.jetSequence += ca15PuppiSequence
process.jetSequence += ca15CHSSequence


###############################


##DEBUG
##print "Process=",process, process.__dict__.keys()
#------------------------------------------------------
process.p = cms.Path(
                        process.jecSequence *
#                        process.fullPatMetSequence *
                        process.puppiSequence *
                        process.puppiJetMETSequence *
                        process.jetSequence *
                        process.SCRAMJet
                    )

## DEBUG -- dump the event content with all the value maps ..
# process.output = cms.OutputModule(
#                 "PoolOutputModule",
#                       fileName = cms.untracked.string('pool.root'),
#                       )
# process.output_step = cms.EndPath(process.output)
# 
# process.schedule = cms.Schedule(
# 		process.p,
# 		process.output_step)
##
