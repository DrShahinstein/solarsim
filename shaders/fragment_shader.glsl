#version 330 core

in vec2 fragCoord;
out vec4 FragColor;

uniform vec3 camera_pos;
uniform vec3 camera_front;
uniform vec3 camera_up;
uniform vec3 camera_right;
uniform float aspect_ratio;
uniform int num_bodies;
uniform bool lighting_enabled;
uniform float G;

struct CelestialBody {
  vec3 position;
  float radius;
  vec3 color;
  float mass;
  int is_black_hole;
};

const int MAX_BODIES = 20;
uniform CelestialBody bodies[MAX_BODIES];

const int MAX_STEPS = 128;
const float MAX_DIST = 200.0;
const float SURF_DIST = 0.001;
const float C = 173.1446; // speed of light in AU/day
const float C_SQ = C * C;
const float PI = 3.14159265359;

vec3 ray_march(vec3 ro, vec3 rd);
vec3 accretion_disk_color(vec3 hit_pos, vec3 black_hole_pos);
float hash(vec2 p);

float sphere_distance(vec3 p, vec3 center, float radius) {
  return length(p - center) - radius;
}

float scene_distance(vec3 p) {
  float min_dist = MAX_DIST;
  for (int i = 0; i < num_bodies; i++) {
    if (bodies[i].is_black_hole == 1) continue;
    float d = sphere_distance(p, bodies[i].position, bodies[i].radius);
    min_dist = min(min_dist, d);
  }
  return min_dist;
}

vec3 estimate_normal(vec3 p) {
  const float eps = 0.0001;
  vec3 n = vec3(scene_distance(vec3(p.x + eps, p.y, p.z)) - scene_distance(vec3(p.x - eps, p.y, p.z)),
                scene_distance(vec3(p.x, p.y + eps, p.z)) - scene_distance(vec3(p.x, p.y - eps, p.z)),
                scene_distance(vec3(p.x, p.y, p.z + eps)) - scene_distance(vec3(p.x, p.y, p.z - eps)));
  return normalize(n);
}

void main() {
  // 1. SETUP THE INITIAL RAY
  vec2 uv = (fragCoord * 2.0 - 1.0) * vec2(aspect_ratio, 1.0);
  vec3 ray_dir = normalize(camera_front + uv.x * camera_right + uv.y * camera_up);
  vec3 ray_origin = camera_pos;

  // 2. APPLY GRAVITATIONAL LENSING
  for (int i = 0; i < num_bodies; i++) {
    if (bodies[i].is_black_hole == 1) {
      float rs = bodies[i].radius * 0.5;

      vec3 p = bodies[i].position - ray_origin;
      float d = dot(p, ray_dir);

      if (d > 0.0) {
        float b_sq = dot(p, p) - d * d;

        // if the ray goes inside the event horizon, it's absorbed.
        if (b_sq < rs * rs) {
          FragColor = vec4(0.0, 0.0, 0.0, 1.0); // pure black
          return;
        }

        // einstein deflection angle: alpha = 4GM / (c^2 * b) -> simplified to 2*rs/b
        float b = sqrt(b_sq);
        float alpha = (2.0 * rs) / b;

        // deflect the ray direction
        vec3 deflection_axis = normalize(cross(ray_dir, p));

        // rotate ray_dir around the deflection axis using Rodrigues' rotation formula
        ray_dir = mix(ray_dir, cross(deflection_axis, ray_dir), sin(alpha)) + ray_dir * (cos(alpha) - 1.0) * 0.5;
        ray_dir = normalize(ray_dir);
      }
    }
  }

  // 3. MARCH THE (POSSIBLY BENT) RAY THROUGH THE SCENE
  vec3 color = ray_march(ray_origin, ray_dir);

  // 4. IF NO OBJECTS WERE HIT, CHECK FOR ACCRETION DISK
  if (color == vec3(0.0)) { // assuming ray_march returns black for no hit
    bool disk_hit = false;
    for (int i = 0; i < num_bodies; i++) {
      if (bodies[i].is_black_hole == 1) {
        vec3 disk_normal = normalize(cross(bodies[i].position, vec3(0, 1, 0.5)));
        float disk_radius = bodies[i].radius * 5.0;
        float event_horizon_radius = bodies[i].radius * 0.5;

        float denom = dot(ray_dir, disk_normal);
        if (abs(denom) > 0.001) { // ray is not parallel to disk
          float t = dot(bodies[i].position - ray_origin, disk_normal) / denom;
          if (t > 0.0) {
            vec3 hit_pos = ray_origin + ray_dir * t;
            float dist_from_center = distance(hit_pos, bodies[i].position);

            if (dist_from_center < disk_radius && dist_from_center > event_horizon_radius) {
              color = accretion_disk_color(hit_pos, bodies[i].position);
              disk_hit = true;
              break; // stop at the first disk hit
            }
          }
        }
      }
    }
    // 5. IF STILL NO HIT, DRAW STARFIELD
    if (!disk_hit) {
      float stars = step(0.99995, fract(sin(fragCoord.x * 723.13 + fragCoord.y * 329.41) * 983.17));
      color = vec3(stars);
    }
  }

  FragColor = vec4(color, 1.0);
}

