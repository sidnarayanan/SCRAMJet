#!/usr/bin/env python
import ROOT as root
from PandaCore.Tools.Load import *
from sys import argv,exit
import argparse


parser = argparse.ArgumentParser(description='plot stuff')
parser.add_argument('--outdir',metavar='outdir',type=str,default='.')
parser.add_argument('--cut',metavar='cut',type=str,default=None)
args = parser.parse_args()

figsdir = args.outdir

if not args.cut:
  print 'please choose a cut'
  exit(1)
elif args.cut=='mass':
  cut = 'massCut_' 
  plotlabel = '100 < m_{SD} < 150 GeV'
elif args.cut=='masstau':
  cut = 'masstauCut_'
  plotlabel = '#splitline{60 < m_{SD} < 120 GeV}{#tau_{21}<0.6}'


Load('Drawers','ROCTool')
roc = root.ROCTool()

fin = root.TFile(figsdir+'/'+cut+'hists.root')

roc.Logy()
roc.SetPlotRange(0.005,1)
roc.InitCanvas()
roc.SetFile(fin)
roc.c.AddPlotLabel(plotlabel,.18,.77,False,42,.04)

variables = [
  ('higgs_ecf_bdt','ecfN BDT'),
#  ('min_secfN_1_3_20','min(1e3)'),
  ('avg_secfN_1_3_20','avg(1e3)'),
  ('alpha1','Leading #alpha'),
  ('alpha2','Subleading #alpha'),
  ('minDR','Min #DeltaR_{subjets}'),
  ('tau21','#tau_{21}'),
  ('tau21SD','groomed #tau_{21}'),
            ] 
for beta in [5,20]:
  variables.append( ('M2_%.2i'%beta,'M2, #beta=%.1f'%(beta/10.)) )
for beta in [5,20]:
  variables.append( ('N2_%.2i'%beta,'N2, #beta=%.1f'%(beta/10.)) )
for beta in [5,20]:
  variables.append( ('D2_%.2i'%beta,'D2, #beta=%.1f'%(beta/10.)) )

for iV in xrange(len(variables)):
  v,vlabel = variables[iV]
  print v
  if 'mW' in v or v=='fitmassW':
    roc.CalcROC('h_%s_Higgs'%v,'h_%s_QCD'%v,vlabel,iV,1,2)
  elif 'BDT' in v or 'bdt' in v:
    roc.CalcROC('h_%s_Higgs'%v,'h_%s_QCD'%v,vlabel,iV,2,1)
  else:
    roc.CalcROC('h_%s_Higgs'%v,'h_%s_QCD'%v,vlabel,iV,1,1)

roc.DrawAll(figsdir,cut+'roc')

