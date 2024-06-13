#include "ElectronSelections.h"
#include "IsolationTools.h"
#include "Config.h"

using namespace tas;

namespace WWZ {
    std::shared_ptr<XGBoostInterface> electron_mvareader_map;
}

bool SS::electronID(int idx, SS::IDLevel id_level, int year) {
    // Common (across years and ID levels) checks
    if (Electron_pt().at(idx) < 7.) { return false; }
    if (!SS::isTriggerSafeNoIso(idx)) { return false; }
    if (fabs(Electron_eta().at(idx) + Electron_deltaEtaSC().at(idx)) > 2.5) { return false; }
    if (!Electron_convVeto().at(idx)) { return false; }
    if (fabs(Electron_dxy().at(idx)) >= 0.05) { return false; }
    if (fabs(Electron_dz().at(idx)) >= 0.1) { return false; }
    if (int(Electron_lostHits().at(idx)) > 1) { return false; }
    if (Electron_miniPFRelIso_all().at(idx) >= 0.4) { return false; }
    if (id_level > SS::IDveto) {
        // Common (across years) fakable(loose) and tight checks
        if (Electron_pt().at(idx) < 10.) { return false; }
        if (Electron_tightCharge().at(idx) != 2) { return false; }
        if (fabs(Electron_sip3d().at(idx)) >= 4) { return false; }
        if (int(Electron_lostHits().at(idx)) > 0) { return false; }
    }
    // Year-specific checks
    switch (year) {
    case (2016):
        return SS::electron2016ID(idx, id_level);
        break;
    case (2017):
        return SS::electron2017ID(idx, id_level);
        break;
    case (2018):
        return SS::electron2018ID(idx, id_level);
        break;
    default:
        throw std::runtime_error("ElectronSelections.cc: ERROR - invalid year");
        return false;
        break;
    }
}

bool SS::electron2016ID(int idx, SS::IDLevel id_level) {
    // Common (for 2016) checks
    if (!passesElectronMVA(idx, SS::vetoNoIso2016, 2016)) { return false; }
    // ID-specific checks
    switch (id_level) {
    case (SS::IDveto):
        break;
    case (SS::IDfakable):
        if (!passesElectronMVA(idx, SS::fakableNoIsoLooseMVA2016, 2016)) { return false; }
        break;
    case (SS::IDtight):
        if (!passesElectronMVA(idx, SS::fakableNoIso2016, 2016)) { return false; }
        if (!passesElectronMVA(idx, SS::mediumNoIso2016, 2016)) { return false; }
        if (!passesLeptonIso(idx, 11, 0.12, 0.80, 7.2)) { return false; }
        break;
    default:
        throw std::runtime_error("ElectronSelections.cc: ERROR - invalid ID level");
        return false;
        break;
    }
    return true;
}

bool SS::electron2017ID(int idx, SS::IDLevel id_level) {
    // Common (for 2017) checks
    if (!passesElectronMVA(idx, SS::vetoNoIso2017, 2017)) { return false; }
    // ID-specific checks
    switch (id_level) {
    case (SS::IDveto):
        break;
    case (SS::IDfakable):
        if (!passesElectronMVA(idx, SS::fakableNoIsoLooseMVA2017, 2017)) { return false; }
        break;
    case (SS::IDtight):
        if (!passesElectronMVA(idx, SS::medium2017, 2017)) { return false; }
        if (!passesLeptonIso(idx, 11, 0.07, 0.78, 8.0)) { return false; }
        break;
    default:
        throw std::runtime_error("ElectronSelections.cc: ERROR - invalid ID level");
        return false;
        break;
    }
    return true;
}

bool SS::electron2018ID(int idx, SS::IDLevel id_level) {
    // Common (for 2018) checks
    if (!passesElectronMVA(idx, SS::vetoNoIso2018, 2018)) { return false; }
    // ID-specific checks
    switch (id_level) {
    case (SS::IDveto):
        break;
    case (SS::IDfakable):
        if (!passesElectronMVA(idx, SS::fakableNoIsoLooseMVA2018, 2018)) { return false; }
        break;
    case (SS::IDtight):
        if (!passesElectronMVA(idx, SS::medium2018, 2018)) { return false; }
        if (!passesLeptonIso(idx, 11, 0.07, 0.78, 8.0)) { return false; }
        break;
    default:
        throw std::runtime_error("ElectronSelections.cc: ERROR - invalid ID level");
        return false;
        break;
    }
    return true;
}


