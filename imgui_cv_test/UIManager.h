#pragma once
#include "stdafx.h"
#include "./src/imgui/imgui.h"

class UIManager
{
public:
	void ShowPopup(const std::string& title, const std::string& msg);
	void Render();
	void SetMessage(const std::string& msg);
	void SetShowMessage(bool show);

private:
	bool showMsg = false;
	std::string messageText;
};

