#!/usr/bin/env python

import ROOT as root
from PandaCore.Tools.Load import *
from os import getenv

Load('Learning','TMVATrainer')

workdir = getenv('HOME')+'/work/skims/scramjet_v1/'

trainer = root.TMVATrainer('ecfbdt',workdir+'training/')
trainer.treename = 'puppiCA15'
trainer.sigweight = 'ptweight*normalizedWeight'
trainer.bgweight = 'ptweight*normalizedWeight'
trainer.sigcut = 'matched==1&&gensize<1.2&&pt<600'
trainer.bgcut = 'pt<600'

ecfnstring = 'ecfN_%i_%i_%.2i'
for N in [2,3,4]:
  for order in [1,2,3]:
    if N==4 and order==3:
      continue
    if N==2 and order!=1:
      continue
    for beta in [5,10,20]:
      trainer.AddVariable(ecfnstring%(order,N,beta),'F')

trainer.AddSpectator('tau32','F')
trainer.AddSpectator('tau32SD','F')
trainer.AddSpectator('pt','F')
trainer.AddSpectator('mSD','F')
#trainer.AddSpectator('TMath::Log(mSD/pt)','F','rho')

trainer.SetFiles(workdir+'/ZpTT.root',workdir+'/QCD.root')
trainer.BookBDT()
trainer.TrainAll()

