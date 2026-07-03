#include <cstdint>

#include <algorithm>
#include <iterator>
#include <string>
#include <vector>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <fmt/format.h>

#include <resdata/well/well_segment_collection.hpp>
#include <resdata/well/well_conn.hpp>
#include <resdata/well/well_segment.hpp>
#include <resdata/well/well_state.hpp>
#include <detail/resdata/cwrap_pybind.hpp>

namespace py = pybind11;

namespace {
PYBIND11_MODULE(well_state, m) {
    register_exceptions(m);
    py::class_<WellState, std::shared_ptr<WellState>>(m, "WellState")
        .def(py::init([]() -> std::shared_ptr<WellState> {
            py::set_error(PyExc_NotImplementedError,
                          "Class can not be instantiated directly");
            throw py::error_already_set();
        }))
        .def("name", &WellState::get_name)
        .def("isOpen", &WellState::is_open)
        .def("wellHead",
             [](py::object py_self) -> py::object {
                 auto &self = py_self.cast<WellState &>();
                 const well_conn_type *wellhead = self.get_global_wellhead();
                 if (!wellhead)
                     return py::none();
                 return WellConnection().attr("createCReference")(
                     reinterpret_cast<std::uintptr_t>(wellhead), self);
             })
        .def("wellNumber", &WellState::get_well_nr)
        .def("reportNumber", &WellState::get_report_nr)
        .def("simulationTime",
             [](WellState &self) -> py::object {
                 return CTime()(self.get_sim_time());
             })
        .def("wellType",
             [](WellState &self) -> py::object {
                 return WellType()(static_cast<int>(self.get_type()));
             })
        .def("hasGlobalConnections", &WellState::has_global_connections)
        .def(
            "globalConnections",
            [](py::object py_self) {
                auto &self = py_self.cast<WellState &>();
                auto well_conns = self.get_global_connections();
                std::vector<py::object> result;
                if (!well_conns)
                    return result;
                std::transform(
                    well_conns->begin(), well_conns->end(),
                    std::back_inserter(result), [py_self](auto &p) {
                        return WellConnection().attr("createCReference")(
                            reinterpret_cast<std::uintptr_t>(p.get()), py_self);
                    });
                return result;
            },
            "The list of well connections for the global grid.\n"
            "\n"
            "Note: Constructs a new list of references to the well "
            "connections.\n")
        .def("__len__", &WellState::num_segments)
        .def("numSegments", &WellState::num_segments)
        .def(
            "segments",
            [](py::object py_self) {
                auto &self = py_self.cast<WellState &>();
                auto *segments = self.get_segments();
                int size = well_segment_collection_get_size(segments);
                std::vector<py::object> result;
                result.reserve(size);
                for (int i = 0; i < size; i++) {
                    well_segment_type *segment =
                        well_segment_collection_iget(segments, i);
                    result.push_back(WellSegment().attr("createCReference")(
                        reinterpret_cast<std::uintptr_t>(segment), self));
                }
                return result;
            },
            "The list of segments in the well.\n"
            "\n"
            "Note: Constructs a new list of references to the well "
            "segments.\n")
        .def(
            "__getitem__",
            [](py::object py_self, py::int_ index) {
                auto &self = py_self.cast<WellState &>();
                auto *segments = self.get_segments();
                int size = well_segment_collection_get_size(segments);
                if (index < py::int_(0))
                    index += py::int_(size);
                if (!((py::int_(0) <= index) && (index < py::int_(size))))
                    throw py::index_error(
                        fmt::format("Invalid index:{} - valid range [0,{})",
                                    index.cast<long>(), size));
                return WellSegment().attr("createCReference")(
                    reinterpret_cast<std::uintptr_t>(
                        well_segment_collection_iget(segments, index)),
                    self);
            },
            py::arg("idx"))
        .def(
            "igetSegment",
            [](py::handle self, py::int_ seg_idx) { return self[seg_idx]; },
            py::arg("seg_idx"))
        .def("isMultiSegmentWell", &WellState::is_MSW)
        .def("hasSegmentData", &WellState::has_segment_data)
        .def("__repr__",
             [](py::object py_self) {
                 auto &self = py_self.cast<WellState &>();
                 std::string msw = self.is_MSW() ? "(multi segment)" : "";
                 std::string open = self.is_open() ? "open" : "shut";
                 std::string type = py::str(py_self.attr("wellType")());
                 return fmt::format("WellState({} {}, "
                                    "number = {}, type = \"{}\", state = {})",
                                    self.get_name(), msw, self.get_well_nr(),
                                    type, open);
             })
        .def("gasRate", &WellState::get_gas_rate,
             "The gas rate, as stored in the restart file.\n"
             "\n"
             "The physical unit depends on the file's unit system: sm3/day "
             "(metric),\n"
             "Mscf/day (field) or cm3/hour (lab). Use :meth:`gasRateSI` to get "
             "the\n"
             "value converted to SI units.\n")
        .def("waterRate", &WellState::get_water_rate,
             "The water rate, as stored in the restart file.\n"
             "\n"
             "The physical unit depends on the file's unit system: sm3/day "
             "(metric),\n"
             "stb/day (field) or cm3/hour (lab). Use :meth:`waterRateSI` to "
             "get the\n"
             "value converted to SI units.\n")
        .def("oilRate", &WellState::get_oil_rate,
             "The oil rate, as stored in the restart file.\n"
             "\n"
             "The physical unit depends on the file's unit system: sm3/day "
             "(metric),\n"
             "stb/day (field) or cm3/hour (lab). Use :meth:`oilRateSI` to get "
             "the\n"
             "value converted to SI units.\n")
        .def("volumeRate", &WellState::get_volume_rate,
             "The volume rate, at reservoir conditions, as stored in the "
             "restart file.\n"
             "\n"
             "The physical unit depends on the file's unit system: sm3/day "
             "(metric),\n"
             "stb/day (field) or cm3/hour (lab). Use :meth:`oilRateSI` to get "
             "the\n")
        .def("gasRateSI", &WellState::get_gas_rate_si,
             "The gas rate converted to SI units (m3/s).\n"
             "\n"
             "This is the raw :meth:`gasRate` multiplied by a unit-system "
             "dependent\n"
             "conversion factor.\n")
        .def("waterRateSI", &WellState::get_water_rate_si,
             "The water rate converted to SI units (m3/s).\n"
             "\n"
             "This is the raw :meth:`waterRate` multiplied by a unit-system "
             "dependent\n"
             "conversion factor.\n")
        .def("oilRateSI", &WellState::get_oil_rate_si,
             "The oil rate converted to SI units (m3/s).\n"
             "\n"
             "This is the raw :meth:`oilRate` multiplied by a unit-system "
             "dependent\n"
             "conversion factor.\n")
        .def("volumeRateSI", &WellState::get_volume_rate_si,
             "The volume rate, at reservoir conditions, converted to SI units "
             "(m3/s).\n"
             "\n"
             "This is the raw :meth:`volumeRate` multiplied by a unit-system "
             "dependent\n"
             "conversion factor.\n");
}
} // namespace
