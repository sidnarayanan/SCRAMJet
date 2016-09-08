#!/usr/bin/env python

from sys import argv,stdout,exit
from ROOT import TCanvas,TFile,gStyle,TH2F,TColor
import numpy as np
import MitPanda.Tools.SquarePlots as SP
from operator import itemgetter
from pickle import dump
from array import array

ncontours=999
stops = [0.,.5,1.]
red = [0.,1.,1.]
green = [0.,1.,0.]
blue = [1.,1.,0.]

s = array('d', stops)
r = array('d', red)
g = array('d', green)
b = array('d', blue)

npoints = len(s)
TColor.CreateGradientColorTable(npoints, s, r, g, b, ncontours)
gStyle.SetNumberContours(ncontours)
#gStyle.SetPalette(50,colors)
#gStyle.SetPalette(root.kRedBlue)


gStyle.SetPaintTextFormat("4.3f")

jetAlgo = argv[1]

workDir = '/home/snarayan/cms/root/topTagging_%s/'%(jetAlgo)
ptlow = int(argv[2])
pthigh = int(argv[3])
etahigh = float(argv[4])

plotter = SP.Plotter(workDir+'/qcd.root')
plotter.AddFriend('disc')
c1 = TCanvas("c1","this is a canvass",800,600)
cut = "massSoftDrop>150&&massSoftDrop<240&&pt<%i&&pt>%i&&TMath::Abs(eta)<%f"%(pthigh,ptlow,etahigh)
#hplot,M = plotter.makeCovariancePlot(['logchi','tau3/tau2','groomedIso'],cut)
#rawVarNames = ['logchi','QGTag','QjetVol','groomedIso','sjqgtag0','sjqgtag1','sjqgtag2','tau32']
#rawVarNames = ['logchi','QGTag']
varNames = ['xformed_'+v for v in rawVarNames]
#varNames = ['xformed_tau32','xformed_QjetVol','xformed_QGTag']
hplot,C = plotter.makeCorrelationPlot(varNames,None,cut)
for iV in xrange(len(rawVarNames)):
  hplot.GetXaxis().SetBinLabel(iV+1,'NN'+rawVarNames[iV])
  hplot.GetYaxis().SetBinLabel(iV+1,'NN'+rawVarNames[iV])
c1.cd()
c1.SetLeftMargin(0.15)
c1.SetBottomMargin(0.15)
hplot.SetStats(0)
hplot.SetTitle('%i < p_{T} < %i'%(ptlow,pthigh))
hplot.SetMaximum(1.)
hplot.SetMinimum(-1.)
hplot.GetXaxis().SetLabelSize(0.05)
hplot.GetYaxis().SetLabelSize(0.05)
hplot.Draw("colz text")
suffix = '_ptGT%.1fANDptLT%.1fANDabsetaLT%.1f'%(ptlow,pthigh,etahigh)
suffix = suffix.replace('.','p')
c1.SaveAs(workDir+'/figs/correlationPCA'+suffix+'.png')
c1.SaveAs(workDir+'/figs/correlationPCA'+suffix+'.pdf')

#D = np.linalg.inv(np.power(np.diag(M)*np.eye(M.shape[0]),.5))
#C = np.dot(D,np.dot(M,D))

alphas,vs = np.linalg.eigh(C)
eigs=[(alphas[i],vs[:,i]) for i in xrange(alphas.shape[0])]
eigs.sort(key=itemgetter(0),reverse=True)
nVars = len(eigs)

vectorStrings=[]
for a,v in eigs:
  newString = ""
  for i in xrange(v.shape[0]):
    newString += "%f*%s+"%(v[i],varNames[i])
  vectorStrings.append(newString[:-1])
