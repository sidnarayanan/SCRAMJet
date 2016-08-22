#!/usr/bin/env python
from PandaCore.Tools.MultiThreading import GenericRunner
from ROOT import gSystem,gROOT
import ROOT as root
from re import sub
from sys import argv,exit
from os import path,getenv,system
from json import load as loadJson
from random import shuffle


if __name__ == "__main__":
  
  gROOT.LoadMacro("${CMSSW_BASE}/src/SCRAMJet/Analyzer/interface/Analyzer.h")
  gSystem.Load('libSCRAMJetAnalyzer.so')
  gSystem.Load('libSCRAMJetObjects.so')

  def fn(shortName,longName,counter,xsec,isData,outPath=None):
    print outPath
    if outPath and path.isfile('%s/%s_%i.root'%(outPath,shortName,counter)):
      print 'found %s/%s_%i.root, skipping!'%(outPath,shortName,counter)
      return
    eosPath = 'root://eoscms//eos/cms/store/user/%s'%(getenv('USER'))
    cernboxPath = 'root://eosuser//eos/user/%s/%s'%(getenv('USER')[0],getenv('USER'))
    cernboxBPath = 'root://eosuser//eos/user/b/bmaier'
    fullPath = sub(r'\${CERNBOXB}',cernboxBPath,sub(r'\${CERNBOX}',cernboxPath,sub(r'\${EOS}',eosPath,longName)))
    print fullPath

    skimmer = root.NeroSkimmer()
    skimmer.fromBambu=False
    
    skimmer.usePuppiMET=True;
    skimmer.set_ak4Jetslabel("puppi")
    skimmer.set_ca15Jetslabel('CA15Puppi')

    skimmer.isData=isData
    applyJson=True
    skimmer.applyJson=applyJson
    processType=root.NeroSkimmer.kNone
    if not isData:
      processType=root.NeroSkimmer.kNone
      if 'ZJets' in fullPath or 'DY' in fullPath:
        processType=root.NeroSkimmer.kZ
      elif 'WJets' in fullPath:
        processType=root.NeroSkimmer.kW
      elif 'GJets' in fullPath:
        processType=root.NeroSkimmer.kA
      elif 'TTJets' in fullPath or 'TT_' in fullPath:
        processType=root.NeroSkimmer.kTT
      elif 'Monotop' in fullPath or 'res' in fullPath:
        processType=root.NeroSkimmer.kMonotopSignal
      skimmer.processType=processType
    
    if isData and applyJson:
      for run,lumi in json.iteritems():
        for l in lumi:
          for lll in xrange(l[0],l[1]+1):
            skimmer.AddLumi(int(run),int(lll))

    #skimmer.SetPreselectionBit(root.NeroSkimmer.kDY)
    #skimmer.SetPreselectionBit(root.NeroSkimmer.kMonotopCA15)
    skimmer.SetDataDir(panda+"/data/")
    fin = root.TFile.Open(fullPath)
    tree = fin.FindObjectAny("events")
    alltree = fin.Get('nero/all')
    skimmer.SetOutputFile('/tmp/%s/skim/%s_%i.root'%(getenv('USER'),shortName,counter))
    skimmer.Init(tree,alltree)
    skimmer.Run(1)
    skimmer.Terminate()
    if outPath:
      mvcmd = 'mv /tmp/%s/skim/%s_%i.root %s'%(getenv('USER'),shortName,counter,outPath)
      print mvcmd
      system(mvcmd)

  system('mkdir -p /tmp/%s/skim/'%(getenv('USER')))
  #system('mkdir -p %s/split/'%getenv('PANDA_FLATDIR'))
  gr = GenericRunner(fn)
  argList = []
  counter=0
  cfg = open(panda+'/Monotop/config/%s.cfg'%(argv[1]))
  for line in cfg:
    ll = line.split()
    isData = not(ll[1]=='MC')
    if len(ll)==5:
      argList.append([ll[0],ll[3],int(ll[4]),float(ll[2]),isData,'%s/batch/'%getenv('PANDA_FLATDIR')])
    else:
      argList.append([ll[0],ll[3],counter,float(ll[2]),isData,'%s/batch/'%getenv('PANDA_FLATDIR')])
    counter+=1
  #fn(*(argList[100])); exit(0)
  shuffle(argList)
  gr.setArgList(argList)
  gr.run(8)
