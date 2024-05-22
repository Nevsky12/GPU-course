#version 420 core
const float PI = 3.14159265359;
const float EPSILON = 0.0001;
const float INFINITY = 1000000.0;

out vec4 fragColor;

layout(location = 0) flat in float time;
layout(location = 1) flat in vec2 resolution;
layout(location = 2) flat in vec3 camPos;
layout(location = 3) flat in vec3 mouse;
layout(location = 4) flat in float FOV;
layout(location = 5) in vec2 uv;

uniform samplerCube galaxy;
uniform sampler2D colorMap;

uniform float adiskEnabled = 1.0;
uniform float adiskParticle = 1.0;
uniform float adiskHeight = 0.2;
uniform float adiskLit = 0.5;
uniform float adiskDensityV = 1.0;
uniform float adiskDensityH = 1.0;
uniform float adiskNoiseScale = 1.0;
uniform float adiskNoiseLOD = 5.0;
uniform float adiskSpeed = 0.5;


// Step size parameters
float timeStep = 1e-4;
float poleMargin = 1e-2;
float poleStep = 1e-4;

float escapeDistance = 10000;
float horizonRadius = 1;
float spinFactor = 0.0;
float diskMax = 1.2;

vec4 permute(vec4 x) { return mod(((x * 34.0) + 1.0) * x, 289.0); }
vec4 taylorInvSqrt(vec4 r) { return 1.79284291400159 - 0.85373472095314 * r; }

float snoise(vec3 v) {
  const vec2 C = vec2(1.0 / 6.0, 1.0 / 3.0);
  const vec4 D = vec4(0.0, 0.5, 1.0, 2.0);

  // First corner
  vec3 i = floor(v + dot(v, C.yyy));
  vec3 x0 = v - i + dot(i, C.xxx);

  // Other corners
  vec3 g = step(x0.yzx, x0.xyz);
  vec3 l = 1.0 - g;
  vec3 i1 = min(g.xyz, l.zxy);
  vec3 i2 = max(g.xyz, l.zxy);

  //  x0 = x0 - 0. + 0.0 * C
  vec3 x1 = x0 - i1 + 1.0 * C.xxx;
  vec3 x2 = x0 - i2 + 2.0 * C.xxx;
  vec3 x3 = x0 - 1. + 3.0 * C.xxx;

  // Permutations
  i = mod(i, 289.0);
  vec4 p = permute(permute(permute(i.z + vec4(0.0, i1.z, i2.z, 1.0)) + i.y +
                           vec4(0.0, i1.y, i2.y, 1.0)) +
                   i.x + vec4(0.0, i1.x, i2.x, 1.0));

  // Gradients
  // ( N*N points uniformly over a square, mapped onto an octahedron.)
  float n_ = 1.0 / 7.0; // N=7
  vec3 ns = n_ * D.wyz - D.xzx;

  vec4 j = p - 49.0 * floor(p * ns.z * ns.z); //  mod(p,N*N)

  vec4 x_ = floor(j * ns.z);
  vec4 y_ = floor(j - 7.0 * x_); // mod(j,N)

  vec4 x = x_ * ns.x + ns.yyyy;
  vec4 y = y_ * ns.x + ns.yyyy;
  vec4 h = 1.0 - abs(x) - abs(y);

  vec4 b0 = vec4(x.xy, y.xy);
  vec4 b1 = vec4(x.zw, y.zw);

  vec4 s0 = floor(b0) * 2.0 + 1.0;
  vec4 s1 = floor(b1) * 2.0 + 1.0;
  vec4 sh = -step(h, vec4(0.0));

  vec4 a0 = b0.xzyw + s0.xzyw * sh.xxyy;
  vec4 a1 = b1.xzyw + s1.xzyw * sh.zzww;

  vec3 p0 = vec3(a0.xy, h.x);
  vec3 p1 = vec3(a0.zw, h.y);
  vec3 p2 = vec3(a1.xy, h.z);
  vec3 p3 = vec3(a1.zw, h.w);

  // Normalise gradients
  vec4 norm =
      taylorInvSqrt(vec4(dot(p0, p0), dot(p1, p1), dot(p2, p2), dot(p3, p3)));
  p0 *= norm.x;
  p1 *= norm.y;
  p2 *= norm.z;
  p3 *= norm.w;

  // Mix final noise value
  vec4 m =
      max(0.6 - vec4(dot(x0, x0), dot(x1, x1), dot(x2, x2), dot(x3, x3)), 0.0);
  m = m * m;
  return 42.0 *
         dot(m * m, vec4(dot(p0, x0), dot(p1, x1), dot(p2, x2), dot(p3, x3)));
}



vec3 rayDirection(float fov, vec2 size, vec2 fragCoord)
{
    vec2 xy = fragCoord - size / 2.0;
    float z = size.y / tan(radians(fov) / 2.0);
    
    return normalize(vec3(xy, z));
}

