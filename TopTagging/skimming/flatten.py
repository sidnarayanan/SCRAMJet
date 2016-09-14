#!/usr/bin/env python

import ROOT as root
from PandaCore.Tools.Load import *
from PandaCore.Tools.Misc import *
from sys import argv

Load('Tools','BranchAdder')

def gethisto(tree,formula,binlo,binhi,additionalcut=None):
  nbins=50
  h = root.TH1F('h','h',nbins,binlo,binhi)
  if additionalcut:
    tree.Draw(formula+'>>h',tTIMES('normalizedWeight',additionalcut))
  else:
    tree.Draw(formula+'>>h','normalizedWeight')

  for iB in xrange(1,nbins+1):
    val = h.GetBinContent(iB);
    if val:
      h.SetBinContent(iB,1./val)
    else:
      print formula,iB

  return h


def addbranches(fpath,additionalcut=None):
  fin = root.TFile(fpath,'UPDATE')
  jets = fin.Get('puppiCA15')
  ba = root.BranchAdder()

  hpt = gethisto(jets,'pt',250,600,additionalcut)
  ba.formula = 'pt'
  ba.newBranchName = 'ptweight'
  ba.AddBranchFromHistogram(jets,hpt)

  hmSD = gethisto(jets,'mSD',0,500,additionalcut)
  ba.formula = 'mSD'
  ba.newBranchName = 'mSDweight'
  ba.AddBranchFromHistogram(jets,hmSD)

  hrho = gethisto(jets,'TMath::Log(mSD/pt)',-10,0,additionalcut)
  ba.formula = 'TMath::Log(mSD/pt)'
  ba.newBranchName = 'rhoweight'
  ba.AddBranchFromHistogram(jets,hrho)

  fin.WriteTObject(jets,'puppiCA15','Overwrite')


addbranches('/afs/cern.ch/user/s/snarayan/work/skims/scramjet_v2/ZpTT.root','matched==1&&gensize<1.2')
addbranches('/afs/cern.ch/user/s/snarayan/work/skims/scramjet_v2/ZpWW.root','matched==1&&gensize<0.6')
addbranches('/afs/cern.ch/user/s/snarayan/work/skims/scramjet_v2/QCD.root')


