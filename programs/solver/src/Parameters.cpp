#include "Parameters.hpp"
#include <iostream>

std::ostream& operator<<(std::ostream& os, const Parameters& param) {
    auto initialSolutionToStr = [](Parameters::InitialSolution val) {
        switch (val) {
            case Parameters::InitialSolution::GT: return "GT";
            case Parameters::InitialSolution::CONSTR: return "CONSTR";
            default: return "UNKNOWN";
        }
    };

    auto dispatchingRuleToStr = [](Parameters::DispatchingRule val) {
        switch (val) {
            case Parameters::DispatchingRule::EDD: return "EDD";
            case Parameters::DispatchingRule::ACS: return "ACS";
            case Parameters::DispatchingRule::RAND: return "RAND";
            default: return "UNKNOWN";
        }
    };

    auto neighborhoodToStr = [](Parameters::Neighborhood val) {
        switch (val) {
            case Parameters::Neighborhood::SWAP_ADJ: return "SWAP_ADJ";
            case Parameters::Neighborhood::SWAP_RAND: return "SWAP_RAND";
            case Parameters::Neighborhood::INSERT_RAND: return "INSERT_RAND";
            case Parameters::Neighborhood::SWAP_PENAL: return "SWAP_PENAL";
            case Parameters::Neighborhood::INSERT_PENAL: return "INSERT_PENAL";
            case Parameters::Neighborhood::CRITICAL_OPER: return "CRITICAL_OPER";
            case Parameters::Neighborhood::CRITICAL_OPER_ALT: return "CRITICAL_OPER_ALT";
            case Parameters::Neighborhood::RELAX_2: return "RELAX_2";
            default: return "UNKNOWN";
        }
    };

    auto nHoodTraversingToStr = [](Parameters::NHoodTraversing val) {
        switch (val) {
            case Parameters::NHoodTraversing::BI: return "BI";
            case Parameters::NHoodTraversing::FI: return "FI";
            case Parameters::NHoodTraversing::ELT_LIST: return "ELT_LIST";
            default: return "UNKNOWN";
        }
    };

    auto searchMethodToStr = [](Parameters::SearchMethod val) {
        switch (val) {
            case Parameters::SearchMethod::LS: return "LS";
            case Parameters::SearchMethod::ILS: return "ILS";
            case Parameters::SearchMethod::TABU: return "TABU";
            default: return "UNKNOWN";
        }
    };

    auto schedulerToStr = [](Parameters::Scheduler val) {
        switch (val) {
            case Parameters::Scheduler::EARLY: return "EARLY";
            case Parameters::Scheduler::CPLEX: return "CPLEX";
            default: return "UNKNOWN";
        }
    };

    os << "Parameters:" << "\n"
       << "  instPath: " << param.instPath << "\n"
       << "  maxMilli: " << param.maxMilli << "\n"
       << "  seed: " << param.seed << "\n"
       << "  initialSolution: " << initialSolutionToStr(param.initialSolution) << "\n"
       << "  dispatchingRule: " << dispatchingRuleToStr(param.dispatchingRule) << "\n"
       << "  sched: " << schedulerToStr(param.sched) << "\n"
       << "  autoConfig: " << (param.autoConfig ? "true" : "false") << "\n"
       << "  currentNHood: " << param.currentNHood << "\n"
       << "  currentSearchMethod: " << param.currentSearchMethod << "\n";

    os << "  searchMethods: [";
    for (size_t i = 0; i < param.searchMethods.size(); ++i) {
        os << searchMethodToStr(param.searchMethods[i]) << (i == param.searchMethods.size() - 1 ? "" : ", ");
    }
    os << "]\n";

    os << "  nHoods: [";
    for (size_t i = 0; i < param.nHoods.size(); ++i) {
        os << neighborhoodToStr(param.nHoods[i]) << (i == param.nHoods.size() - 1 ? "" : ", ");
    }
    os << "]\n";

    os << "  nHoodsTraversings: [";
    for (size_t i = 0; i < param.nHoodsTraversings.size(); ++i) {
        os << nHoodTraversingToStr(param.nHoodsTraversings[i]) << (i == param.nHoodsTraversings.size() - 1 ? "" : ", ");
    }
    os << "]\n";

    os << "  Tabu Params:\n"
       << "    maxD: " << param.maxD << "\n"
       << "    maxC: " << param.maxC << "\n"
       << "    tenure: " << param.tenure << "\n"
       << "    jumpListSize: " << param.jumpListSize << "\n"
       << "    initialJumpLimit: " << param.initialJumpLimit << "\n"
       << "    decreaseDivisor: " << param.decreaseDivisor << "\n";

    os << "  ILS Params:\n"
       << "    perturbationStrength: " << param.perturbationStrength << "\n";

    return os;
}