bool SS::passesElectronMVA(int idx, SS::ElectronMVAIDLevel id_level, int year) {
    // Define discriminant calculations
    auto electronMVA94X = [](int idx_) {
        float not_raw = Electron_mvaFall17V2noIso().at(idx_);
        if (not_raw > 1.0 - 1.e-6) not_raw = 1.0 - 1.e-6;
        if (not_raw < -1.0 + 1.e-6) not_raw = -1.0 + 1.e-6;
        return -0.5 * log((2.0 / (not_raw + 1)) - 1.0);
    };
    // Get MVA discriminant
    float disc;
    switch (year) {
    case (2016):
        disc = Electron_mvaSpring16GP().at(idx);
        break;
    case (2017):
        disc = Electron_mvaFall17V1noIso().at(idx);
        break;
    case (2018):
        disc = electronMVA94X(idx);
        break;
    default:
        throw std::runtime_error("ElectronSelections.cc: ERROR - invalid year");
        return false;
        break;
    }
    // Define MVA cut calculations
    auto electronMVACut = [](float A, float B, float C, float pt) {
        if (pt < 10) { return C; }
        if (pt > 25) {
            return B;
        } else {
            return (A + (B - A) / 15 * (pt - 10));
        }
    };
    auto electron2016MVACut = [](float A, float B, float C, float pt) {
        if (pt > 10) {
            float pt_min = 15;
            float pt_max = 25;
            float D = A + (B - A) / (pt_max - pt_min) * (pt - pt_min);
            return std::min(A, std::max(B, D));
        } else {
            return C;
        }
    };
    // Calculate absolute value of supercluster eta
    float SC_absEta = fabs(Electron_eta().at(idx) + Electron_deltaEtaSC().at(idx));
    // Get "true" pt of the electron
    float pt = Electron_pt().at(idx);
    // Get MVA result (binned in |SC_eta|)
    switch (id_level) {
    case (SS::vetoNoIso2018):
        if (SC_absEta < 0.8) {
            if (disc <= electronMVACut(0.214, 1.204, 1.320, pt)) { return false; }
        }
        if (SC_absEta >= 0.8 && SC_absEta <= 1.479) {
            if (disc <= electronMVACut(-0.411, 0.084, 0.192, pt)) { return false; }
        }
        if (SC_absEta > 1.479) {
            if (disc <= electronMVACut(-0.918, -0.123, 0.362, pt)) { return false; }
        }
        break;
    case (SS::fakableNoIsoLooseMVA2018):
        if (SC_absEta < 0.8) {
            if (disc <= electronMVACut(-1.036, -0.106, 0.053, pt)) { return false; }
        }
        if (SC_absEta >= 0.8 && SC_absEta <= 1.479) {
            if (disc <= electronMVACut(-1.339, -0.769, -0.434, pt)) { return false; }
        }
        if (SC_absEta > 1.479) {
            if (disc <= electronMVACut(-2.091, -1.461, -0.956, pt)) { return false; }
        }
        break;
    case (SS::medium2018):
        if (SC_absEta < 0.8) {
            if (disc <= electronMVACut(2.597, 4.277, 2.597, pt)) { return false; }
        }
        if (SC_absEta >= 0.8 && SC_absEta <= 1.479) {
            if (disc <= electronMVACut(2.252, 3.152, 2.252, pt)) { return false; }
        }
        if (SC_absEta > 1.479) {
            if (disc <= electronMVACut(1.054, 2.359, 1.054, pt)) { return false; }
        }
        break;
    case (SS::vetoNoIso2017):
        if (SC_absEta < 0.8) {
            if (disc <= electronMVACut(-0.788, -0.64, 0.488, pt)) { return false; }
        }
        if (SC_absEta >= 0.8 && SC_absEta <= 1.479) {
            if (disc <= electronMVACut(-0.85, -0.775, -0.045, pt)) { return false; }
        }
        if (SC_absEta > 1.479) {
            if (disc <= electronMVACut(-0.81, -0.733, 0.176, pt)) { return false; }
        }
        break;
    case (SS::fakableNoIsoLooseMVA2017):
        if (SC_absEta < 0.8) {
            if (disc <= electronMVACut(-0.93, -0.887, -0.135, pt)) { return false; }
        }
        if (SC_absEta >= 0.8 && SC_absEta <= 1.479) {
            if (disc <= electronMVACut(-0.93, -0.89, -0.417, pt)) { return false; }
        }
        if (SC_absEta > 1.479) {
            if (disc <= electronMVACut(-0.942, -0.91, -0.470, pt)) { return false; }
        }
        break;
    case (SS::medium2017):
        if (SC_absEta < 0.8) {
            if (disc <= electronMVACut(0.2, 0.68, 0.2, pt)) { return false; }
        }
        if (SC_absEta >= 0.8 && SC_absEta <= 1.479) {
            if (disc <= electronMVACut(0.1, 0.475, 0.1, pt)) { return false; }
        }
        if (SC_absEta > 1.479) {
            if (disc <= electronMVACut(-0.1, 0.32, -0.1, pt)) { return false; }
        }
        break;
    case (SS::vetoNoIso2016):
        if (SC_absEta < 0.8) {
            if (disc <= electron2016MVACut(-0.48, -0.85, -0.46, pt)) { return false; }
        }
        if (SC_absEta >= 0.8 && SC_absEta <= 1.479) {
            if (disc <= electron2016MVACut(-0.67, -0.91, -0.03, pt)) { return false; }
        }
        if (SC_absEta > 1.479) {
            if (disc <= electron2016MVACut(-0.49, -0.83, -0.01, pt)) { return false; }
        }
        break;
    case (SS::fakableNoIso2016):
        if (SC_absEta < 0.8) {
            if (disc <= electron2016MVACut(0.77, 0.52, 0.77, pt)) { return false; }
        }
        if (SC_absEta >= 0.8 && SC_absEta <= 1.479) {
            if (disc <= electron2016MVACut(0.56, 0.11, 0.56, pt)) { return false; }
        }
        if (SC_absEta > 1.479) {
            if (disc <= electron2016MVACut(0.48, -0.01, 0.48, pt)) { return false; }
        }
        break;
    case (SS::fakableNoIsoLooseMVA2016):
        if (SC_absEta < 0.8) {
            if (disc <= electron2016MVACut(-0.86, -0.96, -0.3, pt)) { return false; }
        }
        if (SC_absEta >= 0.8 && SC_absEta <= 1.479) {
            if (disc <= electron2016MVACut(-0.85, -0.96, -0.36, pt)) { return false; }
        }
        if (SC_absEta > 1.479) {
            if (disc <= electron2016MVACut(-0.81, -0.95, -0.63, pt)) { return false; }
        }
        break;
    case (SS::mediumNoIso2016):
        if (SC_absEta < 0.8) {
            if (disc <= electron2016MVACut(0.77, 0.52, 0.77, pt)) { return false; }
        }
        if (SC_absEta >= 0.8 && SC_absEta <= 1.479) {
            if (disc <= electron2016MVACut(0.56, 0.11, 0.56, pt)) { return false; }
        }
        if (SC_absEta > 1.479) {
            if (disc <= electron2016MVACut(0.48, -0.01, 0.48, pt)) { return false; }
        }
        break;
    default:
        throw std::runtime_error("ElectronSelections.cc: ERROR - invalid MVA ID level");
        return false;
        break;
    }
    return true;
}

