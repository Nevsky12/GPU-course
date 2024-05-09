#version 330 core
out vec3 pos;
out vec3 fractalColor;
out float needColor;

uniform float uTime;
uniform float uAspectRatio;
uniform int miniPEKKASize; 
uniform vec3 uMouse;

layout (location = 0) in vec3 aPos;

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
         ctgFOV / uAspectRatio, 0.f, 0.f, 0.f,
         0.f, ctgFOV, 0.f, 0.f,
         0.f, 0.f, -C1, -1.f,
         0.f, 0.f, -C2,  0.f
    );

    float pi = 3.1415926535f;
    float phi   =        pi * uMouse.x;
    float theta = 0.5f * pi * uMouse.y;
    vec3 camPos = uMouse.z * vec3
    (
        cos(theta) * cos(phi),
        cos(theta) * sin(phi),
        sin(theta)
    );
    mat4 R = lookAt(camPos, vec3(0.25f), vec3(0.f, 0.f, 1.f));
   
    gl_PointSize = 2;

    if(gl_VertexID < miniPEKKASize) // King dream
    {
       fractalColor = vec3( 1.f, 0.5f,  1.f);
       needColor = -1.f;
    }
    if(gl_VertexID > miniPEKKASize && gl_VertexID < miniPEKKASize * 2) // tinkerbell
    {
       fractalColor = vec3(0.7f, 0.2f, 0.2f);
       needColor = -1.f;
    }
    if(gl_VertexID > miniPEKKASize * 2) // gigachad
    {
        fractalColor = vec3(1.f, 1.f, 1.f);
        needColor = 1.f;
    }

    pos = aPos;
    gl_Position = proj * R * vec4(aPos, 1.f);
}
