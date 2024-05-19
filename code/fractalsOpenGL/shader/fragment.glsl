#version 420 core

const int marchStepsNo  =  30  ;
const float minDist     =    0.0;
const float maxDist     =  100.0;
const float eps         =  5e-4 ;

const float Power     =  12;
const float CutOff    =  40;
const int maxIterNo   =  10;
const int colorLevels =   5;


const float shadowIntensity  =   7;
const float shadowDiffuse    =  -3; 
const float diffuseStrength  =   1; 
const float orbitStrength    =  10; 

vec4 orbitTrap = vec4(maxDist);
int iters;


layout(location = 0) flat in float time;
layout(location = 1) flat in vec2 resolution;
layout(location = 2) flat in vec3 camPos;
layout(location = 3) flat in vec3 mouse;
layout(location = 4) flat in float FOV;
layout(location = 5) in vec2 uv;

uniform sampler2D uTexture;

layout(location = 0) out vec4 fragColor;

float mandelbulbSDF(vec3 pos) 
{
	vec3 z = pos;
	float dr = 1.0;
	float  r = 0.0;
	
    for (int i = 0; i < maxIterNo ; ++i) 
    {
		r = length(z);
        
		if (r > CutOff) 
            break;

 
		float theta = acos(z.z /   r);
		float phi   = atan(z.y , z.x);
		dr = pow(r, Power - 1.0) * Power * dr + 1;
		
		float zr = pow(r, Power);
		theta = theta   * Power;
		phi   = phi     * Power;
		
		z = zr * vec3
        (
            sin(theta) * cos(phi  ), 
            sin(phi  ) * sin(theta), 
            cos(theta)
        );

		z += pos;

		if (i < colorLevels) 
            orbitTrap = min
            (
                orbitTrap,
                abs(vec4(z, r * r))
            );
	}

	return 0.5 * log(r) * r  / dr;
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

mat4 followMouse()
{
    return rotateOX(-mouse.y * 3) 
         * rotateOY( mouse.x * 3);
}

float mapFractal(vec3 samplePoint) 
{
	vec3 fractalPoint = 
    (
        followMouse() *
		vec4(samplePoint, 1.0)
    ).xyz;

	return mandelbulbSDF(fractalPoint);
}

float rayMarch(vec3 from, vec3 dir, bool isLight) 
{
	float totalDist = 0.0;
	int steps;

	for (steps = 0; steps < marchStepsNo; ++steps) 
    {
		vec3 p = from + totalDist * dir;
		float dist = mapFractal(p);
		totalDist += dist;
		if ( dist > maxDist || 
             dist < eps        ) 
             break;
	}

	iters = steps;
	return totalDist;
}

vec3 rayDirection(float fov, vec2 size, vec2 fragCoord)
{
    vec2 xy = fragCoord - size / 2.0;
    float z = size.y / tan(radians(fov) / 2.0);
    
    return normalize(vec3(xy, z));
}

vec3 compNormal(vec3 samplePoint)
{
    float distToPoint = mapFractal(samplePoint);
    vec2 e = vec2(0.01, 0); 

    vec3 n = distToPoint - vec3
    (
        mapFractal(samplePoint - e.xyy),
        mapFractal(samplePoint - e.yxy),
        mapFractal(samplePoint - e.yyx)
    );

    return normalize(n);
}

float getLight(vec3 samplePoint)
{
    vec3 lightPos = vec3(10.0, 10.0, -10.0);
    vec3 light    = normalize(lightPos - samplePoint);
    vec3 normal   = compNormal(samplePoint);

    float dif = clamp
    (
        dot(normal,light) * diffuseStrength,
        0.0,
        1.0
    );

    float sourceDist = rayMarch(samplePoint + normal * eps * 2.0, light, true); 

    // Visibility function from Даня's methodichka
    if(sourceDist < length(lightPos - samplePoint)) 
       dif *= shadowDiffuse;

    return dif;
}

void main()
{	
    vec3 dir = rayDirection(FOV + 20 * mouse.z, resolution, gl_FragCoord.xy); 
	vec3 eye = vec3(0, 0, -1 - mouse.z);     

    float marchDist = rayMarch(eye, dir, false);

	if(marchDist >= maxDist)
    {
		float glow = iters / 3;
		fragColor = mix
        (
            vec4(0.0, 0.0, 0.0, 0.0),
            vec4(0.3, 0.4, 1.0, 1.0),
            glow * 0.05
        );
	} 
    else 
    {	
		vec3 p = eye + dir * marchDist; 
    
        vec3 pMo = 
        (
            followMouse() *
            vec4(p, 1.f)
        ).xyz;

        vec3 n = normalize(pMo);
        float theta = acos(n.z);
        float phi = sign(n.y) * acos(n.x / (1.f - n.z * n.z));
        
        vec2 UV = (vec2(phi, theta) / 3.1415926535f) + 0.5f;
		float diffuse = getLight(p);

		vec4 baseColor = vec4(orbitTrap.xyz, 1.0)
                       * orbitTrap.w * 3.0
                       + diffuse     * 0.6
                       +               0.2;

		fragColor = mix
        (
            texture
            (
                uTexture,
                UV
            ), 
            mix
            (
                baseColor,
                vec4(0.0, 0.1, 0.3, 0.7),
                clamp(marchDist * 0.2, 0.0, 1.0)
            ),
            1
        );
	}
}
