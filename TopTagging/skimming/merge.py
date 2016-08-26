#!/usr/bin/env python

from ROOT import gSystem, gROOT
import ROOT as root
from array import array
from glob import glob
from re import sub
from sys import argv
from os import environ,system,path
from PandaCore.Tools.process import *
from PandaCore.Tools.Load import *

Load('Tools','Normalizer')

pds = {}
for k,v in processes.iteritems():
  if v[1]=='MC':
    pds[v[0]] = (k,v[2])  
  else:
    pds[v[0]] = (k,-1)

VERBOSE=False

user = environ['USER']
system('mkdir -p /tmp/%s/split'%user) # tmp dir
system('mkdir -p /tmp/%s/merged'%user) # tmp dir

base = environ['SCRAMJETFLAT']
inbase = base+'/batch/'


def hadd(inpath,outpath):
  if type(inpath)==type('str'):
    infiles = glob(inpath)
    print 'hadding %s into %s'%(inpath,outpath)
    cmd = 'hadd -ff -n 100 -f %s %s > /dev/null'%(outpath,inpath)
    system(cmd)
    return
  else:
    infiles = inpath
  if len(infiles)==0:
    print 'WARNING: nothing hadded into',outpath
    return
  elif len(infiles)==1:
    cmd = 'cp %s %s'%(infiles[0],outpath)
  else:
    cmd = 'hadd -ff -n 100 -f %s '%outpath
    for f in infiles:
      if path.isfile(f):
        cmd += '%s '%f
  if VERBOSE: print cmd
  system(cmd+' >/dev/null 2>/dev/null')

def normalizeFast(fpath,opt,treename):
  xsec=-1
  if type(opt)==type(1.) or type(opt)==type(1):
    xsec = opt
  else:
    try:
      xsec = processes[proc][2]
    except KeyError:
      for k,v in processes.iteritems():
        if proc in k:
          xsec = v[2]
  if xsec<0:
    print 'could not find xsec, skipping %s!'%opt
    return
  print 'fast normalizing %s (%s) ...'%(fpath,opt)
  n = root.Normalizer();
  n.treeName = treename
  n.NormalizeTree(fpath,xsec)

def merge(shortnames,mergedname):
  for shortname in shortnames:
    try:
      pd = pds[shortname][0]
      xsec = pds[shortname][1]
    except KeyError:
      for shortname_ in [shortname.split('_')[0],shortname.split('_')[-1]]:
        if shortname_ in pds:
          pd = pds[shortname_][0]
          xsec = pds[shortname_][1]
          break
    inpath = inbase+shortname+'_*.root'
    hadd(inpath,'/tmp/%s/split/%s.root'%(user,shortname))
    if xsec>0:
      normalizeFast('/tmp/%s/split/%s.root'%(user,shortname),xsec,'puppiCA15')
      #normalizeFast('/tmp/%s/split/%s.root'%(user,shortname),xsec,'chsCA15')
  hadd(['/tmp/%s/split/%s.root'%(user,x) for x in shortnames],'/tmp/%s/merged/%s.root'%(user,mergedname))


args = {}
d = { 
  'QCD' : ['QCD_ht%ito%i'%(b[0],b[1]) for b in [(200,300),(300,500),(500,700),(700,100),(1000,1500)]]  
  #'QCD' : ['QCD_ht%ito%i'%(b[0],b[1]) for b in [(200,300),(300,500),(500,700),(700,100),(1000,1500),(1500,2000)]] + ['QCD_ht2000toinf'] 
  }

for pd in argv[1:]:
  try:
    args[pd] = d[pd]
  except KeyError:
    args[pd] = [pd] 

for pd in args:
  merge(args[pd],pd)
  system('cp -r /tmp/%s/merged/%s.root %s'%(user,pd,base))
  print 'finished with',pd

