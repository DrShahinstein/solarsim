#include <algorithm>
#include <cmath>
#include <glm/glm.hpp>
#include <glm/gtx/norm.hpp>
#include "simulation.hpp"

Simulation::Simulation() : G(DEFAULT_G)              { current_integrator = &Simulation::integrate_velocity_verlet; }
void Simulation::add_body(const CelestialBody &body) { bodies.push_back(body); }
void Simulation::clear_bodies()                      { bodies.clear(); }
void Simulation::setG(double value)                  { G = value; }
double Simulation::getG()                      const { return G; }
std::vector<CelestialBody> &Simulation::get_bodies() { return bodies; }

void Simulation::compute_forces() {
  for (auto &body : bodies) {
    body.previous_acceleration = body.acceleration;
    body.acceleration = glm::dvec3(0.0);
  }

  for (size_t i = 0; i < bodies.size(); ++i) {
    for (size_t j = i + 1; j < bodies.size(); ++j) {
      auto &body1 = bodies[i];
      auto &body2 = bodies[j];

      glm::dvec3 r = body2.position - body1.position;
      double distance_sq = glm::length2(r);

      if (distance_sq < 1e-12) continue;

      double force_magnitude = G * body1.mass * body2.mass / distance_sq;
      glm::dvec3 force_dir = glm::normalize(r);
      glm::dvec3 force = force_magnitude * force_dir;

      body1.acceleration += force / body1.mass;
      body2.acceleration -= force / body2.mass;
    }
  }
}

void Simulation::apply_post_newtonian_corrections() {
  const double C_SQ = C * C;

  std::vector<CelestialBody *> blackholes;
  for (auto &body : bodies) {
    if (body.is_black_hole) {
      blackholes.push_back(&body);
    }
  }

  for (auto *bh : blackholes) {
    const double rs = (2.0 * G * bh->mass) / C_SQ;

    for (auto &other : bodies) {
      if (&other == bh || other.is_black_hole) continue;

      glm::dvec3 r = other.position - bh->position;
      double distance = glm::length(r);

      if (distance < 100.0 * rs) continue;
      if (distance < 1e-10)      continue;

      glm::dvec3 direction = glm::normalize(r);
      double v_sq = glm::length2(other.velocity);

      double correction = (3.0 * G * bh->mass) / (C_SQ * distance);
      glm::dvec3 extra_accel = correction * v_sq * direction;

      other.acceleration += extra_accel;
    }
  }
}

void Simulation::update(double dt) {
  (this->*current_integrator)(dt);
}

void Simulation::remove_marked_bodies() {
  bodies.erase(
      std::remove_if(bodies.begin(), bodies.end(),
                     [](const CelestialBody &body) { return body.mass <= 0; }),
      bodies.end());
}

void Simulation::integrate_velocity_verlet(double dt) {
  for (auto &body : bodies) {
    body.position += body.velocity * dt + 0.5 * body.acceleration * (dt * dt);
  }

  compute_forces();
  apply_post_newtonian_corrections();

  for (auto &body : bodies) {
    body.velocity += 0.5 * (body.previous_acceleration + body.acceleration) * dt;
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
      0.2,                       // radius
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