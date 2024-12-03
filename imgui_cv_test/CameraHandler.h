#pragma once
#include "stdafx.h"

#include <opencv2/opencv.hpp>
#include <glad.h>
#include <glfw3.h>
#include <opencv2/opencv.hpp>
#include "./src/imgui/imgui_impl_glfw.h"
#include "./src/imgui/imgui_impl_opengl3.h"

class CameraHandler
{
public:
    CameraHandler();
    CameraHandler(int index);
    ~CameraHandler();
    void StartCapture();
    void CaptureFrame();
    std::string UpdateTexture();
    void RecordVideo(std::string path);
    void SetIndex(int index);
    std::mutex& GetMutex();
    cv::Mat GetFrame();
    cv::VideoCapture GetCapTure();
    GLuint GetTextureID();
    bool GetIsRecoding();
    void SetIsRecoding(bool isRecoding);
    void DeleteResource();

private:
    //std::atomic<bool> isRunning;
    bool isRunning;
    int index;
    bool isOpen;
    cv::VideoCapture cap;
    cv::Mat frame;
    std::mutex frameMutex;
    GLuint textureID;
    std::string errorMsg;
    bool isRecording;
    cv::VideoWriter writer;
    //ThreadPool pool;

};