#version 420 core

out vec4 fragColor;

layout(location = 0) flat in float time;
layout(location = 1) flat in vec2 resolution;
layout(location = 2) flat in vec3 camPos;
layout(location = 3) flat in vec3 mouse;
layout(location = 4) flat in float FOV;

uniform samplerCube galaxy;
uniform sampler2D colorMap;

float sqr(float x)
{
    return x*x;
}

/// by Ian McEwan, Ashima Arts
vec4 permute(vec4 x) 
{ 
    return mod(((x * 34.0) + 1.0) * x, 289.0); 
}

vec4 taylorInvSqrt(vec4 r) { return 1.79284291400159 - 0.85373472095314 * r; }

//Kerr metric params
float a =  0.0;
float m =  1.0;
float Q =  0.0;

float Rs = 1;

const float maxDist = 100.0;
const float eps     = 0.005;
const vec2 dq = vec2(0, eps);

float dt = 0.01;

/// by Ian McEwan, Ashima Arts
float snoise(vec3 v) 
{
  const vec2 C = vec2(1.0 / 6.0, 1.0 / 3.0);
  const vec4 D = vec4(0.0 , 0.5, 1.0 , 2.0);

  vec3 i  = floor(v + dot(v, C.yyy));
  vec3 x0 = v - i + dot(i, C.xxx);

  vec3 g  = step(x0.yzx, x0.xyz);
  vec3 l  = 1.0 - g;
  vec3 i1 = min(g.xyz, l.zxy);
  vec3 i2 = max(g.xyz, l.zxy);

  vec3 x1 = x0 - i1 + 1.0 * C.xxx;
  vec3 x2 = x0 - i2 + 2.0 * C.xxx;
  vec3 x3 = x0 - 1. + 3.0 * C.xxx;

  i = mod(i, 289.0);
  vec4 p = permute
  (
      permute
      (
          permute
          (
              i.z + vec4(0.0, i1.z, i2.z, 1.0)
          ) + 
              i.y + vec4(0.0, i1.y, i2.y, 1.0)
      ) +
              i.x + vec4(0.0, i1.x, i2.x, 1.0)
  );

  float n_ = 1.0 / 7.0; 
  vec3 ns = n_ * D.wyz - D.xzx;
 
  vec4 j = p - 49.0 * floor(p * ns.z * ns.z);

  vec4 x_ = floor(j *     ns.z);
  vec4 y_ = floor(j - 7.0 * x_); 

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

  vec4 norm = taylorInvSqrt
  (
      vec4
      (
          dot(p0, p0), 
          dot(p1, p1), 
          dot(p2, p2), 
          dot(p3, p3)
      )
  );

  p0 *= norm.x;
  p1 *= norm.y;
  p2 *= norm.z;
  p3 *= norm.w;

  vec4 m = max
  (
      0.6 - vec4
      (
          dot(x0, x0), 
          dot(x1, x1), 
          dot(x2, x2), 
          dot(x3, x3)
      ), 
      0.0
  );

  m = m * m;
  
  return 42.0
       * dot
       (
           m * m, 
           vec4
           (
               dot(p0, x0), 
               dot(p1, x1), 
               dot(p2, x2), 
               dot(p3, x3)
           )
       );
}

vec3 toSph(vec3 p) 
{
    float rho   = sqrt( (p.x * p.x) 
                      + (p.y * p.y) 
                      + (p.z * p.z)
                      );
    float theta = atan  (p.z , p.x);
    float phi   = asin  (p.y / rho);

    return vec3(rho, theta, phi);
}