bool SS::isTriggerSafeNoIso(int idx) {
    // Calculate absolute value of supercluster eta
    float SC_absEta = fabs(Electron_eta().at(idx) + Electron_deltaEtaSC().at(idx));
    if (SC_absEta <= 1.479) {
        if (Electron_sieie().at(idx) >= 0.011) { return false; }
        if (Electron_hoe().at(idx) >= 0.08) { return false; }
        if (fabs(Electron_eInvMinusPInv().at(idx)) >= 0.01) { return false; }
    } else if (SC_absEta > 1.479 && SC_absEta < 2.5) {
        if (Electron_sieie().at(idx) >= 0.031) { return false; }
        if (Electron_hoe().at(idx) >= 0.08) { return false; }
        if (fabs(Electron_eInvMinusPInv().at(idx)) >= 0.01) { return false; }
    }
    return true;
}

bool ttH::electronID(int idx, ttH::IDLevel id_level, int year) {
    // Common (across ID levels) checks
    if (Electron_pt().at(idx) <= 7.) { return false; }
    if (fabs(Electron_eta().at(idx) + Electron_deltaEtaSC().at(idx)) >= 2.5) { return false; }
    if (fabs(Electron_dxy().at(idx)) >= 0.05) { return false; }
    if (fabs(Electron_dz().at(idx)) >= 0.1) { return false; }
    if (fabs(Electron_sip3d().at(idx)) >= 8) { return false; }
    if (Electron_miniPFRelIso_all().at(idx) >= 0.4) { return false; }
    if (int(Electron_lostHits().at(idx)) > 1) { return false; }
    if (!Electron_mvaFall17V2noIso_WPL().at(idx)) { return false; }
    // Common fakable(loose) and tight checks
    if (id_level > ttH::IDveto) {
        if (Electron_pt().at(idx) <= 10.) { return false; }
        if (!ttH::isTriggerSafeNoIso(idx)) { return false; }
        if (!Electron_convVeto().at(idx)) { return false; }
        if (Electron_tightCharge().at(idx) != 2) { return false; }
        if (int(Electron_lostHits().at(idx)) > 0) { return false; }
        int jet_idx = Electron_jetIdx().at(idx); // jet_idx = -1 if no matched jet
        if (jet_idx >= 0)
        {
            if (Jet_btagDeepFlavB().at(jet_idx) >= gconf.WP_DeepFlav_medium) { return false; };
        }
    }
    switch (id_level) {
    case (ttH::IDveto):
        break;
    case (ttH::IDfakable):
        if (Electron_mvaTTH().at(idx) <= 0.8) {
            if (!Electron_mvaFall17V2noIso_WP80().at(idx)) { return false; }
            if (Electron_jetRelIso().at(idx) >= 0.7) { return false; }
        }
        break;
    case (ttH::IDtight):
        if (Electron_mvaTTH().at(idx) <= 0.8) { return false; }
        break;
    default:
        throw std::runtime_error("ElectronSelections.cc: ERROR - invalid ID level");
        return false;
        break;
    }

    return true;
}

