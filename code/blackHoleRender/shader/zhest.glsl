#version 420 core

const float PI = 3.1415926535f;

const float poleMargin  = 1e-2f;
const float poleStep    = 1e-4f;
const float timeStep    = 1e-3f;

const int marchStepsNo  =  30  ;
const float ths         =  1e-2;
const float mdTau       =  1e-4;
const float tauStep     =  1e-3;
const float maxDist     =  10000;


layout(location = 0) flat in float time;
layout(location = 1) flat in vec2 resolution;
layout(location = 2) flat in vec3 camPos;
layout(location = 3) flat in vec3 mouse;
layout(location = 4) flat in float FOV;
layout(location = 5) in vec2 uv;

float t = time;

uniform sampler2D uTexture;

layout(location = 0) out vec4 fragColor;

const float Rs      =   1;
const float spin    =   0;
const float maxDisk =   0.7;

struct State
{
    vec3 r;
    vec3 v;
};

mat4 rotateOY(float theta) 
{
    float c = cos(theta);
    float s = sin(theta);

    return mat4
    (
        vec4( c, 0, s, 0),
        vec4( 0, 1, 0, 0),
        vec4(-s, 0, c, 0),
        vec4( 0, 0, 0, 1)
    );
}

mat4 rotateOX(float theta) 
{
    float c = cos(theta);
    float s = sin(theta);

    return mat4
    (
        vec4(1, 0,  0, 0),
        vec4(0, c, -s, 0),
        vec4(0, s,  c, 0),
        vec4(0, 0,  0, 1)
    );
}

mat4 followMouse()
{
    return rotateOX(-mouse.y * 3) 
         * rotateOY( mouse.x * 3);
}

vec3 toSpherical(vec3 cart)
{
    float r   = length(cart);
    float rxz = length(cart.xz);
    float theta = atan(rxz   , cart.y);
    float phi   = atan(cart.z, cart.x);

    return vec3(r, theta, phi);
}

vec3 toSphericalVector(vec3 origin, vec3 dir)
{
    float r = length(origin);
    float rxz = length(origin.xz);

    mat3 L = mat3
    (
        origin.x / r,                        // dr/dx
        origin.y / r,                        // dr/dy
        origin.z / r,                        // dr/dz
        origin.x * origin.y / (r * r * rxz), // dtheta/dx
        -rxz / (r * r),                      // dtheta/dy
        origin.z* origin.y / (r * r * rxz),  // dtheta/dz
        -origin.z / rxz,                     // dphi/dx
        0.0,                                 // dphi/dy
        origin.x / rxz                       // dphi/dz
    );

    return L * dir;
}

// Kerr parameter
float a = spin * Rs / 2.f;

// Calculate change in k along geodesic affine parameter
void CalculateGeodesicDerivative(vec3 x, vec3 u, out vec3 dx, out vec3 du)
{
    // Convert spin factor to Kerr parameter
    float a = spin * Rs / 2.0;

    // Read coordinate values
    float r = x.x;
    float Rs = Rs;
    float th = x.y;
    float sth = sin(th);
    float cth = cos(th);

    // Calculate length scales
    float sig = r * r + a * a * cth * cth;
    float del = r * r - Rs * r + a * a;

    // Calculate metric components
    float gtt = -(1.0 - (Rs * r) / sig);
    float bph = -Rs * r * a * sth * sth / sig;
    float ypp = (r * r + a * a + Rs * r * a * a * sth * sth / sig) * sth * sth;

    // Calculate lapse function and energy
    float alpha = sqrt((bph * bph / ypp) - gtt);
    float u0 = sqrt((del * u.x * u.x / sig) + (u.y * u.y / sig) + (u.z * u.z / ypp)) / alpha;

    // Calculate derivatives of metric
    float dyrdr = (sig * (2.0 * r - Rs) - 2.0 * r * del) / (sig * sig);
    float dyrdt = -2.0 * a * a * cth * sth * del / (sig * sig);
    float dytdr = -2.0 * r / (sig * sig);
    float dytdt = 2.0 * a * a * cth * sth / (sig * sig);
    float dypdr_lower = (2.0 * r + Rs * a * a * sth * sth * (sig - (2.0 * r * r)) / (sig * sig)) * sth * sth;
    float dypdr_upper = -dypdr_lower / (ypp * ypp);
    float dypdt_lower = (Rs * r * a * a / (sig * sig)) * (2.0 * cth * sth * sth * sth) * (2.0 * sig + a * a * sth * sth) + (r * r + a * a) * 2.0 * cth * sth;
    float dypdt_upper = -dypdt_lower / (ypp * ypp);
    float dbpdr_lower = -Rs * a * sth * sth * (sig - 2.0 * r * r) / (sig * sig);
    float dbpdr_upper = (ypp * dbpdr_lower - dypdr_lower * bph) / (ypp * ypp);
    float dbpdt_lower = (-Rs * r * a / (sig * sig)) * (2.0 * sth * cth * sig + 2.0 * a * a * cth * sth * sth * sth);
    float dbpdt_upper = (ypp * dbpdt_lower - dypdt_lower * bph) / (ypp * ypp);
    float dbpsqdr = (2.0 * bph * dbpdr_lower * ypp - bph * bph * dypdr_lower) / (ypp * ypp);
    float dbpsqdt = (2.0 * bph * dbpdt_lower * ypp - bph * bph * dypdt_lower) / (ypp * ypp);
    float dgdr = Rs * (sig - 2.0 * r * r) / (sig * sig);
    float dgdt = 2.0 * Rs * r * a * a * cth * sth / (sig * sig);

    // Calculate derivatives of position
    dx.x = (del / sig) * (u.x / u0);
    dx.y = (1.0 / sig) * (u.y / u0);
    dx.z = (1.0 / ypp) * ((u.z / u0) - bph);

    // Calculate derivatives of velocity
    du.x = (u0 / 2.0) * (dgdr - dbpsqdr);
    du.x += u.z * (dbpdr_upper);
    du.x -= (u.x * u.x * 0.5 / u0) * (dyrdr);
    du.x -= (u.y * u.y * 0.5 / u0) * (dytdr);
    du.x -= (u.z * u.z * 0.5 / u0) * (dypdr_upper);
    du.y = (u0 / 2.0) * (dgdt - dbpsqdt);
    du.y += u.z * (dbpdt_upper);
    du.y -= (u.x * u.x * 0.5 / u0) * (dyrdt);
    du.y -= (u.y * u.y * 0.5 / u0) * (dytdt);
    du.y -= (u.z * u.z * 0.5 / u0) * (dypdt_upper);
    du.z = 0.0;

    return;
}

