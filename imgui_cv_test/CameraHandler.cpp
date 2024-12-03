#pragma once
#include "stdafx.h"
#include "CameraHandler.h"

CameraHandler::CameraHandler() : index(-1), isOpen(false), isRunning(false), isRecording(false)
{
}

CameraHandler::CameraHandler(int index) : index(index), isOpen(false), isRunning(false), isRecording(false) {

    //this->index = index;
    if (!cap.open(this->index)) {
        isOpen = false;
        isRunning = true;
    }
    else {
        isOpen = true;
        isRunning = false;
    }
}

CameraHandler::~CameraHandler() {
    DeleteResource();
}

void CameraHandler::StartCapture()
{
    if (!cap.open(index)) {
        isOpen = false;
        isRunning = false;

    }
    else {
        isOpen = true;
        isRunning = true;
    }
    //pool.enqueue([this] {CaptureFrame(); });
}

void CameraHandler:: CaptureFrame() {
    while (isRunning) {
        cv::Mat TempFrame;
            cap >> TempFrame;

            if (TempFrame.empty()) {
                errorMsg = "Failed to capture frame from camera";
            }

            std::lock_guard<std::mutex> lock(frameMutex);
            frame = TempFrame.clone();

            if (isRecording && !frame.empty()) {
                writer.write(frame);
            }

            // 현재 시간, 나중에 위치 이동
            auto now = std::chrono::system_clock::now();
            auto now_c = std::chrono::system_clock::to_time_t(now);
            struct tm timeinfo;
            localtime_s(&timeinfo, &now_c);
            string path = format("D:\\%04d-%02d-%02d %02d-%02d-%02d.avi",
                timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday,
                timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);

            RecordVideo(path);
            std::this_thread::sleep_for(std::chrono::milliseconds(10));  // CPU 부하 감소
    }
}

std::string CameraHandler::UpdateTexture()
{

    cv::Mat newFrame;
    if (!cap.isOpened()) {
        return errorMsg = "Failed to capture video frame.";
    }

    std::lock_guard<std::mutex> lock(frameMutex); // 스레드 안전성을 위해 잠금
    if (!frame.empty()) {
        newFrame = frame.clone();
    }
    else {
        return errorMsg = "No frame available.";
    }

    // BGR(OpenCV) -> RGB(OpenGL) 변환
    cv::cvtColor(newFrame, newFrame, cv::COLOR_BGR2RGB);

    if (textureID == 0) {
        glGenTextures(1, &textureID);
        glBindTexture(GL_TEXTURE_2D, textureID);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, newFrame.cols, newFrame.rows, 0, GL_RGB, GL_UNSIGNED_BYTE, newFrame.data);
    glGenerateMipmap(GL_TEXTURE_2D);

    // CPU 사용량을 줄이기 위해 약간의 대기
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    
    return "";
}
    
void CameraHandler:: RecordVideo(string path) {
    if (isRecording) {
        if (!writer.isOpened()) {
            // format set
            int fourcc = cv::VideoWriter::fourcc('M', 'J', 'P', 'G'); // MJPG 포맷
            double fps = 30.0;
            cv::Size frameSize(640, 480);

            writer.open(path.c_str(), fourcc, fps, frameSize, true);

            if (!writer.isOpened()) {
                errorMsg = "Failed to capture video frame.";
                return;
            }
        }
        else {
            writer.write(frame);
        }
    }
    else {
        writer.release();
    }
}

void CameraHandler::SetIndex(int index)
{
    this->index = index;
}

std::mutex& CameraHandler::GetMutex()
{
    return frameMutex;
}

cv::Mat CameraHandler::GetFrame()
{
    return frame;
}

cv::VideoCapture CameraHandler::GetCapTure()
{
    return cap;
}

GLuint CameraHandler::GetTextureID()
{
    return textureID;
}

bool CameraHandler::GetIsRecoding()
{
    return isRecording;
}

void CameraHandler::SetIsRecoding(bool isRecording){
    this->isRecording = isRecording;
}

void CameraHandler::DeleteResource()
{
    if (cap.isOpened()) {
        cap.release();
    }
    if (writer.isOpened()) {
        writer.release();
    }
    glDeleteTextures(1, &textureID);
}