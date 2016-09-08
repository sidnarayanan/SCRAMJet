#!/usr/bin/env python

import ROOT as root
import sys
from numpy import arange,array

'''
ptlow = int(sys.argv[1])
pthigh = int(sys.argv[2])
etahigh = float(sys.argv[3])
'''


# root.gROOT.LoadMacro("${CMSSW_BASE}/src/MitPanda/Fillers/src/MVATransformer.cc+")
root.gROOT.LoadMacro("${CMSSW_BASE}/src/MitPanda/Fillers/interface/MVATransformer.h")
root.gSystem.Load("libMitPandaFillers.so")
xformer = root.MVATransformer()

workDir='/home/snarayan/cms/root/tagAndProbe_v3/'

fHist = root.TFile(workDir+'cuts.root','RECREATE')

chains = []

chains.append(root.TChain('DY'))
chains[-1].AddFile(workDir+'/dy.root')

chains.append(root.TChain('QCD'))
chains[-1].AddFile(workDir+"/qcd_pt170to300.root")
chains[-1].AddFile(workDir+"/qcd_pt300to470.root")
chains[-1].AddFile(workDir+"/qcd_pt470to600.root")
chains[-1].AddFile(workDir+"/qcd_pt600to800.root")
chains[-1].AddFile(workDir+"/qcd_pt800to1000.root")
chains[-1].AddFile(workDir+"/qcd_pt1400to1800.root")
chains[-1].AddFile(workDir+"/qcd_pt1800to2400.root")
chains[-1].AddFile(workDir+"/qcd_pt2400to3200.root")
chains[-1].AddFile(workDir+"/qcd_pt3200toinf.root")

chains.append(root.TChain('WJets'))
chains[-1].AddFile(workDir+'/wjets.root')

chains.append(root.TChain('ZNuNuJets'))
chains[-1].AddFile(workDir+'/zjets_nunu_ht100to200.root')
chains[-1].AddFile(workDir+'/zjets_nunu_ht200to400.root')
chains[-1].AddFile(workDir+'/zjets_nunu_ht400to600.root')
chains[-1].AddFile(workDir+'/zjets_nunu_ht600toinf.root')

chains.append(root.TChain('fakeTTbar'))
chains[-1].AddFile(workDir+'/ttjets_unmatched.root')

for chain in chains:
  xformer.AddSample(chain)

xformer.SetVariable("CA15fj1_mSD",30,0,300)
xformer.SetMVAVariable("NNResponse",10000)

efficiencies = arange(0.01,.11,.01)
#efficiencies = array([.01])
names = [('h%.2f'%(e)).replace('.','') for e in efficiencies]

cut = "mcWeight*(CA15fj1_pt>250 && nSelectedCA15fj==1 && nTightElectron+nTightMuon==1)"

for e,n in zip(efficiencies,names):
  xformer.ComputeAll(e,cut)
  h = xformer.GetCuts().Clone(n)
  fHist.WriteTObject(h,n,"Overwrite")
  if e==0.19 and False:
    hTest = root.TH1F("test","test",50,0,500)
    for chain in chains:
      print chain.GetEntries()
      for iE in xrange(chain.GetEntries()):
        if not iE%10000:
          print iE
        chain.GetEntry(iE)
        if chain.puppimet>250 and chain.CA15fj1_pt>250 and chain.nSelectedCA15fj==1:
          mSD = chain.CA15fj1_mSD
          cutVal = h.GetBinContent(h.FindBin(mSD))
          if chain.NNResponse>cutVal:
            hTest.Fill(mSD,chain.mcWeight)
      fHist.WriteTObject(hTest,"htest","Overwrite")

fHist.Close()
