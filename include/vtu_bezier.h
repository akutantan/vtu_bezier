#pragma once

#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <ios>
#include <iostream>
#include <sstream>
#include <vector>

namespace vtu {
namespace fs = std::filesystem;

enum class Format { ASCII,
                    Appended,
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
    if (m_format == Format::ASCII) {
      m_points_ss << "\t\t\t<DataArray type=\"Float64\" Name=\"Points\" NumberOfComponents=\"3\" format=\"ascii\">\n";
      for (size_t i = 0; i < num_points; i++) {
        m_points_ss << "\t\t\t\t"
                    << pos[i * 3 + 0] << " "
                    << pos[i * 3 + 1] << " "
                    << pos[i * 3 + 2] << "\n";
      }
      m_points_ss << "\t\t\t</DataArray>\n";
    } else if (m_format == Format::Appended) {
      m_points_ss << "\t\t\t<DataArray type=\"Float64\" Name=\"Points\" NumberOfComponents=\"3\" format=\"appended\" offset=\"" << m_offset << "\"/>\n";
      std::vector<double> temp_pos(pos, pos + num_points * 3);
      append_binary_data(temp_pos);
    }
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

    std::vector<int64_t> conn_vec;
    std::vector<int64_t> offsets_vec;
    std::vector<uint8_t> types_vec;
    std::vector<int32_t> degree_vec;

    for (size_t i = 0; i < num_cells; i++) {
      size_t offset = i * neb;
      for (size_t j = 0; j < neb; j++) {
        conn_vec.push_back(offset + map[j]);
      }
      offsets_vec.push_back((i + 1) * neb);
      types_vec.push_back(static_cast<uint8_t>(type));
      degree_vec.push_back((npd > 0) ? p[0] : 0);
      degree_vec.push_back((npd > 1) ? p[1] : 0);
      degree_vec.push_back((npd > 2) ? p[2] : 0);
    }

    m_cells_ss << "\t\t<Cells>\n";
    if (m_format == Format::ASCII) {
      // connectivity
      m_cells_ss << "\t\t\t<DataArray type=\"Int64\" Name=\"connectivity\" format=\"ascii\">\n";
      for (size_t i = 0; i < num_cells; i++) {
        m_cells_ss << "\t\t\t\t";
        for (size_t j = 0; j < neb; j++) {
          m_cells_ss << conn_vec[i * neb + j] << " ";
        }
        m_cells_ss << "\n";
      }
      m_cells_ss << "\t\t\t</DataArray>\n";

      // offsets
      m_cells_ss << "\t\t\t<DataArray type=\"Int64\" Name=\"offsets\" format=\"ascii\">\n";
      for (size_t i = 0; i < num_cells; i++) {
        m_cells_ss << "\t\t\t\t" << offsets_vec[i] << "\n";
      }
      m_cells_ss << "\t\t\t</DataArray>\n";

      // types
      m_cells_ss << "\t\t\t<DataArray type=\"UInt8\" Name=\"types\" format=\"ascii\">\n";
      for (size_t i = 0; i < num_cells; i++) {
        m_cells_ss << "\t\t\t\t" << static_cast<int>(types_vec[i]) << "\n";
      }
      m_cells_ss << "\t\t\t</DataArray>\n";

      // HigherOrderDegrees (CellData)
      m_cell_data_ss << "\t\t\t<DataArray type=\"Int32\" Name=\"HigherOrderDegrees\" NumberOfComponents=\"3\" format=\"ascii\">\n";
      for (size_t i = 0; i < num_cells; i++) {
        m_cell_data_ss << "\t\t\t\t"
                       << degree_vec[i * 3 + 0] << " "
                       << degree_vec[i * 3 + 1] << " "
                       << degree_vec[i * 3 + 2] << "\n";
      }
      m_cell_data_ss << "\t\t\t</DataArray>\n";

    } else if (m_format == Format::Appended) {
      // connectivity
      m_cells_ss << "\t\t\t<DataArray type=\"Int64\" Name=\"connectivity\" format=\"appended\" offset=\"" << m_offset << "\" />\n";
      append_binary_data(conn_vec);

      // offsets
      m_cells_ss << "\t\t\t<DataArray type=\"Int64\" Name=\"offsets\" format=\"appended\" offset=\"" << m_offset << "\" />\n";
      append_binary_data(offsets_vec);

      // types
      m_cells_ss << "\t\t\t<DataArray type=\"UInt8\" Name=\"types\" format=\"appended\" offset=\"" << m_offset << "\" />\n";
      append_binary_data(types_vec);

      // HigherOrderDegrees (CellData)
      m_cell_data_ss << "\t\t\t<DataArray type=\"Int32\" Name=\"HigherOrderDegrees\" NumberOfComponents=\"3\" format=\"appended\" offset=\"" << m_offset << "\" />\n";
      append_binary_data(degree_vec);
    }

