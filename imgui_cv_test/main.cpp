#include "stdafx.h"
#include "UIManager.h"
#include "./src/stb/stb_image.h"

#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <cstdlib>
#include <crtdbg.h>

#ifdef _DEBUG
#define new new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
#endif


static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

GLFWwindow* window;
ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

// 전역 변수로 텍스처 ID 추가
GLuint g_ImgTextureID = 0;
bool g_TextureLoaded = false;
int g_ImageWidth = 0;
int g_ImageHeight = 0;

UIManager uiManager;
bool showMessage = false;
std::string messageText;

// video view
atomic<bool> isRunning(true);
bool show_video_window = false;
cv::VideoCapture cap;
cv::Mat realFrame;
mutex frameMutex;

// video recoding
bool isRecording = false;
cv::VideoWriter writer;

// 전역 변수 추가
std::thread videoThread; // 비디오 캡처 스레드

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

void UpdateTextureImage(const wchar_t* filename) {

    int width = 0, height = 0, channels = 0;

    // wchar_t*를 char*로 변환
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, filename, -1, NULL, 0, NULL, NULL);
    char* cFilename = new char[size_needed];
    WideCharToMultiByte(CP_UTF8, 0, filename, -1, cFilename, size_needed, NULL, NULL);
    
    unsigned char* data = stbi_load(cFilename, &width, &height, &channels, 0);
    if (data == nullptr) {
        uiManager.SetMessage(string(cFilename));
        uiManager.SetShowMessage(true);
        delete[] cFilename;
        return;
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

    g_ImgTextureID = texture;

    glDeleteTextures(1, &texture);
    delete[] cFilename;

}

// 이미지 로드 UI 함수
void ImageLoad() {

    // 파일 선택 대화 상자 열기
    wchar_t filename[MAX_PATH] = L"";
    OPENFILENAME ofn;
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = NULL;
    ofn.lpstrFile = filename;
    ofn.nMaxFile = sizeof(filename) / sizeof(wchar_t);
    ofn.lpstrFilter = L"Image Files\0*.bmp;*.png;*.jpg;*.jpeg\0All Files\0*.*\0";
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

    if (GetOpenFileName(&ofn)) {
        UpdateTextureImage(filename);
    }
}

void RecordVideo(cv::Mat TempFrame, string path) {

    if (isRecording) {
        if (!writer.isOpened()) {
            int fourcc = cv::VideoWriter::fourcc('M', 'J', 'P', 'G'); // 코덱 선택 (예: MJPG)
            double fps = 60.0;
            cv::Size frameSize(640, 480);

            writer.open(path.c_str(), fourcc, fps, frameSize, true);

            if (!writer.isOpened()) {
                uiManager.SetMessage("Failed to capture video frame.");
                uiManager.SetShowMessage(true);
                return;
            }
        }
        else {
            if (TempFrame.empty()) return;
            TempFrame = realFrame.clone();
            writer.write(TempFrame);
        }
    }
    else {
        writer.release(); // 저장 종료
    }
}

// 비디오 프레임을 텍스처로 변환
void UpdateTextureVideo(cv::VideoCapture& cap) {

    cv::Mat newFrame;

    while (isRunning) {
        cap >> newFrame;
        if (newFrame.empty()) break;

        std::lock_guard<std::mutex> lock(frameMutex);
        realFrame = newFrame.clone(); // realFrame 업데이트
        RecordVideo(realFrame, "D:\\web_cam_test.avi");
    }
}

