#include <cmath>

#include <mesh.h>

Mesh::Mesh(int npd_, Int3 p_, Int3 ne_) : npd(npd_), p(p_), ne(ne_) {

  if (npd == 1) {
    type = vtu::TopologyType::BezierCurve;
  } else if (npd == 2) {
    type = vtu::TopologyType::BezierQuadrilateral;
  } else if (npd == 3) {
    type = vtu::TopologyType::BezierHexahedron;
  } else {
    type = vtu::TopologyType::Unknown;
  }

  int nex = ne[0];
  int ney = (npd > 1) ? ne[1] : 1;
  int nez = (npd > 2) ? ne[2] : 1;
  num_ele = nex * ney * nez;

  int px = p[0];
  int py = (npd > 1) ? p[1] : 0;
  int pz = (npd > 2) ? p[2] : 0;

  int nen_x = px + 1;
  int nen_y = py + 1;
  int nen_z = pz + 1;

  int nen = nen_x * nen_y * nen_z;

  num_bezier_pos = num_ele * nen;

  bezier_pos.resize(num_bezier_pos * 3, 0.0);
  weights.resize(num_bezier_pos, 1.0);

  int idx = 0;
  for (int ez = 0; ez < nez; ++ez) {
    for (int ey = 0; ey < ney; ++ey) {
      for (int ex = 0; ex < nex; ++ex) {

        for (int k = 0; k < nen_z; ++k) {
          for (int j = 0; j < nen_y; ++j) {
            for (int i = 0; i < nen_x; ++i) {

              double u = (ex + (double)i / px) / nex;
              double v = (npd > 1) ? (ey + (double)j / py) / ney : 0.0;
              double w = (npd > 2) ? (ez + (double)k / pz) / nez : 0.0;

              double x = u * 10.0;
              double y = v * 10.0;
              double z = w * 10.0;

              if (npd == 2) {
                z = 2.0 * std::sin(x) * std::cos(y);
              } else if (npd == 3) {
                double bow = 2.0 * std::sin(u * 3.14) * std::sin(v * 3.14);
                z += bow;
              } else {
                y = 2.0 * std::sin(x);
              }

              bezier_pos[idx].x = x;
              bezier_pos[idx].y = y;
              bezier_pos[idx].z = z;

              weights[idx] = 1.0;

              idx++;
            }
          }
        }
      }
    }
  }
}