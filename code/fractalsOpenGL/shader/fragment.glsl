#version 330 core
in vec3 pos;
in vec3 fractalColor;
in float needColor;

layout(location = 0) out vec4 outColor;
void main()
{
    if(needColor < 0.f)
    {
        outColor = vec4(fractalColor, 1.f);
    }
    else
    {
        vec2 c = pos.xz;
        vec2 z = vec2(0.f, 0.f);
        int it = 0;
        int maxIter = 200;

        for(; (dot(z, z) < 4.0) && (it < maxIter); ++it)
        {
            vec2 tmp = vec2(z.x * z.x - z.y * z.y, 2.0 * z.x * z.y);
            z = tmp + c;
        }

        float scale = -log(float(it + 1) / maxIter) / log(float(maxIter));
        outColor = vec4(vec3(scale, scale, scale), 1.f);
    }
}
