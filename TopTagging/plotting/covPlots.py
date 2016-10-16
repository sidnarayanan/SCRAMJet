#!/usr/bin/env python

from os import system,mkdir,getenv
from sys import argv
from array import array
import argparse

basedir = getenv('SCRAMJETFLAT')
figsdir = basedir+'/figs'

parser = argparse.ArgumentParser(description='plot stuff')
parser.add_argument('--indir',metavar='indir',type=str,default=basedir)
parser.add_argument('--outdir',metavar='outdir',type=str,default=figsdir)
args = parser.parse_args()

figsdir = args.outdir
basedir = args.indir
argv=[]

import ROOT as root
from PandaCore.Tools.Load import *
from PandaCore.Tools.Misc import *
from PandaCore.Drawers.SquarePlots import SquarePlotter
##Color palette
ncontours = 999;
# stops = [ 0.0000,         0.10,     0.200,      0.30,    0.4000,      0.50,     0.6000,      0.70,   0.800,     0.900,    1.0000]
# red   = [ 255./255., 240./255., 240./255., 240./255., 243./255., 255./255.,  169./255.,  91./255., 36./255.,   8./255.,   6./255.]
# green = [   0./255.,  46./255.,  99./255., 149./255., 194./255., 255./255.,  194./255., 149./255., 99./255.,  46./255.,   0./255.]
# blue  = [   6./255.,   8./255.,  36./255.,  91./255., 169./255., 255./255.,  243./255., 240./255.,240./255., 240./255., 233./255.]
stops = [ 0.0000,         0.50,    1.0000]
red   = [ 239./255., 255./255.,  92./255.]
green = [  87./255., 255./255., 123./255.]
blue  = [  76./255., 255./255., 237./255.]
stopsArray = array('d',stops)
redArray   = array('d',red)
greenArray = array('d',green)
blueArray  = array('d',blue)
root.TColor.CreateGradientColorTable(len(stops), stopsArray, redArray, greenArray, blueArray, ncontours);
root.gStyle.SetNumberContours(999);
# root.gStyle.SetPalette(root.kLightTemperature)
root.gStyle.SetPaintTextFormat(".1g")


Load('Drawers','CanvasDrawer')

plot = root.CanvasDrawer()
plot.SetTDRStyle()
root.gStyle.SetPadRightMargin(0.15)
root.gStyle.SetPadLeftMargin(0.2)
root.gStyle.SetPadBottomMargin(0.2)

c = root.TCanvas()

varlist = [
      ('tau32','#tau_{32}'),
      ('tau32SD','#tau_{32}^{SD}'),
      ('maxcsv','max CSV',(50,0,1)),
      ('(ecfN_1_2_20/pow(ecfN_1_2_10,2.00))','e(1,2,2)/e(1,2,1)^{2}',(50,2,10)),
      ('ecfN_1_3_40/ecfN_2_3_20','e(1,3,4)/e(2,3,2)',(50,0,1)),
      ('ecfN_3_3_10/pow(ecfN_1_3_40,.75)','e(3,3,1)/e(1,3,4)^{3/4}',(50,.5,4)),
      ('ecfN_3_3_10/pow(ecfN_2_3_20,.75)','e(3,3,1)/e(2,3,2)^{3/4}',(50,.4,1.4)),
      ('ecfN_3_3_20/pow(ecfN_3_3_40,.5)','e(3,3,2)/e(3,3,4)^{1/2}',(50,0,.25)),
      ('ecfN_1_4_20/pow(ecfN_1_3_10,2)','e(1,4,2)/e(1,3,1)^{2}',(50,0,2)),
      ('ecfN_1_4_40/pow(ecfN_1_3_20,2)','e(1,4,4)/e(1,3,2)^{2}',(50,0,2.5)),
      ('ecfN_2_4_05/pow(ecfN_1_3_05,2)','e(2,4,0.5)/e(1,3,0.5)^{2}',(50,1,3)),
      ('ecfN_2_4_10/pow(ecfN_1_3_10,2)','e(2,4,1)/e(1,3,1)^{2}',(50,1,4)),
      ('ecfN_2_4_10/pow(ecfN_2_3_05,2)','e(2,4,1)/e(2,3,0.5)^{2}',(50,0,1.5)),
      ('ecfN_2_4_20/pow(ecfN_1_3_20,2)','e(2,4,2)/e(1,3,2)^{2}',(50,0,5)),
      ('top_ecfv6_bdt','ECF BDT',(50,-1.2,1)),
      ('top_ecfv7_bdt','ECF+#tau_{32}^{SD} BDT',(50,-1.2,1)),
    ]

sp = SquarePlotter(args.indir+'/ZpTT.root',args.indir+'/QCD_evt10.root','puppiCA15')
hcovsig,hcovbg = sp.makeCorrPlots(varlist,'ptweight*normalizedWeight*(matched==1&&gensize<1.2&&mSD<210&&mSD>110)','ptweight_analytic*normalizedWeight*(mSD<210&&mSD>110)')
for h in [hcovsig,hcovbg]:
      h.SetMarkerSize(.75)
      h.GetXaxis().LabelsOption('v')
      h.GetXaxis().SetLabelSize(0.03)
      h.GetYaxis().SetLabelSize(0.03)
      # h.GetZaxis().SetTitleSize(0.03)

c.Clear(); c.cd()
hcovsig.Draw('colz')
hcovsig.SetMinimum(-1)
hcovsig.SetMaximum(1)
plot.SetCanvas(c)
plot.SetGrid()
plot.Draw(args.outdir+'/','corr_sig')

c.Clear(); c.cd()
hcovbg.Draw('colz')
hcovbg.SetMinimum(-1)
hcovbg.SetMaximum(1)
plot.SetCanvas(c)
plot.SetGrid()
plot.Draw(args.outdir+'/','corr_bg')

c.Clear(); c.cd()
hcovsig.Draw('colz text')
hcovsig.SetMinimum(-1)
hcovsig.SetMaximum(1)
plot.SetCanvas(c)
plot.SetGrid()
plot.Draw(args.outdir+'/','corr_sig_text')

c.Clear(); c.cd()
hcovbg.Draw('colz text')
hcovbg.SetMinimum(-1)
hcovbg.SetMaximum(1)
plot.SetCanvas(c)
plot.SetGrid()
plot.Draw(args.outdir+'/','corr_bg_text')