#version 330 core

// ---------------------------------------------------------------------------
// Solarsim fragment shader
//
// Every pixel is rendered by tracing one ray from the camera through a scene
// made of analytic spheres. The pipeline is:
//
//   1. build a camera ray for the pixel;
//   2. bend the ray near black holes (an approximation of gravitational
//      lensing);
//   3. sphere-trace the union of all body surfaces;
//   4. intersect the ray with each accretion disk;
//   5. shade whichever surface or disk is closest, otherwise draw stars.
//
// Positions and radii are in AU. The simulation deliberately exaggerates body
// radii so that planets stay visible, and the shader uses those values as-is.
// The shader only reads uniforms, so the CPU remains the source of truth for
// the scene.
// ---------------------------------------------------------------------------

in vec2 fragCoord;
out vec4 FragColor;

uniform vec3 camera_pos;
uniform vec3 camera_front;
uniform vec3 camera_up;
uniform vec3 camera_right;
uniform float aspect_ratio;

uniform int num_bodies;
uniform int light_index;
uniform bool lighting_enabled;
uniform int raymarch_steps;

struct CelestialBody {
  vec3  position;
  float radius;
  vec3  color;
  float event_horizon;
  int   is_black_hole;
};

uniform CelestialBody bodies[MAX_BODIES];

const int   MAX_MARCH_STEPS   = 256;    // hard bound; the GUI varies steps up to it
const float FAR_DISTANCE      = 200.0;  // well beyond the outermost orbit
const float SURFACE_EPSILON   = 0.001;  // sphere-tracing hit threshold, in AU
const float MAX_DEFLECTION    = 1.5;    // radians; keeps the lensed image stable
const float DISK_OUTER_FACTOR = 10.0;   // disk radius in units of the horizon
const float SPECULAR_POWER    = 48.0;   // Blinn-Phong specular exponent

// ---------------------------------------------------------------------------
// Noise
//
// The accretion disk is the only textured surface, so its detail is generated
// here instead of loading a texture. Value noise provides the base pattern and
// three octaves of fBm add structure at smaller scales.
// ---------------------------------------------------------------------------

float hash21(vec2 p) {
  p = fract(p * vec2(123.34, 456.21));
  p += dot(p, p + 45.32);
  return fract(p.x * p.y);
}

float value_noise(vec2 p) {
  vec2 cell         = floor(p);
  vec2 local        = fract(p);
  vec2 smooth_local = local * local * (3.0 - 2.0 * local);

  float a = hash21(cell);
  float b = hash21(cell + vec2(1.0, 0.0));
  float c = hash21(cell + vec2(0.0, 1.0));
  float d = hash21(cell + vec2(1.0, 1.0));

  return mix(mix(a, b, smooth_local.x),
             mix(c, d, smooth_local.x),
             smooth_local.y);
}

float fbm(vec2 p) {
  float sum       = 0.0;
  float amplitude = 0.5;

  for (int octave = 0; octave < 3; ++octave) {
    sum += amplitude * value_noise(p);
    p *= 2.0;
    amplitude *= 0.5;
  }

  return sum;
}

// ---------------------------------------------------------------------------
// Scene geometry
//
// Each body is a sphere. For a black hole the render surface is its event
// horizon, which is intentionally smaller than the radius stored by the
// simulation.
// ---------------------------------------------------------------------------

float render_radius(int index) {
  return bodies[index].is_black_hole == 1
             ? bodies[index].event_horizon
             : bodies[index].radius;
}

float sphere_distance(vec3 point, int index) {
  return length(point - bodies[index].position) - render_radius(index);
}

struct NearestSurface {
  float distance;
  int   body;
};

// The union of spheres has the exact signed distance min(sphere distances).
// Tracking the responsible body here avoids a second loop after a hit.
NearestSurface nearest_surface(vec3 point) {
  NearestSurface nearest = NearestSurface(FAR_DISTANCE, -1);

  for (int i = 0; i < num_bodies; ++i) {
    float distance = sphere_distance(point, i);

    if (distance < nearest.distance) {
      nearest.distance = distance;
      nearest.body     = i;
    }
  }

  return nearest;
}

