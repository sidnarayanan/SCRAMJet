import FWCore.ParameterSet.Config as cms

from subprocess import check_output
import os


#------------------------------------------------------
SCRAMJet = cms.EDAnalyzer("Producer",

    info = cms.string("SCRAMJet"),
    cmssw = cms.string( os.environ['CMSSW_VERSION'] ) , # no need to ship it with the grid option

    vertices = cms.InputTag("offlineSlimmedPrimaryVertices"),
    rho = cms.InputTag("fixedGridRhoFastjetAll"),
    muons = cms.InputTag("slimmedMuons"),
    electrons = cms.InputTag("slimmedElectrons"),
    taus = cms.InputTag("slimmedTaus"),
    photons = cms.InputTag("slimmedPhotons"),

    doCHSCands = cms.bool(True),
    doPuppiCands = cms.bool(True),
    savePuppiCA15Cands = cms.bool(True), # not strictly necessary, but speeds things up offline
    saveCHSCA15Cands = cms.bool(True), 
    savePuppiAK8Cands = cms.bool(True),
    saveCHSAK8Cands = cms.bool(True), 

    chsAK4 = cms.InputTag("slimmedJets"),
    puppiAK4 = cms.InputTag("patJetsPFAK4Puppi"),
    #chsAK8 = cms.InputTag("slimmedJetsAK8"),
    chsAK8 = cms.InputTag("packedPatJetsPFchsAK8"),
    puppiAK8 = cms.InputTag("packedPatJetsPFpuppiAK8"),
    chsCA15 = cms.InputTag("packedPatJetsPFchsCA15"),
    puppiCA15 = cms.InputTag("packedPatJetsPFpuppiCA15"),

    mets = cms.InputTag("slimmedMETs"),
    metsPuppi = cms.InputTag("type1PuppiMET"),
    metsPuppiUncorrected = cms.InputTag("pfMETPuppi"),

    puppiPFCands = cms.InputTag("puppi"),
    chsPFCands = cms.InputTag('packedPFCandidates'),
    #chsPFCands = cms.InputTag('pfCHS'),

    # gen
    generator = cms.InputTag("generator"),
    genjets = cms.InputTag("slimmedGenJets"),
    prunedgen = cms.InputTag("prunedGenParticles"),
    packedgen = cms.InputTag("packedGenParticles"),

    #ak4
    minAK4Pt  = cms.double (15.),
    maxAK4Eta = cms.double (4.7),

    #ak8
    minAK8Pt  = cms.double (180.),
    maxAK8Eta = cms.double (2.5),

    #ca15
    minCA15Pt  = cms.double (180.),
    maxCA15Eta = cms.double (2.5),

    #gen
    minGenParticlePt = cms.double(5.),
    minGenJetPt = cms.double(20.),
                      
)
#------------------------------------------------------


