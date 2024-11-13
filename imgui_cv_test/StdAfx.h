// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but are changed infrequently

#pragma once

#include "./src/imgui/imgui.h"
#include "./src/imgui/imgui_impl_glfw.h"
#include "./src/imgui/imgui_impl_opengl3.h"
#include "./src/stb/stb_image.h"

#include <glad.h>
#define STB_IMAGE_IMPLEMENTATION
#include <glfw3.h>

#include <opencv2/opencv.hpp>

#include <windows.h>
#include <string>
#include <stdio.h>
#include <format>
using namespace std;

#pragma comment(lib, "Comdlg32.lib")