void accretionDiskColor(vec3 pos, vec3 dir, inout vec3 color, float alpha) 
{
  float inR = 3   * Rs;
  float ouR = 18  * Rs;

  float diskHeight = 0.2;

  float density = max
  (
      0.0, 
      1.0 - length(pos.xyz / vec3(ouR, diskHeight, ouR))
  );

  if(density <  1e-3)
     return;

     density *= smoothstep(inR, inR * 1.1, length(pos));

  if(density <  1e-3)
     return;

  vec3 sphPos = toSph(pos);

  sphPos.y *= 2.0;
  sphPos.z *= 4.0;

  density *= 16000.0 / pow(abs(sphPos.x), 0.5);

  float noise = 1.0;
  for (int i = 0; i < int(8.0); i++) 
  {
      noise *= 0.5 * snoise(sphPos * pow(i, 2) * 0.5) + 0.5;

      if (i % 2 == 0) 
          sphPos.y += time * 5;
      else 
          sphPos.y -= time * 5;
  }

   vec3 dustColor = vec3(pow((sphPos.x - inR) / (ouR - inR), 1));

   color += density * alpha * abs(noise) * dustColor;
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

mat4 diag(vec4 a)
{
    return mat4
    (
        a.x,   0,   0,   0,
          0, a.y,   0,   0,
          0,   0, a.z,   0,
          0,   0,   0, a.w
    );
}

//optimized inverse of symmetric matrix from michael 0884
mat4 invSym(mat4 m) 
{
	float n11 = m[0][0], n12 = m[1][0], n13 = m[2][0], n14 = m[3][0];
	float n22 = m[1][1], n23 = m[2][1], n24 = m[3][1];
	float n33 = m[2][2], n34 = m[3][2];
	float n44 = m[3][3];

	float t11 = 2.0 * n23 * n34 * n24 - n24 * n33 * n24 - n22 * n34 * n34 - n23 * n23 * n44 + n22 * n33 * n44;
	float t12 = n14 * n33 * n24 - n13 * n34 * n24 - n14 * n23 * n34 + n12 * n34 * n34 + n13 * n23 * n44 - n12 * n33 * n44;
	float t13 = n13 * n24 * n24 - n14 * n23 * n24 + n14 * n22 * n34 - n12 * n24 * n34 - n13 * n22 * n44 + n12 * n23 * n44;
	float t14 = n14 * n23 * n23 - n13 * n24 * n23 - n14 * n22 * n33 + n12 * n24 * n33 + n13 * n22 * n34 - n12 * n23 * n34;

	float det = n11 * t11 + n12 * t12 + n13 * t13 + n14 * t14;
	float idet = 1.0f / det;

	mat4 ret;

	ret[0][0] = t11 * idet;
	ret[0][1] = (n24 * n33 * n14 - n23 * n34 * n14 - n24 * n13 * n34 + n12 * n34 * n34 + n23 * n13 * n44 - n12 * n33 * n44) * idet;
	ret[0][2] = (n22 * n34 * n14 - n24 * n23 * n14 + n24 * n13 * n24 - n12 * n34 * n24 - n22 * n13 * n44 + n12 * n23 * n44) * idet;
	ret[0][3] = (n23 * n23 * n14 - n22 * n33 * n14 - n23 * n13 * n24 + n12 * n33 * n24 + n22 * n13 * n34 - n12 * n23 * n34) * idet;

	ret[1][0] = ret[0][1];
	ret[1][1] = (2.0 * n13 * n34 * n14 - n14 * n33 * n14 - n11 * n34 * n34 - n13 * n13 * n44 + n11 * n33 * n44) * idet;
	ret[1][2] = (n14 * n23 * n14 - n12 * n34 * n14 - n14 * n13 * n24 + n11 * n34 * n24 + n12 * n13 * n44 - n11 * n23 * n44) * idet;
	ret[1][3] = (n12 * n33 * n14 - n13 * n23 * n14 + n13 * n13 * n24 - n11 * n33 * n24 - n12 * n13 * n34 + n11 * n23 * n34) * idet;

	ret[2][0] = ret[0][2];
	ret[2][1] = ret[1][2];
    ret[2][2] = (2.0 * n12 * n24 * n14 - n14 * n22 * n14 - n11 * n24 * n24 - n12 * n12 * n44 + n11 * n22 * n44) * idet;
	ret[2][3] = (n13 * n22 * n14 - n12 * n23 * n14 - n13 * n12 * n24 + n11 * n23 * n24 + n12 * n12 * n34 - n11 * n22 * n34) * idet;

	ret[3][0] = ret[0][3];
	ret[3][1] = ret[1][3];
	ret[3][2] = ret[2][3];
	ret[3][3] = (2.0 * n12 * n23 * n13 - n13 * n22 * n13 - n11 * n23 * n23 - n12 * n12 * n33 + n11 * n22 * n33) * idet;

	return ret;
}


float intersectSph(in vec3 orig, in vec3 dir, in vec4 sph)
{
    vec3 os = orig - sph.xyz;
    float b = dot(os, dir);
    float c = dot(os, os ) - sph.w * sph.w;
    float D = b * b - c;
    
    if (D < 0.0)
        return 1e8;
    
    return -b - sqrt(D);
}


bool adaptiveStep(vec4 P, vec4 q)
{
    vec3 p = q.yzw; // from 4-vector
                    
    float rho = dot(p, p) - a * a;

    float r2 = 0.5 * 
    (
        rho + sqrt
        (
            pow(rho, 2) + 
            pow(a  , 2) * 
            pow(p.z, 2) * 4
        )
    );
    float r = sqrt(r2);
    
    dt = mix
    (
        0.1, 
        10.0, 
        max(r - 1.0, 0.0) / maxDist
    );
    
    if( r < 1.0 && a <= 1.0 || 
        length ( P ) > 45.0    ) 
        return true;
}


mat4 G(vec4 q) // metric tensor in Kerr-Schild coords
{
    vec3 p = q.yzw;
    
    float rho = dot(p, p) - a * a;
    
    float r2 = 0.5 * 
    (
        rho + sqrt
        (
            pow(rho, 2) + 
            pow(a  , 2) * 
            pow(p.z, 2) * 4
        )
    );
    float r = sqrt(r2);
    
    vec4 k = vec4
    (
        1, 
        (r * p.x + a * p.y) / (r2 + a * a), 
        (r * p.y - a * p.x) / (r2 + a * a), 
        p.z / r
    );

    float f = smoothstep
    (
        maxDist * 0.5, 
        0.0, 
        r
    )
    * r2 
    * (2.0 * m * r - Q * Q)
    / (r2 * r2 + a * a * p.z * p.z); 

    return f * mat4
    (
        k.x * k, 
        k.y * k, 
        k.z * k, 
        k.w * k
    ) + diag(vec4(-1, 1, 1, 1));
}


float L(vec4 qt, mat4 g)
{
    return dot(g * qt, qt);
}

float L(vec4 qt, vec4 q)
{
    return L(qt, G(q));
}

vec4 gMoment(vec3 v, vec3 x)
{
    return 2.0 
         * G(vec4(0.0, x)) 
         *   vec4(1.0, v); 
} 

vec4 gCoord (vec4 p, vec4 q)
{
    return invSym(G(q)) * p;
}


float H(vec4 p, mat4 ginv)
{
    return dot(ginv * p, p);
}

float H(vec4 p, vec4 q)
{
    mat4 g    =      G(q);
    mat4 ginv = invSym(g);
    return H(p, ginv);
}

void dH(vec4 p, vec4 q, out vec4 dHdq, out vec4 dHdp)
{
    mat4 g    =      G(q);
    mat4 ginv = invSym(g);
    
    dHdp = 2.0 * vec4
    (
        dot(ginv[0], p), 
        dot(ginv[1], p), 
        dot(ginv[2], p), 
        dot(ginv[3], p)
    );
    
    dHdq = 
    ( vec4
        (
            H(p, q + dq.yxxx),
            H(p, q + dq.xyxx),
            H(p, q + dq.xxyx),
            H(p, q + dq.xxxy)
        ) - H(p,        ginv)
    ) / eps; 
}


mat2x4 integrate(in mat2x4 s, float dt)
{
    vec4 p = s[0]
       , q = s[1];
    
    mat4 g    =      G(q);
    mat4 ginv = invSym(g);
    vec4 qt   = ginv * p;
    
    vec4 dHdq = -
    (
        vec4
        (
            L(qt, q + dq.yxxx),
            L(qt, q + dq.xyxx),
            L(qt, q + dq.xxyx),
            L(qt, q + dq.xxxy)
        ) - L(qt,      g     )
    ) / eps; 
    

    mat2x4 dqp;
    dqp[0] = -dHdq          * dt;
    dqp[1] = 2.0 * ginv * p * dt;
    
    return dqp;
}

void update(inout vec4 p, inout vec4 q)
{    
    float dt1 = clamp(1. / length(p), 0.1, 4.0);
    
    mat2x4 state = mat2x4(p, q); 
    
    mat2x4 dqp = integrate(state, dt1 * dt);
    
    state += dqp;
    
    p = state[0];
    q = state[1];
}

vec4 rayMarch(vec3 pos, vec3 dir)
{
    vec3 color = vec3(0.0);

    float t0 = intersectSph(pos, dir, vec4(0, 0, 0, maxDist));
    if(   t0 > 0.0 && 
          t0 < 1e6    ) 
       pos  += dir * t0;

    vec4 Q = vec4(0.0, pos);
    vec4 P = gMoment(dir, Q.yzw);

    float p0 = P.x;

    float alpha = 1;
    
    for(int i = 0; i < 80; ++i)
    {
        update(P, Q);
        if(adaptiveStep(P, Q)) break;
     
        if(dot(Q.yzw, Q.yzw) < pow(Rs, 2))
           return vec4(0, 0, 0, 1);

        accretionDiskColor(Q.yzw, normalize(P.yzw), color, alpha);
    }

    vec4 qt = gCoord(P, Q);
    dir = normalize(qt.yzw);

    float p1 = P.x;
    float redshift = p0 / p1;

    if(length(Q.yzw) > Rs)
       color += texture(galaxy, dir).rgb * redshift * 50;
    
    return vec4(color, 1.0);
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
	vec3 eye = vec3(0, 0, -30 - mouse.z);     

    fragColor = rayMarch(followMouse(eye), followMouse(dir));
}
