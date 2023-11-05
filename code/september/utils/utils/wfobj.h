#pragma once
#include <array>
#include <cstdio>
#include <istream>
#include <vector>
#include <string>

#include "triangle.h"

inline auto parseOBJ(std::istream &in)
{
    std::vector<Triangle> triangle;

    std::vector<vec3> pos;
    std::vector<vec2> tex;
    std::vector<vec3> norm;

    using i3 = std::array<u32, 3>;
    std::vector<i3> idx;

    std::string line;
    while(std::getline(in, line))
    {
        char const * const cstr = line.c_str();
        vec3 v;
        vec2 vt;
        if(3 == std::sscanf(cstr, "v %f %f %f", &v.x, &v.y, &v.z))
            pos.push_back(v);
        else if(2 == std::sscanf(cstr, "vt %f %f", &vt.x, &vt.y))
            tex.push_back(vt);
        else if(3 == std::sscanf(cstr, "vn %f %f %f", &v.x, &v.y, &v.z))
            norm.push_back(v);
        else if(cstr[0] == 'f')
        {
            char const *cptr = cstr + 2;
            i3 i;
            int eaten;
            while(3 == std::sscanf(cptr, "%u/%u/%u%n", &i[0], &i[1], &i[2], &eaten))
            {
                idx.push_back(i);
                cptr += eaten;
            }

            for(auto k = 1u; k + 1u < idx.size(); k++)
                triangle.push_back
                ({
                    pos[idx[0    ][0] - 1],
                    pos[idx[k    ][0] - 1],
                    pos[idx[k + 1][0] - 1],
                });
                /*
                triangle.push_back
                ({
                    .pos  = { pos[idx[0][0] - 1],  pos[idx[i][0] - 1],  pos[idx[i + 1][0] - 1]},
                    .tex  = { tex[idx[0][1] - 1],  tex[idx[i][1] - 1],  tex[idx[i + 1][1] - 1]},
                    .norm = {norm[idx[0][2] - 1], norm[idx[i][2] - 1], norm[idx[i + 1][2] - 1]},
                });
                */

            idx.clear();
        }
    }
    return triangle;
}
