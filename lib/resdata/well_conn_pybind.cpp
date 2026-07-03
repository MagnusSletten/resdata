#include <string>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <fmt/format.h>

#include <resdata/well/well_conn.hpp>
#include <detail/resdata/cwrap_pybind.hpp>

namespace py = pybind11;

namespace {
PYBIND11_MODULE(well_connection, m) {
    register_exceptions(m);
    py::enum_<WellConnDir>(m, "WellConnectionDirection")
        .value("well_conn_dirX", WellConnDir::X)
        .value("well_conn_dirY", WellConnDir::Y)
        .value("well_conn_dirZ", WellConnDir::Z)
        .value("well_conn_fracX", WellConnDir::fracX)
        .value("well_conn_fracY", WellConnDir::fracY)
        .export_values();

    py::class_<WellConnection, std::shared_ptr<WellConnection>>(
        m, "WellConnection")
        .def(py::init([]() -> std::shared_ptr<WellConnection> {
            py::set_error(PyExc_NotImplementedError,
                          "Class can not be instantiated directly");
            throw py::error_already_set();
        }))
        .def("isOpen", &WellConnection::is_open)
        .def("ijk",
             [](WellConnection &self) -> std::tuple<int, int, int> {
                 return {self.get_i(), self.get_j(), self.get_k()};
             })
        .def("direction", &WellConnection::get_dir)
        .def("segmentId", &WellConnection::get_segment_id)
        .def("isFractureConnection", &WellConnection::is_fracture_connection)
        .def("isMatrixConnection", &WellConnection::is_matrix_connection)
        .def("connectionFactor", &WellConnection::get_connection_factor)
        .def("__eq__", &WellConnection::operator==)
        .def("isMultiSegmentWell", &WellConnection::is_MSW)
        .def("__repr__",
             [](py::object py_self) {
                 auto &self = py_self.cast<WellConnection &>();
                 std::string ijk = py::str(py_self.attr("ijk")());
                 std::string frac =
                     self.is_fracture_connection() ? "fracture" : "";
                 std::string open = self.is_open() ? "open" : "shut";
                 std::string msw = self.is_MSW() ? "(multi segment)" : "";
                 std::string direction = py::str(py_self.attr("direction")());
                 return fmt::format("WellConnection({} {} {} {}, "
                                    "rates = (O:{},G:{},W:{}, direction = {}",
                                    ijk, frac, open, msw, self.get_oil_rate(),
                                    self.get_gas_rate(), self.get_water_rate(),
                                    direction);
             })
        .def("gasRate", &WellConnection::get_gas_rate,
             "The gas rate, as stored in the restart file.\n"
             "\n"
             "The physical unit depends on the file's unit system: sm3/day "
             "(metric),\n"
             "Mscf/day (field) or cm3/hour (lab). Use :meth:`gasRateSI` to get "
             "the\n"
             "value converted to SI units.\n")
        .def("waterRate", &WellConnection::get_water_rate,
             "The water rate, as stored in the restart file.\n"
             "\n"
             "The physical unit depends on the file's unit system: sm3/day "
             "(metric),\n"
             "stb/day (field) or cm3/hour (lab). Use :meth:`waterRateSI` to "
             "get the\n"
             "value converted to SI units.\n")
        .def("oilRate", &WellConnection::get_oil_rate,
             "The oil rate, as stored in the restart file.\n"
             "\n"
             "The physical unit depends on the file's unit system: sm3/day "
             "(metric),\n"
             "stb/day (field) or cm3/hour (lab). Use :meth:`oilRateSI` to get "
             "the\n"
             "value converted to SI units.\n")
        .def("volumeRate", &WellConnection::get_volume_rate,
             "The volume rate, at reservoir conditions, as stored in the "
             "restart file.\n"
             "\n"
             "The physical unit depends on the file's unit system: sm3/day "
             "(metric),\n"
             "stb/day (field) or cm3/hour (lab). Use :meth:`oilRateSI` to get "
             "the\n")
        .def("gasRateSI", &WellConnection::get_gas_rate_si,
             "The gas rate converted to SI units (m3/s).\n"
             "\n"
             "This is the raw :meth:`gasRate` multiplied by a unit-system "
             "dependent\n"
             "conversion factor.\n")
        .def("waterRateSI", &WellConnection::get_water_rate_si,
             "The water rate converted to SI units (m3/s).\n"
             "\n"
             "This is the raw :meth:`waterRate` multiplied by a unit-system "
             "dependent\n"
             "conversion factor.\n")
        .def("oilRateSI", &WellConnection::get_oil_rate_si,
             "The oil rate converted to SI units (m3/s).\n"
             "\n"
             "This is the raw :meth:`oilRate` multiplied by a unit-system "
             "dependent\n"
             "conversion factor.\n")
        .def("volumeRateSI", &WellConnection::get_volume_rate_si,
             "The volume rate, at reservoir conditions, converted to SI units "
             "(m3/s).\n"
             "\n"
             "This is the raw :meth:`volumeRate` multiplied by a unit-system "
             "dependent\n"
             "conversion factor.\n");
}
} // namespace
