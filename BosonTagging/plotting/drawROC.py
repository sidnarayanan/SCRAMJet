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
roc.SetPlotRange(0.03,1)
roc.InitCanvas(.7,.15,.94,.5)
roc.SetFile(fin)
roc.c.AddPlotLabel(plotlabel,.18,.77,False,42,.04)

variables = [
  ('higgs_final_bdt','All BDT',1,1),
  ('higgs_ecf_bdt','ECF BDT',0,1),
  ('avg_secfN_1_3_20','avg(1e3)',2,2),
  ('alpha1','#alpha angle',3,2),
  ('M2_20','M2, #beta=2.0',11,2),
  ('D2_20','D2, #beta=2.0',6,2),
  ('N2_20','N2, #beta=2.0',10,2),
  ('tau21','#tau_{21}',2,3),
  ('tau21SD','groomed #tau_{21}',3,3),
  ('qtau21','Q-vol(tau21)',4,3),
            ] 

for iV in xrange(len(variables)):
  if len(variables[iV])==2:
    v,vlabel = variables[iV]
    vcolor = iV
    vstyle = 1 if 'bdt' in v else 2
  else:
    v,vlabel,vcolor,vstyle = variables[iV]
  roc.CalcROC('h_%s_Higgs'%v,'h_%s_QCD'%v,vlabel,vcolor,vstyle,1)

roc.DrawAll(figsdir,cut+'roc')

