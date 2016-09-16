#!/usr/bin/env python

from sys import argv

fin = open(argv[1])
if len(argv)>2:
  nperjob=int(argv[2])
else:
  nperjob=10000

outlines = []
outformat = '{0:<25} {1:<10} {2:<10} {3:<180}\n'

iF=0
for line in fin:
  ll = line.split()
  nickname = ll[0]+'_%i'
  nevents = int(ll[3])
  fpath = ll[4]
  for iE in xrange(nevents/nperjob+1):
    if iE*nperjob>=nevents:
      break
    outline = outformat.format(nickname%iF,iE*nperjob,min(nevents,(iE+1)*nperjob),fpath)
    outlines.append(outline)
    iF+=1

with open(argv[1].replace('.cfg','_trans.cfg'),'w') as fout:
  for line in outlines:
    fout.write(line)



