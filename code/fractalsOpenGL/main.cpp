#include "gui.h"
#include "shader.h"
#include "window.h"

#include <fstream>
#include <chrono>
#include <vector>
#include <cmath>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>


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

    GLuint VAO;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    GLuint tex;
    {
        glGenTextures(1, &tex);
        glBindTexture(GL_TEXTURE_2D, tex);
        
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        int width, height, nrChannels;
        unsigned char *data  = stbi_load("../diamond_ore.png", &width, &height, &nrChannels, 0);
        if (data)
        {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);
        }
        else
        {
            std::cout << "Failed to load texture" << std::endl;
        }
        stbi_image_free(data);
    }

    auto const t0 = std::chrono::steady_clock::now();

    glEnable(GL_DEPTH_TEST);
    while(!window.shouldClose())
    {
        int width, height;

        glfwGetFramebufferSize(window.pointer, &width, &height);

        glClearColor(0.1f, 0.1f, 0.3f, 1.f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glBindTexture(GL_TEXTURE_2D, tex);

        glUseProgram(shader);

        {
            auto const t = std::chrono::steady_clock::now();
            std::chrono::duration<float, std::ratio<1>> const dt = t - t0;
            glUniform1f (glGetUniformLocation(shader, "uTime"       ), dt.count());
            glUniform3f (glGetUniformLocation(shader, "uMouse"      ), gui.mouseX, gui.mouseY, gui.mouseZ);
            glUniform2f (glGetUniformLocation(shader, "uResolution" ), float(width) , float(height));
            glUniform1f (glGetUniformLocation(shader, "uAspectRatio"), float(width) / float(height));
            glUniform1ui(glGetUniformLocation(shader, "uTexture"    ), 0u);
        }

        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        gui.render();
    }
}
