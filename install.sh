#!/bin/bash

cd $CMSSW_BASE/src/

mkdir -p fastjet
cd fastjet

FASTJET_URL="http://fastjet.fr/repo"
FASTJET_TGZ="fastjet-3.2.1.tar.gz"
FASTJET_DIR=`echo $PWD/$FASTJET_TGZ | sed 's/.tar.gz//'`
FASTJET_VER=`echo $FASTJET_TGZ | sed 's/.tar.gz//' |cut -d'-' -f2`
export FASTJET_BASE=$PWD
wget "$FASTJET_URL/$FASTJET_TGZ" -O $FASTJET_TGZ
tar fzx $FASTJET_TGZ
rm -rf $FASTJET_TGZ
cd $FASTJET_DIR
./configure --prefix=$FASTJET_BASE --enable-cmsiterativecone --enable-atlascone
make -j$(nproc)
make check  
make install 
cd ..

CONTRIB_DIR=fjcontrib
svn checkout http://fastjet.hepforge.org/svn/contrib/trunk $CONTRIB_DIR
cd $CONTRIB_DIR
echo "EnergyCorrelator                               trunk" >> contribs.local
sed -i 's?https?http?g' scripts/internal/common.sh # ???
./scripts/update-contribs.sh
./configure --fastjet-config=$FASTJET_BASE/bin/fastjet-config CXXFLAGS="-I$FASTJET_BASE/include -I$FASTJET_BASE/tools"
make -j$(nproc)
make check  # optional
make install
make fragile-shared -j$(nproc)
make fragile-shared-install 

cp $CMSSW_BASE/src/SCRAMJet/fastjet.xml $CMSSW_BASE/config/toolbox/$SCRAM_ARCH/tools/selected/fastjet.xml
scram setup fastjet

cd ${FASTJET_BASE}/lib
for f in *; do 
    ln -sf ${PWD}/${f} ${CMSSW_BASE}/lib/slc6_amd64_gcc530/ # so we can run on CRAB
    ln -sf ${PWD}/${f} ${CMSSW_BASE}/external/slc6_amd64_gcc530/lib # does scram setup not do this anymore?
  done

cd $CMSSW_BASE/src/SCRAMJet/
git clone git@github.com:sidnarayanan/SDAlgorithm.git

cd $CMSSW_BASE/src/
git clone git@gitlab.com:sidnarayanan/PandaCore.git

scram b -j$(nproc)
