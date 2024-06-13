#ifndef XGBOOSTINTERFACE_H
#define XGBOOSTINTERFACE_H

#include <xgboost/c_api.h>
//#include "${XGBOOST_PATH}/include/xgboost/c_api.h"
//#include "/cvmfs/cms.cern.ch/slc7_amd64_gcc900/external/py3-xgboost/0.90-ghbfee2/lib/python3.8/site-packages/xgboost/include/xgboost/c_api.h"
#include "MLWrapper.h"

class XGBoostInterface : public MLWrapper
{
    protected:
        BoosterHandle *booster;
        float defval;
        std::vector<std::string> variable_names;

    public:
        XGBoostInterface();
        virtual ~XGBoostInterface();

        bool build(std::string fname, std::vector<std::string> const &varnames, float missing_entry_val);

        std::vector<std::string> const &getVariableNames() const { return variable_names; }

        BoosterHandle *const &getBooster() const { return booster; }

        template <typename T> bool eval(std::unordered_map<std::string, float> const &vars, std::vector<T> &res);
        template <typename T> bool eval(std::unordered_map<std::string, float> const &vars, T &res);
};

#endif

