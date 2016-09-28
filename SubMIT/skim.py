#!/usr/bin/env python

from ROOT import gSystem,gROOT
import ROOT as root
from re import sub
from sys import argv,exit
from os import system,getenv,path
from json import load as loadJson
from time import clock
from random import shuffle
from PandaCore.Tools.Misc import *
from PandaCore.Tools.MultiThreading import GenericRunner
from PandaCore.Tools.Load import *

which = int(argv[1])

sname = argv[0]

if __name__ == "__main__":
  
  Load('SCRAMJetAnalyzer','Analyzer')

  def fn(shortName,longName,first,last):
    outfilename = shortName+'.root'

    start=clock()

    eosPath = 'root://eoscms.cern.ch//store/user/snarayan'
    eosEXOPath = 'root://eoscms.cern.ch//store/group/phys_exotica'
    cernboxPath = 'root://eosuser//eos/user/s/snarayan'
    cernboxBPath = 'root://eosuser//eos/user/b/bmaier'
    fullPath = sub(r'\${CERNBOXB}',cernboxBPath,
        sub(r'\${CERNBOX}',cernboxPath,
          sub(r'\${EOS}',eosPath,
            sub(r'\${EOSEXOTICA}',eosEXOPath,longName))))
    PInfo(sname,fullPath)

    system('xrdcp %s input.root'%fullPath)

    skimmer = root.Analyzer()
    skimmer.isData=False
    skimmer.doHeatMap=False
    skimmer.doECF=True
    skimmer.doKinFit=False
    skimmer.doQjets=False
    skimmer.firstEvent=first
    skimmer.lastEvent=last

    skimmer.maxJets=2
    if 'TT' in fullPath:
      skimmer.processType=root.Analyzer.kTop
    elif 'WW' in fullPath:
      skimmer.processType=root.Analyzer.kV
    elif ('ZH' in fullPath) or ('A0h' in fullPath):
      skimmer.processType=root.Analyzer.kH
    else:
      skimmer.processType=root.Analyzer.kQCD
      skimmer.maxJets=1
    
    fin = root.TFile.Open('input.root')
    tree = fin.FindObjectAny("events")

    skimmer.SetOutputFile('output.root')
    skimmer.Init(tree)
    skimmer.AddFatJetFromTree("puppiCA15","puppiCA15",root.Analyzer.kPuppi,1.5,root.Analyzer.kCA)

    skimmer.Run()
    skimmer.Terminate()

    mvcmd = 'lcg-cp -v -D srmv2 -b file://$PWD/output.root srm://t3serv006.mit.edu:8443/srm/v2/server?SFN=/mnt/hadoop/cms/store/user/snarayan/scramjet/v5/batch/%s'%outfilename
    PInfo(sname,mvcmd)
    system(mvcmd)
    system('rm input.root')

    PInfo(sname,'finished in %f'%(clock()-start)); start=clock()

  
  cfg = open('local.cfg')
  lines = list(cfg)
  ll = lines[which].split()
  shortname = ll[0]
  first = int(ll[1])
  last = int(ll[2])
  longname = ll[3]
  fn(shortname,longname,first,last)