// ---------------------------------------------------------------------------
// Star field
//
// The stars are a fixed screen-space pattern. Sampling a world-space direction
// with a high-frequency hash makes the dots shimmer as the camera turns,
// because each pixel lands on a different hash cell. Anchoring the pattern to
// the pixel keeps the background stable.
// ---------------------------------------------------------------------------

vec3 starfield(vec2 pixel) {
  float star = step(0.99995, fract(sin(pixel.x * 723.13 + pixel.y * 329.41) * 983.17));
  return vec3(star);
}

// ---------------------------------------------------------------------------
// Gravitational lensing
//
// This is an artistic approximation, not a geodesic integration. For each
// black hole the ray receives the small-angle Einstein deflection
//
//   alpha = 2 * r_s / b
//
// where r_s is the rendered event horizon and b is the impact parameter.
// The deflection is applied once at the camera instead of being integrated
// along the path, which is enough for this screen-space effect. Rays aimed
// inside the horizon are captured. The cap keeps near-horizon rays from
// folding the image back on itself.
// ---------------------------------------------------------------------------

vec3 rotate_around_axis(vec3 vector, vec3 axis, float angle) {
  float cosine = cos(angle);
  float sine   = sin(angle);

  return vector * cosine
       + cross(axis, vector) * sine
       + axis * dot(axis, vector) * (1.0 - cosine);
}

vec3 lens_ray(vec3 origin, vec3 direction, out bool captured) {
  captured = false;

  for (int i = 0; i < num_bodies; ++i) {
    if (bodies[i].is_black_hole != 1) {
      continue;
    }

    vec3  to_black_hole    = bodies[i].position - origin;
    float forward_distance = dot(to_black_hole, direction);

    if (forward_distance <= 0.0) {
      continue;
    }

    float impact_squared = dot(to_black_hole, to_black_hole) - (forward_distance*forward_distance);
    float horizon        = bodies[i].event_horizon;

    if (impact_squared < horizon*horizon) {
      captured = true;
      return direction;
    }

    float impact     = sqrt(max(impact_squared, 1e-8));
    float deflection = min(2.0 * horizon / impact, MAX_DEFLECTION);
    vec3  axis       = cross(direction, to_black_hole);

    // A zero axis means the ray points at the horizon center, which the
    // capture test above should already have rejected. Guard anyway so a
    // degenerate configuration cannot produce NaNs.
    if (dot(axis, axis) < 1e-12) {
      continue;
    }

    direction = normalize(rotate_around_axis(direction, normalize(axis), deflection));
  }

  return direction;
}

// ---------------------------------------------------------------------------
// Ray marching
//
// Sphere tracing: the union distance is a lower bound on the distance to the
// nearest surface, so stepping by it can never pass through geometry. The GUI
// exposes a lower iteration count; MAX_MARCH_STEPS is only the hard bound.
// ---------------------------------------------------------------------------

struct SceneHit {
  bool  hit;
  float distance;
  int   body;
};

SceneHit trace_scene(vec3 origin, vec3 direction) {
  float traveled = 0.0;

  for (int step = 0; step < MAX_MARCH_STEPS; ++step) {
    if (step >= raymarch_steps) {
      break;
    }

    vec3           point   = origin + direction * traveled;
    NearestSurface surface = nearest_surface(point);

    if (surface.distance < SURFACE_EPSILON) {
      return SceneHit(true, traveled, surface.body);
    }

    traveled += surface.distance;

    if (traveled > FAR_DISTANCE) {
      break;
    }
  }

  return SceneHit(false, FAR_DISTANCE, -1);
}

// ---------------------------------------------------------------------------
// Accretion disk
//
// The disk is a flat annulus around its black hole, tilted in world space.
// A ray intersects the supporting plane and is accepted only between the
// event horizon and the outer edge.
// ---------------------------------------------------------------------------

