# Running SCRAMJetAnalyzer on SubMIT

## Analyzer
------

The `Analyzer` is defined in `SCRAMJet/Analyzer/src/Analyzer.cc`. It is instantiated in `SubMIT/skim.py`. The configuration is all done in this script.

## Configuration
------

Create a configuration file in `SubMIT/config`. It must be of the form:
```
#<output nickname>        <first event>  <last event>   <path to input file>
QCD_ht1000to1500_0        0              4840           ${EOS}/scramjet/QCD_HT1000to1500_TuneCUETP8M1_13TeV-madgraphMLM-pythia8/QCD_HT1000to1500_TuneCUETP8M1_13TeV-madgraphMLM-pythia8/160817_223811/0000/scramjet_1.root

```
Set the name of this config file in `setup.sh` as `SUBMIT_CFG`. Also set the log directory and the work directory (files for submission are stored here) in this config file. Source this file.

## Packaging the environment 
------

First, edit `exec.sh` to do what you want after the job starts. It currently sets up the appropriate environment, X509, and then calls the actual executable.

Then, call `cd $SUBMIT_WORKDIR; makeTar.sh`. It packages up the relevant CMSSW sub-directories. Then, it makes a tarball containing CMSSW, X509, the skimmer, and the config file. All this, and `exec.sh` are moved to `$SUBMIT_WORKDIR`. These steps can be executed by simply calling `runchain.sh`.

## Submitting stuff
------

Finally, call `submit.py`. It creates the condor JDL file and submits the jobs.
