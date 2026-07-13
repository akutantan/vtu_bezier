#include <cmath>
#include <cstring>

#include "mesh.h"
#include "vec3.h"
#include "vtu_bezier.h"

using Double3 = vec3<double>;
using Int3    = vec3<int>;

using namespace std;
namespace fs = std::filesystem;

int main() {
  constexpr int npd   = 2;
  constexpr int deg[] = {2, 2, 2};
  constexpr int ele[] = {3, 3, 3};

  Int3 p;
  Int3 ne;
  for (int i = 0; i < npd; i++) {
    p[i]  = deg[i];
    ne[i] = ele[i];
  }
  Mesh mesh(npd, p, ne);

  fs::path filename = "test.vtu";
  fs::path dirpath  = "results";
  if (!fs::exists(dirpath)) {
    fs::create_directory(dirpath);
  }
  fs::path filepath = dirpath / filename;

#if 0
  vtu::Writer writer(filepath, vtu::Format::ASCII);
#else
  vtu::Writer writer(filepath, vtu::Format::Appended);
#endif
  writer.set_geometry((double*)(mesh.bezier_pos.data()), mesh.num_bezier_pos);
  writer.set_topology(mesh.num_ele, mesh.type, (int*)&p);
  writer.add_attribute(mesh.weights.data(), mesh.num_bezier_pos, vtu::AttributeType::Scalar, vtu::AttributeCenter::Node, "RationalWeights");
  writer.write();

  fs::path pvd_filepath = dirpath / "results.pvd";
  vtu::PvdWriter pvd_writer(pvd_filepath);
  pvd_writer.write(0.5, filename);

  return 0;
}