struct DiskHit {
  bool  hit;
  float distance;
  vec3  position;
  int   black_hole;
};

// The disk plane is fixed in world space, so all black holes share the same
// orientation. The simulation has no spin axis to align it with.
vec3 accretion_disk_normal() {
  return normalize(vec3(0.0, 1.0, 0.5));
}

// A stable second axis used to build 2D texture coordinates inside the plane.
vec3 accretion_disk_tangent(vec3 normal) {
  return normalize(cross(normal, vec3(0.0, 0.0, 1.0)));
}

bool intersect_disk(int index, vec3 origin, vec3 direction,
                    out float hit_distance, out vec3 hit_position) {
  hit_distance = -1.0;
  hit_position = vec3(0.0);

  vec3  normal      = accretion_disk_normal();
  float denominator = dot(direction, normal);

  if (abs(denominator) < 0.001) {
    return false;
  }

  float plane_distance = dot(bodies[index].position - origin, normal) / denominator;

  if (plane_distance <= 0.0) {
    return false;
  }

  vec3  intersection = origin + direction * plane_distance;
  float radius       = length(intersection - bodies[index].position);
  float horizon      = bodies[index].event_horizon;

  if (radius <= horizon || radius >= horizon * DISK_OUTER_FACTOR) {
    return false;
  }

  hit_distance = plane_distance;
  hit_position = intersection;
  return true;
}

DiskHit find_accretion_disk(vec3 origin, vec3 direction) {
  DiskHit nearest = DiskHit(false, FAR_DISTANCE, vec3(0.0), -1);

  for (int i = 0; i < num_bodies; ++i) {
    if (bodies[i].is_black_hole != 1) {
      continue;
    }

    float distance;
    vec3  position;

    if (intersect_disk(i, origin, direction, distance, position)
        && distance < nearest.distance) {
      nearest = DiskHit(true, distance, position, i);
    }
  }

  return nearest;
}

vec3 shade_accretion_disk(DiskHit hit, vec3 ray_origin) {
  int   index             = hit.black_hole;
  vec3  center            = bodies[index].position;
  vec3  offset            = hit.position - center;
  float radius            = length(offset);
  float horizon           = bodies[index].event_horizon;
  float outer_radius      = horizon * DISK_OUTER_FACTOR;
  float normalized_radius = (radius - horizon) / (outer_radius - horizon);

  vec3 normal    = accretion_disk_normal();
  vec3 tangent   = accretion_disk_tangent(normal);
  vec3 bitangent = cross(normal, tangent);

  // The base pattern uses Cartesian disk coordinates, which stay continuous
  // across atan()'s wrap-around at +/- pi. The spiral offset is built from
  // sin/cos of the phase for the same reason: using the raw angle would leave
  // a visible seam where the angle jumps.
  vec2  plane         = vec2(dot(offset, tangent), dot(offset, bitangent)) / outer_radius;
  float angle         = atan(plane.y, plane.x);
  float spiral_phase  = angle * 2.0 - normalized_radius * 7.0;
  vec2  spiral_offset = 0.6 * vec2(cos(spiral_phase), sin(spiral_phase));
  float turbulence    = fbm(plane * 8.0 + spiral_offset);

  // The inner disk is hotter and whiter; the outer disk cools toward orange.
  vec3 color = mix(vec3(1.0, 0.95, 0.82),
                   vec3(1.0, 0.30, 0.05),
                   smoothstep(0.0, 1.0, normalized_radius));
  color *= pow(max(1.0 - normalized_radius, 0.0), 0.7);
  color *= 0.55 + 0.9 * turbulence;

  // Beaming: material orbiting toward the camera appears brighter.
  vec3  orbital_direction = normalize(cross(normal, offset));
  vec3  view_direction    = normalize(ray_origin - hit.position);
  float beaming           = 1.0 + 0.35 * dot(orbital_direction, view_direction);
  color *= clamp(beaming, 0.55, 1.45);

  // Light climbing out of the gravity well loses energy near the horizon.
  float redshift = sqrt(max(1.0 - horizon / radius, 0.0));
  color *= mix(0.55, 1.0, redshift);

  return color;
}

