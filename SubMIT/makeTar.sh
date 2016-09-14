#!/bin/bash

WD=$PWD

cd $CMSSW_BASE/
tar -chvzf scramjet.tgz biglib bin lib objs test external # h = --dereference symlinks

mv scramjet.tgz $WD
