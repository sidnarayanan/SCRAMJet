#!/usr/bin/env python

import ROOT as root
from PandaCore.Tools.Load import *
from PandaCore.Tools.Misc import *
from os import getenv

import cfgECFN as cfg

Load('Learning','TMVATrainer')

workdir = getenv('SCRAMJETFLAT')

trainer = root.TMVATrainer('top_ecfbdt',workdir+'training/')
trainer.treename = 'puppiCA15'
trainer.sigweight = 'ptweight*normalizedWeight'
trainer.bgweight = 'ptweight*normalizedWeight'
sanitycut='tau32>0 && pt<600 && mSD>110 && mSD<210'
trainer.bgcut = sanitycut
trainer.sigcut = tAND(sanitycut,'matched==1 && gensize<1.2')

for v in cfg.variables:
  trainer.AddVariable(v[0],v[1])

for s in cfg.spectators:
  trainer.AddSpectator(s[0],s[1])

trainer.SetFiles(workdir+'/ZpTT.root',workdir+'/QCD.root')
trainer.BookBDT()
trainer.TrainAll()