void VideoFrame() {

    ImGui::SetNextWindowSize(ImVec2(700, 500), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowPos(ImVec2(1500, 0), ImGuiCond_FirstUseEver);

    GLuint textureID = 0;
    cv::Mat newFrame;

    std::lock_guard<std::mutex> lock(frameMutex); // 스레드 안전성을 위해 잠금

    if (!cap.isOpened()) {
        uiManager.SetMessage("Failed to capture video frame.");
        uiManager.SetShowMessage(true);
        return; // 비디오 캡처가 열리지 않은 경우 종료
    }

    if (!realFrame.empty()) {
        newFrame = realFrame.clone(); // realFrame이 비어있지 않은 경우에만 복사
    }
    else {
        uiManager.SetMessage("No frame available.");
        uiManager.SetShowMessage(true);
        return; // realFrame이 비어있으면 종료
    }

    // BGR(OpenCV) -> RGB(OpenGL) 변환
    cv::cvtColor(newFrame, newFrame, cv::COLOR_BGR2RGB);

    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, newFrame.cols, newFrame.rows, 0, GL_RGB, GL_UNSIGNED_BYTE, newFrame.data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // CPU 사용량을 줄이기 위해 약간의 대기
    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    ImGui::Begin("CamView");
    if (cap.isOpened() && !newFrame.empty()) {
        ImGui::Image((void*)(intptr_t)textureID, ImVec2(newFrame.cols, newFrame.rows));
    }

    if (!show_video_window) {
        glDeleteTextures(1, &textureID); // 더 이상 텍스처를 사용하지 않을 때 삭제
        textureID = 0; // 안전을 위해 0으로 초기화
    }

    newFrame.release();
    uiManager.Render();
    ImGui::End();
}

void MainFrame() {
    // 창의 크기와 위치 설정
    ImGui::SetNextWindowSize(ImVec2(1280, 720), ImGuiCond_FirstUseEver); // 초기 크기 설정
    ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_FirstUseEver); // 초기 위치 설정

    ImGui::Begin("MainView", nullptr, ImGuiWindowFlags_NoCollapse); // 닫기 버튼 및 크기 조정 비활성화
    
    if (ImGui::Button("Load Image")) {
        ImageLoad();
    }
    
    // 이미지 화면 표시
    if (g_ImgTextureID != 0) {
        ImGui::Image((void*)(intptr_t)g_ImgTextureID, ImVec2(g_ImageWidth, g_ImageHeight));
    }

    if (ImGui::Button("Load Video")) {
        show_video_window = !show_video_window;

        if (show_video_window) {
            if (!cap.isOpened()) {
                if (cap.open(2)) { // 비디오 캡처 시작
                    // 비디오 데이터 로드 스레드 생성
                    isRunning = true; // 비디오 스레드 실행 플래그 설정
                    videoThread = std::thread(UpdateTextureVideo, std::ref(cap));
                    videoThread.detach(); // 스레드를 분리하여 메인 스레드와 독립적으로 실행
                }
                if (!cap.isOpened()) {
                    uiManager.SetMessage("Failed to open video capture.");
                    uiManager.SetShowMessage(true);
                    return;
                }
            }
        } else {
            isRunning = false; // 비디오 스레드 종료 플래그 설정
            cap.release(); // 비디오 캡처 종료
            //if (videoThread.joinable()) {
            //    videoThread.join(); // 스레드 종료 대기
            }
    }

    if (ImGui::Button("Video Recorde")) {
        //if (show_video_window) isRecording = !isRecording;
        if (cap.isOpened()) isRecording = !isRecording;
    }
    ImGui::Text("Recording Status: %s", isRecording ? "Recording" : "Not Recording");

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
    MainFrame();
    if (show_video_window) {
        VideoFrame();
    }

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
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
    _CrtSetBreakAlloc(6016);

    glfwSetErrorCallback(glfw_error_callback);
    // 윈도우와 OpenGL 초기화
    if (!InitWindow()) return -1;

    // GLAD 초기화
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) return -1;

    // ImGui 초기화
    InitImGui();

    // cap.open(2);
    // 비디오 데이터 로드 스레드 생성
    // std::thread videoThread(UpdateTextureVideo, std::ref(cap));

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

    // writer와 cap 해제
    writer.release(); // 비디오 작성을 종료
    cap.release(); // 비디오 캡처 종료

    // 종료 처리
    //isRunning = false; // 비디오 스레드 종료 플래그 설정
    //videoThread.join(); // 스레드 종료 대기

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    //_CrtDumpMemoryLeaks();
    return 0;
}
