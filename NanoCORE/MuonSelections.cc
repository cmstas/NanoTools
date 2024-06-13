#include "MuonSelections.h"
#include "IsolationTools.h"
#include "Config.h"

using namespace tas;

namespace WWZ {
    std::shared_ptr<XGBoostInterface> muon_mvareader_map;
}

bool SS::muonID(unsigned int idx, SS::IDLevel id_level, int year) {
    // Common (across years and ID levels) checks
    if (Muon_pt().at(idx) < 5.) { return false; }
    if (fabs(Muon_eta().at(idx)) > 2.4) { return false; }
    if (fabs(Muon_dxy().at(idx)) > 0.05) { return false; }
    if (fabs(Muon_dz().at(idx)) > 0.1) { return false; }
    if (!Muon_looseId().at(idx)) { return false; } // loose POG ID
    if (Muon_miniPFRelIso_all().at(idx) > 0.4) { return false; }
    if (id_level > SS::IDveto) {
        // Common (across years) fakable(loose) and tight checks
        if (Muon_pt().at(idx) < 10.) { return false; }
        if (fabs(Muon_sip3d().at(idx)) >= 4) { return false; }
        if (!Muon_mediumId().at(idx)) { return false; } // medium POG ID
        if (Muon_ptErr().at(idx) / Muon_pt().at(idx) >= 0.2) { return false; }
    }
    switch (year) {
    case (2016):
        return muon2016ID(idx, id_level);
        break;
    case (2017):
        return muon2017ID(idx, id_level);
        break;
    case (2018):
        return muon2018ID(idx, id_level);
        break;
    default:
        throw std::runtime_error("MuonSelections.cc: ERROR - invalid year");
        return false;
        break;
    }
}

bool SS::muon2016ID(unsigned int idx, SS::IDLevel id_level) {
    // ID-specific checks
    switch (id_level) {
    case (SS::IDveto):
        break;
    case (SS::IDfakable):
        break;
    case (SS::IDtight):
        if (!passesLeptonIso(idx, 13, 0.16, 0.76, 7.2)) { return false; }
        break;
    default:
        throw std::runtime_error("MuonSelections.cc: ERROR - invalid ID level");
        return false;
        break;
    }
    return true;
}

bool SS::muon2017ID(unsigned int idx, SS::IDLevel id_level) {
    // ID-specific checks
    switch (id_level) {
    case (SS::IDveto):
        break;
    case (SS::IDfakable):
        break;
    case (SS::IDtight):
        if (!passesLeptonIso(idx, 13, 0.11, 0.74, 6.8)) { return false; }
        break;
    default:
        throw std::runtime_error("MuonSelections.cc: ERROR - invalid ID level");
        return false;
        break;
    }
    return true;
}

bool SS::muon2018ID(unsigned int idx, SS::IDLevel id_level) {
    // ID-specific checks
    switch (id_level) {
    case (SS::IDveto):
        break;
    case (SS::IDfakable):
        // Same as 2017 ID
        break;
    case (SS::IDtight):
        // Same as 2017 ID
        if (!passesLeptonIso(idx, 13, 0.11, 0.74, 6.8)) { return false; }
        break;
    default:
        throw std::runtime_error("MuonSelections.cc: ERROR - invalid ID level");
        return false;
        break;
    }
    return true;
}

bool ttH::muonID(unsigned int idx, ttH::IDLevel id_level, int year) {
    // Common (across ID levels) checks
    if (Muon_pt().at(idx) <= 5.) { return false; }
    if (fabs(Muon_eta().at(idx)) >= 2.4) { return false; }
    if (fabs(Muon_dxy().at(idx)) >= 0.05) { return false; }
    if (fabs(Muon_dz().at(idx)) >= 0.1) { return false; }
    if (fabs(Muon_sip3d().at(idx)) >= 8) { return false; }
    if (Muon_miniPFRelIso_all().at(idx) >= 0.4) { return false; }
    if (!Muon_looseId().at(idx)) { return false; } // loose POG ID
    // Common fakable(loose) and tight checks
    if (id_level > ttH::IDveto) {
        if (Muon_pt().at(idx) <= 10.) { return false; }
        int jet_idx = Muon_jetIdx().at(idx); // jet_idx = -1 if no matched jet
        if (jet_idx >= 0)
        {
            if (Jet_btagDeepFlavB().at(jet_idx) >= gconf.WP_DeepFlav_medium) { return false; };
        }
        if (Muon_ptErr().at(idx) / Muon_pt().at(idx) >= 0.2) { return false; }
    }
    switch (id_level) {
    case (ttH::IDveto):
        break;
    case (ttH::IDfakable):
        if (Muon_mvaTTH().at(idx) <= 0.85) { 
            if (Muon_jetRelIso().at(idx) >= 0.5) { return false; }
        }
        break;
    case (ttH::IDtight):
        if (!Muon_mediumId().at(idx)) { return false; } // medium POG ID
        if (Muon_mvaTTH().at(idx) <= 0.85) { return false; }
        break;
    default:
        throw std::runtime_error("MuonSelections.cc: ERROR - invalid ID level");
        return false;
        break;
    }
    return true;
}

