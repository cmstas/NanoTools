#ifndef XGBOOSTINTERFACE_HPP
#define XGBOOSTINTERFACE_HPP

#include <cassert>
//#include "/cvmfs/cms.cern.ch/slc7_amd64_gcc900/external/py3-xgboost/0.90-ghbfee2/lib/python3.8/site-packages/xgboost/include/xgboost/c_api.h"
#include "XGBoostInterface.h"

using namespace std;

#define SAFE_XGBOOST(CALL) \
{ int err_call = (CALL); if (err_call!=0){ std::cout << "Call '" << #CALL << "' returned error code " << err_call << ". XGBoost last error: " << XGBGetLastError() << std::endl; } }

template<typename T> bool XGBoostInterface::eval(std::unordered_map<std::string, float> const& vars, std::vector<T>& res){

  res.clear();

  constexpr unsigned long long nSample = 1;
  const unsigned long long nFeatures = variable_names.size();
  float* data_arr = new float[nFeatures];  // array containing the data values input into XGBoost
  float* data_arr_ptr = &(data_arr[0]);
  for (auto& vv:variable_names){
    auto it_vars = vars.find(vv);
    if (it_vars==vars.end()) *data_arr_ptr = defval;
    else *data_arr_ptr = it_vars->second;
    data_arr_ptr++;
  };

  bst_ulong nout = 0;
  const float* score;
  DMatrixHandle dvalues;
  SAFE_XGBOOST(XGDMatrixCreateFromMat(data_arr, nSample, nFeatures, defval, &dvalues));
  SAFE_XGBOOST(XGBoosterPredict(*booster, dvalues, 0, 0, &nout, &score));
  SAFE_XGBOOST(XGDMatrixFree(dvalues));

  res.clear();
  res.reserve(nout);
  for (bst_ulong rr=0; rr<nout; rr++) res.push_back(static_cast<T>(score[rr]));

  delete[] data_arr; 
  return true;
}

template<typename T> bool XGBoostInterface::eval(std::unordered_map<std::string, float> const& vars, T& res){
  std::vector<T> vres;
  bool success = this->eval(vars, vres);
  if (vres.empty() || vres.size()!=1){
    std::cout << "XGBoostInterface::eval: The vector of results has size = " << vres.size() << " != 1." << std::endl;
    assert(0);
    success = false;
  }

  res = vres.front();
  return success;
}

template bool XGBoostInterface::eval<float>(std::unordered_map<std::string,float> const& vars, float& res);
template bool XGBoostInterface::eval<double>(std::unordered_map<std::string,float> const& vars, double& res);

#endif
