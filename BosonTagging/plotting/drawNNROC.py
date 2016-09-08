import ROOT as root

root.gROOT.LoadMacro('${CMSSW_BASE}/src/MitPanda/Tools/interface/ROCTool.h')
root.gSystem.Load('libMitPandaTools.so')

roc = root.ROCTool()

baseDir = '/home/snarayan/public_html/figs/topTagging/CA15_new/training/nowindow/' 

fin = root.TFile(baseDir+'outputHists.root')

roc.Logy()
roc.SetPlotRange(0.001,1)
roc.InitCanvas()
roc.SetFile(fin)
variables = [
  ('logchi','ln #chi'),
  ('dR_dR0','min #DeltaR'),
  ('mW_dR0','m_{W} (min #DeltaR)'),
  ('sumQG_sumQG0','min Sum QG'),
  ('iotaRMS','RMS p_{T,i}'),
  ('iotaRMS','RMS M_{i}'),
  ('iotaDRRMS','RMS #Delta R_{i}'),
  ('tau32','#tau_{32}'),
  ('NNresponse','NN response')
            ] 
for iV in xrange(len(variables)):
  v,vlabel = variables[iV]
  if 'mW' in v:
    roc.CalcROC('s%s'%v,'b%s'%v,vlabel,iV+2,2,2)
  elif 'response' in v:
  #if 'response' in v:
    print v,vlabel
    roc.CalcROC('hSig','hBg','NN response',0,1,1)
  else:
    roc.CalcROC('s%s'%v,'b%s'%v,vlabel,iV+2,2,1)
roc.DrawAll(baseDir,'roc')

