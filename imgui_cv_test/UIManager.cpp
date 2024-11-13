#include "UIManager.h"

void UIManager::ShowPopup(const string& title, const string& msg)
{
    if (showMsg) {
        ImGui::OpenPopup(title.c_str());
    }

    if (ImGui::BeginPopupModal(title.c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("%s", msg.c_str());
        ImGui::Separator();
        if (ImGui::Button("Close")) {
            showMsg = false;
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}

void UIManager::Render()
{
    ShowPopup("error message: ", messageText);
}

void UIManager::SetMessage(const string& msg)
{
    messageText = msg;
}

void UIManager::SetShowMessage(bool show)
{
    showMsg = show;
}



