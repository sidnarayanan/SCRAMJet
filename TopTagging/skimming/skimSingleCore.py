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

nPerJob = int(argv[2])
sname = argv[0]

if __name__ == "__main__":
  
  Load('SCRAMJetAnalyzer','Analyzer')

  def fn(shortName,longName,counter,xsec,isData,outPath):
    outfileName = '%s_%i'%(shortName,counter)
    if path.isfile(outPath+'/'+outfileName+'.lock'):
      return # another job is processing this file
    if path.isfile(outPath+'/'+outfileName+'.root'):
      return # another job has processed this file

    system('touch %s/%s.lock'%(outPath,outfileName))
    start=clock()

    eosPath = 'root://eoscms//eos/cms/store/user/%s'%(getenv('USER'))
    cernboxPath = 'root://eosuser//eos/user/%s/%s'%(getenv('USER')[0],getenv('USER'))
    cernboxBPath = 'root://eosuser//eos/user/b/bmaier'
    fullPath = sub(r'\${CERNBOXB}',cernboxBPath,sub(r'\${CERNBOX}',cernboxPath,sub(r'\${EOS}',eosPath,longName)))
    PInfo(sname,fullPath)

    skimmer = root.Analyzer()
    skimmer.isData=False
    skimmer.doHeatMap=False
    skimmer.doECF=True
    skimmer.doKinFit=True
    skimmer.doQjets=False

    if 'TT' in fullPath:
      skimmer.processType=root.Analyzer.kTop
    elif 'WW' in fullPath:
      skimmer.processType=root.Analyzer.kV
    else:
      skimmer.processType=root.Analyzer.kQCD
    
    fin = root.TFile.Open(fullPath)
    tree = fin.FindObjectAny("events")

    skimmer.SetOutputFile('%s_%i.root'%(shortName,counter))
    skimmer.Init(tree)
    skimmer.AddFatJetFromTree("puppiCA15","puppiCA15",root.Analyzer.kPuppi,1.5,root.Analyzer.kCA)
#    skimmer.AddFatJetFromTree("chsCA15","chsCA15",root.Analyzer.kCHS,1.5,root.Analyzer.kCA)

    skimmer.Run()
    skimmer.Terminate()

    mvcmd = 'mv %s_%i.root %s'%(shortName,counter,outPath)
    PInfo(sname,mvcmd)
    system(mvcmd)
    system('rm %s/%s.lock'%(outPath,outfileName))

    PInfo(sname,'finished in %f'%(clock()-start)); start=clock()

  
  argList = []
  which = int(argv[1])
  counter=0
  cfg = open('local.cfg')
  for line in cfg:
    ll = line.split()
    isData = not(ll[1]=='MC')
    if len(ll)==5:
      argList.append([ll[0],ll[3],int(ll[4]),float(ll[2]),isData,argv[3]])
    else:
      argList.append([ll[0],ll[3],counter,float(ll[2]),isData,argv[3]])
    counter+=1
  shuffle(argList)
  for x in argList:
    fn(*x)
