#include <algorithm>
#include <cmath>
#include <glm/glm.hpp>
#include <glm/gtx/norm.hpp>
#include "simulation.hpp"

Simulation::Simulation() : G(DEFAULT_G)              { current_integrator = &Simulation::integrate_velocity_verlet; }
void Simulation::add_body(const CelestialBody &body) {
  if (bodies.size() >= MAX_BODIES) return;
  bodies.push_back(body);
  recompute_accelerations();
}
void Simulation::clear_bodies()                      { bodies.clear(); }
void Simulation::setG(double value)                  { G = value; recompute_accelerations(); }
double Simulation::getG()                      const { return G; }
std::vector<CelestialBody> &Simulation::get_bodies() { return bodies; }

void Simulation::compute_forces() {
  for (auto &body : bodies) {
    body.acceleration = glm::dvec3(0.0);
  }

  for (size_t i = 0; i < bodies.size(); ++i) {
    for (size_t j = i + 1; j < bodies.size(); ++j) {
      auto &body1 = bodies[i];
      auto &body2 = bodies[j];

      if (body1.mass <= 0.0 || body2.mass <= 0.0) continue;

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
  for (size_t i = 0; i < bodies.size(); ++i) {
    auto &bh = bodies[i];
    if (!bh.is_black_hole || bh.mass <= 0.0) continue;

    const double rs = bh.event_horizon_radius(G);

    for (size_t j = 0; j < bodies.size(); ++j) {
      if (i == j) continue;

      auto &other = bodies[j];
      if (other.is_black_hole || other.mass <= 0.0) continue;

      glm::dvec3 r = other.position - bh.position;
      double distance = glm::length(r);

      if (distance < 100.0 * rs) continue;
      if (distance < 1e-10)      continue;

      glm::dvec3 direction = r / distance;
      double v_sq = glm::length2(other.velocity - bh.velocity);

      double correction = (3.0 * G * bh.mass) / ((C * C) * distance);
      glm::dvec3 extra_accel = correction * v_sq * direction;

      other.acceleration += extra_accel;
      bh.acceleration -= (other.mass / bh.mass) * extra_accel;
    }
  }
}

void Simulation::recompute_accelerations() {
  compute_forces();
  apply_post_newtonian_corrections();
}

void Simulation::update(double dt) {
  (this->*current_integrator)(dt);
}

void Simulation::remove_marked_bodies() {
  bodies.erase(
      std::remove_if(bodies.begin(), bodies.end(),
                     [](const CelestialBody &body) { return body.marked_for_deletion; }),
      bodies.end());
}

void Simulation::integrate_velocity_verlet(double dt) {
  for (auto &body : bodies) {
    body.position += body.velocity * dt + 0.5 * body.acceleration * (dt * dt);
    body.previous_acceleration = body.acceleration;
  }

  recompute_accelerations();

  for (auto &body : bodies) {
    body.velocity += 0.5 * (body.previous_acceleration + body.acceleration) * dt;
  }
}

void Simulation::reset_to_solar_system() {
  clear_bodies();
  G = DEFAULT_G;

  constexpr double sun_mass = 1.0;
  glm::dvec3 sun_velocity(0.0);

  add_body({
      .position = glm::dvec3(0.0),
      .velocity = glm::dvec3(0.0),
      .mass = sun_mass,
      .radius = 0.2,
      .color = glm::vec3(1.0, 1.0, 0.0),
      .is_star = true,
  });

  struct Planet {
    double distance;
    double mass;
    double radius;
    glm::vec3 color;
  };

  const Planet planets[] = {
      {0.4, 1.65e-7, 0.02, glm::vec3(0.8, 0.8, 0.8)},
      {0.8, 2.45e-6, 0.03, glm::vec3(0.9, 0.7, 0.0)},
      {1.5, 3.0e-6, 0.03, glm::vec3(0.0, 0.0, 1.0)},
      {2.5, 3.2e-7, 0.02, glm::vec3(1.0, 0.0, 0.0)},
      {5.2, 9.5e-4, 0.08, glm::vec3(0.8, 0.6, 0.4)},
      {9.5, 2.75e-4, 0.07, glm::vec3(0.9, 0.8, 0.5)},
      {19.2, 4.4e-5, 0.05, glm::vec3(0.5, 0.8, 1.0)},
      {30.1, 5.15e-5, 0.05, glm::vec3(0.0, 0.0, 0.8)},
  };

  for (const auto &planet : planets) {
    const double total_mass = sun_mass + planet.mass;
    const double relative_speed = std::sqrt(G * total_mass / planet.distance);
    const double sun_share = planet.mass / total_mass;
    const double planet_share = sun_mass / total_mass;

    sun_velocity.y -= relative_speed * sun_share;

    add_body({
        .position = glm::dvec3(planet.distance, 0.0, 0.0),
        .velocity = glm::dvec3(0.0, relative_speed * planet_share, 0.0),
        .mass = planet.mass,
        .radius = planet.radius,
        .color = planet.color,
    });
  }

  bodies[0].velocity = sun_velocity;
  recompute_accelerations();
}
