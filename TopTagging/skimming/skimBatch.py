#!/usr/bin/env python

from ROOT import gSystem,gROOT
import ROOT as root
from MitPanda.Tools.MultiThreading import GenericRunner
from MitPanda.Tools.Misc import *
from re import sub
from sys import argv
from os import system,getenv,path
from json import load as loadJson
import cPickle as pickle
from time import clock

nPerJob = int(argv[2])
sname = argv[0]

if __name__ == "__main__":
  
  gROOT.LoadMacro("${CMSSW_BASE}/src/MitPanda/Skimmers/interface/NeroSkimmer.h")
  gSystem.Load('libMitPandaSkimmers.so')
  gSystem.Load('libNeroProducerCore.so')

  with open('data/Cert_271036-276811_13TeV_PromptReco_Collisions16_JSON_NoL1T.txt') as jsonFile:
    json=loadJson(jsonFile)

  def fn(shortName,longName,counter,xsec,isData,outPath=None):
    PInfo(sname,outPath)
    start = clock()
    #if outPath and path.isfile('%s/%s_%i.root'%(outPath,shortName,counter)):
    #  PWarning(sname,'found %s/%s_%i.root, skipping!'%(outPath,shortName,counter))
    #  return
    start = clock()
    eosPath = 'root://eoscms//eos/cms/store/user/%s'%(getenv('USER'))
    cernboxPath = 'root://eosuser//eos/user/%s/%s'%(getenv('USER')[0],getenv('USER'))
    cernboxBPath = 'root://eosuser//eos/user/b/bmaier'
    fullPath = sub(r'\${CERNBOXB}',cernboxBPath,sub(r'\${CERNBOX}',cernboxPath,sub(r'\${EOS}',eosPath,longName)))
    PInfo(sname,fullPath)
    fin = root.TFile.Open(fullPath)
    tree = fin.FindObjectAny("events")
    alltree = fin.Get('nero/all')
    PInfo(sname,'opened input %f'%(clock()-start)); start=clock()

    skimmer = root.NeroSkimmer()
    skimmer.usePuppiMET=True;
    skimmer.fromBambu=False;
    skimmer.set_ak4Jetslabel("puppi")
    skimmer.set_ca15Jetslabel("CA15Puppi")
    skimmer.SetPreselectionBit(root.NeroSkimmer.kMonotopCA15)
    #skimmer.SetPreselectionBit(root.NeroSkimmer.kDY)
#    if isData:
#      skimmer.SetPreselectionBit(root.NeroSkimmer.kTriggers)
#    skimmer.SetPreselectionBit(root.NeroSkimmer.kMonojet)
    PInfo(sname,'finished loading skimmer object %f'%(clock()-start)); start=clock()

    applyJson=False
    skimmer.isData=isData
    skimmer.applyJson=applyJson
    if isData and applyJson:
      for run,lumi in json.iteritems():
        for l in lumi:
          for lll in xrange(l[0],l[1]+1):
            skimmer.AddLumi(int(run),int(lll))
    PInfo(sname,'finished loading event quality %f'%(clock()-start)); start=clock()

    processType=root.NeroSkimmer.kNone
    if not isData:
      if 'ZJets' in fullPath or 'DY' in fullPath:
        processType=root.NeroSkimmer.kZ
      elif 'WJets' in fullPath:
        processType=root.NeroSkimmer.kW
      elif 'GJets' in fullPath:
        processType=root.NeroSkimmer.kA
      elif 'TTJets' in fullPath or 'TT_' in fullPath:
        processType=root.NeroSkimmer.kTT
    skimmer.processType=processType

    skimmer.SetDataDir("data/")
    PInfo(sname,'finished reading data dir %f'%(clock()-start)); start=clock()
    skimmer.SetOutputFile('%s_%i.root'%(shortName,counter))
    skimmer.Init(tree,alltree)
    skimmer.Run(1)
    skimmer.Terminate()
    PInfo(sname,'finished output %f'%(clock()-start));start=clock();
    if outPath:
      mvcmd = 'mv %s_%i.root %s'%(shortName,counter,outPath)
      PInfo(sname,mvcmd)
      system(mvcmd)
      PInfo(sname,'moved output %f'%(clock()-start)); start=clock()
  
  gr = GenericRunner(fn)
  argList = []
  which = int(argv[1])
  counter=0
  cfg = open('local.cfg')
  for line in cfg:
    if counter>=which*nPerJob and counter<(which+1)*nPerJob:
      ll = line.split()
      isData = not(ll[1]=='MC')
      if len(ll)==5:
        argList.append([ll[0],ll[3],int(ll[4]),float(ll[2]),isData,argv[3]])
      else:
        argList.append([ll[0],ll[3],counter,float(ll[2]),isData,argv[3]])
    counter+=1
  gr.setArgList(argList)
  gr.run(4)


