#ifndef SCRAMJET_OBJECTS_LINKDEF_H
#define SCRAMJET_OBJECTS_LINKDEF_H
#include "SCRAMJet/Objects/interface/PObject.h"
#include "SCRAMJet/Objects/interface/PEvent.h"
#include "SCRAMJet/Objects/interface/PPFCand.h"
#include "SCRAMJet/Objects/interface/PJet.h"
#include "SCRAMJet/Objects/interface/PFatJet.h"
#include "SCRAMJet/Objects/interface/PGenParticle.h"
#endif

#ifdef __CLING__
#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;
#pragma link C++ nestedclass;
#pragma link C++ nestedtypedef;
#pragma link C++ namespace scramjet;

#pragma link C++ class scramjet::PObject+;
#pragma link C++ class scramjet::PEvent+;
#pragma link C++ class scramjet::PPFCand+;
#pragma link C++ class scramjet::PJet+;
#pragma link C++ class scramjet::PFatJet+;
#pragma link C++ class scramjet::PGenParticle+;
#endif
