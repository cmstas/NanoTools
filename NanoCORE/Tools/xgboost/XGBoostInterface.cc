#include <iostream>
#include <limits>
#include <ostream>
//#include <xgboost/c_api.h>
//#include "/cvmfs/cms.cern.ch/slc7_amd64_gcc900/external/py3-xgboost/0.90-ghbfee2/lib/python3.8/site-packages/xgboost/include/xgboost/c_api.h"
#include "XGBoostInterface.hpp"

XGBoostInterface::XGBoostInterface() : MLWrapper(), booster(nullptr), defval(0) {}

XGBoostInterface::~XGBoostInterface()
{
    SAFE_XGBOOST(XGBoosterFree(*booster));
    delete booster;
}

bool XGBoostInterface::build(std::string fname, std::vector<std::string> const &varnames, float missing_entry_val)
{

    if (booster)
    {
        std::cerr << "XGBoostInterface::build: The booster is already built." << endl;
        return false;
    }
    if (fname == "")
    {
        std::cerr << "XGBoostInterface::build: The file name is an empty string. This function should be called to load models from a file." << endl;
        assert(0);
    }

    defval = missing_entry_val;
    variable_names = varnames;

    booster = new BoosterHandle;
    SAFE_XGBOOST(XGBoosterCreate(nullptr, 0, booster));

    // std::cout << "XGBoostInterface::build: A new xgboost is created. Loading the model in " << fname << "..." << endl;

    SAFE_XGBOOST(XGBoosterLoadModel(*booster, fname.data()));

    return true;
}
