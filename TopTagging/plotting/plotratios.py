#!/usr/bin/env python

from ROOT import gROOT, gSystem
import ROOT as root
from PandaCore.Tools.Load import *
from PandaCore.Tools.Misc import *
from os import system,mkdir,getenv
from sys import argv
import argparse
tcut = root.TCut

basedir = getenv('SCRAMJETFLAT')
figsdir = basedir+'/figs'

parser = argparse.ArgumentParser(description='plot stuff')
parser.add_argument('--indir',metavar='indir',type=str,default=basedir)
parser.add_argument('--outdir',metavar='outdir',type=str,default=figsdir)
parser.add_argument('--cut',metavar='cut',type=str,default=None)
args = parser.parse_args()

figsdir = args.outdir
basedir = args.indir

Load('Drawers','PlotUtility')

### DEFINE REGIONS ###
cut = 'pt<600'
#cut = ''
if not args.cut:
  label = 'nocut'
  plotlabel = None
elif args.cut=='mass':
  cut = tAND(cut,'mSD>110 && mSD<210')
  label = 'massCut_'
  plotlabel = '110 < m_{SD} < 210 GeV'
elif args.cut=='masstau':
  cut = tAND(cut,'mSD>110 && mSD<210 && tau32<0.6')
  label = 'masstauCut_'
  plotlabel = '#splitline{110 < m_{SD} < 210 GeV}{#tau_{32}<0.6}'
elif args.cut=='tau':
  cut = tAND(cut,'tau32<0.6')
  label = 'tauCut_'
  plotlabel = '#tau_{32}<0.6'

### LOAD PLOTTING UTILITY ###
plot = root.PlotUtility()
plot.InitLegend(0.7,0.7,0.88,0.9)
plot.Stack(False)
plot.SetNormFactor(True)
plot.SetCut(tcut(cut))
plot.SetMCWeight('normalizedWeight*ptweight')
#plot.CloneTrees(True) # doesn't work if friends are added

### DEFINE PROCESSES ###
matched = root.Process('Top',root.kExtra1); matched.additionalCut = tcut('matched==1 && gensize<1.2')
unmatched = root.Process('Unmatched',root.kExtra2); unmatched.additionalCut = tcut('matched==0 || gensize>1.2')
qcd = root.Process('QCD',root.kExtra3)
#processes = [qcd,unmatched,matched]
processes = [qcd,matched]

for p in processes:
  p.Init("puppiCA15")

### ASSIGN FILES TO PROCESSES ###
matched.AddFile(basedir+'ZpTT.root')
unmatched.AddFile(basedir+'ZpTT.root')
qcd.AddFile(basedir+'QCD_evt25.root')
#qcd.AddFile(basedir+'QCD.root')

for p in processes:
  plot.AddProcess(p)

### CHOOSE DISTRIBUTIONS, LABELS ###
plot.SetTDRStyle()
plot.AddCMSLabel()
if plotlabel:
  plot.AddPlotLabel(plotlabel,.18,.77,False,42,.04)

dists = []

with open('invar.txt') as invarfile:
  counter=0
  for var_ in invarfile:
    var = var_.strip()
    dist=root.Distribution(var,0,5,1000,var,'Events',999,-999,'ratio_%i'%counter)
    dists.append(dist)
    counter += 1

for d in dists:
  plot.AddDistribution(d)

### DRAW AND CATALOGUE ###
plot.DrawAll(figsdir+'/'+label)

