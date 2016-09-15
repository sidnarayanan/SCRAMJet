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
  plotlabel = '110 < m_{SD} < 210 GeV'
elif args.cut=='masstau':
  cut = 'masstauCut_'
  plotlabel = '#splitline{110 < m_{SD} < 210 GeV}{#tau_{32}<0.6}'


Load('Drawers','ROCTool')
roc = root.ROCTool()

fin = root.TFile(figsdir+'/'+cut+'hists.root')

roc.Logy()
roc.SetPlotRange(0.005,1)
roc.InitCanvas()
roc.SetFile(fin)
roc.c.AddPlotLabel(plotlabel,.18,.77,False,42,.04)

variables = [
  ('BDT','ecfN BDT'),
  ('min_secfN_1_3_20','min(1e3)'),
  ('avg_secfN_1_3_20','avg(1e3)'),
  ('mW_minalpha','m_{W}, min pull'),
  ('mW_minDR','m_{W}, min #DeltaR'),
  ('mW_best','best m_{W}'),
  ('alpha1','Leading #alpha'),
  ('alpha2','Subleading #alpha'),
  ('fitprob','P(kin fit)'),
  ('fitmassW','m_{W}, kin fit'),
  ('N3_05','N3, #beta=0.5'),
  ('N3_10','N3, #beta=1.0'),
  ('N3_20','N3, #beta=2.0'),
  ('tau32','#tau_{32}'),
  ('tau32SD','groomed #tau_{32}'),
            ] 
for iV in xrange(len(variables)):
  v,vlabel = variables[iV]
  if 'mW' in v or v=='fitmassW':
    roc.CalcROC('h_%s_Top'%v,'h_%s_QCD'%v,vlabel,iV,1,2)
  elif 'BDT' in v:
    roc.CalcROC('h_%s_Top'%v,'h_%s_QCD'%v,vlabel,iV,2,1)
  else:
    roc.CalcROC('h_%s_Top'%v,'h_%s_QCD'%v,vlabel,iV,1,1)

roc.DrawAll(figsdir,cut+'roc')

