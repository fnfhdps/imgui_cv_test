#include "./imgui/imgui.h"
#include "./imgui/imgui_impl_glfw.h"
#include "./imgui/imgui_impl_opengl3.h"
#include <glfw3.h>

// GLFW 초기화
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

// ImGui 초기화
void InitImGui() {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 130");  // OpenGL 3.3 이상 버전
}

// 렌더링 코드
void Render() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // UI 구성
    ImGui::Text("Hello, ImGui!");
    if (ImGui::Button("Click Me")) {
        // 버튼 클릭 시 처리
    }

    ImGui::Render();
    glClearColor(0.45f, 0.55f, 0.60f, 1.00f);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glfwSwapBuffers(window);
}

int main() {
    // 윈도우와 OpenGL 초기화
    if (!InitWindow()) {
        return -1;
    }

    // ImGui 초기화
    InitImGui();

    // 메인 루프
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents(); // 이벤트 처리

        // 렌더링
        Render();
    }

    // 종료 처리
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
