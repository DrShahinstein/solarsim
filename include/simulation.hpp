#pragma once

#include <glm/glm.hpp>
#include <cstdint>
#include <vector>

constexpr double C = 173.1446;
constexpr double DEFAULT_G = 0.000295912208;

constexpr int MAX_BODIES = 20;
constexpr double BLACK_HOLE_VISUAL_SCALE = 2.5e7;

struct CelestialBody {
  glm::dvec3 position{0.0};
  glm::dvec3 velocity{0.0};
  glm::dvec3 acceleration{0.0};
  double mass = 0.0;
  double radius = 0.0;
  glm::vec3 color{0.0};
  bool is_black_hole = false;
  bool is_star = false;
  bool marked_for_deletion = false;
  glm::dvec3 previous_acceleration{0.0};

  double event_horizon_radius(double gravity) const {
    return is_black_hole ? (2.0 * gravity * mass) / (C * C) : 0.0;
  }
};

class Simulation {
public:
  Simulation();
  void add_body(const CelestialBody &body);
  void clear_bodies();
  void remove_last_body();
  void setG(double value);
  double getG() const;
  uint64_t get_physics_steps() const;
  size_t body_count() const;
  const std::vector<CelestialBody> &get_bodies() const;
  CelestialBody &body(size_t index);
  const CelestialBody &body(size_t index) const;
  void update(double dt);
  void reset_to_solar_system();
  void remove_marked_bodies();

private:
  void compute_forces();
  void apply_post_newtonian_corrections();
  void recompute_accelerations();
  void integrate_velocity_verlet(double dt);
  std::vector<CelestialBody> bodies;
  uint64_t physics_steps = 0;
  double G;
};
