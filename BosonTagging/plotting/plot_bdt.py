#!/usr/bin/env python

from ROOT import gROOT, gSystem
import ROOT as root
from PandaCore.Tools.Load import *
from PandaCore.Tools.Misc import *
from os import system,mkdir,getenv
from sys import argv
import argparse
tcut = root.TCut

basedir = getenv('SCRAMJETFLAT')+'/training/'
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
cut = ''
#cut = ''
if not args.cut:
  label = 'massCut_'
  plotlabel = '60 < m_{SD} < 120 GeV'
elif args.cut=='tau':
  cut = 'tau21<0.4'
  label = 'masstauCut_'
  plotlabel = '#splitline{60 < m_{SD} < 120 GeV}{#tau_{21}<0.4}'

### LOAD PLOTTING UTILITY ###
plot = root.PlotUtility()
plot.InitLegend(0.7,0.7,0.88,0.9)
plot.Stack(False)
plot.SetNormFactor(True)
plot.SetCut(tcut(cut))
plot.SetMCWeight('weight')
#plot.CloneTrees(True) # doesn't work if friends are added

### DEFINE PROCESSES ###
matched = root.Process('W',root.kExtra1); matched.additionalCut = tcut('classID==0')
qcd = root.Process('QCD',root.kExtra3); qcd.additionalCut = tcut('classID==1')
processes = [qcd,matched]

for p in processes:
  p.Init("TestTree")
  p.AddFile(basedir+'W.root')
  plot.AddProcess(p)

### CHOOSE DISTRIBUTIONS, LABELS ###
plot.SetTDRStyle()
plot.AddCMSLabel()
if plotlabel:
  plot.AddPlotLabel(plotlabel,.18,.77,False,42,.04)

dists = []

bdt = root.Distribution('BDT',-0.5,0.5,50,'BDT response','Events')
dists.append(bdt)


for d in dists:
  plot.AddDistribution(d)

### DRAW AND CATALOGUE ###
plot.DrawAll(figsdir+'/'+label)

