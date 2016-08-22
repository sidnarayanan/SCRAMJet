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
    eosPath = 'root://eoscms//eos/cms/store/user/%s'%(getenv('USER'))
    cernboxPath = 'root://eosuser//eos/user/%s/%s'%(getenv('USER')[0],getenv('USER'))
    cernboxBPath = 'root://eosuser//eos/user/b/bmaier'
    fullPath = sub(r'\${CERNBOXB}',cernboxBPath,sub(r'\${CERNBOX}',cernboxPath,sub(r'\${EOS}',eosPath,longName)))

    skimmer = root.Analyzer()
    skimmer.isData=False

    if 'TT' in fullPath:
      skimmer.processType=root.Analyzer.kTop
    elif 'WW' in fullPath:
      skimmer.processType=root.Analyzer.kV
    else:
      skimmer.processType=root.NeroSkimmer.QCD
    
    fin = root.TFile.Open(fullPath)
    tree = fin.FindObjectAny("events")

    skimmer.SetOutputFile('/tmp/%s/skim/%s_%i.root'%(getenv('USER'),shortName,counter))
    skimmer.Init(tree)
    skimmer.AddFatJetFromTree("puppiCA15","puppiCA15",root.Analyzer.kPuppi,1.5,root.Analyzer.kCA)
    skimmer.AddFatJetFromTree("chsCA15","chsCA15",root.Analyzer.kCHS,1.5,root.Analyzer.kCA)

    skimmer.Run()
    skimmer.Terminate()

    if outPath:
      mvcmd = 'mv /tmp/%s/skim/%s_%i.root %s'%(getenv('USER'),shortName,counter,outPath)
      print mvcmd
      system(mvcmd)

  system('mkdir -p /tmp/%s/skim/'%(getenv('USER')))
  gr = GenericRunner(fn)
  argList = []
  counter=0
  cfg = open('%s/src/SCRAMJet/TopTagging/config/%s.cfg'%(getenv('CMSSW_BASE'),argv[1]))
  for line in cfg:
    ll = line.split()
    isData = not(ll[1]=='MC')
    if len(ll)==5:
      argList.append([ll[0],ll[3],int(ll[4]),float(ll[2]),isData,'%s/batch/'%getenv('SCRAMJETFLAT')])
    else:
      argList.append([ll[0],ll[3],counter,float(ll[2]),isData,'%s/batch/'%getenv('SCRAMJETFLAT')])
    counter+=1
  shuffle(argList)
  gr.setArgList(argList)
  gr.run(8)
