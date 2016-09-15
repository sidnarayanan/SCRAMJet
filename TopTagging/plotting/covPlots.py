#!/usr/bin/env python

from ROOT import gROOT, gSystem
import ROOT as root
from PandaCore.Tools.Load import *
from PandaCore.Tools.Misc import *
from PandaCore.Drawers.SquarePlots import SquarePlotter
from os import system,mkdir,getenv
from sys import argv
from array import array
import argparse

##Color palette
ncontours = 999;
stops = [ 0.0000,         0.10,     0.200,      0.30,    0.4000,      0.50,    0.7500,    0.8750,    1.0000]
red   = [ 243./255., 243./255., 240./255., 240./255., 241./255., 255./255., 186./255., 151./255., 129./255.]
green = [   0./255.,  46./255.,  99./255., 149./255., 194./255., 255./255., 183./255., 166./255., 147./255.]
blue  = [   6./255.,   8./255.,  36./255.,  91./255., 169./255., 255./255., 246./255., 240./255., 233./255.]
stopsArray = array('d',stops)
redArray   = array('d',red)
greenArray = array('d',green)
blueArray  = array('d',blue)
root.TColor.CreateGradientColorTable(9, stopsArray, redArray, greenArray, blueArray, ncontours);
root.gStyle.SetNumberContours(ncontours);

basedir = getenv('SCRAMJETFLAT')
figsdir = basedir+'/figs'

parser = argparse.ArgumentParser(description='plot stuff')
parser.add_argument('--indir',metavar='indir',type=str,default=basedir)
parser.add_argument('--outdir',metavar='outdir',type=str,default=figsdir)
parser.add_argument('--cut',metavar='cut',type=str,default=None)
args = parser.parse_args()

figsdir = args.outdir
basedir = args.indir

Load('Drawers','CanvasDrawer')

plot = root.CanvasDrawer()
plot.SetTDRStyle()
root.gStyle.SetPadRightMargin(0.15)

c = root.TCanvas()

varlist = [
      ('tau32','tau32'),
      ('tau32SD','tau32SD'),
      ('fitprob','fitprob'),
      ('fitmassW','fitmassW'),
      ('mW_minDR','mW_minDR'),
      ('TMath::Sqrt(dR2_minDR)','minDR'),
      ('ecfN_2_4_05/TMath::Power(ecfN_1_3_05,2)','N3_0.5'),
      ('ecfN_2_4_10/TMath::Power(ecfN_1_3_10,2)','N3_1.0'),
      ('ecfN_2_4_20/TMath::Power(ecfN_1_3_20,2)','N3_2.0'),
    ]

sp = SquarePlotter(args.indir+'/ZpTT.root',args.indir+'/QCD.root','puppiCA15')
hcovsig,hcovbg = sp.makeCorrPlots(varlist,'ptweight*normalizedWeight*(matched==1&&gensize<1.2&&mSD<210&&mSD>110)','ptweight*normalizedWeight*(mSD<210&&mSD>110)')

c.Clear(); c.cd()
hcovsig.Draw('colz')
hcovsig.SetMinimum(-1)
hcovsig.SetMaximum(1)
plot.SetCanvas(c)
plot.Draw(args.outdir+'/','corr_sig')

c.Clear(); c.cd()
hcovbg.Draw('colz')
hcovbg.SetMinimum(-1)
hcovbg.SetMaximum(1)
plot.SetCanvas(c)
plot.Draw(args.outdir+'/','corr_bg')
