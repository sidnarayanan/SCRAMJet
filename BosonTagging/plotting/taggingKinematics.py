#!/usr/bin/env python

from PandaCore.Tools.Load import *
from PandaCore.Tools.Misc import *
from os import system,mkdir,getenv
from sys import argv
from array import array
import argparse

counter=0

class Variable():
  def __init__(self,formula,lo,hi,nickname,normal=True):
    self.formula=formula
    self.lo=lo
    self.hi=hi
    self.nickname=nickname
    self.normal=normal

# parse args
basedir = getenv('SCRAMJETFLAT')
figsdir = basedir+'/figs'

parser = argparse.ArgumentParser(description='plot stuff')
parser.add_argument('--indir',metavar='indir',type=str,default=basedir)
parser.add_argument('--outdir',metavar='outdir',type=str,default=figsdir)
parser.add_argument('--cut',metavar='cut',type=str,default=None)
parser.add_argument('--signal',action='store_true')
args = parser.parse_args()

figsdir = args.outdir
basedir = args.indir

if args.signal:
  colors = {
      0.0 : root.kBlack,
      0.25: root.kRed+4,
      0.5 : root.kRed+2,
      0.75: root.kRed+0,
      }    
  thresholds=[0.0,.25,.5,.75]
  #thresholds=[0,0.5,.75,.9,.95,.99]
else:
  colors = {
      0.0 : root.kBlack,
      0.5 : root.kRed+4,
      0.75: root.kRed+3,
      0.9 : root.kRed+2,
      0.95: root.kRed+1,
      0.98: root.kRed+0,
      }    
  #thresholds=[0,0.5,.75,.9,.95]
  thresholds=[0,0.5,.75,.9,.95,.98]

argv = [] # clear before loading root
from ROOT import gROOT, gSystem
import ROOT as root

Load('Drawers','HistogramDrawer')

plot = root.HistogramDrawer()
plot.SetTDRStyle()
plot.InitLegend()
plot.SetNormFactor(True)

ctmp = root.TCanvas()

def getPtShapes(tree,vtag,vkin,cut):
  global counter
  ctmp.cd()
  nkinbins=20
  h2 = root.TH2F('h2_%i'%counter,'h2_%i'%counter,1000,vtag.lo,vtag.hi,nkinbins,vkin.lo,vkin.hi)
  tree.Draw('%s:%s>>h2_%i'%(vkin.formula,vtag.formula,counter),cut,'colz')
  counter += 1
  shapes = {}
  scan = range(1,1001)
  if not vtag.normal: # cut is inverted:
    scan.reverse()
  total = h2.Integral()
  iT=0
  for iB_ in xrange(len(scan)):
    if iT==len(thresholds):
      break
    threshold = thresholds[iT]
    iB = scan[iB_]
    if vtag.normal:
      val = h2.Integral(1,iB,1,nkinbins)
    else:
      val = h2.Integral(iB,1000,1,nkinbins)
    if val/total >= threshold:
      h1 = root.TH1F('h1_%i'%counter,'h1_%i'%counter,nkinbins,vkin.lo,vkin.hi)
      for jB in scan[iB_:]:
        for kB in range(1,nkinbins+1):
          h1.SetBinContent(kB,h1.GetBinContent(kB)+h2.GetBinContent(jB,kB))
      shapes[threshold] = h1
      counter += 1
      iT += 1
  return shapes

def getShapes(tree,vtag,vkin,cut,weight):
  global counter
  ctmp.cd()
  nkinbins=20
  h1 = root.TH1F('h1_%i'%counter,'h1_%i'%counter,1000,vtag.lo,vtag.hi)
  tree.Draw('%s>>h1_%i'%(vtag.formula,counter),tTIMES(weight,tAND('mSD>100&&mSD<150',cut)),'')
  counter += 1

  shapes = {}
  scan = range(1,1001)
  if not vtag.normal: # cut is inverted:
    scan.reverse()
  total = h1.Integral()
  iT=0
  for iB_ in xrange(len(scan)):
    if iT==len(thresholds):
      break
    threshold = thresholds[iT]
    iB = scan[iB_]
    if vtag.normal:
      val = h1.Integral(1,iB)
    else:
      val = h1.Integral(iB,1000)
    if val/total >= threshold:
      h1kin = root.TH1F('h1_%i'%counter,'h1_%i'%counter,nkinbins,vkin.lo,vkin.hi)
      tag = h1.GetBinCenter(iB)
      if vtag.normal:
        tree.Draw('%s>>h1_%i'%(vkin.formula,counter),tTIMES(weight,tAND('%s>%f'%(vtag.formula,tag),cut)),'')
      else:
        tree.Draw('%s>>h1_%i'%(vkin.formula,counter),tTIMES(weight,tAND('%s<%f'%(vtag.formula,tag),cut)),'')
      shapes[threshold] = h1kin
      counter += 1
      iT += 1
  return shapes