void WWZ::muonLoadMVA(std::string fname)
{

    if (muon_mvareader_map)
    {
        std::cout << "WARNING: XGBoost already loaded, but is trying to load again!" << std::endl;
        return;
    }

    std::cout << "muonLoadMVA(): Loading XGBoost binary file = " << fname << std::endl;

    std::vector<std::string> varnames;
    varnames = std::vector<std::string>{
        "pt",
        "eta",
        "jetNDauCharged",
        "miniPFRelIso_chg",
        "miniPFRelIso_diff_all_chg", // = miniPFRelIso_all - miniPFRelIso_chg
        "jetPtRelv2",
        "jetPtRatio", // = 1/(jetRelIso+1)
        "pfRelIso03_all",
        "ak4jet:btagDeepFlavB", // B tagging discriminant score
        "sip3d",
        "log_abs_dxy",
        "log_abs_dz",
        "segmentComp",
    };
    float missing_entry_val = std::numeric_limits<float>::quiet_NaN();
    muon_mvareader_map = std::make_shared<XGBoostInterface>();
    muon_mvareader_map->build(fname, varnames, missing_entry_val);
}

float WWZ::computeMuonTopMVAScore(unsigned int idx)
{

    float res = -999;
    std::unordered_map<std::string, float> input_vars;

    auto const &vnames = muon_mvareader_map->getVariableNames();
    for (auto const &vname : vnames)
    {
        if (vname == "pt")
            input_vars[vname] = static_cast<float>(tas::Muon_pt().at(idx));
        else if (vname == "eta")
            input_vars[vname] = static_cast<float>(tas::Muon_eta().at(idx));
        else if (vname == "miniPFRelIso_diff_all_chg")
            input_vars[vname] = static_cast<float>(tas::Muon_miniPFRelIso_all().at(idx) - tas::Muon_miniPFRelIso_chg().at(idx));
        else if (vname == "jetPtRatio")
            input_vars[vname] = static_cast<float>(1. / (tas::Muon_jetRelIso().at(idx) + 1.));
        else if (vname == "log_abs_dxy")
            input_vars[vname] = static_cast<float>(std::log(std::abs(tas::Muon_dxy().at(idx))));
        else if (vname == "log_abs_dz")
            input_vars[vname] = static_cast<float>(std::log(std::abs(tas::Muon_dz().at(idx))));
        else if (vname == "sip3d")
            input_vars[vname] = static_cast<float>(tas::Muon_sip3d().at(idx));
        else if (vname == "segmentComp")
            input_vars[vname] = static_cast<float>(tas::Muon_segmentComp().at(idx));
        else if (vname == "miniPFRelIso_chg")
            input_vars[vname] = static_cast<float>(tas::Muon_miniPFRelIso_chg().at(idx));
        else if (vname == "jetPtRelv2")
            input_vars[vname] = static_cast<float>(tas::Muon_jetPtRelv2().at(idx));
        else if (vname == "jetNDauCharged")
            input_vars[vname] = static_cast<int>(tas::Muon_jetNDauCharged().at(idx));
        else if (vname == "pfRelIso03_all")
            input_vars[vname] = static_cast<float>(tas::Muon_pfRelIso03_all().at(idx));
        else if (vname == "ak4jet:btagDeepFlavB")
        {
            if (tas::Muon_jetIdx().at(idx) == -1) input_vars[vname] = static_cast<float>(0.);
            if (tas::Muon_jetIdx().at(idx) != -1) input_vars[vname] = static_cast<float>(tas::Jet_btagDeepFlavB().at(tas::Muon_jetIdx().at(idx)));
        }
        else
        {
            std::cerr << "WWZ::computeMuonTopMVAScore: Input variable name " << vname << " does not match to a corresponding variable" << endl;
            std::cerr << "Have you loaded the XGBoost binary? i.e. did you call muonLoadMVA()?" << std::endl;
            assert(0);
        }
    }

    muon_mvareader_map->eval(input_vars, res);

    return res;
}

bool WWZ::muonID(int idx, WWZ::IDLevel id_level, int year) {
    // Year-specific checks
    switch (year) {
    case (2016):
        return WWZ::muonRun2ID(idx);
        break;
    case (2017):
        return WWZ::muonRun2ID(idx);
        break;
    case (2018):
        return WWZ::muonRun2ID(idx);
        break;
    case (2022):
        return WWZ::muon2022ID(idx, id_level);
        break;
    default:
        throw std::runtime_error("MuonSelections.cc: ERROR - invalid year");
        return false;
        break;
    }
}

bool WWZ::muonRun2ID(unsigned int idx) {
    // These cuts are common for all ID levels of Muon
    if (not (tas::Muon_pt().at(idx)               > 10.  )) return false;
    if (not (fabs(tas::Muon_eta().at(idx))        < 2.4  )) return false;
    if (not (fabs(tas::Muon_dxy().at(idx))        < 0.05 )) return false;
    if (not (fabs(tas::Muon_dz().at(idx))         < 0.1  )) return false;
    if (not (fabs(tas::Muon_sip3d().at(idx))      < 8    )) return false;
    if (not (tas::Muon_miniPFRelIso_all().at(idx) < 0.4  )) return false;
    if (not (tas::Muon_mediumId().at(idx)                )) return false;
    if (not (computeMuonTopMVAScore(idx)          > 0.64 )) return false;
    return true;
}

bool WWZ::muon2022ID(unsigned int idx, WWZ::IDLevel id_level) {
    if (not (Muon_pt().at(idx)               >  10.  )) return false;
    if (not (fabs(Muon_eta().at(idx))        <  2.4  )) return false;
    if (not (fabs(Muon_dxy().at(idx))        <  0.05 )) return false;
    if (not (fabs(Muon_dz().at(idx))         <  0.1  )) return false;
    if (not (Muon_mediumId().at(idx)                 )) return false;
    if (id_level == WWZ::IDveto)
    {
        if (not (Muon_pfRelIso03_all().at(idx)   <  0.40 )) return false;
    }
    else if (id_level == WWZ::IDtight)
    {
        if (not (Muon_pfRelIso03_all().at(idx)   <  0.15 )) return false;
    }
    return true;
}
