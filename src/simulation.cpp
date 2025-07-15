#include <cmath>
#include <glm/glm.hpp>
#include <glm/gtx/norm.hpp>
#include "simulation.hpp"

// gravitational constant in AU^3 / (M_sun * day^2)
constexpr double DEFAULT_G = 0.000295912208;

Simulation::Simulation() : G(DEFAULT_G) {}
void Simulation::add_body(const CelestialBody &body) { bodies.push_back(body); }
void Simulation::clear_bodies() { bodies.clear(); }
void Simulation::setG(double value) { G = value; }
std::vector<CelestialBody> &Simulation::get_bodies() { return bodies; }

void Simulation::compute_forces() {
  // reset accelerations
  for (auto &body : bodies) {
    body.acceleration = glm::dvec3(0.0);
  }

  // compute gravitational forces
  for (size_t i = 0; i < bodies.size(); ++i) {
    for (size_t j = i + 1; j < bodies.size(); ++j) {
      auto &body1 = bodies[i];
      auto &body2 = bodies[j];

      glm::dvec3 r = body2.position - body1.position;
      double distance_sq = glm::length2(r);
      double distance = std::sqrt(distance_sq);

      // avoid division by zero
      if (distance < 1e-10)
        continue;

      double force_magnitude = G * body1.mass * body2.mass / distance_sq;
      glm::dvec3 force_dir = glm::normalize(r);
      glm::dvec3 force = force_magnitude * force_dir;

      body1.acceleration += force / body1.mass;
      body2.acceleration -= force / body2.mass;
    }
  }
}

// simplified GR corrections for black holes
void Simulation::apply_post_newtonian_corrections() {
  const double C = 63241.1; // speed of light in AU/day

  for (auto &body : bodies) {
    if (!body.is_black_hole)
      continue;

    for (auto &other : bodies) {
      if (&body == &other)
        continue;

      glm::dvec3 r = other.position - body.position;
      double distance = glm::length(r);
      if (distance < 1e-10)
        continue;

      glm::dvec3 direction = glm::normalize(r);
      double v_sq = glm::length2(other.velocity);

      // post-newtonian correction (simplified)
      double correction = (3.0 * G * body.mass) / (C * C * distance);
      glm::dvec3 extra_accel = correction * v_sq * direction;

      other.acceleration += extra_accel;
    }
  }
}

void Simulation::update(double dt) {
  compute_forces();
  apply_post_newtonian_corrections();

  for (auto &body : bodies) {
    body.velocity += body.acceleration * dt;
    body.position += body.velocity * dt;
  }
}

void Simulation::reset_to_solar_system() {
  clear_bodies();

  // sun
  add_body({
      glm::dvec3(0.0, 0.0, 0.0), // position
      glm::dvec3(0.0),           // velocity
      glm::dvec3(0.0),           // acceleration
      1.0,                       // solar mass
      0.1,                       // radius
      glm::vec3(1.0, 1.0, 0.0)   // color
  });

  // earth
  add_body({glm::dvec3(1.0, 0.0, 0.0),   // 1 AU
           glm::dvec3(0.0, 0.017, 0.0),  // ~30 km/s
           glm::dvec3(0.0),
           3e-6,                         // solar mass
           0.02, glm::vec3(0.0, 0.0, 1.0)});

  // mars
  add_body({glm::dvec3(1.52, 0.0, 0.0), glm::dvec3(0.0, 0.015, 0.0),
           glm::dvec3(0.0), 3.2e-7, 0.015, glm::vec3(1.0, 0.0, 0.0)});
}