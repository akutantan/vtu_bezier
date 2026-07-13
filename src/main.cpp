#include <cmath>
#include <cstring>

#include "vec3.h"
#include "mesh.h"
#include "vtu_bezier.h"

using Double3 = vec3<double>;
using Int3    = vec3<int>;

using namespace std;
namespace fs = std::filesystem;

int main() {
  constexpr int npd       = 3;
  constexpr int deg[]     = {2, 2, 2};
  constexpr int ele[] = {3, 3, 3};

  Int3 p;
  Int3 ne;
  for (int i = 0; i < npd; i++) {
    p[i]  = deg[i];
    ne[i] = ele[i];
  }
  Mesh mesh(npd, p, ne);


  fs::path filepath = "results";
  if (!fs::exists(filepath)) {
    fs::create_directory(filepath);
  }
  fs::path filename = filepath / "test.vtu";

  vtu::Writer writer(filename, vtu::Format::ASCII);
  writer.set_geometry((double*)(mesh.bezier_pos.data()), mesh.num_bezier_pos);
  writer.set_topology(mesh.num_ele, mesh.type, (int*)&p);
  writer.add_attribute(mesh.weights.data(), mesh.num_bezier_pos, vtu::AttributeType::Scalar, vtu::AttributeCenter::Node, "RationalWeights");

  writer.write();

  return 0;
}