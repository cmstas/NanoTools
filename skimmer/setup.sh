#!/bin/bash

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

#echo "Setting up ROOT"
export SCRAM_ARCH=slc7_amd64_gcc900   # or whatever scram_arch you need for your desired CMSSW release
export CMSSW_VERSION=CMSSW_11_2_0_pre5
source /cvmfs/cms.cern.ch/cmsset_default.sh
cd /cvmfs/cms.cern.ch/$SCRAM_ARCH/cms/cmssw/$CMSSW_VERSION/src
eval `scramv1 runtime -sh`
cd - > /dev/null

echo 'Setup following ROOT'
which root

xgboost_cfg=${CMSSW_BASE}/config/toolbox/${SCRAM_ARCH}/tools/selected/py3-xgboost.xml
xgboost_version="$(grep -e 'tool name' ${xgboost_cfg} | grep 'py3-xgboost')"
xgboost_version="${xgboost_version##*=\"}"
xgboost_version="${xgboost_version%%\"*}"
xgboost_path="/cvmfs/cms.cern.ch/${SCRAM_ARCH}/external/py3-xgboost/${xgboost_version}/lib/python3.8/site-packages/xgboost"
export LD_LIBRARY_PATH=${xgboost_path}/lib:$LD_LIBRARY_PATH
export XGBOOST_PATH=${xgboost_path}

#eof
