#!/usr/bin/env python

from glob import glob
from os import stat,getenv
from PandaCore.Tools.process import *
from re import sub
import sys
import argparse

print 'starting'

parser = argparse.ArgumentParser(description='make config file')
parser.add_argument('outfile',metavar='outfile',type=str)
parser.add_argument('--include',nargs='+',type=str,default=None)
parser.add_argument('--exclude',nargs='+',type=str,default=None)
args = parser.parse_args()

sys.argv=[]
from ROOT import TFile,TTree

#eos = getenv('EOS')
eos = getenv('HOME')+'/eos'

eoshistdirs=[
      eos+'/cms/store/user/snarayan/scramjet/',
      eos+'/cms/store/group/phys_exotica/monotop/scramjet/'
    ]

listOfFiles=[]
for dire in eoshistdirs:
  print 'searching in',dire
  listOfFiles += glob(dire+'/*/*/*/*/scramjet_*.root')

def checkDS(nickname,include,exclude):
  included=False
  if include:
    for i in include:
      if i in nickname:
        included=True
        break
  else:
    included=True
  excluded=False
  if exclude:
    for e in exclude:
      if e in nickname:
        excluded=True
        break
  else:
    excluded=False
  return (included and not(excluded))

print 'found %i files'%len(listOfFiles)
cfgFile = open(args.outfile,'w')

couldNotFind = []

fcounter=0
for f in sorted(listOfFiles):
  if stat(f).st_size==0:
    continue
  if fcounter%100==0:
    print fcounter,'/',len(listOfFiles)
  fcounter += 1
  ff = f.split('/')
  if 'cernbox' in f:
    if 'bmaier' in f:
      pd = ff[-5]
      start=10
      fileName = "${CERNBOXB}"
    else:
      pd = ff[-5]
      start=10
      fileName = "${CERNBOX}"
  else:
    pd = ff[-5]
    start=11
    if 'phys_exotica' in f:
      fileName = "${EOSEXOTICA}"
    else:
      fileName = "${EOS}"
  for iF in xrange(start,len(ff)):
    fileName += "/"
    fileName += ff[iF]
  try:
    properties = processes[pd]
    nickname = properties[0]
    if 'fast' in f:
      nickname = nickname+'_fast'
  except KeyError:
    if pd not in couldNotFind:
      couldNotFind.append(pd)
    properties = ('UNKNOWN','UNKNOWN',-1)
    continue
  if checkDS(nickname,args.include,args.exclude):
    tf = TFile.Open(f)
    t = tf.FindObjectAny('events')
    nevents = t.GetEntries()
    tf.Close()
    cfgFile.write('{0:<25} {2:<10} {3:<15} {4:<8} {1:<180}\n'.format(nickname,fileName,properties[1],properties[2],nevents)) 

if len(couldNotFind)>0: 
  print 'could not find:'
for pd in couldNotFind:
  print '\t',pd