mat3 lookAt(vec3 origin, vec3 target, float roll) {
  vec3 rr = vec3(sin(roll), cos(roll), 0.0);
  vec3 ww = normalize(target - origin);
  vec3 uu = normalize(cross(ww, rr));
  vec3 vv = normalize(cross(uu, ww));

  return mat3(uu, vv, ww);
}

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

vec3 followMouse(vec3 pos)
{
    return vec4
    (
        rotateOX(-mouse.y * 3) 
      * rotateOY( mouse.x * 3)
      * vec4(pos, 1)
    ).xyz;
}

vec3 accel(float h2, vec3 pos) {
  float r2 = dot(pos, pos);
  float r5 = pow(r2, 2.5);
  vec3 acc = -1.5 * h2 * pos / r5 * 1.0;
  return acc;
}

vec3 toSpherical(vec3 p) {
  float rho = sqrt((p.x * p.x) + (p.y * p.y) + (p.z * p.z));
  float theta = atan(p.z, p.x);
  float phi = asin(p.y / rho);
  return vec3(rho, theta, phi);
}

vec3 toSpherical2(vec3 pos) {
  vec3 radialCoords;
  radialCoords.x = length(pos) * 1.5 + 0.55;
  radialCoords.y = atan(-pos.x, -pos.z) * 1.5;
  radialCoords.z = abs(pos.y);
  return radialCoords;
}

vec3 ToSphericalVector(vec3 origin, vec3 dir)
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


// Calculate change in k along geodesic affine parameter
void CalculateGeodesicDerivative(vec3 x, vec3 u, out vec3 dx, out vec3 du)
{
    // Convert spin factor to Kerr parameter
    float a = spinFactor * horizonRadius / 2.0;

    // Read coordinate values
    float r = x.x;
    float rs = horizonRadius;
    float th = x.y;
    float sth = sin(th);
    float cth = cos(th);

    // Calculate length scales
    float sig = r * r + a * a * cth * cth;
    float del = r * r - rs * r + a * a;

    // Calculate metric components
    float gtt = -(1.0 - (rs * r) / sig);
    float bph = -rs * r * a * sth * sth / sig;
    float ypp = (r * r + a * a + rs * r * a * a * sth * sth / sig) * sth * sth;

    // Calculate lapse function and energy
    float alpha = sqrt((bph * bph / ypp) - gtt);
    float u0 = sqrt((del * u.x * u.x / sig) + (u.y * u.y / sig) + (u.z * u.z / ypp)) / alpha;

    // Calculate derivatives of metric
    float dyrdr = (sig * (2.0 * r - rs) - 2.0 * r * del) / (sig * sig);
    float dyrdt = -2.0 * a * a * cth * sth * del / (sig * sig);
    float dytdr = -2.0 * r / (sig * sig);
    float dytdt = 2.0 * a * a * cth * sth / (sig * sig);
    float dypdr_lower = (2.0 * r + rs * a * a * sth * sth * (sig - (2.0 * r * r)) / (sig * sig)) * sth * sth;
    float dypdr_upper = -dypdr_lower / (ypp * ypp);
    float dypdt_lower = (rs * r * a * a / (sig * sig)) * (2.0 * cth * sth * sth * sth) * (2.0 * sig + a * a * sth * sth) + (r * r + a * a) * 2.0 * cth * sth;
    float dypdt_upper = -dypdt_lower / (ypp * ypp);
    float dbpdr_lower = -rs * a * sth * sth * (sig - 2.0 * r * r) / (sig * sig);
    float dbpdr_upper = (ypp * dbpdr_lower - dypdr_lower * bph) / (ypp * ypp);
    float dbpdt_lower = (-rs * r * a / (sig * sig)) * (2.0 * sth * cth * sig + 2.0 * a * a * cth * sth * sth * sth);
    float dbpdt_upper = (ypp * dbpdt_lower - dypdt_lower * bph) / (ypp * ypp);
    float dbpsqdr = (2.0 * bph * dbpdr_lower * ypp - bph * bph * dypdr_lower) / (ypp * ypp);
    float dbpsqdt = (2.0 * bph * dbpdt_lower * ypp - bph * bph * dypdt_lower) / (ypp * ypp);
    float dgdr = rs * (sig - 2.0 * r * r) / (sig * sig);
    float dgdt = 2.0 * rs * r * a * a * cth * sth / (sig * sig);

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
void RK4Step(float tStep, inout vec3 x, inout vec3 u)
{
    // Calculate k-factors
    vec3 dx1, du1, dx2, du2, dx3, du3, dx4, du4;
    CalculateGeodesicDerivative(x, u, dx1, du1);
    CalculateGeodesicDerivative(x + dx1 * (tStep / 2.0), u + du1 * (tStep / 2.0), dx2, du2);
    CalculateGeodesicDerivative(x + dx2 * (tStep / 2.0), u + du2 * (tStep / 2.0), dx3, du3);
    CalculateGeodesicDerivative(x + dx3 * tStep,         u + du3 * tStep,         dx4, du4);

    // Calculate full update
    x += (tStep / 6.0) * (dx1 + 2.0 * dx2 + 2.0 * dx3 + dx4);
    u += (tStep / 6.0) * (du1 + 2.0 * du2 + 2.0 * du3 + du4);
}




// Generate affine parameter step size
float CalculateStepSize(vec3 x, vec3 u)
{
    float rs = horizonRadius;

    // Check if near singular points
    if (x.y < poleMargin * PI || x.y > (1.0 - poleMargin) * PI)
    {
        return poleStep;
    }

    // Check if near horizon
    if (abs(x.x) < 2.0 * rs)
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
        return max(timeStep * (abs(x.x) - (2.0 * rs)), timeStep);
    }
}

