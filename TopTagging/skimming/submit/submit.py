#!/usr/bin/env python

from os import system,environ
from sys import exit,stdout

scramdir=environ['CMSSW_BASE']
cfgName='all'
#cfgName='small_rnd'
nPerJob=4
queue = '2nw4cores' 
#queue = '2nd'

def submit(l):
  outpath='/afs/cern.ch/work/s/%s/logs/%s_%i'%(environ['USER'],cfgName,l)
  cmd = 'bsub -o %s.out -e %s.err -q %s run_skim.sh %i %s %i'%(outpath,outpath,queue,l,cfgName,nPerJob)
  print cmd
  system(cmd)


with open("%s/src/SCRAMJet/TopTagging/config/%s.cfg"%(scramdir,cfgName)) as cfgFile:
  nJobs = len(list(cfgFile))/nPerJob+1
  for iJ in xrange(nJobs):
    submit(iJ)

