#include <cstdint>
#include <string>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <fmt/format.h>

#include <resdata/well/well_ts.hpp>
#include <detail/resdata/cwrap_pybind.hpp>

namespace py = pybind11;

namespace {
PYBIND11_MODULE(well_time_line, m) {
    register_exceptions(m);
    py::class_<WellTimeLine, std::shared_ptr<WellTimeLine>>(m, "WellTimeLine")
        .def(py::init<std::string>(), py::arg("name"))
        .def("getName", &WellTimeLine::name)
        .def("__len__", &WellTimeLine::size)
        .def(
            "__getitem__",
            [](py::object py_self, py::int_ index) {
                auto &self = py_self.cast<WellTimeLine &>();
                if (index < py::int_(0))
                    index += py::int_(self.size());
                if (!((py::int_(0) <= index) &&
                      (index < py::int_(self.size()))))
                    throw py::index_error(
                        fmt::format("Index must be in range 0 <= {} < {}",
                                    index.cast<long>(), self.size()));
                return self.at(index.cast<size_t>());
            },
            py::arg("index"))
        .def("__repr__", [](WellTimeLine &self) {
            return fmt::format("WellTimeLine(name = {}, size = {})",
                               self.name(), self.size());
        });
}
} // namespace
