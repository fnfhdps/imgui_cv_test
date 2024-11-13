#pragma once

#include "stdafx.h"

class UIManager
{
public:
	void ShowPopup(const string& title, const string& msg);
	void Render();
	void SetMessage(const string& msg);
	void SetShowMessage(bool show);

private:
	bool showMsg = false;
	string messageText;
};

