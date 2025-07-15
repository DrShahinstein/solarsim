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
      1.0,                       // mass (solar masses)
      0.1,                       // radius
      glm::vec3(1.0, 1.0, 0.0)   // yellow
  });

  // mercury
  add_body({
      glm::dvec3(0.4, 0.0, 0.0),   // position (AU)
      glm::dvec3(0.0, 0.031, 0.0), // velocity (AU/day)
      glm::dvec3(0.0),             // acceleration
      1.65e-7,                     // mass (solar masses)
      0.02,                        // radius
      glm::vec3(0.8, 0.8, 0.8)     // gray
  });

  // venus
  add_body({
      glm::dvec3(0.8, 0.0, 0.0),   // position (AU)
      glm::dvec3(0.0, 0.023, 0.0), // velocity (AU/day)
      glm::dvec3(0.0),             // acceleration
      2.45e-6,                     // mass (solar masses)
      0.03,                        // radius
      glm::vec3(0.9, 0.7, 0.0)     // yellowish
  });

  // earth
  add_body({
      glm::dvec3(1.5, 0.0, 0.0),   // position (AU)
      glm::dvec3(0.0, 0.017, 0.0), // velocity (AU/day)
      glm::dvec3(0.0),             // acceleration
      3e-6,                        // mass (solar masses)
      0.03,                        // radius
      glm::vec3(0.0, 0.0, 1.0)     // blue
  });

  // mars
  add_body({
      glm::dvec3(2.5, 0.0, 0.0),   // position (AU)
      glm::dvec3(0.0, 0.015, 0.0), // velocity (AU/day)
      glm::dvec3(0.0),             // acceleration
      3.2e-7,                      // mass (solar masses)
      0.02,                        // radius
      glm::vec3(1.0, 0.0, 0.0)     // red
  });

  // jupiter
  add_body({
      glm::dvec3(5.2, 0.0, 0.0),   // position (AU)
      glm::dvec3(0.0, 0.008, 0.0), // velocity (AU/day)
      glm::dvec3(0.0),             // acceleration
      9.5e-4,                      // mass (solar masses)
      0.08,                        // larger radius
      glm::vec3(0.8, 0.6, 0.4)     // brownish
  });

  // saturn
  add_body({
      glm::dvec3(9.5, 0.0, 0.0),   // position (AU)
      glm::dvec3(0.0, 0.006, 0.0), // velocity (AU/day)
      glm::dvec3(0.0),             // acceleration
      2.75e-4,                     // mass (solar masses)
      0.07,                        // radius
      glm::vec3(0.9, 0.8, 0.5)     // light brown
  });

  // uranus
  add_body({
      glm::dvec3(19.2, 0.0, 0.0),  // position (AU)
      glm::dvec3(0.0, 0.004, 0.0), // velocity (AU/day)
      glm::dvec3(0.0),             // acceleration
      4.4e-5,                      // mass (solar masses)
      0.05,                        // radius
      glm::vec3(0.5, 0.8, 1.0)     // light blue
  });

  // neptune
  add_body({
      glm::dvec3(30.1, 0.0, 0.0),  // position (AU)
      glm::dvec3(0.0, 0.003, 0.0), // velocity (AU/day)
      glm::dvec3(0.0),             // acceleration
      5.15e-5,                     // mass (solar masses)
      0.05,                        // radius
      glm::vec3(0.0, 0.0, 0.8)     // blue
  });
}