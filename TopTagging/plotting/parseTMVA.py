#!/usr/bin/env python

from os import system,mkdir,getenv
from sys import argv
from array import array
import argparse

counter=0

# parse args

parser = argparse.ArgumentParser(description='plot tmva stuff')
parser.add_argument('--inpath',metavar='inpath',type=str)
parser.add_argument('--outpath',metavar='outpath',type=str)
args = parser.parse_args()

argv = [] # clear before loading root
from PandaCore.Tools.Load import *
from PandaCore.Tools.Misc import *
from ROOT import gROOT, gSystem
import ROOT as root

Load('Drawers','HistogramDrawer')

plot = root.HistogramDrawer()
plot.SetTDRStyle()
plot.AddCMSLabel()
plot.InitLegend()
plot.SetNormFactor(True)

ctmp = root.TCanvas()

def getShape(tree,classid,mvaname='BDT',mvalo=-1,mvahi=1.):
  global counter
  ctmp.cd()
  nbins=50
  h = root.TH1F('h%i'%counter,'h%i'%counter,nbins,mvalo,mvahi)
  tree.Draw('%s>>h%i'%(mvaname,counter),'weight*(classID==%i)'%classid)
  h.Scale(1./h.Integral())
  h.GetXaxis().SetTitle(mvaname)
  h.GetYaxis().SetTitle('Probability')
  counter += 1
  return h


tmvafile = root.TFile(args.inpath)
test = tmvafile.Get('TestTree')
train = tmvafile.Get('TrainTree')

csig = root.kCyan+2
cbkg = root.kGray+3

htestsig = getShape(test,0); htestsig.SetFillColor(csig); htestsig.SetFillStyle(3005)
htestbkg = getShape(test,1); htestbkg.SetFillColor(cbkg); htestbkg.SetFillStyle(3004)
htrainsig = getShape(train,0)
htrainbkg = getShape(train,1)

plot.AddHistogram(htestsig,'Test Signal',8,csig,'hist')
plot.AddHistogram(htrainsig,'Train Signal',10,csig,'elp')
plot.AddHistogram(htestbkg,'Test Background',9,cbkg,'hist')
plot.AddHistogram(htrainbkg,'Train Background',11,cbkg,'elp')

plot.Draw(args.outpath+'/','response')
