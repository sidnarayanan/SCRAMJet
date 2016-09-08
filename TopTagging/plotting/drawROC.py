import ROOT as root
from PandaCore.Tools.Load import *

Load('Drawers','ROCTool.h')

roc = root.ROCTool()

basedir = '/home/snarayan/public_html/figs/topTagging/CA15_new/'
fin = root.TFile(basedir+'hists.root')
print fin
roc.Logy()
roc.SetPlotRange(0.001,1)
roc.InitCanvas()
roc.SetFile(fin)
roc.c.AddPlotLabel('110 < m_{SD} < 210 GeV',.16,.8,False,42,.04)
variables = [
#  ('DeltaM0','M_{0}-m_{SD}'),
#  ('DeltaM9','M_{9}-m_{SD}'),
#  ('averageDeltaM','#LTM_{i}-m_{SD}#GT'),
#  ('M9-M0','M_{9}-M_{0}'),
  ('mW_dR0','m_{W} (min #DeltaR)'),
  ('mW_sumQG0','m_{W} (max sum QG)'),
  ('iotaMRMS','Tel. volatility'),
  ('QjetVol','Q-jet volatility'),
  ('tau3Overtau2',"#tau_{32}"),
  ('logchi','ln#chi'),
  ('sumQG_sumQG0','max sum QG'),
  ('dR_dR0','min #DeltaR'),
  ('nowindowedNN','NN'),
  ('windowedNN','NN (mass window)'),
            ] 
for iV in xrange(len(variables)):
  v,vlabel = variables[iV]
  if 'mW' in v:
    roc.CalcROC('h_%s_Signal'%v,'h_%s_QCD'%v,vlabel,iV,2,2)
  elif 'NN' in v:
    roc.CalcROC('h_%s_Signal'%v,'h_%s_QCD'%v,vlabel,iV,1,1)
  else:
    roc.CalcROC('h_%s_Signal'%v,'h_%s_QCD'%v,vlabel,iV,2,1)
roc.DrawAll(basedir,'roc')

