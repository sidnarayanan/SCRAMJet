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
elif args.cut=='btag':
  cut = 'btagCut_' 
  plotlabel = '#splitline{110 < m_{SD} < 210 GeV}{max SJ CSV>0.46}'
elif args.cut=='masstau':
  cut = 'masstauCut_'
  plotlabel = '#splitline{110 < m_{SD} < 210 GeV}{#tau_{32}<0.6}'


Load('Drawers','ROCTool')
roc = root.ROCTool()

fin = root.TFile(figsdir+'/'+cut+'hists.root')

roc.Logy()
roc.SetPlotRange(0.005,1)
roc.InitCanvas(.65,.15,.94,.6)
roc.SetFile(fin)
roc.c.AddPlotLabel(plotlabel,.18,.77,False,42,.04)

variables = [
  ('top_ecfv7_bdt','ECF+#tau_{32}^{SD} BDT',1,1),
  ('top_ecfv6_bdt','ECF BDT',2,1),
  ('tau32SD','Groomed #tau_{32}',3,2),
  ('tau32','#tau_{32}',4,2),
  ('input0','e(1,2,2)/e(1,2,1)^{2}',3,3),
  ('input1','e(1,3,4)/e(2,3,2)',4,3),
  ('input2','e(3,3,1)/e(1,3,4)^{3/4}',5,3),
  ('input3','e(3,3,1)/e(2,3,2)^{3/4}',6,3),
  ('input4','e(3,3,2)/e(3,3,4)^{1/2}',7,3),
  ('input5','e(1,4,2)/e(1,3,1)^{2}',8,3),
  ('input6','e(1,4,4)/e(1,3,2)^{2}',9,3),
  ('input7','e(2,4,0.5/e(1,3,0.5)^{2}',10,3),
  ('input8','e(2,4,1)/e(1,3,1)^{2}',11,3),
  ('input9','e(2,4,1)/e(2,3,0.5)^{2}',12,3),
  ('input10','e(2,4,2)/e(1,3,2)^{2}',13,3),
  # ('N3_05','N3, #beta=0.5',11,2),
  # ('N3_10','N3, #beta=1.0',6,2),
  # ('N3_20','N3, #beta=2.0',10,2),
            ] 
for iV in xrange(len(variables)):
  print variables[iV]
  if len(variables[iV])==2:
    v,vlabel = variables[iV]
    vcolor = iV
    vstyle = 1 if 'bdt' in v else 2
  else:
    v,vlabel,vcolor,vstyle = variables[iV]
  roc.CalcROC('h_%s_Top'%v,'h_%s_QCD'%v,vlabel,vcolor,vstyle,1)

roc.DrawAll(figsdir,cut+'roc')

