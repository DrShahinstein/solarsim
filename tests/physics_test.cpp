#include <cmath>
#include <cstdio>
#include <glm/glm.hpp>
#include <glm/gtx/norm.hpp>
#include "simulation.hpp"

namespace {

int failures = 0;

void check(bool condition, const char *message) {
  if (!condition) {
    std::fprintf(stderr, "FAIL: %s\n", message);
    ++failures;
  }
}

double total_energy(const Simulation &simulation) {
  const auto &bodies = simulation.get_bodies();
  double energy = 0.0;

  for (size_t i = 0; i < bodies.size(); ++i) {
    energy += 0.5 * bodies[i].mass * glm::length2(bodies[i].velocity);

    for (size_t j = i + 1; j < bodies.size(); ++j) {
      const double distance = glm::length(bodies[j].position - bodies[i].position);
      energy -= simulation.getG() * bodies[i].mass * bodies[j].mass / distance;
    }
  }

  return energy;
}

glm::dvec3 total_momentum(const Simulation &simulation) {
  glm::dvec3 momentum(0.0);
  for (const auto &body : simulation.get_bodies()) {
    momentum += body.mass * body.velocity;
  }
  return momentum;
}

void test_energy_and_momentum() {
  Simulation simulation;
  simulation.reset_to_solar_system();

  const double initial_energy = total_energy(simulation);
  const glm::dvec3 initial_momentum = total_momentum(simulation);

  for (int step = 0; step < 1000; ++step) {
    simulation.update(0.01);
  }

  const double relative_error = (total_energy(simulation) - initial_energy) / std::abs(initial_energy);
  check(std::abs(relative_error) < 1e-10, "energy drift is too large");
  check(glm::length(initial_momentum) < 1e-15, "initial momentum is not balanced");
  check(glm::length(total_momentum(simulation)) < 1e-15, "momentum is not conserved");
}

void test_circular_orbit() {
  Simulation simulation;

  constexpr double sun_mass = 1.0;
  constexpr double planet_mass = 3.0e-6;
  constexpr double distance = 1.0;

  CelestialBody sun{};
  sun.mass = sun_mass;
  sun.radius = 0.2;
  sun.is_star = true;
  simulation.add_body(sun);

  const double relative_speed = std::sqrt(DEFAULT_G * (sun_mass + planet_mass) / distance);

  CelestialBody planet{};
  planet.position = glm::dvec3(distance, 0.0, 0.0);
  planet.mass = planet_mass;
  planet.radius = 0.01;
  planet.velocity = glm::dvec3(0.0, relative_speed * sun_mass / (sun_mass + planet_mass), 0.0);
  simulation.add_body(planet);
  simulation.body(0).velocity = glm::dvec3(0.0, -relative_speed * planet_mass / (sun_mass + planet_mass), 0.0);

  const double period = 2.0 * 3.141592653589793 * std::sqrt(distance * distance * distance / (DEFAULT_G * (sun_mass + planet_mass)));
  const double dt = 0.01;
  const int steps = static_cast<int>(std::lround(period / dt));

  for (int step = 0; step < steps; ++step) {
    simulation.update(dt);
  }

  const double final_distance = glm::length(simulation.body(1).position - simulation.body(0).position);
  check(std::abs(final_distance - distance) < 1e-3, "circular orbit drifted");
}

void test_body_lifecycle() {
  Simulation simulation;
  simulation.reset_to_solar_system();

  const size_t initial_count = simulation.body_count();

  CelestialBody body{};
  body.mass = 1e-6;
  body.position = glm::dvec3(2.0, 0.0, 0.0);
  simulation.add_body(body);
  check(simulation.body_count() == initial_count + 1, "body was not added");

  simulation.body(simulation.body_count() - 1).marked_for_deletion = true;
  simulation.remove_marked_bodies();
  check(simulation.body_count() == initial_count, "body was not removed");
}

void test_body_limit() {
  Simulation simulation;

  CelestialBody body{};
  body.mass = 1e-9;

  for (int i = 0; i < MAX_BODIES + 5; ++i) {
    body.position = glm::dvec3(static_cast<double>(i + 1), 0.0, 0.0);
    simulation.add_body(body);
  }

  check(simulation.body_count() == MAX_BODIES, "body limit is not enforced");
}

}

int main() {
  test_energy_and_momentum();
  test_circular_orbit();
  test_body_lifecycle();
  test_body_limit();

  if (failures != 0) {
    std::fprintf(stderr, "%d physics test(s) failed\n", failures);
    return 1;
  }

  std::printf("all physics tests passed\n");
  return 0;
}
