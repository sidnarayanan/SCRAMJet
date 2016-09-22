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
roc.InitCanvas(.7,.15,.94,.5)
roc.SetFile(fin)
roc.c.AddPlotLabel(plotlabel,.18,.77,False,42,.04)

variables = [
  ('top_all_bdt','All BDT',1,1),
  ('top_ecf_bdt','ECF BDT',0,1),
  #('min_secfN_1_3_20','min(1e3)'),
  ('avg_secfN_1_3_20','avg(1e3)',2,2),
  #('mW_minalpha','m_{W}, min pull'),
  #('mW_minDR','m_{W}, min #DeltaR'),
  #('mW_best','best m_{W}'),
  ('alpha1','#alpha angle',3,2),
  #('alpha2','Subleading #alpha'),
  ('fitprob','P(kin fit)',4,2),
  #('fitmassW','m_{W}, kin fit'),
  ('N3_05','N3, #beta=0.5',11,2),
  ('N3_10','N3, #beta=1.0',6,2),
  ('N3_20','N3, #beta=2.0',10,2),
  ('tau32','#tau_{32}',2,3),
  ('tau32SD','groomed #tau_{32}',3,3),
  #('qmass','Q-vol(mass)'),
  ('qtau32','Q-vol(tau32)',4,3),
            ] 
for iV in xrange(len(variables)):
  if len(variables[iV])==2:
    v,vlabel = variables[iV]
    vcolor = iV
    vstyle = 1 if 'bdt' in v else 2
  else:
    v,vlabel,vcolor,vstyle = variables[iV]
  roc.CalcROC('h_%s_Top'%v,'h_%s_QCD'%v,vlabel,vcolor,vstyle,1)

roc.DrawAll(figsdir,cut+'roc')

