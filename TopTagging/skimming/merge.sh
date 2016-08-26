#!/bin/bash 
echo $(echo QCD; for m in 500 750 1000 1250 1500 2000 2500 3000 3500 4000; do echo ZpTT_med-$m; done; for m in 800 1000 1200 1400 1600 1800 2000 2500; do echo ZpWW_med-$m; done) | xargs -n 1 -P 6 python merge.py