bool ttH::isTriggerSafeNoIso(int idx) {
    // Calculate absolute value of supercluster eta
    float SC_absEta = fabs(Electron_eta().at(idx) + Electron_deltaEtaSC().at(idx));
    if (Electron_hoe().at(idx) >= 0.1) { return false; }
    if (Electron_eInvMinusPInv().at(idx) <= -0.04) { return false; }
    if (SC_absEta <= 1.479) {
        // Barrel
        if (Electron_sieie().at(idx) >= 0.011) { return false; }
    } else if (SC_absEta > 1.479 && SC_absEta < 2.5) {
        // Endcaps
        if (Electron_sieie().at(idx) >= 0.030) { return false; }
    }
    return true;
}

void WWZ::electronLoadMVA(std::string fname)
{

    if (electron_mvareader_map)
    {
        std::cout << "WARNING: XGBoost already loaded, but is trying to load again!" << std::endl;
        return;
    }

    std::cout << "electronLoadMVA(): Loading XGBoost binary file = " << fname << std::endl;

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
    };
    varnames.push_back("mvaFall17V2noIso");
    float missing_entry_val = std::numeric_limits<float>::quiet_NaN();
    electron_mvareader_map = std::make_shared<XGBoostInterface>();
    electron_mvareader_map->build(fname, varnames, missing_entry_val);
}