// ---------------------------------------------------------------------------
// Surface shading
//
// Interplanetary shadows are deliberately omitted. Planet radii are exaggerated
// so the bodies stay visible, which would exaggerate their shadows too: from
// the Sun, Mercury would darken a large part of Earth. Without shadow rays
// every planet shows a clean lit and dark side, and the default aligned solar
// system stays readable.
// ---------------------------------------------------------------------------

vec3 shade_body(int index, vec3 hit_position, vec3 ray_origin) {
  if (bodies[index].is_black_hole == 1) {
    return vec3(0.0);
  }

  vec3 base_color = bodies[index].color;

  if (index == light_index) {
    return base_color;
  }

  if (!lighting_enabled || light_index < 0) {
    return base_color;
  }

  // The hit is on an analytic sphere, so the exact outward normal is simply
  // the normalized offset from the center; no gradient sampling is needed.
  vec3  normal          = normalize(hit_position - bodies[index].position);
  vec3  light_direction = normalize(bodies[light_index].position - hit_position);
  vec3  view_direction  = normalize(ray_origin - hit_position);
  vec3  half_vector     = light_direction + view_direction;
  float half_length_sq  = dot(half_vector, half_vector);

  float diffuse  = max(dot(normal, light_direction), 0.0);
  float specular = 0.0;

  // The half-vector vanishes when the light is exactly behind the surface
  // point as seen from the camera. No highlight is visible in that case, so
  // skipping the term also avoids normalize(0).
  if (half_length_sq > 1e-8) {
    vec3 half_direction = half_vector * inversesqrt(half_length_sq);
    specular = pow(max(dot(normal, half_direction), 0.0), SPECULAR_POWER);
  }

  return base_color * (0.08 + diffuse)
       + vec3(0.8) * specular;
}

// ---------------------------------------------------------------------------
// Entry point
// ---------------------------------------------------------------------------

void main() {
  // fragCoord is in [0,1]; remap it to [-1,1] and correct for the aspect ratio.
  vec2 screen_position = fragCoord * 2.0 - 1.0;
  screen_position.x *= aspect_ratio;

  // The camera basis is orthonormal, so the offset can be added directly to
  // the forward vector before normalizing.
  vec3 ray_origin    = camera_pos;
  vec3 ray_direction = normalize(camera_front
                               + screen_position.x * camera_right
                               + screen_position.y * camera_up);

  bool captured         = false;
  vec3 lensed_direction = lens_ray(ray_origin, ray_direction, captured);

  // A captured ray ends on the horizon, so bending it further has no meaning.
  // Trace the original direction instead: anything in front of the black hole
  // (a planet or the near side of the disk) must still be visible, while the
  // horizon itself shades to black and naturally hides what is behind it.
  vec3 traced_direction = captured ? ray_direction : lensed_direction;

  SceneHit surface = trace_scene(ray_origin, traced_direction);
  DiskHit  disk    = find_accretion_disk(ray_origin, traced_direction);

  vec3 color;

  // Compare distances so a disk in front of a planet still occludes it.
  if (disk.hit && (!surface.hit || disk.distance < surface.distance)) {
    color = shade_accretion_disk(disk, ray_origin);
  } else if (surface.hit && surface.body >= 0) {
    vec3 hit_position = ray_origin + traced_direction * surface.distance;
    color = shade_body(surface.body, hit_position, ray_origin);
  } else {
    color = starfield(fragCoord);
  }

  FragColor = vec4(color, 1.0);
}

// ---------------------------------------------------------------------------
// This shader code was generated using DeepSeek V4.1 Flash.
// ---------------------------------------------------------------------------
