#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <algorithm>

#include <imgui.h>
#include <imgui/backends/imgui_impl_glfw.h>
#include <imgui/backends/imgui_impl_opengl3.h>

struct GUI
{
    GLFWwindow *window;

    float mouseX = 0.f, mouseY = 0.f, mouseZ = 2.f;
    bool show = true;

    GUI(GLFWwindow * const pWindow) noexcept;
    ~GUI()
    {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }
    void render() noexcept;
};

inline GUI::GUI(GLFWwindow * const pWindow) noexcept
    : window(pWindow)
{
    glfwSetWindowUserPointer(window, this);
    glfwSetKeyCallback(window, +[](GLFWwindow * const w, int key, int, int action, int) noexcept
    {
        if(action == GLFW_RELEASE)
            return;

        GUI &gui = *reinterpret_cast<GUI *>(glfwGetWindowUserPointer(w));
        if(key == GLFW_KEY_ESCAPE)
            gui.show = !gui.show;
    });

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGui::StyleColorsDark();
    //ImGui::StyleColorsClassic();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330 core");
}
inline void GUI::render() noexcept
{
    //glfwSetInputMode(window, GLFW_CURSOR, show ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);

    ImGuiIO &io = ImGui::GetIO();
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    if(!io.WantCaptureMouse)
    {
        float const dz = 1e-2f * io.MouseWheel;
        mouseZ = std::clamp(mouseZ - dz, 0.5f, 10.f);

        if(io.MouseDown[0])
        {
            auto const [dx, dy] = io.MouseDelta;

            float const m = 1e-3f;
            mouseX -= m * dx;
            mouseY = std::clamp(mouseY + m * dy, -0.999f, 0.999f);
        }
    }

    if(show)
    {
        ImGui::SetNextWindowPos(ImVec2(0.f, 0.f));
        ImGui::Begin("Информация", nullptr, ImGuiWindowFlags_NoMove);
        if(ImGui::Button("Quit"))
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        ImGui::Text("%.2f ms, %.1f FPS", 1000.0f / io.Framerate, io.Framerate);
        ImGui::Text("ESC to toggle GUI focus");
        ImGui::Text("%g", io.MouseWheel);
        ImGui::End();
    }

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}
