#ifndef SIMULATION_HPP
#define SIMULATION_HPP

#include <glm/glm.hpp>
#include <vector>

#define DEFAULT_G 0.000295912208

struct CelestialBody {
  glm::dvec3 position;
  glm::dvec3 velocity;
  glm::dvec3 acceleration;
  double mass;
  double radius;
  glm::vec3 color;
  bool is_black_hole = false;
  glm::dvec3 previous_acceleration;
};

class Simulation;
using Integrator = void (Simulation::*)(double);

class Simulation {
public:
  Simulation();
  void add_body(const CelestialBody &body);
  void clear_bodies();
  void setG(double value);
  double getG() const;
  std::vector<CelestialBody> &get_bodies();
  void update(double dt);
  void reset_to_solar_system();
  void remove_marked_bodies();
  std::vector<CelestialBody> bodies;

private:
  void compute_forces();
  void apply_post_newtonian_corrections();
  void integrate_velocity_verlet(double dt);
  Integrator current_integrator;
  double G;
};

#endif