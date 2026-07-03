#pragma once

#include <ctime>

#include <map>
#include <vector>
#include <string>
#include <memory>

#include <resdata/rd_file.hpp>
#include <resdata/rd_grid.hpp>
#include <resdata/rd_units.hpp>

#include <resdata/well/well_conn.hpp>
#include <resdata/well/well_const.hpp>
#include <resdata/well/well_segment_collection.hpp>
#include <resdata/well/well_branch_collection.hpp>

#define GLOBAL_GRID_NAME                                                       \
    "GLOBAL" // The name assigned to the global grid for name based lookup.

/*

Connections, segments and branches
----------------------------------


   +-----+
   |     |  <- Wellhead
   |     |
   +-----+ _________ Segment 2
      |\  /
      | \/         Segment 1               Segment 0
      |  \-----0---------------0--<----------------------O           <-- Branch: 0
      \        |               |      |                  |
       \    +-----+         +-----++-----+            +-----+
        \   | C3  |         | C2  || C1  |            | C0  |
         \  |     |         |     ||     |            |     |
          \ +-----+         +-----++-----+            +-----+
           \
Segment 5   \
             \
              \        Segment 4                Segment 3
               \-<--O-------<-------O----------------<------------O   <-- Branch: 1
                    |               |          |                  |
                 +-----+         +-----+    +-----+            +-----+
                 | C7  |         | C6  |    | C5  |            | C4  |
                 |     |         |     |    |     |            |     |
                 +-----+         +-----+    +-----+            +-----+




The boxes show connections C0 - C7; the connections serve as sinks (or
sources in the case of injectors) removing fluids from the
reservoir. As indicated by the use of isolated boxes the model
contains no geomtric concept linking the different connections into a
connected 'well-like' object.

Ordinary wells in the model are just a collection of
connections like these illustrated boxes, and to draw a connected 1D
object heuristics of some kind must be used to determine how the
various connections should be connected. In particular for wells which
consist of multiple branches this heuristic is non obvious.

More advanced (i.e. newer) wells are modelles as multisegment wells;
the important thing about multisegment wells is that the 1D segments
constituting the flowpipe are modelled explicitly as 'segments', and
the equations of fluid flow are solved by ECLIPSE in these 1D
domains. The figure above shows a multisegment well with six segments
marked as Segment0 ... Segment5. The segments themselves are quite
abstract objects not directly linked to the grid, but indriectly
through the connections. In the figure the segment <-> connection
links are as follows:

  Segment0: C0, C1
  Segment1: C2
  Segment2: C3
  Segment3: C4, C5
  Segment4: C6
  Segment5: C7

Each segment has an outlet segment, which will link the segments
together into a geometry.

The connection can in general be both to the main global grid, and to
an LGR. Hence all questions about connections must be LGR aware. This
is in contrast to the segments and branches which are geometric
objects, not directly coupled to a specific grid; however the segments
have a collection of connections - and these connections are coupled to
a grid.
*/

/**
   The WellState structure contains state information about one
   well for one particular point in time.
*/
class WellState {
    std::string name;
    time_t valid_from_time;
    int valid_from_report;
    int global_well_nr;
    bool open;
    well_type_enum type;
    bool is_MSW_well;
    double oil_rate;
    double gas_rate;
    double water_rate;
    double volume_rate;
    ert_rd_unit_enum unit_system;

    std::map<std::string, std::vector<well_conn_ptr>> connections;
    well_segment_collection_ptr segments{nullptr, well_segment_collection_free};
    well_branch_collection_ptr branches{nullptr, well_branch_collection_free};

    // The index_wellhead will own the reference to the well_conn
    // and the name_wellhead has a non-owning reference
    std::vector<well_conn_ptr>
        index_wellhead; // An well_conn_type instance representing the wellhead - indexed by grid_nr.
    std::map<std::string, well_conn_type *>
        name_wellhead; // An well_conn_type instance representing the wellhead - indexed by lgr_name.

    void add_wellhead(const RSTHead &header, const rd_kw_type *iwel_kw,
                      int well_nr, const std::string &grid_name, int grid_nr);
    bool add_rates(rd_file_view_type *rst_view, int well_nr);
    int get_lgr_well_nr(const rd_file_view_type *file_view);
    void add_connections(const rd_file_view_type *rst_view,
                         const std::string &grid_name, int grid_nr,
                         int well_nr);
    void add_global_connections(const rd_file_view_type *rst_view, int well_nr);
    void add_LGR_connections(const rd_grid_type *grid,
                             rd_file_view_type *file_view);
    well_conn_type *get_wellhead(const std::string &grid_name) {
        const auto it = name_wellhead.find(grid_name);
        return it != name_wellhead.end() ? it->second : nullptr;
    }

public:
    WellState(std::string well_name, int global_well_nr, bool open,
              well_type_enum type, int report_nr, time_t valid_from);
    double get_oil_rate() { return oil_rate; }
    double get_gas_rate() { return gas_rate; }
    double get_water_rate() { return water_rate; }
    double get_volume_rate() { return volume_rate; }
    int get_report_nr() { return valid_from_report; }
    bool is_MSW() { return is_MSW_well; }
    well_type_enum get_type() { return type; }
    int get_well_nr() { return global_well_nr; }
    std::string get_name() { return name; }
    bool is_open() { return open; }
    time_t get_sim_time() { return valid_from_time; }
    double get_oil_rate_si() {
        return oil_rate * liquid_conversion_factor(unit_system);
    }
    double get_gas_rate_si() {
        return gas_rate * gas_conversion_factor(unit_system);
    }
    double get_water_rate_si() {
        return water_rate * liquid_conversion_factor(unit_system);
    }
    double get_volume_rate_si() {
        return volume_rate * liquid_conversion_factor(unit_system);
    }
    bool has_segment_data() { return num_segments() > 0; }

    int num_segments() {
        return well_segment_collection_get_size(segments.get());
    }

    well_segment_collection_type *get_segments() { return segments.get(); }

    well_branch_collection_type *get_branches() { return branches.get(); }

    well_conn_type *get_global_wellhead() {
        return get_wellhead(RD_GRID_GLOBAL_GRID);
    }

    bool has_grid_connections(const std::string &grid_name) {
        return (connections.find(grid_name) != connections.end());
    }
    bool has_global_connections() {
        return has_grid_connections(RD_GRID_GLOBAL_GRID);
    }
    void add_connections(const rd_grid_type *grid, rd_file_view_type *rst_view,
                         int well_nr);
    bool add_MSW(rd_file_view_type *rst_view, int well_nr,
                 bool load_segment_information);

    static std::shared_ptr<WellState>
    read_wells_in_restart(rd_file_type *rd_file, const rd_grid_type *grid,
                          int report_nr, int global_well_nr,
                          bool load_segment_information);
    static std::shared_ptr<WellState>
    read_wells_in_restart(rd_file_view_type *file_view,
                          const rd_grid_type *grid, int report_nr,
                          int global_well_nr, bool load_segment_information);
    std::vector<well_conn_ptr> *
    get_grid_connections(const std::string &grid_name) {
        auto it = connections.find(grid_name);
        return it != connections.end() ? &it->second : nullptr;
    }
    const std::vector<well_conn_ptr> *get_global_connections() {
        return get_grid_connections(RD_GRID_GLOBAL_GRID);
    }
};

well_type_enum well_state_translate_rd_type_int(int int_type);
