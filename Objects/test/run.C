{
  gSystem->CompileMacro("testTree.C");
  testTree();
  TTree *t = getTree();
}
