#!/usr/bin/env python

from sys import exit
from glob import glob
from os import stat,getenv
from PandaCore.Tools.process import *
from re import sub
import argparse

parser = argparse.ArgumentParser(description='make config file')
parser.add_argument('outfile',metavar='outfile',type=str)
parser.add_argument('--include',nargs='+',type=str,default=None)
parser.add_argument('--exclude',nargs='+',type=str,default=None)
args = parser.parse_args()

eos = getenv('EOS')
histdir = getenv('SCRAMJETHIST')
#cernboxdir = getenv('SCRAMJETHIST_CERNBOX')
#cernboxbdir = getenv('SCRAMJETHIST_CERNBOXB')
listOfFiles=[]
#for dire in [histdir,cernboxdir,cernboxbdir]:
for dire in [histdir]:
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

for f in sorted(listOfFiles):
  if stat(f).st_size==0:
    continue
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
#    print 'COULD NOT FIND',pd
    properties = ('UNKNOWN','UNKNOWN',-1)
    continue
  if 'memtest' in fileName: 
    continue
  if checkDS(nickname,args.include,args.exclude):
    cfgFile.write('{0:<25} {2:<10} {3:<15} {1:<180}\n'.format(nickname,fileName,properties[1],properties[2])) 

if len(couldNotFind)>0: 
  print 'could not find:'
for pd in couldNotFind:
  print '\t',pd
