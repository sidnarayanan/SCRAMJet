import ROOT as root

root.gROOT.LoadMacro('${CMSSW_BASE}/src/MitPanda/Tools/interface/CanvasDrawer.h')
root.gSystem.Load('libMitPandaTools.so')
root.gStyle.SetNumberContours(999)
root.gStyle.SetOptStat(0)
root.gStyle.SetPalette(56)
plot = root.CanvasDrawer()
plot.SetTDRStyle()
root.gStyle.SetPadRightMargin(0.15)

c = root.TCanvas()
c.SetLogz()

nBins=50

nnName = 'tau3/tau2'

basedir = '/local/snarayan/ca15/'
#fin = root.TFile(basedir+'zprime.root')
fin = root.TFile(basedir+'qcd.root')
tin = fin.Get('events')
#tin.AddFriend(fin.Get(nnName))

hist = root.TH2D('hist','Mass vs '+nnName,nBins,0,400,nBins,0,1.5)
hist.GetYaxis().SetTitle('#tau_{32} [pdf]')
hist.GetXaxis().SetTitle('m_{SD} [GeV]')
hist.GetYaxis().SetTitleOffset(1.5)
#hist.GetZaxis().SetLimits(0.0005,0.99)
hist.SetMinimum(0.0005)
hist.SetMaximum(0.99)
tin.Draw('%s:mSD>>hist'%nnName,'mcweight','colz')
proj = hist.ProjectionX()
for xB in xrange(1,nBins+1):
  denom = proj.GetBinContent(xB)
  for yB in xrange(1,nBins+1):
    hist.SetBinContent(xB,yB,hist.GetBinContent(xB,yB)/denom)
prof = hist.ProfileX()
prof.SetFillStyle(0)
prof.SetLineWidth(3)
prof.SetLineColor(1)
prof.Draw('same hist')

plot.SetCanvas(c)

plot.Draw(basedir+'figs/','massVstau32')
