#!/usr/bin/env python

import ROOT as root
from PandaCore.Tools.Load import *
from PandaCore.Tools.Misc import *
from os import getenv

import cfgInvarH as cfg
#import makeInvariantRatios as cfg

Load('Learning','TMVATrainer')

workdir = getenv('SCRAMJETFLAT')

trainer = root.TMVATrainer('higgs_ecfbdt',workdir+'trainingH/')
trainer.treename = 'puppiCA15'
trainer.sigweight = 'ptweight*normalizedWeight'
trainer.bgweight = 'ptweight*normalizedWeight'
sanitycut='tau21>0 && pt<1000 && mSD>100 && mSD<150'
trainer.bgcut = sanitycut
trainer.sigcut = tAND(sanitycut,'matched==1 && gensize<1.2')

for v in cfg.variables:
  trainer.AddVariable(v[0],v[1])

for v in cfg.formulae:
  trainer.AddVariable(v[0],v[1])

for s in cfg.spectators:
  trainer.AddSpectator(s[0],s[1])

trainer.SetFiles(workdir+'/ZpA0h.root',workdir+'/QCD_evt25.root')
trainer.BookBDT()
trainer.TrainAll()