// Runge-Kutta 4th Order integration
State RK4Step(float tStep, vec3 x, vec3 u)
{
    // Calculate k-factoRs
    vec3 dx1, du1, dx2, du2, dx3, du3, dx4, du4;
    CalculateGeodesicDerivative(x, u, dx1, du1);
    CalculateGeodesicDerivative(x + dx1 * (tStep / 2.0), u + du1 * (tStep / 2.0), dx2, du2);
    CalculateGeodesicDerivative(x + dx2 * (tStep / 2.0), u + du2 * (tStep / 2.0), dx3, du3);
    CalculateGeodesicDerivative(x + dx3 * tStep,         u + du3 * tStep,         dx4, du4);

    return State
    (
        x + (tStep / 6.0) * (dx1 + 2.0 * dx2 + 2.0 * dx3 + dx4),
        u + (tStep / 6.0) * (du1 + 2.0 * du2 + 2.0 * du3 + du4)
    );
}

// Generate affine parameter step size
float CalculateStepSize(vec3 x, vec3 u)
{
    float Rs = Rs;

    // Check if near singular points
    if (x.y < poleMargin * PI || x.y > (1.0 - poleMargin) * PI)
    {
        return poleStep;
    }
    
    // Check if near horizon
    if (abs(x.x) < 2.0 * Rs)
    {
        // Near horizon regime
        return timeStep;
    }

    // Check if receding
    if (u.x * x.x > 0.0)
    {
        // Far from horizon, receding
        return max(timeStep * x.x * x.x, timeStep);
    }
    else
    {
        // Far from horizon, approaching
        return max(timeStep * (abs(x.x) - (2.0 * Rs)), timeStep);
        }
}

// Check if geodesic inevitably crosses horizon
bool HorizonCheck(vec3 x, vec3 u)
{
    // Check if inside of photon sphere
    if (abs(x.x) < 1.5 * Rs) {
        return true;
    }


    // Check if stuck near poles
    if (x.y < poleMargin * PI || (x.y > (1.0 - poleMargin) * PI))
    {
        return true;
    }

    // Check if stuck in orbit
    if (abs(x.x) < 2.0 * Rs) {

        // Pre-calculate factoRs
        float r = x.x;
        float Rs = Rs;
        float th = x.y;
        float sth = sin(th);
        float A = 1.0 - (Rs / r);
        float a = sqrt(A);

        // Calculate motion constants
        float E = sqrt((A * u.x * u.x) + (u.y * u.y / (r * r)) + (u.z * u.z / (r * r * sth * sth))) / a;
        float C = u.z * u.z / (E * E);

        // Check if geodesic originates inside of horizon
        if (C > (27.0 / 4.0) * Rs * Rs) {
            return true;
        }
        else {
            return (x.x * u.x < 0.0);
        }
    }

    // Otherwise, keep going
    return false;
}


vec4 rayMarch(vec3 pos, vec3 vel) 
{
    State start = State
    (
        toSpherical      (pos     ), 
        toSphericalVector(pos, vel)
    );

    State curr = start;
    State next = start;

	for (int steps = 0; steps < marchStepsNo; ++steps) 
    {
        next = RK4Step(CalculateStepSize(curr.r, curr.v), curr.r, curr.v);
    
        if(HorizonCheck(next.r, next.v))
        {
           return followMouse() * vec4(0.f , 0.f , 0.f, 1.f);
        }

        if(abs(next.r.x) > maxDist * Rs)
           return vec4(0.5f, 0.2f, 1.f, 1.f);
        
        curr = next;
	}
    
    return vec4(0.5, 0.5, 0.5, 1.f);
}

vec3 rayDirection(float fov, vec2 size, vec2 fragCoord)
{
    vec2 xy = fragCoord - size / 2.0;
    float z = size.y / tan(radians(fov) / 2.0);
    
    return normalize(vec3(xy, z));
}


void main()
{	
    vec3 dir = rayDirection(FOV + 20 * mouse.z, resolution, gl_FragCoord.xy); 
	vec3 eye = vec3(0, 0, -4 - mouse.z);     

    fragColor = rayMarch(eye, dir);
}
