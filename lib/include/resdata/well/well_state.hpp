#pragma once
#include <ctime>

#include <vector>
#include <string>
#include <memory>
#include <map>

#include <resdata/rd_file.hpp>
#include <resdata/rd_grid.hpp>

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
   The well_state_type structure contains state information about one
   well for one particular point in time.
*/
struct well_state_struct {
    UTIL_TYPE_ID_DECLARATION;
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
};

typedef struct well_state_struct well_state_type;

well_state_type *well_state_alloc(const char *well_name, int global_well_nr,
                                  bool open, well_type_enum type, int report_nr,
                                  time_t valid_from);
well_state_type *well_state_alloc_from_file(rd_file_type *rd_file,
                                            const rd_grid_type *grid,
                                            int report_step, int well_nr,
                                            bool load_segment_information);
well_state_type *well_state_alloc_from_file2(rd_file_view_type *file_view,
                                             const rd_grid_type *grid,
                                             int report_nr, int global_well_nr,
                                             bool load_segment_information);

void well_state_add_connections2(well_state_type *well_state,
                                 const rd_grid_type *grid,
                                 rd_file_view_type *rst_view, int well_nr);

bool well_state_add_MSW2(well_state_type *well_state,
                         rd_file_view_type *rst_view, int well_nr,
                         bool load_segment_information);

bool well_state_is_MSW(const well_state_type *well_state);

bool well_state_has_segment_data(const well_state_type *well_state);

well_segment_collection_type *
well_state_get_segments(const well_state_type *well_state);
well_branch_collection_type *
well_state_get_branches(const well_state_type *well_state);

void well_state_free(well_state_type *well);
const char *well_state_get_name(const well_state_type *well);
int well_state_get_report_nr(const well_state_type *well_state);
time_t well_state_get_sim_time(const well_state_type *well_state);
well_type_enum well_state_get_type(const well_state_type *well_state);
bool well_state_is_open(const well_state_type *well_state);
int well_state_get_well_nr(const well_state_type *well_state);

const well_conn_type *
well_state_get_global_wellhead(const well_state_type *well_state);
well_type_enum well_state_translate_rd_type_int(int int_type);

bool well_state_has_grid_connections(const well_state_type *well_state,
                                     const char *grid_name);
bool well_state_has_global_connections(const well_state_type *well_state);

double well_state_get_oil_rate(const well_state_type *well_state);
double well_state_get_gas_rate(const well_state_type *well_state);
double well_state_get_water_rate(const well_state_type *well_state);
double well_state_get_volume_rate(const well_state_type *well_state);
double well_state_get_water_rate_si(const well_state_type *well_state);
double well_state_get_oil_rate_si(const well_state_type *well_state);
double well_state_get_volume_rate_si(const well_state_type *well_state);
double well_state_get_gas_rate_si(const well_state_type *well_state);

UTIL_IS_INSTANCE_HEADER(well_state);

const std::vector<well_conn_ptr> *
well_state_get_grid_connections(const well_state_type *well_state,
                                const std::string &grid_name);
const std::vector<well_conn_ptr> *
well_state_get_global_connections(const well_state_type *well_state);

using well_state_ptr = std::unique_ptr<well_state_type>;
