
import FWCore.ParameterSet.Config as cms
#-----------------------FAT JET CLUSTERING-----------------------
from RecoJets.JetProducers.ak4PFJets_cfi import ak4PFJets
from RecoJets.JetProducers.ak4PFJetsPuppi_cfi import ak4PFJetsPuppi
from RecoJets.JetProducers.ak4GenJets_cfi import ak4GenJets
from PhysicsTools.PatAlgos.tools.pfTools import *
from RecoJets.JetProducers.nJettinessAdder_cfi import Njettiness
from RecoBTag.Configuration.RecoBTag_cff import *
from RecoJets.JetProducers.QGTagger_cfi import *


def initFatJets(process,isData):
  # adaptPVs(process, pvCollection=cms.InputTag('offlineSlimmedPrimaryVertices'))
  isMC = not isData

  ########################################
  ##         INITIAL SETUP              ##
  ########################################

  ## Load standard PAT objects
  process.load("PhysicsTools.PatAlgos.producersLayer1.patCandidates_cff")
  process.load("PhysicsTools.PatAlgos.selectionLayer1.selectedPatCandidates_cff")

  process.pfCHS = cms.EDFilter("CandPtrSelector", src = cms.InputTag("packedPFCandidates"), cut = cms.string("fromPV"))
  addGenNoNu=False
  if isMC and not hasattr(process,'packedGenParticlesForJetsNoNu'):
    process.packedGenParticlesForJetsNoNu = cms.EDFilter("CandPtrSelector", src = cms.InputTag("packedGenParticles"), cut = cms.string("abs(pdgId) != 12 && abs(pdgId) != 14 && abs(pdgId) != 16"))
    addGenNoNu=True

  if isMC and addGenNoNu:
    process.fatjetInitSequence = cms.Sequence(
        process.packedGenParticlesForJetsNoNu
    )
  else:
    process.fatjetInitSequence = cms.Sequence()
  process.fatjetInitSequence += process.pfCHS

  return process.fatjetInitSequence

