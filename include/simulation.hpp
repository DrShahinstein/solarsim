#ifndef SIMULATION_HPP
#define SIMULATION_HPP

#include <glm/glm.hpp>
#include <vector>

struct CelestialBody {
  glm::dvec3 position;
  glm::dvec3 velocity;
  glm::dvec3 acceleration;
  double mass;
  double radius;
  glm::vec3 color;
  bool is_black_hole = false;
};

class Simulation {
public:
  Simulation();
  void update(double dt);
  void add_body(const CelestialBody &body);
  void clear_bodies();
  void setG(double value);
  std::vector<CelestialBody> &get_bodies();
  void reset_to_solar_system();
  void apply_post_newtonian_corrections();

private:
  std::vector<CelestialBody> bodies;
  double G;
  void compute_forces();
};

#endif