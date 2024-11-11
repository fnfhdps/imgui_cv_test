#include "./src/imgui/imgui.h"
#include "./src/imgui/imgui_impl_glfw.h"
#include "./src/imgui/imgui_impl_opengl3.h"
#include <glad.h>

#include <glfw3.h>
#define STB_IMAGE_IMPLEMENTATION

#include "./src/stb/stb_image.h"

#include <string>
using namespace std;




// GLFW �ʱ�ȭ
GLFWwindow* window;

// Main loop
bool show_demo_window = true;
bool show_another_window = false;
ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

// ���� ������ �ؽ�ó ID �߰�
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
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);  // OpenGL 3.3 �̻� ���

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

GLuint LoadTextureFromFile(const char* filename)
{
    // �̹��� �ε�
    int width, height, channels;
    unsigned char* data = stbi_load(filename, &width, &height, &channels, 0);
    if (data == nullptr)
    {
        printf("Failed to load image: %s\n", filename);
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

    // �̹��� ������ ����
    stbi_image_free(data);

    // �̹��� ũ�� ����
    g_ImageWidth = width;
    g_ImageHeight = height;

    return texture;
}

void imageLoad(const char* path) {
    static bool showImage = false;

    ImGui::Begin("Image Viewer"); // ������ ����
    ImGui::Text("Hello, ImGui!");
    if (ImGui::Button("Load Image")) {
        if (!g_TextureLoaded) {
            g_TextureID = LoadTextureFromFile(path);
            g_TextureLoaded = (g_TextureID != 0);
        }
        showImage = !showImage;
    }

    // �̹����� �ε�Ǿ��ٸ� ǥ��
    if (g_TextureLoaded && g_TextureID != 0 && showImage) {
        ImGui::Image((void*)(intptr_t)g_TextureID, ImVec2(g_ImageWidth, g_ImageHeight));
    }
    ImGui::End(); // ������ ����
}

// ������ �ڵ�, �� �����Ӹ��� �ݺ�
void Render() {

    // 1. �� ������ ����
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // 2. UI ����
    const char *filePath = "D:\\stick.bmp";
    imageLoad(filePath);

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

    // ������� OpenGL �ʱ�ȭ
    if (!InitWindow()) {
        return -1;
    }

    // GLAD �ʱ�ȭ
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
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
