#pragma once

#include <resdata/well/well_state.hpp>

class WellTimeLine {
    std::string well_name;
    std::vector<std::shared_ptr<WellState>> ts;

public:
    explicit WellTimeLine(std::string well_name) : well_name(well_name) {};

    void add_well(std::shared_ptr<WellState> &well_state) {
        ts.push_back(well_state);
    }
    std::string name() { return well_name; }
    std::shared_ptr<WellState> at(size_t n) { return ts.at(n); }
    size_t size() { return ts.size(); }
};
