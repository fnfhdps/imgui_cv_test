#include "stdafx.h"
#include "UIManager.h"
#include "./src/stb/stb_image.h"

static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

// GLFW �ʱ�ȭ
GLFWwindow* window;

cv::VideoCapture cap;

// Main loop
bool show_demo_window = true;
bool show_another_window = false;
ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

// ���� ������ �ؽ�ó ID �߰�
GLuint g_TextureID = 0;
bool g_TextureLoaded = false;
int g_ImageWidth = 0;
int g_ImageHeight = 0;
bool showMessage = false; // �޽��� �ڽ� ǥ�� ����
std::string messageText; // �޽��� �ڽ��� ǥ���� �޽���

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
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);  // OpenGL 3.3 �̻� ���

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync
    return true;
}

// ImGui �ʱ�ȭ
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
    ImGui_ImplOpenGL3_Init("#version 130");  // OpenGL 3.3 �̻� ����
}

GLuint LoadTextureFromFile(const wchar_t* filename)
{
    int width, height, channels;

    // wchar_t*�� char*�� ��ȯ
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

    // �ؽ�ó ����
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    // �ؽ�ó ���͸� �� �� ����
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // �ؽ�ó ������ ���ε�
    GLenum format = (channels == 4) ? GL_RGBA : GL_RGB;
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    stbi_image_free(data);

    g_ImageWidth = width;
    g_ImageHeight = height;

    delete[] cFilename;

    return texture;
}

// ���� �������� �ؽ�ó�� ��ȯ
void UpdateTexture() {
    if (ImGui::Button("Load Video")) {
        cv::Mat frame;
        if (cap.read(frame)) {
            // OpenCV�� BGR �̹����� RGB�� ��ȯ
            cv::cvtColor(frame, frame, cv::COLOR_BGR2RGB);

            // �ؽ�ó ����
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

// �̹��� �ε� UI �Լ�
void imageLoad() {
    //ImGui::Begin("MainView", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize);
    
    if (ImGui::Button("Load Image")) {
        // ���� ���� ��ȭ ���� ����
        wchar_t filename[MAX_PATH] = L"";
        OPENFILENAME ofn;
        ZeroMemory(&ofn, sizeof(ofn));
        ofn.lStructSize = sizeof(ofn);
        ofn.hwndOwner = NULL; // ������ ������ �ڵ�
        ofn.lpstrFile = filename;
        ofn.nMaxFile = sizeof(filename) / sizeof(wchar_t);
        ofn.lpstrFilter = L"Image Files\0*.bmp;*.png;*.jpg;*.jpeg\0All Files\0*.*\0";
        ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

        if (GetOpenFileName(&ofn)) {
            g_TextureID = LoadTextureFromFile(filename);
            g_TextureLoaded = (g_TextureID != 0);
        } 
    }

    // �̹����� �ε�Ǿ��ٸ� ǥ��
    if (g_TextureLoaded && g_TextureID != 0) {
        ImGui::Image((void*)(intptr_t)g_TextureID, ImVec2(g_ImageWidth, g_ImageHeight));
    }
    //ImGui::End(); // ������ ����
}

void mainFrame() {
    // â�� ũ��� ��ġ ����
    ImGui::SetNextWindowSize(ImVec2(1280, 720), ImGuiCond_FirstUseEver); // �ʱ� ũ�� ����
    ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_FirstUseEver); // �ʱ� ��ġ ����

    ImGui::Begin("mainView", nullptr, ImGuiWindowFlags_NoCollapse); // �ݱ� ��ư �� ũ�� ���� ��Ȱ��ȭ
    imageLoad();
    UpdateTexture();
    uiManager.Render();

    ImGui::End(); // ������ ����
}

// ������ �ڵ�, �� �����Ӹ��� �ݺ�
void Render() {

    // 1. �� ������ ����
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // 2. UI ����
    mainFrame();

    // 3. ������ �غ� �� ����, ui Ŀ�ǵ� ����
    ImGui::Render();

    // 4. ȭ�� Ŭ���� �� ���� �׸���
    glClearColor(0.45f, 0.55f, 0.60f, 1.00f);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    // 5. ���� ���۸� ����, ������ ����
    glfwSwapBuffers(window);
}



// ���� ����
// 1. GLFW�� ������ init -> OpenGL context create
// 2. GLAD init (OpenGL fn ��� ��)
// 3. ImGui init
// 4. main roop���� randering and events
int main() {

    glfwSetErrorCallback(glfw_error_callback);
    // ������� OpenGL �ʱ�ȭ
    if (!InitWindow()) return -1;

    // GLAD �ʱ�ȭ
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) return -1;

    // ImGui �ʱ�ȭ
    InitImGui();

    // ���� ����
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents(); // �̺�Ʈ ó��
        if (glfwGetWindowAttrib(window, GLFW_ICONIFIED) != 0) {
            ImGui_ImplGlfw_Sleep(10);
            continue;
        }
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