vec3 ray_march(vec3 ro, vec3 rd) {
  float total_dist = 0.0;
  for (int i = 0; i < MAX_STEPS; i++) {
    vec3 p = ro + total_dist * rd;
    float dist = scene_distance(p);
    if (dist < SURF_DIST) {
      // we hit something, now determine its color
      int body_index = -1;
      float min_dist = MAX_DIST;
      for (int j = 0; j < num_bodies; j++) {
        if (bodies[j].is_black_hole == 1)
          continue;
        float d = sphere_distance(p, bodies[j].position, bodies[j].radius);
        if (d < min_dist) {
          min_dist = d;
          body_index = j;
        }
      }

      if (body_index < 0)
        return vec3(0.0); // should not happen, but safe

      vec3 color = bodies[body_index].color;
      if (body_index == 0)
        return color; // sun is always lit

      if (lighting_enabled) {
        vec3 normal      = estimate_normal(p);
        vec3 light_dir   = normalize(bodies[0].position - p);
        float diff       = max(dot(normal, light_dir), 0.0);
        vec3 view_dir    = normalize(ro - p);
        vec3 reflect_dir = reflect(-light_dir, normal);
        float spec       = pow(max(dot(view_dir, reflect_dir), 0.0), 32.0);
        float ambient    = 0.1;
        return color * (ambient + diff) + vec3(0.8) * spec;
      } else {
        return color;
      }
    }
    total_dist += dist;
    if (total_dist >= MAX_DIST) {
      return vec3(0.0); // hit nothing, return black
    }
  }
  return vec3(0.0); // hit nothing, return black
}

float hash(vec2 p) {
  return fract(sin(dot(p, vec2(12.9898, 78.233))) * 43758.5453123);
}

vec3 accretion_disk_color(vec3 hit_pos, vec3 black_hole_pos) {
  vec3 p = hit_pos - black_hole_pos;
  float r = length(p) * 0.2;
  float angle = atan(p.y, p.x);

  float noise = hash(vec2(r * 3.0, angle * 6.0)) * 0.6 +
                hash(vec2(r * 12.0, angle * 24.0)) * 0.4;

  vec3 color;
  if (r < 0.3) {
    color = mix(vec3(0.8, 0.9, 1.1), vec3(1.0, 0.8, 0.6), smoothstep(0.0, 0.3, r));
  } else {
    float t = smoothstep(0.3, 1.0, r);
    color = mix(vec3(1.0, 0.5, 0.2),
                vec3(1.0, 0.2, 0.1),
                pow(t, 1.5));
  }

  color *= (0.7 + 0.3 * noise) * pow(1.0 - r, 0.5);

  vec3 disk_normal = normalize(cross(p, vec3(0, 1, 0)));
  float doppler = 1.0 + 0.5 * dot(normalize(p), disk_normal);
  color.rgb *= mix(1.0, doppler, 0.8);

  return pow(color * 1.5, vec3(1.3));
}
