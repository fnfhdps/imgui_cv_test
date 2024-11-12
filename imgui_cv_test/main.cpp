#include <windows.h>
#include <string>
using namespace std;

#include <glad.h>
#define STB_IMAGE_IMPLEMENTATION
#include <glfw3.h>

#include "./src/imgui/imgui.h"
#include "./src/imgui/imgui_impl_glfw.h"
#include "./src/imgui/imgui_impl_opengl3.h"
#include "./src/stb/stb_image.h"
#include <format>

#pragma comment(lib, "Comdlg32.lib")

// GLFW 초기화
GLFWwindow* window;

// Main loop
bool show_demo_window = true;
bool show_another_window = false;
ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

// 전역 변수로 텍스처 ID 추가
GLuint g_TextureID = 0;
bool g_TextureLoaded = false;
int g_ImageWidth = 0;
int g_ImageHeight = 0;

bool InitWindow() {

    if (!glfwInit()) {
        return false;
    }

    window = glfwCreateWindow(1280, 720, "ImGui Example", NULL, NULL);
    if (window == NULL) {
        glfwTerminate();
        return false;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);  // OpenGL 3.3 이상 사용

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

GLuint LoadTextureFromFile(const wchar_t* filename)
{
    int width, height, channels;

    //string path = format("%s", filename);

    // wchar_t*를 char*로 변환
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, filename, -1, NULL, 0, NULL, NULL);
    char* cFilename = new char[size_needed];
    WideCharToMultiByte(CP_UTF8, 0, filename, -1, cFilename, size_needed, NULL, NULL);
    
    unsigned char* data = stbi_load(cFilename, &width, &height, &channels, 0);
    
    unsigned char* res;
    if (data == nullptr)
    {
        printf("Failed to load image: %s\n", cFilename);
        delete[] cFilename;
        return 0;
    }

    // 텍스처 생성
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    // 텍스처 필터링 및 랩 설정
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // 텍스처 데이터 업로드
    GLenum format = (channels == 4) ? GL_RGBA : GL_RGB;
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    stbi_image_free(data);

    g_ImageWidth = width;
    g_ImageHeight = height;

    delete[] cFilename;

    return texture;
}

void imageLoad() {
    static bool showImage = false;

    ImGui::Begin("Image Viewer"); // 윈도우 시작
    ImGui::Text("Hello, ImGui!");

    if (ImGui::Button("Load Image")) {
    
        // 파일 선택 대화 상자 열기
        wchar_t filename[MAX_PATH] = L"";
        OPENFILENAME ofn;
        ZeroMemory(&ofn, sizeof(ofn));
        ofn.lStructSize = sizeof(ofn);
        ofn.hwndOwner = NULL; // 소유자 윈도우 핸들
        ofn.lpstrFile = filename;
        ofn.nMaxFile = sizeof(filename) / sizeof(wchar_t);
        ofn.lpstrFilter = L"Image Files\0*.bmp;*.png;*.jpg;*.jpeg\0All Files\0*.*\0";
        ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

        if (GetOpenFileName(&ofn)) {

            g_TextureID = LoadTextureFromFile(filename);
            g_TextureLoaded = (g_TextureID != 0);
            showImage = !showImage;
        }
    }

    // 이미지가 로드되었다면 표시
    if (g_TextureLoaded && g_TextureID != 0 && showImage) {
        ImGui::Image((void*)(intptr_t)g_TextureID, ImVec2(g_ImageWidth, g_ImageHeight));
    }
    ImGui::End(); // 윈도우 종료
}

// 렌더링 코드, 매 프레임마다 반복
void Render() {

    // 1. 새 프레임 시작
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // 2. UI 구성
    imageLoad();

    // 3. 렌더링 준비 및 실행, ui 커맨드 생성
    ImGui::Render();

    // 4. 화면 클리어 및 실제 그리기
    glClearColor(0.45f, 0.55f, 0.60f, 1.00f);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    // 5. 더블 버퍼링 스왑, 깜박임 방지
    glfwSwapBuffers(window);
}



// 실행 순서
// 1. GLFW로 윈도우 init -> OpenGL context create
// 2. GLAD init (OpenGL fn 사용 전)
// 3. ImGui init
// 4. main roop에서 randering and events
int main() {

    // 윈도우와 OpenGL 초기화
    if (!InitWindow()) {
        return -1;
    }

    // GLAD 초기화
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
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
