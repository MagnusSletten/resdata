#pragma once

#include <resdata/well/well_state.hpp>

class WellTimeLine {
    std::string well_name;
    std::vector<well_state_ptr> ts;

public:
    explicit WellTimeLine(std::string well_name) : well_name(well_name) {};

    void add_well(well_state_ptr &&well_state) {
        ts.push_back(std::move(well_state));
    }
    std::string name() { return well_name; }
    well_state_type *at(size_t n) { return ts.at(n).get(); }
    size_t size() { return ts.size(); }
};
