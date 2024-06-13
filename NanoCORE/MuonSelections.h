#ifndef MUONSELECTIONS_H
#define MUONSELECTIONS_H
#include "Nano.h"
#include "Base.h"
#include "Tools/xgboost/XGBoostInterface.h"

namespace SS {
    bool muonID(unsigned int idx, SS::IDLevel id_level, int year);
    bool muon2016ID(unsigned int idx, SS::IDLevel id_level);
    bool muon2017ID(unsigned int idx, SS::IDLevel id_level);
    bool muon2018ID(unsigned int idx, SS::IDLevel id_level);
}

namespace ttH {
    bool muonID(unsigned int idx, ttH::IDLevel id_level, int year);
}

namespace WWZ {
    extern std::shared_ptr<XGBoostInterface> muon_mvareader_map;
    void muonLoadMVA(std::string fname);
    float computeMuonTopMVAScore(unsigned int idx);
    bool muonID(int idx, WWZ::IDLevel id_level, int year);
    bool muonRun2ID(unsigned int idx);
    bool muon2022ID(unsigned int idx, WWZ::IDLevel id_level);
}

#endif
