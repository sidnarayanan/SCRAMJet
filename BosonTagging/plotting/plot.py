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
if not args.cut:
  label = 'nocut'
  plotlabel = None
elif args.cut=='mass':
  cut = tAND(cut,'mSD>100 && mSD<150')
  label = 'massCut_'
  plotlabel = '100 < m_{SD} < 150 GeV'
elif args.cut=='masstau':
  cut = tAND(cut,'mSD>60 && mSD<120 && tau21<0.4')
  label = 'masstauCut_'
  plotlabel = '#splitline{60 < m_{SD} < 120 GeV}{#tau_{21}<0.4}'
elif args.cut=='tau':
  cut = tAND(cut,'tau21<0.4')
  label = 'tauCut_'
  plotlabel = '#tau_{21}<0.4'

### LOAD PLOTTING UTILITY ###
plot = root.PlotUtility()
plot.InitLegend(0.7,0.7,0.88,0.9)
plot.Stack(False)
plot.SetNormFactor(True)
plot.SetCut(tcut(cut))
plot.SetMCWeight('normalizedWeight')
#plot.CloneTrees(True) # doesn't work if friends are added

### DEFINE PROCESSES ###
matched = root.Process('Higgs',root.kExtra4); 
matched.additionalCut = tcut('matched==1 && gensize<1.2')
matched.additionalWeight = tcut('ptweight')

qcd = root.Process('QCD',root.kExtra3)
qcd.additionalWeight = tcut('ptweight_analytic')
#processes = [qcd,unmatched,matched]
processes = [qcd,matched]

for p in processes:
  p.Init("puppiCA15")

### ASSIGN FILES TO PROCESSES ###
matched.AddFile(basedir+'ZpA0h.root')
#qcd.AddFile(basedir+'QCD.root')
qcd.AddFile(basedir+'QCD_evt10.root')

for p in processes:
  plot.AddProcess(p)

### CHOOSE DISTRIBUTIONS, LABELS ###
plot.SetTDRStyle()
plot.AddCMSLabel()
if plotlabel:
  plot.AddPlotLabel(plotlabel,.18,.77,False,42,.04)

dists = []

allbdt = root.Distribution('higgs_all_bdt',-.5,.5,50,'All BDT','Events')
dists.append(allbdt)

finalbdt = root.Distribution('higgs_final_bdt',-.5,.5,50,'Final BDT','Events')
dists.append(finalbdt)

allnodrbdt = root.Distribution('higgs_allbutminDR_bdt',-.5,.5,50,'All (no min #DeltaR) BDT','Events')
dists.append(allnodrbdt)

ecfbdt = root.Distribution('higgs_ecf_bdt',-.5,.5,50,'ECF BDT','Events')
dists.append(ecfbdt)

min_secfN = root.Distribution('min_secfN_1_3_20',0,0.001,50,'min(_{1}e_{3},subjets) #beta=2','Events')
#dists.append(min_secfN)

sum_secfN = root.Distribution('sum_secfN_1_3_20',0,0.05,50,'sum(_{1}e_{3},subjets) #beta=2','Events')
#dists.append(sum_secfN)

avg_secfN = root.Distribution('avg_secfN_1_3_20',0,0.01,50,'avg(_{1}e_{3},subjets) #beta=2','Events')
dists.append(avg_secfN)

minalpha = root.Distribution('minpullalpha',0,3.14,50,'Min subjet pair pull #phi','Events',999,-999,'minalpha')
dists.append(minalpha)

alpha1 = root.Distribution('TMath::Abs(alphapull1)',0,3.14,50,'Leading pT #alpha angle','Events',999,-999,'alpha1')
dists.append(alpha1)

alpha2 = root.Distribution('TMath::Abs(alphapull2)',0,3.14,50,'Subleading pT #alpha angle','Events',999,-999,'alpha2')
dists.append(alpha2)

beta1 = root.Distribution('betapull1',-3.14,3.14,50,'Leading pT #beta angle','Events',999,-999,'beta1')
#dists.append(beta1)

beta2 = root.Distribution('betapull2',-3.14,3.14,50,'Subleading pT #beta angle','Events',999,-999,'beta2')
#dists.append(beta2)

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

msd = root.Distribution("mSD",0,500,50,'m_{SD} [GeV]','Events/10 GeV')
dists.append(msd)

pt = root.Distribution("pt",250,1000,50,'p_{T} [GeV]','Events/15 GeV')
dists.append(pt)

rho = root.Distribution("TMath::Log(TMath::Power(mSD,2)/TMath::Power(pt,2))",-10,2,50,'#rho','Events',999,-999,'rho')
dists.append(rho)

qtau21 = root.Distribution("qtau21",0,1,50,'Q-vol(#tau_{21})','Events/0.02')
dists.append(qtau21)

for d in dists:
  plot.AddDistribution(d)

### DRAW AND CATALOGUE ###
plot.DrawAll(figsdir+'/'+label)