    m_cells_ss << "\t\t</Cells>\n";
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

    if (m_format == Format::ASCII) {
      *target_ss << "\t\t\t<DataArray type=\"Float64\" Name=\"" << name
                 << "\" NumberOfComponents=\"" << num_comps
                 << "\" format=\"ascii" << "\">\n";
      *target_ss << std::scientific << std::setprecision(15);
      for (size_t i = 0; i < num_items; i++) {
        *target_ss << "\t\t\t\t";
        for (int j = 0; j < num_comps; j++) {
          *target_ss << data[i * num_comps + j] << " ";
        }
        *target_ss << "\n";
      }
      *target_ss << "\t\t\t</DataArray>\n";
    } else if (m_format == Format::Appended) {
      *target_ss << "\t\t\t<DataArray type=\"Float64\" Name=\"" << name
                 << "\" NumberOfComponents=\"" << num_comps
                 << "\" format=\"appended\" offset=\"" << m_offset << "\" />\n";

      std::vector<double> temp_data(data, data + num_items * num_comps);
      append_binary_data(temp_data);
    }
  };

  void write() {
    std::ofstream ofs(m_filename, std::ios::binary);
    if (!ofs.is_open()) {
      std::cerr << "Failed to open file: " << m_filename << std::endl;
      exit(EXIT_FAILURE);
    }

    ofs << "<?xml version=\"1.0\"?>\n";
    ofs << "<VTKFile type=\"UnstructuredGrid\" version=\"1.0\" byte_order=\"LittleEndian\" header_type=\"UInt32\">\n";
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

    if (m_format == Format::Appended) {
      ofs << "\t<AppendedData encoding=\"raw\">\n";
      ofs << "_";

      ofs.write(m_appended_data.data(), m_appended_data.size());

      ofs << "\n\t</AppendedData>\n";
    }

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

  std::vector<char> m_appended_data;
  size_t m_offset = 0;

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

  template <typename T>
  void append_binary_data(const std::vector<T>& data) {
    uint32_t num_bytes   = data.size() * sizeof(T);
    const char* size_ptr = reinterpret_cast<const char*>(&num_bytes);
    m_appended_data.insert(m_appended_data.end(), size_ptr, size_ptr + sizeof(uint32_t));

    const char* data_ptr = reinterpret_cast<const char*>(data.data());
    m_appended_data.insert(m_appended_data.end(), data_ptr, data_ptr + num_bytes);

    m_offset += sizeof(uint32_t) + num_bytes;
  }
};

class PvdWriter {
  public:
  PvdWriter(const fs::path& pvd_filepath) : m_pvd_filepath(pvd_filepath) {}

  void write(double current_time, const std::string& vtu_filename) {
    std::vector<std::string> history;

    if (fs::exists(m_pvd_filepath)) {
      std::ifstream ifs(m_pvd_filepath);
      std::string line;
      while (std::getline(ifs, line)) {
        if (line.find("<DataSet") != std::string::npos) {
          auto start = line.find("timestep=\"");
          if (start != std::string::npos) {
            start += 10;
            auto end = line.find("\"", start);
            if (end != std::string::npos) {
              double t = std::stod(line.substr(start, end - start));
              if (t < current_time) {
                history.push_back(line);
              }
            }
          }
        }
      }
    }

    std::ofstream ofs(m_pvd_filepath);
    if (!ofs.is_open()) {
      return;
    }

    ofs << "<?xml version=\"1.0\"?>\n";
    ofs << "<VTKFile type=\"Collection\" version=\"0.1\" byte_order=\"LittleEndian\">\n";
    ofs << "\t<Collection>\n";

    for (const auto& past_line : history) {
      ofs << past_line << "\n";
    }

    ofs << "\t\t<DataSet timestep=\"" << current_time
        << "\" group=\"\" part=\"0\" file=\"" << vtu_filename << "\"/>\n";

    ofs << "\t</Collection>\n";
    ofs << "</VTKFile>\n";
  }

  private:
  fs::path m_pvd_filepath;
};
} // namespace vtu
