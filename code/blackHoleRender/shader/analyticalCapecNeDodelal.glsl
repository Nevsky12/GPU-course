#version 420 core

const float PI = 3.1415926535f;

const int marchStepsNo  =  30  ;
const float ths         =  1e-2;
const float mdTau       =  1e-4;
const float tauStep     =  1e-3;
const float maxDist     =  100;


layout(location = 0) flat in float time;
layout(location = 1) flat in vec2 resolution;
layout(location = 2) flat in vec3 camPos;
layout(location = 3) flat in vec3 mouse;
layout(location = 4) flat in float FOV;
layout(location = 5) in vec2 uv;

float t = time;

uniform sampler2D uTexture;

layout(location = 0) out vec4 fragColor;

const float Rs      =   5;
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



State geodesicDispls(State st)
{
    float r  = st.r.x;
    float th = st.r.y;
    vec3 u = st.v;

      // Convert spin factor to Kerr parameter
    float a = spin * Rs / 2.0;

    // Read coordinate values
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

    State displs;

    // Calculate derivatives of position
    displs.r.x = (del / sig) * (u.x / u0);
    displs.r.y = (1.0 / sig) * (u.y / u0);
    displs.r.z = (1.0 / ypp) * ((u.z / u0) - bph);

    // Calculate derivatives of velocity
    displs.v.x = (u0 / 2.0) * (dgdr - dbpsqdr);
    displs.v.x += u.z * (dbpdr_upper);
    displs.v.x -= (u.x * u.x * 0.5 / u0) * (dyrdr);
    displs.v.x -= (u.y * u.y * 0.5 / u0) * (dytdr);
    displs.v.x -= (u.z * u.z * 0.5 / u0) * (dypdr_upper);
    displs.v.y = (u0 / 2.0) * (dgdt - dbpsqdt);
    displs.v.y += u.z * (dbpdt_upper);
    displs.v.y -= (u.x * u.x * 0.5 / u0) * (dyrdt);
    displs.v.y -= (u.y * u.y * 0.5 / u0) * (dytdt);
    displs.v.y -= (u.z * u.z * 0.5 / u0) * (dypdt_upper);
    displs.v.z = 0.0;

    return displs;
}

bool crossEventHorizon(State st)
{
    float r     = st.r.x;
    float theta = st.r.y;
    vec3  v     = st.v  ;

    // inside photon sphere
    if(abs(r) < 1.5 * Rs)
       return true;

    if
    (
        theta <        ths  * PI ||
        theta > (1.0 - ths) * PI
    )
        return true;
    
    if(abs(r) < 2.0 * Rs)
    {
        float sinT  = sin(theta);
        float A  = 1.0 - Rs / r;
        float la = sqrt(A);

        // motion integrals
        float E = sqrt
        (
              pow(v.x, 2.0) * A
            + pow(v.y, 2.0) / pow(r       , 2.0)
            + pow(v.z, 2.0) / pow(r * sinT, 2.0)
        ) / la;

        float C = pow(v.z, 2) 
                / pow(E  , 2);

        if(C > 27.0 / 4.0 * pow(Rs, 2))
           return true;
        else
           return false;
    }

    return false;
}


float deltaProperTime(State st)
{
    float r     = st.r.x;
    float theta = st.r.y;
    vec3  v     = st.v  ;

    if
    (
        theta <        ths  * PI ||
        theta > (1.0 - ths) * PI
    )
        return mdTau;

    if(abs(r) < 2.0 * Rs)
       return tauStep;

    if(v.x * r > 0.0)
    {
        return max
        (
            tauStep,
            tauStep * abs(r) 
        );
    }
    else
    {
        return max
        (
            tauStep,
            tauStep * (abs(r) - 2.0 * Rs)
        );
    }
}

State RK4(State st)
{
    float dTau = deltaProperTime(st);

    t += dTau;

    State d1 = geodesicDispls
    (
        st
    );

    State d2 = geodesicDispls
    (
        State
        (
            st.r + d1.r * dTau / 2.0, 
            st.v + d1.v * dTau / 2.0
        )
    );

    State d3 = geodesicDispls
    (
        State
        (
            st.r + d2.r * dTau / 2.0,
            st.v + d2.v * dTau / 2.0
        )
    );

    State d4 = geodesicDispls
    (
        State
        (
            st.r + d3.r * dTau,
            st.v + d3.v * dTau
        )
    );

    return State
    (
        st.r + dTau / 6.0 * (d1.r + 2.0 * d2.r + 2.0 * d3.r + d4.r),
        st.v + dTau / 6.0 * (d1.v + 2.0 * d2.v + 2.0 * d3.v + d4.v)
    );
}


vec4 rayMarch(vec3 pos, vec3 vel) 
{
    State start = State
    (
        toSpherical      (pos     ), 
        toSphericalVector(pos, vel)
    );

    /*
    start.v.x *= (1.0 - Rs / start.r.x);
    start.v.y *=  pow(start.r.x, 2.0);
    start.v.z *= start.r.x * sin(start.r.y);
    */
    State curr = start;
    State next = start;

	for (int steps = 0; steps < marchStepsNo; ++steps) 
    {
        next = RK4(curr);
    
        /*
        if(crossAccretionDisk(curr, next)  )
           return   diskColor(curr, next, time);
        */
        if(crossEventHorizon (next))
        {
           return followMouse() * vec4(0.f , 0.f , 0.f, 1.f);
        }

        if(abs(next.r.x) > maxDist * Rs)
           return vec4(0.5f, 0.2f, 1.f, 1.f);
        
        curr = next;
	}
    
    return vec4(next.r.xy, t, 1);//vec4(next.v, 1.f);
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


vec2 KerrScaleParams(State st)
{
    float r     = st.r.x;
    float theta = st.r.y;

    float r2 = pow(r, 2);
    float a2 = pow(a, 2);

    return vec2
    (
        r2 + a2 * pow(cos(theta), 2),
        r2 + a2 - Rs * r
    );
}

// In Boyer–Lindquist coordinates (G = c = 1)
mat4 KerrMetricTensor(State st)
{
    float r     = st.r.x;
    float theta = st.r.y;

    float r2 = pow(r, 2);
    float a2 = pow(a, 2);
    float cosT  = cos(theta);
    float sinT  = sin(theta);
    float sinT2 = pow(sinT, 2);

    vec2 sp = KerrScaleParams(st);
    float Sigma = sp.x;
    float delta = sp.y;

    // before
    float c2dt2      =  -(1.0 - (Rs * r) / Sigma);
    float dr2        =  Sigma / delta;
    float dTheta2    =  Sigma;
    float dPhi2      =  (r2 + a2 + Rs * r * a2    / Sigma * sinT2) * sinT2;
    float cdtdPhi    =  - 2.0 * Rs * r  * a * sinT2 / Sigma;

    return mat4
    (
        c2dt2, 0  , 0      , cdtdPhi,
        0    , dr2, 0      , 0      ,
        0    , 0  , dTheta2, 0      , 
        0    , 0  , 0      , dPhi2
    );
}
