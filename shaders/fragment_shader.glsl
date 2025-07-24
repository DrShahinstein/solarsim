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

struct CelestialBody {
    vec3 position;
    float radius;
    vec3 color;
};

const int MAX_BODIES = 20;
uniform CelestialBody bodies[MAX_BODIES];

const int MAX_STEPS = 100;
const float MAX_DIST = 100.0;
const float SURF_DIST = 0.001;

float sphere_distance(vec3 p, vec3 center, float radius) {
    return length(p - center) - radius;
}

float scene_distance(vec3 p) {
    float min_dist = MAX_DIST;

    for (int i = 0; i < num_bodies; i++) {
        float d = sphere_distance(p, bodies[i].position, bodies[i].radius);
        min_dist = min(min_dist, d);
    }

    return min_dist;
}

vec3 estimate_normal(vec3 p) {
    const float eps = 0.001;
    vec3 n = vec3(
        scene_distance(vec3(p.x + eps, p.y, p.z)) - scene_distance(vec3(p.x - eps, p.y, p.z)),
        scene_distance(vec3(p.x, p.y + eps, p.z)) - scene_distance(vec3(p.x, p.y - eps, p.z)),
        scene_distance(vec3(p.x, p.y, p.z + eps)) - scene_distance(vec3(p.x, p.y, p.z - eps))
    );
    return normalize(n);
}

vec3 ray_march(vec3 ro, vec3 rd) {
    float total_dist = 0.0;
    vec3 p = ro;

    for (int i = 0; i < MAX_STEPS; i++) {
        p = ro + total_dist * rd;
        float dist = scene_distance(p);
        if (dist < SURF_DIST) {
            break;
        }
        total_dist += dist;
        if (total_dist >= MAX_DIST) {
            float stars = step(0.99995, fract(sin(fragCoord.x * 723.13 + fragCoord.y * 329.41) * 983.17));
            return vec3(stars);
        }
    }

    if (total_dist >= MAX_DIST) {
        return vec3(0.0);
    }

    int body_index = -1;
    float min_dist = MAX_DIST;
    for (int i = 0; i < num_bodies; i++) {
        float d = sphere_distance(p, bodies[i].position, bodies[i].radius);
        if (d < min_dist) {
            min_dist = d;
            body_index = i;
        }
    }

    if (body_index < 0) {
        return vec3(1.0, 0.0, 1.0);
    }
    
    vec3 color = bodies[body_index].color;
    if (body_index == 0) return color; // sun is always lit

    if (lighting_enabled) {
      vec3 normal      = estimate_normal(p);
      vec3 light_dir   = normalize(bodies[0].position - p);
      float diff       = max(dot(normal, light_dir), 0.0);
      vec3 view_dir    = normalize(ro - p);
      vec3 reflect_dir = reflect(-light_dir, normal);
      float spec       = pow(max(dot(view_dir, reflect_dir), 0.0), 32.0);
      float ambient    = 0.1;
      return color * (ambient + diff) + vec3(0.8) * spec;
    } else return color;
}


void main() {
    vec2 uv = (fragCoord * 2.0 - 1.0) * vec2(aspect_ratio, 1.0);
    vec3 ray_dir = normalize(camera_front + uv.x * camera_right + uv.y * camera_up);
    vec3 color = ray_march(camera_pos, ray_dir);

    FragColor = vec4(color, 1.0);
}
