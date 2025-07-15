#version 330 core

in vec2 fragCoord;
out vec4 FragColor;

uniform vec3 camera_pos;
uniform vec3 camera_front;
uniform vec3 camera_up;
uniform vec3 camera_right;
uniform float aspect_ratio;
uniform int num_bodies;

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

vec3 ray_march(vec3 ro, vec3 rd) {
    float total_dist = 0.0;
    for (int i = 0; i < MAX_STEPS; i++) {
        vec3 p = ro + total_dist * rd;
        float dist = scene_distance(p);
        if (dist < SURF_DIST) {
            break;
        }
        total_dist += dist;
        if (total_dist >= MAX_DIST) {
            break;
        }
    }

    if (total_dist >= MAX_DIST) {
        float stars = step(0.99995, fract(sin(fragCoord.x * 723.13 + fragCoord.y * 329.41) * 983.17));
        return vec3(stars);
    }

    vec3 p = ro + total_dist * rd;

    for (int i = 0; i < num_bodies; i++) {
        float d = sphere_distance(p, bodies[i].position, bodies[i].radius);
        if (abs(d) < SURF_DIST) {
            return bodies[i].color;
        }
    }

    return vec3(0.0);
}

void main() {
    vec2 uv = (fragCoord * 2.0 - 1.0) * vec2(aspect_ratio, 1.0);
    vec3 ray_dir = normalize(camera_front + uv.x * camera_right + uv.y * camera_up);
    vec3 color = ray_march(camera_pos, ray_dir);

    FragColor = vec4(color, 1.0);
}
