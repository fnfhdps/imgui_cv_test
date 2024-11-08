#include "./imgui/imgui.h"
#include "./imgui/imgui_impl_glfw.h"
#include "./imgui/imgui_impl_opengl3.h"
#include <glfw3.h>

// GLFW �ʱ�ȭ
GLFWwindow* window;

bool InitWindow() {
    if (!glfwInit()) {
        return false;
    }

    window = glfwCreateWindow(1280, 720, "ImGui Example", NULL, NULL);
    if (window == NULL) {
        glfwTerminate();
        return false;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync
    return true;
}

// ImGui �ʱ�ȭ
void InitImGui() {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 130");  // OpenGL 3.3 �̻� ����
}

// ������ �ڵ�
void Render() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // UI ����
    ImGui::Text("Hello, ImGui!");
    if (ImGui::Button("Click Me")) {
        // ��ư Ŭ�� �� ó��
    }

    ImGui::Render();
    glClearColor(0.45f, 0.55f, 0.60f, 1.00f);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glfwSwapBuffers(window);
}

int main() {
    // ������� OpenGL �ʱ�ȭ
    if (!InitWindow()) {
        return -1;
    }

    // ImGui �ʱ�ȭ
    InitImGui();

    // ���� ����
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents(); // �̺�Ʈ ó��

        // ������
        Render();
    }

    // ���� ó��
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
