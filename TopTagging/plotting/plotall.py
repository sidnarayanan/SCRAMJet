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
cut = 'pt<1000'
#cut = ''
if args.cut=='union':
  masslo=100
  masshi=210
elif args.cut=='higgs':
  masslo=100
  masshi=150
elif args.cut=='top':
  masslo=110
  masshi=210

label = args.cut+'_'
cut=tAND(cut,'mSD>%i && mSD<%i'%(masslo,masshi))
plotlabel = '%i < m_{SD} < %i GeV'%(masslo,masshi)

### LOAD PLOTTING UTILITY ###
plot = root.PlotUtility()
plot.InitLegend(0.7,0.7,0.88,0.9)
plot.Stack(False)
plot.SetNormFactor(True)
plot.SetCut(tcut(cut))
plot.SetMCWeight('normalizedWeight')

### DEFINE PROCESSES ###
top = root.Process('Top',root.kExtra1)
top.additionalCut = tcut('matched==1 && gensize<1.2')
top.additionalWeight = tcut('ptweight')

higgs = root.Process('Higgs',root.kExtra4)
higgs.additionalCut = tcut('matched==1 && gensize<1.2')
higgs.additionalWeight = tcut('ptweight')

qcd = root.Process('QCD',root.kExtra3)
qcd.additionalWeight = tcut('ptweight_analytic')
#processes = [qcd,unmatched,matched]
processes = [qcd,higgs,top]

for p in processes:
  p.Init("puppiCA15")

### ASSIGN FILES TO PROCESSES ###
higgs.AddFile(basedir+'ZpA0h.root')
top.AddFile(basedir+'ZpTT.root')
qcd.AddFile(basedir+'QCD_evt10.root')

for p in processes:
  plot.AddProcess(p)

### CHOOSE DISTRIBUTIONS, LABELS ###
plot.SetTDRStyle()
plot.AddCMSLabel()
if plotlabel:
  plot.AddPlotLabel(plotlabel,.18,.77,False,42,.04)

dists = []


topbdt = root.Distribution('top_ecf_bdt',-.5,.5,50,'Top ECF BDT','Events')
dists.append(topbdt)


higgsbdt = root.Distribution('higgs_ecf_bdt',-.5,.5,50,'Higgs ECF BDT','Events')
dists.append(higgsbdt)

for d in dists:
  plot.AddDistribution(d)

### DRAW AND CATALOGUE ###
plot.DrawAll(figsdir+'/'+label)

