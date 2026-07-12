#pragma once

#include <filesystem>
#include <iomanip>
#include <sstream>

namespace vtu {
namespace fs = std::filesystem;

enum class Format { ASCII,
                    Binary,
                    Unknown };

enum class TopologyType : uint8_t {
  BezierCurve         = 75,
  BezierQuadrilateral = 77,
  BezierHexahedron    = 79,
  Unknown             = 0
};

enum class AttributeType : uint8_t {
  Scalar  = 1,
  Vector  = 3,
  Tensor6 = 6,
  Tensor9 = 9,
  Matrix,
  Unknown = 0
};

enum class AttributeCenter { Node,
                             Cell,
                             Unknown };

class Writer {
  public:
  Writer(const fs::path& filename, Format format)
      : m_filename(filename), m_format(format), m_num_points(0),
        m_num_cells(0) {}

  void set_geometry(const double* pos, size_t num_points) {
    m_num_points = num_points;
    m_points_ss << "\t\t<Points>\n";
    m_points_ss << "\t\t\t<DataArray type=\"Float64\" NumberOfComponents=\"3\" format=\"ascii\">\n";
    for (size_t i = 0; i < num_points; i++) {
      m_points_ss << "\t\t\t\t" << pos[i * 3 + 0]
                  << " " << pos[i * 3 + 1]
                  << " " << pos[i * 3 + 2] << "\n";
    }

    m_points_ss << "\t\t\t</DataArray>\n";
    m_points_ss << "\t\t</Points>\n";
  };

  void set_topology(size_t num_cells, TopologyType type, int p, int q) {};

  void add_attribute(const double* data, size_t num_items, AttributeType type,
                     AttributeCenter center, const std::string& name) {};

  void write() {};

  private:
  fs::path m_filename;
  Format m_format;
  size_t m_num_points;
  size_t m_num_cells;

  std::stringstream m_points_ss;
  std::stringstream m_cells_ss;
  std::stringstream m_point_data_ss;
  std::stringstream m_cell_data_ss;
};
} // namespace vtu
