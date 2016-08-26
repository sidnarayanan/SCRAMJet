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
if not args.cut:
  cut = ''
  label = ''
  plotlabel = None
elif args.cut=='mass':
  cut = 'mSD>60 && mSD<120'
  label = 'massCut_'
  plotlabel = '60 < m_{SD} < 120 GeV'
elif args.cut=='masstau':
  cut = 'mSD>60 && mSD<120 && tau21<0.4'
  label = 'masstauCut_'
  plotlabel = '60 < m_{SD} < 120 GeV, #tau_{32}<0.4'

### LOAD PLOTTING UTILITY ###
plot = root.PlotUtility()
plot.Stack(False)
plot.SetNormFactor(True)
plot.SetCut(tcut(cut))
plot.SetMCWeight('1')
#plot.CloneTrees(True) # doesn't work if friends are added

### DEFINE PROCESSES ###
matched = root.Process('Matched',root.kExtra1); matched.additionalCut = tcut('matched==1 && gensize<1.2')
unmatched = root.Process('Unmatched',root.kExtra2); unmatched.additionalCut = tcut('matched==0 || gensize>1.2')
qcd = root.Process('QCD',root.kQCD)
#processes = [qcd,unmatched,matched]
processes = [qcd,matched]

for p in processes:
  p.Init("puppiCA15")

### ASSIGN FILES TO PROCESSES ###
matched.AddFile(basedir+'ZpWW.root')
unmatched.AddFile(basedir+'ZpWW.root')
qcd.AddFile(basedir+'QCD.root')

for p in processes:
  plot.AddProcess(p)

### CHOOSE DISTRIBUTIONS, LABELS ###
plot.SetTDRStyle()
plot.AddCMSLabel()
if plotlabel:
  plot.AddPlotLabel(plotlabel,.16,.8,False,42,.04)

dists = []

'''
fitprob = root.Distribution('fitprob',-1.1,1.1,44,'P(fit)','Events')
dists.append(fitprob)

fitmassW = root.Distribution('fitmassW',-1.1,300,50,'Fit W mass [GeV]','Events/6 GeV')
dists.append(fitmassW)

mW_minDR = root.Distribution('mW_minDR',0,300,50,'Disubjet mass, closest subjets [GeV]','Events/6 GeV',999,-999,'mW_minDR')
dists.append(mW_minDR)

mW_best = root.Distribution('mW_best',0,300,50,'Best subjet W mass [GeV]','Events/6 GeV',999,-999,'mW_best')
dists.append(mW_best)

N3_10 = root.Distribution("ecfN_2_4_10/TMath::Power(ecfN_1_3_10,2)",0,5,50,"N_{3}(#beta=1.0)","Events",999,-999,"N3_10");
dists.append(N3_10)

N3_05 = root.Distribution("ecfN_2_4_05/TMath::Power(ecfN_1_3_05,2)",0,5,50,"N_{3}(#beta=0.1)","Events",999,-999,"N3_05");
dists.append(N3_05)

N3_20 = root.Distribution("ecfN_2_4_20/TMath::Power(ecfN_1_3_20,2)",0,5,50,"N_{3}(#beta=2.0)","Events",999,-999,"N3_20");
dists.append(N3_20)

'''

for beta in [5,10,20]:
  M2 = root.Distribution('ecfN_1_3_%.2i/ecfN_1_2_%.2i'%(beta,beta),0,.25,50,'M_{2}(#beta=%.1f)'%(beta/10.),'Events',999,-999,'M2_%.2i'%beta)
  dists.append(M2)

  N2 = root.Distribution('ecfN_2_3_%.2i/TMath::Power(ecfN_1_2_%.2i,2)'%(beta,beta),0,.5,50,'N_{2}(#beta=%.1f)'%(beta/10.),'Events',999,-999,'N2_%.2i'%beta)
  dists.append(N2)

  D2 = root.Distribution('ecfN_3_3_%.2i/TMath::Power(ecfN_1_2_%.2i,3)'%(beta,beta),0,5,50,'D_{2}(#alpha=#beta=%.1f)'%(beta/10.),'Events',999,-999,'D2_%.2i'%beta)
  dists.append(D2)



minDR = root.Distribution('TMath::Sqrt(dR2_minDR)',0,1.5,50,'min #Delta R_{subjets}','Events',999,-999,'minDR')
dists.append(minDR)

tau21 = root.Distribution("tau21",0,1,50,'#tau_{21}','Events/0.02')
dists.append(tau21)

tau21SD = root.Distribution("tau21SD",0,1,50,'Groomed #tau_{21}','Events/0.02')
dists.append(tau21SD)

if plotlabel==None:
  msd = root.Distribution("mSD",0,150,50,'m_{SD} [GeV]','Events/3 GeV')
  dists.append(msd)

  pt = root.Distribution("pt",250,2250,50,'p_{T} [GeV]','Events/40 GeV')
  dists.append(pt)

'''

'''


for d in dists:
  plot.AddDistribution(d)

### DRAW AND CATALOGUE ###
plot.DrawAll(figsdir+'/'+label)

