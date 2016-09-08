import ROOT as root
from array import array
import sys
from numpy import arange

histFile = root.TFile(sys.argv[2])
fileToCorrect = root.TFile(sys.argv[1],"UPDATE")

efficiencies = arange(0.01,.5,.01)
names = [('h%.2f'%(e)).replace('.','') for e in efficiencies]
hists = []
for n in names:
  hists.append(histFile.Get(n))

jets = fileToCorrect.Get("jets")

scaledTree = root.TTree("scaledMVA","scaledMVA")
scaledval = array('f',[1.])
scaledTree.Branch('scaledNNResponse',scaledval,'scaledNNResponse/F')

nEntries = jets.GetEntries()
# nEntries = 100

for iE in xrange(nEntries):
  jets.GetEntry(iE)
  mSD = jets.massSoftDrop
  nnResponse = jets.NNResponse
  scaledval[0]=0.
  for e,h in zip(efficiencies,hists):
#    print nnResponse,e
    if nnResponse>h.GetBinContent(h.FindBin(mSD)):
      scaledval[0] = 1.-e
      break
  scaledTree.Fill()

fileToCorrect.WriteTObject(scaledTree,"scaledMVA","Overwrite")
  