#plotter.setBins(1,500,-1,1)
#plotter.setBins(2,500,-1,1)
#plotter.setBins(2,200,0,600)
plotter.customBins=True
stringsWithBins = [(s,(500,-1,1)) for s in vectorStrings]
varsWithBins = [(s,(500,0,1)) for s in varNames]
hPCACorr,CPCA = plotter.makeCorrelationPlot(varsWithBins+[('massSoftDrop',(500,0,600))],stringsWithBins,cut) # note CPCA is not square
for iV in xrange(len(vectorStrings)):
  hPCACorr.GetYaxis().SetBinLabel(iV+1,"V_{%i}"%(iV))
for iV in xrange(len(rawVarNames)):
  hPCACorr.GetXaxis().SetBinLabel(iV+1,'NN'+rawVarNames[iV])


c1.Clear()
c1.cd()
c1.SetLeftMargin(0.1)
hPCACorr.SetStats(0)
hPCACorr.SetTitle('%i < p_{T} < %i'%(ptlow,pthigh))
hPCACorr.SetMaximum(1.)
hPCACorr.SetMinimum(-1.)
hPCACorr.GetXaxis().SetLabelSize(0.05)
hPCACorr.GetYaxis().SetLabelSize(0.05)
hPCACorr.Draw('colz text')
c1.SaveAs(workDir+'/figs/decomp'+suffix+'.png')
c1.SaveAs(workDir+'/figs/decomp'+suffix+'.pdf')

hCombs = TH2F("combs","combs",nVars,0,nVars,nVars,0,nVars)
xaxis = hCombs.GetXaxis()
yaxis = hCombs.GetYaxis()
for iV in xrange(nVars):
  yaxis.SetBinLabel(iV+1,"V%i"%(iV))
  xaxis.SetBinLabel(iV+1,'NN'+rawVarNames[iV])
  for jV in xrange(nVars):
    hCombs.SetBinContent(jV+1,iV+1,eigs[iV][1][jV])

c1.Clear()
c1.cd()
c1.SetLeftMargin(0.1)
hCombs.SetStats(0)
hCombs.SetTitle('%i < p_{T} < %i'%(ptlow,pthigh))
hCombs.SetMaximum(1.)
hCombs.SetMinimum(-1.)
hCombs.GetXaxis().SetLabelSize(0.05)
hCombs.GetYaxis().SetLabelSize(0.05)
hCombs.Draw('colz text')
c1.SaveAs(workDir+'/figs/lincombs'+suffix+'.png')
c1.SaveAs(workDir+'/figs/lincombs'+suffix+'.pdf')

'''
maxCorr=0
maxIdx=0
sortedEigs = []
for i in xrange(alphas.shape[0]):
  if CPCA[nVars,i]>maxCorr:
    maxCorr=CPCA[nVars,i]
    maxIdx = i
  sortedEigs.append((abs(CPCA[nVars,i]),eigs[i][1]))

sortedEigs.sort(key=itemgetter(0),reverse=True)
print sortedEigs
with open(workDir+'/pca_%i_%i_%s.pkl'%(int(ptlow),int(pthigh),jetAlgo),'wb') as pklFile:
  dump(sortedEigs,pklFile)

iccPCA = open(workDir+'/pca_%i_%i_%s.icc'%(int(ptlow),int(pthigh),jetAlgo),'w')
objectSuffix = '_%i_%i_%s'%(int(ptlow),int(pthigh),jetAlgo)
iccPCA.write('float **pca'+objectSuffix+' = new float*[%i];\n'%(nVars-1))
for i in xrange(len(sortedEigs)):
  (alpha,v) = sortedEigs[i]
  if i>0:
    iccPCA.write('pca%s[%i] = new float[%i];\n'%(objectSuffix,i-1,nVars))
    for j in xrange(nVars):
      iccPCA.write('pca%s[%i][%i] = %f;\n'%(objectSuffix,i-1,j,v[j]))
    
iccPCA.close()
'''
fout = TFile(workDir+'/figs/correlationPCA'+suffix+'.root',"RECREATE")
fout.WriteTObject(hplot,'c')
fout.WriteTObject(hPCACorr,'corr')
fout.Close()
