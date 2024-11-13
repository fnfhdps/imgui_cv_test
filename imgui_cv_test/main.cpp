#include "stdafx.h"
#include "UIManager.h"
#include "./src/stb/stb_image.h"

static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

// GLFW 초기화
GLFWwindow* window;

cv::VideoCapture cap;

// Main loop
bool show_demo_window = true;
bool show_another_window = false;
ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

// 전역 변수로 텍스처 ID 추가
GLuint g_TextureID = 0;
bool g_TextureLoaded = false;
int g_ImageWidth = 0;
int g_ImageHeight = 0;
bool showMessage = false; // 메시지 박스 표시 여부
std::string messageText; // 메시지 박스에 표시할 메시지

UIManager uiManager;

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
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 130");  // OpenGL 3.3 이상 버전
}

GLuint LoadTextureFromFile(const wchar_t* filename)
{
    int width, height, channels;

    // wchar_t*를 char*로 변환
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, filename, -1, NULL, 0, NULL, NULL);
    char* cFilename = new char[size_needed];
    WideCharToMultiByte(CP_UTF8, 0, filename, -1, cFilename, size_needed, NULL, NULL);
    
    unsigned char* data = stbi_load(cFilename, &width, &height, &channels, 0);
    
    if (data == nullptr)
    {
        uiManager.SetMessage(string(cFilename));
        uiManager.SetShowMessage(true);
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

// 비디오 프레임을 텍스처로 변환
void UpdateTexture() {
    if (ImGui::Button("Load Video")) {
        cv::Mat frame;
        if (cap.read(frame)) {
            // OpenCV의 BGR 이미지를 RGB로 변환
            cv::cvtColor(frame, frame, cv::COLOR_BGR2RGB);

            // 텍스처 생성
            if (!g_TextureLoaded) {
                glGenTextures(1, &g_TextureID);
                g_TextureLoaded = true;
            }

            glBindTexture(GL_TEXTURE_2D, g_TextureID);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, frame.cols, frame.rows, 0, GL_RGB, GL_UNSIGNED_BYTE, frame.data);
            glGenerateMipmap(GL_TEXTURE_2D);
        }
        else {
            // fail msg
            uiManager.SetMessage("Failed to capture video frame.");
            uiManager.SetShowMessage(true);
        }
    }
}

// 이미지 로드 UI 함수
void imageLoad() {
    //ImGui::Begin("MainView", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize);
    
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
        } 
    }

    // 이미지가 로드되었다면 표시
    if (g_TextureLoaded && g_TextureID != 0) {
        ImGui::Image((void*)(intptr_t)g_TextureID, ImVec2(g_ImageWidth, g_ImageHeight));
    }
    //ImGui::End(); // 윈도우 종료
}

void mainFrame() {
    // 창의 크기와 위치 설정
    ImGui::SetNextWindowSize(ImVec2(1280, 720), ImGuiCond_FirstUseEver); // 초기 크기 설정
    ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_FirstUseEver); // 초기 위치 설정

    ImGui::Begin("mainView", nullptr, ImGuiWindowFlags_NoCollapse); // 닫기 버튼 및 크기 조정 비활성화
    imageLoad();
    UpdateTexture();
    uiManager.Render();

    ImGui::End(); // 윈도우 종료
}

// 렌더링 코드, 매 프레임마다 반복
void Render() {

    // 1. 새 프레임 시작
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // 2. UI 구성
    mainFrame();

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

    glfwSetErrorCallback(glfw_error_callback);
    // 윈도우와 OpenGL 초기화
    if (!InitWindow()) return -1;

    // GLAD 초기화
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) return -1;

    // ImGui 초기화
    InitImGui();

    // 메인 루프
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents(); // 이벤트 처리
        if (glfwGetWindowAttrib(window, GLFW_ICONIFIED) != 0) {
            ImGui_ImplGlfw_Sleep(10);
            continue;
        }
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
