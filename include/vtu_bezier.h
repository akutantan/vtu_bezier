#pragma once

#include <cstddef>
#include <cstdlib>
#include <filesystem>
#include <iomanip>
#include <ios>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

namespace vtu {
namespace fs = std::filesystem;

enum class Format { ASCII,
                    Binary,
                    Appended,
                    Unknown };

inline std::string to_string(Format format) {
  if (format == Format::ASCII) {
    return "ascii";
  }
  if (format == Format::Binary) {
    return "binary";
  }
  if (format == Format::Appended) {
    return "appended";
  }
  return "unknown";
}

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
  Matrix  = 9,
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
    m_points_ss << "\t\t\t<DataArray type=\"Float64\" Name=\"Points\" NumberOfComponents=\"3\" format=\""
                << to_string(m_format) << "\">\n";
    if (m_format == Format::ASCII) {
      for (size_t i = 0; i < num_points; i++) {
        m_points_ss << "\t\t\t\t"
                    << pos[i * 3 + 0] << " "
                    << pos[i * 3 + 1] << " "
                    << pos[i * 3 + 2] << "\n";
      }
    } else if (m_format == Format::Binary) {
      std::cout << "Binary format is not supported yet." << std::endl;
    } else if (m_format == Format::Appended) {
      std::cout << "Appended format is not supported yet." << std::endl;
    }
    m_points_ss << "\t\t\t</DataArray>\n";
    m_points_ss << "\t\t</Points>\n";
  };

  void set_topology(size_t num_cells, TopologyType type, const int* p) {
    m_num_cells = num_cells;
    int npd;
    if (type == TopologyType::BezierCurve) {
      npd = 1;
    } else if (type == TopologyType::BezierQuadrilateral) {
      npd = 2;
    } else if (type == TopologyType::BezierHexahedron) {
      npd = 3;
    } else {
      std::cerr << "Unknown topology type." << std::endl;
      return;
    }
    int neb = 1;
    for (int i = 0; i < npd; i++) {
      neb *= (p[i] + 1);
    }
    std::vector<int> map = get_vtk_cell_map(type, p);

    m_cells_ss << "\t\t<Cells>\n";

    m_cells_ss << "\t\t\t<DataArray type=\"Int64\" Name=\"connectivity\" format=\""
               << to_string(m_format) << "\">\n";
    for (size_t i = 0; i < num_cells; i++) {
      m_cells_ss << "\t\t\t\t";
      size_t offset = i * neb;
      for (size_t j = 0; j < neb; j++) {
        m_cells_ss << (offset + map[j]) << " ";
      }
      m_cells_ss << "\n";
    }
    m_cells_ss << "\t\t\t</DataArray>\n";

    m_cells_ss << "\t\t\t<DataArray type=\"Int64\" Name=\"offsets\" format=\""
               << to_string(m_format) << "\">\n";
    for (size_t i = 0; i < num_cells; i++) {
      m_cells_ss << "\t\t\t\t" << (i + 1) * neb << "\n";
    }
    m_cells_ss << "\t\t\t</DataArray>\n";

    m_cells_ss << "\t\t\t<DataArray type=\"UInt8\" Name=\"types\" format=\""
               << to_string(m_format) << "\">\n";
    for (size_t i = 0; i < num_cells; i++) {
      m_cells_ss << "\t\t\t\t" << static_cast<int>(type) << "\n";
    }
    m_cells_ss << "\t\t\t</DataArray>\n";
    m_cells_ss << "\t\t</Cells>\n";

    std::vector<int> degree(3, 0);
    degree[0] = (npd > 0) ? p[0] : 0;
    degree[1] = (npd > 1) ? p[1] : 0;
    degree[2] = (npd > 2) ? p[2] : 0;

    m_cell_data_ss << "\t\t\t<DataArray type=\"Int32\" Name=\"HigherOrderDegrees\" NumberOfComponents=\"3\" format=\""
                   << to_string(m_format) << "\">\n";
    for (size_t i = 0; i < num_cells; i++) {
      m_cell_data_ss << "\t\t\t\t"
                     << degree[0] << " "
                     << degree[1] << " "
                     << degree[2] << "\n";
    }
    m_cell_data_ss << "\t\t\t</DataArray>\n";
  };

  void add_attribute(const double* data, size_t num_items,
                     AttributeType type, AttributeCenter center, const std::string& name) {
    std::stringstream* target_ss = nullptr;
    if (center == AttributeCenter::Node) {
      target_ss = &m_point_data_ss;
    } else if (center == AttributeCenter::Cell) {
      target_ss = &m_cell_data_ss;
    } else {
      std::cerr << "Unknown attribute center." << std::endl;
      exit(EXIT_FAILURE);
    }

    int num_comps = static_cast<int>(type);
    if (type == AttributeType::Matrix) {
      num_comps = 9;
    } else if (num_comps == 0) {
      std::cerr << "Unknown attribute type." << std::endl;
      exit(EXIT_FAILURE);
    }

    *target_ss << "\t\t\t<DataArray type=\"Float64\" Name=\"" << name
               << "\" NumberOfComponents=\"" << num_comps
               << "\" format=\"" << to_string(m_format) << "\">\n";

    if (m_format == Format::ASCII) {
      *target_ss << std::scientific << std::setprecision(15);
      for (size_t i = 0; i < num_items; i++) {
        *target_ss << "\t\t\t\t";
        for (int j = 0; j < num_comps; j++) {
          *target_ss << data[i * num_comps + j] << " ";
        }
        *target_ss << "\n";
      }
    } else if (m_format == Format::Binary) {
      std::cout << "Binary format is not supported yet." << std::endl;
    } else if (m_format == Format::Appended) {
      std::cout << "Appended format is not supported yet." << std::endl;
    }
    *target_ss << "\t\t\t</DataArray>\n";
  };

  void write() {
    std::ofstream ofs(m_filename);
    if (!ofs.is_open()) {
      std::cerr << "Failed to open file: " << m_filename << std::endl;
      exit(EXIT_FAILURE);
    }
    ofs << "<?xml version=\"1.0\"?>\n";
    ofs << "<VTKFile type=\"UnstructuredGrid\" version=\"1.0\" byte_order=\"LittleEndian\">\n";
    ofs << "\t<UnstructuredGrid>\n";
    ofs << "\t\t<Piece NumberOfPoints=\"" << m_num_points
        << "\" NumberOfCells=\"" << m_num_cells << "\">\n";
    ofs << m_points_ss.str();
    ofs << m_cells_ss.str();
    ofs << "\t\t<PointData>\n";
    ofs << m_point_data_ss.str();
    ofs << "\t\t</PointData>\n";
    ofs << "\t\t<CellData>\n";
    ofs << m_cell_data_ss.str();
    ofs << "\t\t</CellData>\n";
    ofs << "\t\t</Piece>\n";
    ofs << "\t</UnstructuredGrid>\n";
    ofs << "</VTKFile>\n";
    ofs.close();
  };

  private:
  fs::path m_filename;
  Format m_format;
  size_t m_num_points;
  size_t m_num_cells;

  std::stringstream m_points_ss;
  std::stringstream m_cells_ss;
  std::stringstream m_point_data_ss;
  std::stringstream m_cell_data_ss;

  std::vector<int> get_vtk_cell_map(TopologyType type, const int* p) {
    std::vector<int> map;
    if (type == TopologyType::BezierCurve) {
      int p1 = p[0];
      map.resize(p1 + 1);
      map[0] = 0;
      map[1] = p1;

      int vtk_idx = 2;
      for (int i = 1; i < p1; i++) {
        map[vtk_idx++] = i;
      }
    } else if (type == TopologyType::BezierQuadrilateral) {
      int p1 = p[0];
      int p2 = p[1];
      map.resize((p1 + 1) * (p2 + 1));

      auto get_idx = [&](int i, int j) { return i + j * (p1 + 1); };

      int vtk_idx    = 0;
      map[vtk_idx++] = get_idx(0, 0);
      map[vtk_idx++] = get_idx(p1, 0);
      map[vtk_idx++] = get_idx(p1, p2);
      map[vtk_idx++] = get_idx(0, p2);

      for (int i = 1; i < p1; i++) {
        map[vtk_idx++] = get_idx(i, 0);
      }
      for (int j = 1; j < p2; j++) {
        map[vtk_idx++] = get_idx(p1, j);
      }
      for (int i = 1; i < p1; i++) {
        map[vtk_idx++] = get_idx(i, p2);
      }
      for (int j = 1; j < p2; j++) {
        map[vtk_idx++] = get_idx(0, j);
      }
      for (int j = 1; j < p2; j++) {
        for (int i = 1; i < p1; i++) {
          map[vtk_idx++] = get_idx(i, j);
        }
      }
    } else if (type == TopologyType::BezierHexahedron) {
      int p1 = p[0];
      int p2 = p[1];
      int p3 = p[2];
      map.resize((p1 + 1) * (p2 + 1) * (p3 + 1));

      auto get_idx = [&](int i, int j, int k) {
        return i + j * (p1 + 1) + k * (p1 + 1) * (p2 + 1);
      };

      int vtk_idx = 0;

      map[vtk_idx++] = get_idx(0, 0, 0);
      map[vtk_idx++] = get_idx(p1, 0, 0);
      map[vtk_idx++] = get_idx(p1, p2, 0);
      map[vtk_idx++] = get_idx(0, p2, 0);
      map[vtk_idx++] = get_idx(0, 0, p3);
      map[vtk_idx++] = get_idx(p1, 0, p3);
      map[vtk_idx++] = get_idx(p1, p2, p3);
      map[vtk_idx++] = get_idx(0, p2, p3);

      for (int i = 1; i < p1; i++) {
        map[vtk_idx++] = get_idx(i, 0, 0);
      }
      for (int j = 1; j < p2; j++) {
        map[vtk_idx++] = get_idx(p1, j, 0);
      }
      for (int i = 1; i < p1; i++) {
        map[vtk_idx++] = get_idx(i, p2, 0);
      }
      for (int j = 1; j < p2; j++) {
        map[vtk_idx++] = get_idx(0, j, 0);
      }
      for (int i = 1; i < p1; i++) {
        map[vtk_idx++] = get_idx(i, 0, p3);
      }
      for (int j = 1; j < p2; j++) {
        map[vtk_idx++] = get_idx(p1, j, p3);
      }
      for (int i = 1; i < p1; i++) {
        map[vtk_idx++] = get_idx(i, p2, p3);
      }
      for (int j = 1; j < p2; j++) {
        map[vtk_idx++] = get_idx(0, j, p3);
      }
      for (int k = 1; k < p3; k++) {
        map[vtk_idx++] = get_idx(0, 0, k);
      }
      for (int k = 1; k < p3; k++) {
        map[vtk_idx++] = get_idx(p1, 0, k);
      }
      for (int k = 1; k < p3; k++) {
        map[vtk_idx++] = get_idx(p1, p2, k);
      }
      for (int k = 1; k < p3; k++) {
        map[vtk_idx++] = get_idx(0, p2, k);
      }

      for (int k = 1; k < p3; k++) {
        for (int i = 1; i < p1; i++) {
          map[vtk_idx++] = get_idx(i, 0, k);
        }
      }
      for (int k = 1; k < p3; k++) {
        for (int j = 1; j < p2; j++) {
          map[vtk_idx++] = get_idx(p1, j, k);
        }
      }
      for (int k = 1; k < p3; k++) {
        for (int i = 1; i < p1; i++) {
          map[vtk_idx++] = get_idx(i, p2, k);
        }
      }
      for (int k = 1; k < p3; k++) {
        for (int j = 1; j < p2; j++) {
          map[vtk_idx++] = get_idx(0, j, k);
        }
      }
      for (int j = 1; j < p2; j++) {
        for (int i = 1; i < p1; i++) {
          map[vtk_idx++] = get_idx(i, j, 0);
        }
      }
      for (int j = 1; j < p2; j++) {
        for (int i = 1; i < p1; i++) {
          map[vtk_idx++] = get_idx(i, j, p3);
        }
      }

      for (int k = 1; k < p3; k++) {
        for (int j = 1; j < p2; j++) {
          for (int i = 1; i < p1; i++) {
            map[vtk_idx++] = get_idx(i, j, k);
          }
        }
      }
    } else {
      std::cerr << "Unknown topology type." << std::endl;
      exit(EXIT_FAILURE);
    }
    return map;
  }
};
} // namespace vtu
