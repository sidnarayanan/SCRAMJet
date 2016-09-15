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
qcd.AddFile(basedir+'QCD.root')

for p in processes:
  plot.AddProcess(p)

### CHOOSE DISTRIBUTIONS, LABELS ###
plot.SetTDRStyle()
plot.AddCMSLabel()
if plotlabel:
  plot.AddPlotLabel(plotlabel,.18,.77,False,42,.04)

dists = []

min_secfN = root.Distribution('min_secfN_1_3_20',0,0.001,50,'min(_{1}e_{3},subjets) #beta=2','Events')
dists.append(min_secfN)

sum_secfN = root.Distribution('sum_secfN_1_3_20',0,0.05,50,'sum(_{1}e_{3},subjets) #beta=2','Events')
dists.append(sum_secfN)

avg_secfN = root.Distribution('avg_secfN_1_3_20',0,0.01,50,'avg(_{1}e_{3},subjets) #beta=2','Events')
dists.append(avg_secfN)

mW_minalpha = root.Distribution('mW_minalphapull',0,300,50,'Disubjet mass, min pull [GeV]','Events/6 GeV',999,-999,'mW_minalpha')
dists.append(mW_minalpha)

minalpha = root.Distribution('minpullalpha',0,3.14,50,'Min subjet pair pull #phi','Events',999,-999,'minalpha')
dists.append(minalpha)

alpha1 = root.Distribution('TMath::Abs(alphapull1)',0,3.14,50,'Leading pT #alpha angle','Events',999,-999,'alpha1')
dists.append(alpha1)

alpha2 = root.Distribution('TMath::Abs(alphapull2)',0,3.14,50,'Subleading pT #alpha angle','Events',999,-999,'alpha2')
dists.append(alpha2)

beta1 = root.Distribution('betapull1',-3.14,3.14,50,'Leading pT #beta angle','Events',999,-999,'beta1')
dists.append(beta1)

beta2 = root.Distribution('betapull2',-3.14,3.14,50,'Subleading pT #beta angle','Events',999,-999,'beta2')
dists.append(beta2)

fitprob = root.Distribution('fitprob',-1.1,1.1,44,'P(fit)','Events')
dists.append(fitprob)

fitmassW = root.Distribution('fitmassW',-1.1,300,50,'Fit W mass [GeV]','Events/6 GeV')
dists.append(fitmassW)

minDR = root.Distribution('TMath::Sqrt(dR2_minDR)',0,1.5,50,'min #Delta R_{subjets}','Events',999,-999,'minDR')
dists.append(minDR)

mW_minDR = root.Distribution('mW_minDR',0,300,50,'Disubjet mass, closest subjets [GeV]','Events/6 GeV',999,-999,'mW_minDR')
dists.append(mW_minDR)

mW_best = root.Distribution('mW_best',0,300,50,'Best subjet W mass [GeV]','Events/6 GeV',999,-999,'mW_best')
dists.append(mW_best)

N3_10 = root.Distribution("ecfN_2_4_10/TMath::Power(ecfN_1_3_10,2)",0.5,3.5,50,"N_{3}(#beta=1.0)","Events",999,-999,"N3_10");
dists.append(N3_10)

N3_05 = root.Distribution("ecfN_2_4_05/TMath::Power(ecfN_1_3_05,2)",1,2.5,50,"N_{3}(#beta=0.5)","Events",999,-999,"N3_05");
dists.append(N3_05)

N3_20 = root.Distribution("ecfN_2_4_20/TMath::Power(ecfN_1_3_20,2)",0,5,50,"N_{3}(#beta=2.0)","Events",999,-999,"N3_20");
dists.append(N3_20)

tau32 = root.Distribution("tau32",0,1,50,'#tau_{32}','Events/0.02')
dists.append(tau32)

tau32SD = root.Distribution("tau32SD",0,1,50,'Groomed #tau_{32}','Events/0.02')
dists.append(tau32SD)

msd = root.Distribution("mSD",0,500,50,'m_{SD} [GeV]','Events/10 GeV')
dists.append(msd)

pt = root.Distribution("pt",250,600,50,'p_{T} [GeV]','Events/7 GeV')
dists.append(pt)

rho = root.Distribution("TMath::Log(TMath::Power(mSD,2)/TMath::Power(pt,2))",-10,2,50,'#rho','Events',999,-999,'rho')
dists.append(rho)

dists = [alpha1,alpha2]

for d in dists:
  plot.AddDistribution(d)

### DRAW AND CATALOGUE ###
plot.DrawAll(figsdir+'/'+label)

