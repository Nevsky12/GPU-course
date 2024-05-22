#version 420 core

layout(location = 0) flat out float time;
layout(location = 1) flat out vec2  resolution;
layout(location = 2) flat out vec3  camPos;
layout(location = 3) flat out vec3  mouse;
layout(location = 4) flat out float ctgFOV;
layout(location = 5) out vec2  uv;

uniform float uTime;
uniform float uAspectRatio;
uniform vec3  uMouse;
uniform vec2  uResolution;

mat4 lookAt(vec3 eye, vec3 at, vec3 up)
{
    vec3 z = normalize(eye - at);
    vec3 x = normalize(cross(up, z));
    vec3 y = cross(z, x);
    return mat4
    (
        vec4(x.x, y.x, z.x, 0.f),
        vec4(x.y, y.y, z.y, 0.f),
        vec4(x.z, y.z, z.z, 0.f),
        vec4(-eye * mat3(x, y, z), 1.f)
    );
}

void main()
{
    // projection parameters:
    float ctgFOV = 1.5f;
    float f = 10.f;
    float n = 0.1f;

    float C1 =      (f + n) / (f - n);
    float C2 = 2.f * f * n  / (f - n);

    mat4 proj = mat4
    (
         ctgFOV / uAspectRatio, 0.f   , 0.f,  0.f,
         0.f                  , ctgFOV, 0.f,  0.f,
         0.f                  , 0.f   , -C1, -1.f,
         0.f                  , 0.f   , -C2,  0.f
    );

    float pi = 3.1415926535f;
    float phi   =        pi * uMouse.x;
    float theta = 0.5f * pi * uMouse.y;
    vec3 uCamPos = uMouse.z * vec3
    (
        cos(theta) * cos(phi),
        cos(theta) * sin(phi),
        sin(theta)
    );

    vec3 p[] = vec3[]
    (
        vec3( 1.f,  1.f, 0.f),
        vec3( 1.f, -1.f, 0.f),
        vec3(-1.f, -1.f, 0.f),
        vec3(-1.f,  1.f, 0.f)
    );
    int idx[] = int[]
    (
        0, 1, 3,
        1, 2, 3
    );

    gl_Position = vec4(p[idx[gl_VertexID]], 1.f); 

    camPos     = uCamPos;
    time       = uTime;
    resolution = uResolution;
    mouse      = uMouse;
    ctgFOV     = ctgFOV;

    uv         = 0.5f + 0.5f * gl_Position.xy;
}

