#!/usr/bin/env python

from ROOT import gSystem,gROOT
import ROOT as root
from re import sub
from sys import argv,exit
from os import system,getenv

if __name__ == "__main__":

  gROOT.LoadMacro("${CMSSW_BASE}/src/SCRAMJet/Analyzer/interface/Analyzer.h")
  gSystem.Load('libSCRAMJetAnalyzer.so')
  gSystem.Load('libSCRAMJetObjects.so')
  
  def fn(fullPath):

    skimmer = root.Analyzer()
   
    skimmer.maxEvents = 100
    skimmer.isData=False
    skimmer.processType = root.Analyzer.kTop
    eosPath = 'root://eoscms//eos/cms/store/user/%s'%(getenv('USER'))
    cernboxPath = 'root://eosuser//eos/user/%s/%s'%(getenv('USER')[0],getenv('USER'))
    cernboxBPath = 'root://eosuser//eos/user/b/bmaier'

    fullPath = sub(r'\${CERNBOXB}',cernboxBPath,sub(r'\${CERNBOX}',cernboxPath,sub(r'\${EOS}',eosPath,fullPath)))
    fin = root.TFile.Open(fullPath)

    print fullPath
    print fin

    tree = fin.FindObjectAny("events")
    print tree

    skimmer.SetOutputFile('/tmp/%s/testskim.root'%getenv('USER'))
    skimmer.Init(tree)
    skimmer.AddFatJetFromTree("puppiCA15","puppiCA15",root.Analyzer.kPuppi,1.5,root.Analyzer.kCA)

    skimmer.Run()
    print 'done running'
    skimmer.Terminate()
    print 'done terminating'

fn(argv[1]) 