def drawShapes(listOfShapes,nickname,axis,labels):
  plot.Reset()
  plot.AddCMSLabel()
  pt=5
  for l in labels:
    plot.AddPlotLabel(labels[l],.18,l,False,42,.04)
  for t in thresholds:
    h = listOfShapes[t]
    h.GetXaxis().SetTitle(axis+' [GeV]')
    h.GetYaxis().SetTitle('a.u.')
    h.SetLineColor(colors[t])
    h.SetNormFactor()
    h.SetLineWidth(3)
    '''
    '''
    if t==0:
      plot.AddHistogram(h,'inclusive',pt,colors[t])
    elif args.signal:
      plot.AddHistogram(h,'%2i%% eff.'%(100-int(t*100)),pt,colors[t])
    else:
      plot.AddHistogram(h,'%2i%% rej.'%(int(t*100)),pt,colors[t])
    pt += 1
  plot.Draw(args.outdir+'/',nickname+'_'+axis)

varlist = [
      Variable('higgs_final_bdt',-0.5,0.5,'higgs_final_bdt'),
      Variable('higgs_all_bdt',-0.5,0.5,'higgs_all_bdt'),
      Variable('higgs_allbutminDR_bdt',-0.5,0.5,'higgs_allbutminDR_bdt'),
      Variable('higgs_ecf_bdt',-0.5,0.5,'higgs_ecf_bdt'),
      Variable('avg_secfN_1_3_20',0,0.1,'avg_secfN_1_3_20',False),
      Variable('tau21',0,1,'tau21',False),
      Variable('tau21SD',0,1,'tau21SD',False),
      Variable('alphapull1',0,3.14,'alpha1',False),
      Variable('qtau21',0,1,'qtau21',False),
      Variable('ecfN_1_3_20/ecfN_1_2_20',0,0.25,'M2',False),
      Variable('ecfN_2_3_20/pow(ecfN_1_2_20,2)',0,0.5,'N2',False),
      Variable('ecfN_3_3_20/pow(ecfN_1_2_20,2)',0,5,'D2',False),
      #Variable('ecfN_2_4_05/pow(ecfN_1_3_05,2)',1,2.5,'N3_05',False),
      #Variable('ecfN_2_4_10/pow(ecfN_1_3_10,2)',0.5,3.5,'N3_10',False),
      #Variable('ecfN_2_4_20/pow(ecfN_1_3_20,2)',0,5,'N3_20',False),
    ]

if args.signal:
  fin = root.TFile(args.indir + '/ZpA0h.root')
else:
  fin = root.TFile(args.indir + '/QCD_evt10.root')
tree = fin.FindObjectAny('puppiCA15')

for v in varlist:
  if args.signal:
    weight = 'ptweight*normalizedWeight'
  else:
    weight = 'ptweight_analytic*normalizedWeight'
  cut = 'pt<1000'
  label = 'Signal' if args.signal else 'QCD'
  if args.signal:
    cut = tAND(cut,'matched==1&&gensize<1.2')

  ptshapes = getShapes(tree,v,Variable('pt',250,1000,'pt'),tAND(cut,'mSD>100&&mSD<150'),weight)
  labels = {.76:v.nickname,
            .80:label,
            .72:'100<m_{SD}<150 [GeV]'}
  drawShapes(ptshapes,label+v.nickname,'pt',labels)

  massshapes = getShapes(tree,v,Variable('mSD',20,500,'mSD'),cut,weight)
  labels = {.76:v.nickname,
            .80:label}
  drawShapes(massshapes,label+v.nickname,'mSD',labels)

  #massshapes = getShapes(tree,v,Variable('2*log(mSD/pt)',-6,0,'rho'),tAND(cut,'mSD>50'),'normalizedWeight')
  #drawShapes(massshapes,label+v.nickname+'_rho')