float WWZ::computeElectronTopMVAScore(unsigned int idx)
{

    float res = -999;
    std::unordered_map<std::string, float> input_vars;

    auto const &vnames = electron_mvareader_map->getVariableNames();
    for (auto const &vname : vnames)
    {
        if (vname == "pt")
            input_vars[vname] = static_cast<float>(tas::Electron_pt().at(idx));
        else if (vname == "eta")
            input_vars[vname] = static_cast<float>(tas::Electron_eta().at(idx));
        else if (vname == "miniPFRelIso_diff_all_chg")
            input_vars[vname] = static_cast<float>(tas::Electron_miniPFRelIso_all().at(idx) - tas::Electron_miniPFRelIso_chg().at(idx));
        else if (vname == "jetPtRatio")
            input_vars[vname] = static_cast<float>(1. / (tas::Electron_jetRelIso().at(idx) + 1.));
        else if (vname == "log_abs_dxy")
            input_vars[vname] = static_cast<float>(std::log(std::abs(tas::Electron_dxy().at(idx))));
        else if (vname == "log_abs_dz")
            input_vars[vname] = static_cast<float>(std::log(std::abs(tas::Electron_dz().at(idx))));
        else if (vname == "sip3d")
            input_vars[vname] = static_cast<float>(tas::Electron_sip3d().at(idx));
        else if (vname == "miniPFRelIso_chg")
            input_vars[vname] = static_cast<float>(tas::Electron_miniPFRelIso_chg().at(idx));
        else if (vname == "jetPtRelv2")
            input_vars[vname] = static_cast<float>(tas::Electron_jetPtRelv2().at(idx));
        else if (vname == "jetNDauCharged")
            input_vars[vname] = static_cast<int>(tas::Electron_jetNDauCharged().at(idx));
        else if (vname == "pfRelIso03_all")
            input_vars[vname] = static_cast<float>(tas::Electron_pfRelIso03_all().at(idx));
        else if (vname == "mvaFall17V2noIso")
            input_vars[vname] = static_cast<float>(tas::Electron_mvaFall17V2noIso().at(idx));
        else if (vname == "ak4jet:btagDeepFlavB")
        {
            input_vars[vname] = float(0);
            if (tas::Electron_jetIdx().at(idx) == -1) input_vars[vname] = static_cast<float>(0.);
            if (tas::Electron_jetIdx().at(idx) != -1) input_vars[vname] = static_cast<float>(tas::Jet_btagDeepFlavB().at(tas::Electron_jetIdx().at(idx)));
        }
        else
        {
            std::cerr << "WWZ::computeElectronTopMVAScore: Input variable name " << vname << " does not match to a corresponding variable" << endl;
            std::cerr << "Have you loaded the XGBoost binary? i.e. did you call muonLoadMVA()?" << std::endl;
            assert(0);
        }
    }

    electron_mvareader_map->eval(input_vars, res);

    return res;
}

bool WWZ::electronID(int idx, WWZ::IDLevel id_level, int year) {
    // Year-specific checks
    switch (year) {
    case (2016):
        return WWZ::electronRun2ID(idx);
        break;
    case (2017):
        return WWZ::electronRun2ID(idx);
        break;
    case (2018):
        return WWZ::electronRun2ID(idx);
        break;
    case (2022):
        return WWZ::electron2022ID(idx, id_level);
        break;
    default:
        throw std::runtime_error("ElectronSelections.cc: ERROR - invalid year");
        return false;
        break;
    }
}

bool WWZ::electronRun2ID(int idx) {
    if (not (tas::Electron_pt().at(idx)               >  10. )) return false;
    if (not (fabs(tas::Electron_eta().at(idx))        <  2.5 )) return false;
    if (not (fabs(tas::Electron_dxy().at(idx))        <  0.05)) return false;
    if (not (fabs(tas::Electron_dz().at(idx))         <  0.1 )) return false;
    if (not (fabs(tas::Electron_sip3d().at(idx))      <  8   )) return false;
    if (not (tas::Electron_miniPFRelIso_all().at(idx) <  0.4 )) return false;
    if (not (tas::Electron_lostHits().at(idx)         <= 1   )) return false;
    if (not (tas::Electron_convVeto().at(idx)                )) return false;
    if (not (tas::Electron_tightCharge().at(idx)      == 2   )) return false;
    if (not (computeElectronTopMVAScore(idx)          > 0.81 )) return false;
    return true;
}

bool WWZ::electron2022ID(int idx, WWZ::IDLevel id_level) {
    float abseta = fabs(Electron_eta().at(idx) + Electron_deltaEtaSC().at(idx));
    if (not (abseta                              < 2.5   )) return false;
    if (not (abseta > 1.566 or abseta < 1.444            )) return false;
    if (not (Electron_pt().at(idx)               > 10.   )) return false;
    if (not (fabs(Electron_dxy().at(idx))        <  0.05 )) return false;
    if (not (fabs(Electron_dz().at(idx))         <  0.1  )) return false;
    if (not (Electron_mvaNoIso_WP90().at(idx)            )) return false;
    if (id_level == WWZ::IDveto)
    {
        if (not (Electron_pfRelIso03_all().at(idx)   <  0.40 )) return false;
    }
    else if (id_level == WWZ::IDtight)
    {
        if (not (Electron_pfRelIso03_all().at(idx)   <  0.15 )) return false;
    }
    return true;
}

