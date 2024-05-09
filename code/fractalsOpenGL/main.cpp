#include "gui.h"
#include "shader.h"
#include "window.h"

#include <fstream>
#include <chrono>
#include <vector>
#include <numeric>
#include <ranges>
#include <cmath>

int main()
{
    Window window;
    GUI gui(window.pointer);

    ImGuiIO &io = ImGui::GetIO();
    io.FontGlobalScale = 1.f / std::min(window.xscale, window.yscale);
    io.Fonts->AddFontFromFileTTF(FONT_DIR "Roboto-Medium.ttf", 16.f / io.FontGlobalScale, nullptr, io.Fonts->GetGlyphRangesCyrillic());

    auto const readFile = [](char const * const filename) noexcept
    {
        std::ifstream in(filename);
        char buffer[16384] = {'\0'};
        in.read(buffer, sizeof(buffer));
        return std::string(buffer);
    };
    Shader const shader =
    {
        readFile(SHADER_DIR "vertex.glsl"  ).c_str(),
        readFile(SHADER_DIR "fragment.glsl").c_str(),
    };

    
    // The King's dream fractal
     
    std::vector<float> vertices;
    
    float const A =  2.879879f;
    float const B = -0.765145f;
    float const C = -0.966918f;
    float const D =  0.744728f;

    int const maxIterNo = 10'000'0;

    float x, y;

    float xK = -0.1f, yK = -0.1f;
    for(int i = 0; i < maxIterNo; ++i)
    {
        x = std::sin(A * xK) + B * std::sin(A * yK);
        y = std::sin(C * xK) + D * std::sin(C * yK);

        vertices.push_back(x / window.xscale);
        vertices.push_back(y / window.yscale);
        vertices.push_back(-0.9f);

        xK = x; yK = y;
    }

    // Tinkerbell

   // std::vector<float> verticesT;

    float const AT =  0.9f;
    float const BT = -0.6013f;
    float const CT =  2.0f;
    float const DT =  0.5f;
     
    float xT = 0.01f;
    float yT = 0.01f;

    for(int i = 0; i < maxIterNo; ++i)
    {
        x = xT * xT - yT * yT + AT * xT + BT * yT;
        y =  2 * xT * yT      + CT * xT + DT * yT;

        vertices.push_back(1.5f);
        vertices.push_back(x / window.xscale);
        vertices.push_back(y / window.yscale + 1.9f);

        xT = x; yT = y;
    }

    // Gigachad
    
    float xG = -0.1f;
    float yG =  0.0f;

    int const GB = 1000'000;

    for(int i = 0; i < GB; ++i)
    {
        x = 1 - yG + std::abs(xG);
        y = xG;
   
        xG = x; yG = y;

        vertices.push_back(xG / window.xscale - 1.5f);
        vertices.push_back(-1.9f);
        vertices.push_back(yG / window.yscale + 0.9f);
    }


    /*
    int displs[] = {0                    , (int)verticesK.size()};
    int counts[] = {(int)verticesK.size(), (int)verticesT.size()};
    */

    unsigned int vao, vbo;
    glGenVertexArrays(1, &vao);
    glGenBuffers     (1, &vbo);

    glBindVertexArray(vao);

    //int const total = verticesK.size() + verticesT.size();

    {
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, (long int)(vertices.size() * sizeof(float)), &(vertices[0]), GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
    }

    /*
    {
        glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
        glBufferData(GL_ARRAY_BUFFER, (long int)(verticesT.size() * sizeof(float)), &(verticesT[0]), GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

    }
    */

    auto const t0 = std::chrono::steady_clock::now();

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);


    while(!window.shouldClose())
    {
        int width, height;
        glfwGetFramebufferSize(window.pointer, &width, &height);

        glClearColor(0.1f, 0.1f, 0.3f, 1.f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(shader);

        {
            auto const t = std::chrono::steady_clock::now();
            std::chrono::duration<float, std::ratio<1>> const dt = t - t0;
            glUniform1i(glGetUniformLocation(shader, "miniPEKKASize"), maxIterNo);
            glUniform1f(glGetUniformLocation(shader, "uTime"), dt.count());
            glUniform3f(glGetUniformLocation(shader, "uMouse"), gui.mouseX, gui.mouseY, gui.mouseZ);
            glUniform1f(glGetUniformLocation(shader, "uAspectRatio"), float(width) / float(height));
        }

        glBindVertexArray(vao);
        glDrawArrays(GL_POINTS, 0, (int)vertices.size());
        //glMultiDrawArrays(GL_POINTS, displs, counts, 2);
        gui.render();
    }
}