def makeFatJets(process,isData,pfCandidates,algoLabel,jetRadius):
  
  isMC = not isData
  postfix='PFlow'
  if pfCandidates=='particleFlow':
    # mini aod needs a different config
    pfCandidates = 'pfCHS'
  
  if pfCandidates=='pfCHS':
    puMethod='chs'
  else:
    puMethod='puppi'

  rLabel = algoLabel+str(int(jetRadius*10))
  #customLabel = rLabel+puMethod
  customLabel = puMethod+rLabel

  if algoLabel=='CA':
    jetAlgo = 'CambridgeAachen'
  else:
    jetAlgo = 'AntiKt'

  if jetRadius<1:
    sdZcut = 0.1
    sdBeta = 0.0
  else:
    sdZcut = 0.15
    sdBeta = 1.0

  setattr(process,customLabel+'Sequence',cms.Sequence())
  newSeq = getattr(process,customLabel+'Sequence')
  
  ## Various collection names
  genParticles = 'prunedGenParticles'
  pvSource = 'offlineSlimmedPrimaryVertices'
  svSource = 'slimmedSecondaryVertices'
  muSource = 'slimmedMuons'
  elSource = 'slimmedElectrons'
  bTagInfos = [
      'pfImpactParameterTagInfos'
     ,'pfSecondaryVertexTagInfos'
     ,'pfInclusiveSecondaryVertexFinderTagInfos'
     ,'softPFMuonsTagInfos'
     ,'softPFElectronsTagInfos'
  ]
  ## b-tag discriminators
  bTagDiscriminators = [
      'pfCombinedSecondaryVertexV2BJetTags'
      ,'pfCombinedInclusiveSecondaryVertexV2BJetTags'
  ]


  bTagInfosSubjets = [
      'pfImpactParameterTagInfos'
     ,'pfSecondaryVertexTagInfos'
     ,'pfInclusiveSecondaryVertexFinderTagInfos'
     ,'softPFMuonsTagInfos'
     ,'softPFElectronsTagInfos'
  ]
  ## b-tag discriminators
  bTagDiscriminatorsSubjets = [
      'pfCombinedSecondaryVertexV2BJetTags'
      ,'pfCombinedInclusiveSecondaryVertexV2BJetTags'
  ]

  bTagInfosSubjets=['None']
  bTagDiscriminatorsSubjets=['None']

  ### jet clustering ### 
  ########################################
  ##           REMAKE JETS              ##
  ########################################

  addingGenJets = False

  if not(hasattr(process,"genJetsNoNu"+rLabel)) and isMC:
    addingGenJets = True
    setattr(process,"genJetsNoNu"+rLabel, ak4GenJets.clone(
                                           jetAlgorithm = cms.string(jetAlgo),
                                           rParam = cms.double(jetRadius),
                                           src = cms.InputTag("packedGenParticlesForJetsNoNu")
                                         )
    )
  setattr(process,"PFJets"+customLabel, ak4PFJets.clone(
                                                jetAlgorithm = cms.string(jetAlgo),
                                                rParam = cms.double(jetRadius),
                                                src = cms.InputTag(pfCandidates),
                                                jetPtMin = cms.double(180)
                                            )
  )
  if not(hasattr(process,"genJetsNoNuSoftDrop"+rLabel)) and isMC:
    addingGenJets = True
    setattr(process,"genJetsNoNuSoftDrop"+rLabel, getattr(process,'genJetsNoNu'+rLabel).clone(
                                                      R0 = cms.double(jetRadius),
                                                      useSoftDrop = cms.bool(True),
                                                      zcut = cms.double(sdZcut),
                                                      beta = cms.double(sdBeta),
                                                      writeCompound = cms.bool(True),
                                                      useExplicitGhosts = cms.bool(True),
                                                      jetCollInstanceName=cms.string("SubJets")
                                                  )
    )
  setattr(process,"PFJets"+"SoftDrop"+customLabel, getattr(process,'PFJets'+customLabel).clone(
                                                          useSoftDrop = cms.bool(True),
                                                          R0 = cms.double(jetRadius),
                                                          zcut = cms.double(sdZcut),
                                                          beta = cms.double(sdBeta),
                                                          writeCompound = cms.bool(True),
                                                          useExplicitGhosts = cms.bool(True),
                                                          jetCollInstanceName=cms.string("SubJets"),
                                                          jetPtMin = cms.double(180)
                                                      )
  )
  process.dump = cms.EDAnalyzer("EventContentAnalyzer")
  if addingGenJets and not(isData):
    print 'addingGenJets', 'genJetsNoNu'+rLabel
    newSeq += getattr(process,'genJetsNoNu'+rLabel)
    newSeq += getattr(process,'genJetsNoNuSoftDrop'+rLabel)
  newSeq += getattr(process,'PFJets'+customLabel)
  newSeq += getattr(process,'PFJets'+"SoftDrop"+customLabel)
  
  ########################################
  ##           SUBSTRUCTURE             ##
  #######################################

  setattr(process,customLabel+'Njettiness',                          
          Njettiness.clone(                                      
            src = cms.InputTag('PFJets'+customLabel),       
            R0 = cms.double(jetRadius),
            Njets = cms.vuint32(1,2,3,4)
          )
  )

  setattr(process,customLabel+'SDKinematics',
      cms.EDProducer('RecoJetDeltaRValueMapProducer',
            src = cms.InputTag('PFJets'+customLabel),
            matched = cms.InputTag('PFJets'+"SoftDrop"+customLabel),
            distMax = cms.double(1.5),
            values = cms.vstring('mass'),
            valueLabels = cms.vstring('Mass'),
      )
  )

  newSeq += getattr(process,customLabel+'SDKinematics')
  newSeq += getattr(process,customLabel+'Njettiness')

  ### subjet qg-tagging ###

  setattr(process,customLabel+'SubQGTag',
      QGTagger.clone(
        srcJets = cms.InputTag('PFJets'+"SoftDrop"+customLabel,'SubJets'),
        jetsLabel = cms.string('QGL_AK4PFchs')
      )
  )

  newSeq += getattr(process,customLabel+'SubQGTag')

  ### subjet b-tagging ###

  setattr(process,customLabel+'PFImpactParameterTagInfos',
      pfImpactParameterTagInfos.clone(
          jets      = cms.InputTag('PFJets'+"SoftDrop"+customLabel,'SubJets'),
          maxDeltaR = cms.double(0.4),
          primaryVertex = cms.InputTag('offlineSlimmedPrimaryVertices'),
          candidates = cms.InputTag('packedPFCandidates')
      )
  )

  setattr(process,customLabel+'PFInclusiveSecondaryVertexFinderTagInfos',
    pfInclusiveSecondaryVertexFinderTagInfos.clone(
      trackIPTagInfos = cms.InputTag(customLabel+'PFImpactParameterTagInfos'),
      extSVCollection = cms.InputTag('slimmedSecondaryVertices')
    )
  )
  setattr(process,customLabel+'PFCombinedInclusiveSecondaryVertexV2BJetTags',
    pfCombinedInclusiveSecondaryVertexV2BJetTags.clone(
      tagInfos = cms.VInputTag( 
        cms.InputTag(customLabel+"PFImpactParameterTagInfos"), 
        cms.InputTag(customLabel+"PFInclusiveSecondaryVertexFinderTagInfos") 
      )
    )
  )
  newSeq += getattr(process,customLabel+'PFImpactParameterTagInfos')
  newSeq += getattr(process,customLabel+'PFInclusiveSecondaryVertexFinderTagInfos')
  newSeq += getattr(process,customLabel+'PFCombinedInclusiveSecondaryVertexV2BJetTags')
  
  bTagInfos = ['None']
  bTagDiscriminators = ['None']

  ########################################
  ##          MAKE PAT JETS             ##
  ########################################

  addJetCollection(
      process,
      labelName='PF'+customLabel,
      jetSource=cms.InputTag('PFJets'+customLabel),
      algo=algoLabel,           # needed for jet flavor clustering
      rParam=jetRadius, # needed for jet flavor clustering
      pfCandidates = cms.InputTag('packedPFCandidates'),
      pvSource = cms.InputTag(pvSource),
      svSource = cms.InputTag(svSource),
      muSource = cms.InputTag(muSource),
      elSource = cms.InputTag(elSource),
      btagInfos = bTagInfos,
      btagDiscriminators = bTagDiscriminators,
      genJetCollection = cms.InputTag('genJetsNoNu'+rLabel),
      genParticles = cms.InputTag(genParticles),
      getJetMCFlavour = False, # jet flavor disabled
  )
  getattr(process,'selectedPatJetsPF'+customLabel).cut = cms.string("abs(eta) < " + str(2.5))
  ## SOFT DROP ##
  addJetCollection(
      process,
      labelName='SoftDropPF'+customLabel,
      jetSource=cms.InputTag('PFJets'+"SoftDrop"+customLabel),
      pfCandidates = cms.InputTag('packedPFCandidates'),
      algo=algoLabel,
      rParam=jetRadius,
      btagInfos = ['None'],
      btagDiscriminators = ['None'],
      genJetCollection = cms.InputTag('genJetsNoNu'+rLabel),
      genParticles = cms.InputTag(genParticles),
      getJetMCFlavour = False, # jet flavor disabled
  )
  addJetCollection(
      process,
      labelName='SoftDropSubjetsPF'+customLabel,
      jetSource=cms.InputTag('PFJets'+"SoftDrop"+customLabel,'SubJets'),
      algo=algoLabel,  
      rParam=jetRadius, 
      pfCandidates = cms.InputTag('packedPFCandidates'),
      pvSource = cms.InputTag(pvSource),
      svSource = cms.InputTag(svSource),
      muSource = cms.InputTag(muSource),
      elSource = cms.InputTag(elSource),
      btagInfos = bTagInfosSubjets,
      btagDiscriminators = bTagDiscriminatorsSubjets,
      genJetCollection = cms.InputTag('genJetsNoNuSoftDrop'+rLabel,'SubJets'),
      genParticles = cms.InputTag(genParticles),
      explicitJTA = True,  # needed for subjet b tagging
      svClustering = True, # needed for subjet b tagging
      fatJets = cms.InputTag('PFJets'+customLabel),              # needed for subjet flavor clustering
      groomedFatJets = cms.InputTag('PFJets'+"SoftDrop"+customLabel), # needed for subjet flavor clustering
      runIVF = False,
      getJetMCFlavour = False, # jet flavor disabled
  )
  
  isMC = not(isData)
  if isMC:
    newSeq += getattr(process,'patJetPartonMatchPF'+customLabel)
    newSeq += getattr(process,'patJetGenJetMatchPF'+customLabel)
  newSeq += getattr(process,'patJetsPF'+customLabel)
  newSeq += getattr(process,'selectedPatJetsPF'+customLabel)
  
  if isMC:
    newSeq += getattr(process,'patJetPartonMatchSoftDropPF'+customLabel)
    newSeq += getattr(process,'patJetGenJetMatchSoftDropPF'+customLabel)
  newSeq += getattr(process,'patJetsSoftDropPF'+customLabel)
  newSeq += getattr(process,'selectedPatJetsSoftDropPF'+customLabel)
  
  if isMC:
    newSeq += getattr(process,'patJetPartonMatchSoftDropSubjetsPF'+customLabel)
    newSeq += getattr(process,'patJetGenJetMatchSoftDropSubjetsPF'+customLabel)
  newSeq += getattr(process,'patJetsSoftDropSubjetsPF'+customLabel)
  newSeq += getattr(process,'selectedPatJetsSoftDropSubjetsPF'+customLabel)
  
  setattr(process,"selectedPatJetsSoftDropPF"+"Packed"+customLabel, 
          cms.EDProducer("BoostedJetMerger",    
                          jetSrc=cms.InputTag("selectedPatJetsSoftDropPF"+customLabel),
                          subjetSrc=cms.InputTag("selectedPatJetsSoftDropSubjetsPF"+customLabel)  
          )
  )
  ## PACK ##
  setattr(process,"packedPatJetsPF"+customLabel, 
          cms.EDProducer("JetSubstructurePacker",
                          jetSrc = cms.InputTag('selectedPatJetsPF'+customLabel),
                          distMax = cms.double(jetRadius),
                          algoTags = cms.VInputTag(),
                          algoLabels = cms.vstring(),
                          fixDaughters = cms.bool(False)
                        )
  )
  getattr(process,"packedPatJetsPF"+customLabel).algoTags.append(
      cms.InputTag('selectedPatJetsSoftDropPF'+"Packed"+customLabel)
  )
  getattr(process,"packedPatJetsPF"+customLabel).algoLabels.append(
      'SoftDrop'
  )
  getattr(process,'patJetsPF'+customLabel).userData.userFloats.src += [customLabel+'Njettiness:tau1']
  getattr(process,'patJetsPF'+customLabel).userData.userFloats.src += [customLabel+'Njettiness:tau2']
  getattr(process,'patJetsPF'+customLabel).userData.userFloats.src += [customLabel+'Njettiness:tau3']
  getattr(process,'patJetsPF'+customLabel).userData.userFloats.src += [customLabel+'Njettiness:tau4']
  getattr(process,'patJetsPF'+customLabel).userData.userFloats.src += [customLabel+'SDKinematics:Mass']

  for m in ['patJetsPF'+customLabel,'patJetsSoftDropSubjetsPF'+customLabel]:
    if hasattr(process,m) and getattr( getattr(process,m),'addBTagInfo'):
      setattr( getattr(process,m), 'addTagInfos', cms.bool(True))
    #if hasattr(process,m):
    #  setattr( getattr(process,m), 'addJetFlavourInfo', cms.bool(True))

  newSeq += cms.Sequence(
      getattr(process,'selectedPatJetsSoftDropPF'+"Packed"+customLabel)+
      getattr(process,'packedPatJetsPF'+customLabel)
  )

#  if isData:
#      toRemove = ['Photons', 'Electrons','Muons', 'Taus', 'Jets', 'METs', 'PFElectrons','PFTaus','PFMuons']
#      removeMCMatching(process, toRemove, outputModules = [])
  return newSeq
