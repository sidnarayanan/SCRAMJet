#!/usr/bin/env python

import ROOT as root
from PandaCore.Tools.Load import *
from PandaCore.Tools.Misc import *
from sys import argv
from os import getenv

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
      print formula,h.GetBinCenter(iB)

  return h


def addbranchesFormula(fpath,additionalcut=None):
  fin = root.TFile(fpath,'UPDATE')
  jets = fin.Get('puppiCA15')
  ba = root.BranchAdder()

  ba.formula = '1./(250*TMath::Exp((232-pt)*0.0396)+200*TMath::Exp((235-pt)*0.0157)+TMath::Exp((583.-pt)*0.00672))'
  ba.newBranchName = 'ptweight_analytic'
  ba.AddBranchFromFormula(jets)

  fin.WriteTObject(jets,'puppiCA15','Overwrite')
  fin.Close()

def addbranches(fpath,additionalcut=None):
  fin = root.TFile(fpath,'UPDATE')
  jets = fin.Get('puppiCA15')
  ba = root.BranchAdder()

  hpt = gethisto(jets,'pt',250,1000,additionalcut)
  ba.formula = 'pt'
  ba.newBranchName = 'ptweight'
  ba.AddBranchFromHistogram(jets,hpt)

  '''
  hmSD = gethisto(jets,'mSD',0,500,additionalcut)
  ba.formula = 'mSD'
  ba.newBranchName = 'mSDweight'
  ba.AddBranchFromHistogram(jets,hmSD)

  hrho = gethisto(jets,'TMath::Log(mSD/pt)',-10,0,additionalcut)
  ba.formula = 'TMath::Log(mSD/pt)'
  ba.newBranchName = 'rhoweight'
  ba.AddBranchFromHistogram(jets,hrho)
  '''

  fin.WriteTObject(jets,'puppiCA15','Overwrite')
  fin.Close()

which = argv[1]
scramjet = getenv('SCRAMJETFLAT')

if which=='ZpTT':
  addbranches(scramjet+'/ZpTT.root','matched==1&&gensize<1.2')
elif which=='ZpWW':
  addbranches(scramjet+'/ZpWW.root','matched==1&&gensize<1.2')
elif which=="ZpA0h":
  #addbranches(scramjet+'/ZpA0h.root')
  addbranches(scramjet+'/ZpA0h.root','matched==1&&gensize<1.2')
else:
  addbranchesFormula(scramjet+'/'+which+'.root')
  addbranches(scramjet+'/'+which+'.root')