// Check if geodesic inevitably crosses horizon
bool HorizonCheck(vec3 x, vec3 u)
{
    // Check if inside of photon sphere
    if (abs(x.x) < 1.5 * horizonRadius) {
        return true;
    }

    // Check if stuck near poles
    if (x.y < poleMargin * PI || (x.y > (1.0 - poleMargin) * PI))
    {
        return true;
    }

    // Check if stuck in orbit
    if (abs(x.x) < 2.0 * horizonRadius) {

        // Pre-calculate factors
        float r = x.x;
        float rs = horizonRadius;
        float th = x.y;
        float sth = sin(th);
        float A = 1.0 - (rs / r);
        float a = sqrt(A);

        // Calculate motion constants
        float E = sqrt((A * u.x * u.x) + (u.y * u.y / (r * r)) + (u.z * u.z / (r * r * sth * sth))) / a;
        float C = u.z * u.z / (E * E);

        // Check if geodesic originates inside of horizon
        if (C > (27.0 / 4.0) * rs * rs) {
            return true;
        }
        else {
            return (x.x * u.x < 0.0);
        }
    }

    // Otherwise, keep going
    return false;
}


vec3 traceColor(vec3 pos, vec3 dir) 
{
  vec3 color = vec3(0.0);
  float alpha = 1.0;

  float STEP_SIZE = 0.2;
  dir *= STEP_SIZE;

  // Initial values
  vec3 h = cross(pos, dir);
  float h2 = dot(h, h);

  vec3 x  = toSpherical(pos);
  vec3 x1 = x  ;

  vec3 u = ToSphericalVector(pos, dir);

  for (int i = 0; i < 200; i++) 
  {
       float dt = CalculateStepSize(x, u);
       RK4Step(dt, x1, u); 
       
       //vec3 acc = accel(h2, pos);
       //dir += acc;

      // Reach event horizon
      if (dot(x.x, x.x) < pow(horizonRadius, 2)) 
      {
          return vec3(1);
      }

      //adiskColor(pos, dir, color, alpha);

       x = x1;

      //pos += dir;
  }

  // Sample skybox color
  color += texture(galaxy, dir).rgb * alpha * 10;
  return color;
}



void main() 
{
    vec3 dir = followMouse(rayDirection(FOV + 20 * mouse.z, resolution, gl_FragCoord.xy)); 
	vec3 eye = vec3(0, 0, -1 - mouse.z);   

    fragColor.rgb = traceColor(camPos, dir); 
}


void adiskColor(vec3 pos, vec3 dir, inout vec3 color, inout float alpha) {
  float innerRadius = 2.0;
  float outerRadius = 6.0;

  // Density linearly decreases as the distance to the blackhole center
  // increases.
  float density = max(
      0.0, 1.0 - length(pos.xyz / vec3(outerRadius, adiskHeight, outerRadius)));
  if (density < 0.001) {
    return;
  }

  density *= pow(1.0 - abs(pos.y) / adiskHeight, adiskDensityV);

  // Set particale density to 0 when radius is below the inner most stable
  // circular orbit.
  density *= smoothstep(innerRadius, innerRadius * 1.1, length(pos));

  // Avoid the shader computation when density is very small.
  if (density < 0.001) {
    return;
  }

  vec3 sphericalCoord = toSpherical(pos);

  // Scale the rho and phi so that the particales appear to be at the correct
  // scale visually.
  sphericalCoord.y *= 2.0;
  sphericalCoord.z *= 4.0;

  density *= 1.0 / pow(sphericalCoord.x, adiskDensityH);
  density *= 16000.0;

  if (adiskParticle < 0.5) {
    color += vec3(0.0, 1.0, 0.0) * density * 0.02;
    return;
  }

  float noise = 1.0;
  for (int i = 0; i < int(adiskNoiseLOD); i++) {
    noise *= 0.5 * snoise(sphericalCoord * pow(i, 2) * adiskNoiseScale) + 0.5;
    if (i % 2 == 0) {
      sphericalCoord.y += time * adiskSpeed;
    } else {
      sphericalCoord.y -= time * adiskSpeed;
    }
  }

  vec3 dustColor = vec3(1 / abs(sphericalCoord.x)); //texture(colorMap, vec2(sphericalCoord.x / outerRadius, 0.5)).rgb;

  color += density * adiskLit * alpha * abs(noise) * dustColor;
}
