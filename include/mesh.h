#pragma once
#include <vector>

#include "vtu_bezier.h"
#include "vec3.h"

using Double3 = vec3<double>;
using Int3    = vec3<int>;


class Mesh {
  public:
  vtu::TopologyType type; 

  int npd;
  Int3 p;
  Int3 ne;
  Int3 nn;

  int num_ele;
  int num_bezier_pos;

  std::vector<Double3> cp_pos;
  std::vector<std::vector<double>> beo;
  std::vector<std::vector<double>> beo_t;
  std::vector<Double3> bezier_pos;
  std::vector<double> weights;

  public:
  Mesh (int npd_, Int3 p_, Int3 ne_);
};
