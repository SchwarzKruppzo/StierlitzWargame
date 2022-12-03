#pragma once

#include "framework.h"
#include "Hack.h"

ImFont* m_pFont;
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace Menu
{
	WNDPROC oWndProc = NULL;

	LRESULT __stdcall WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
		{
			return true;
		}

		return CallWindowProc(oWndProc, hWnd, msg, wParam, lParam);
	}

	void BeginStyle()
	{
		ImGui::StyleColorsDark();
		ImGuiStyle& style = ImGui::GetStyle();

		style.Alpha = 1.f;
		style.WindowPadding = ImVec2(28, 14);
		style.WindowMinSize = ImVec2(32, 32);
		style.WindowRounding = 0.0f; //4.0 for slight curve
		style.WindowTitleAlign = ImVec2(0.5f, 0.5f);
		style.ChildRounding = 0.0f;
		style.FramePadding = ImVec2(4, 3);
		style.FrameRounding = 0.0f; //2.0
		style.ItemSpacing = ImVec2(8, 4);
		style.ItemInnerSpacing = ImVec2(4, 4);
		style.TouchExtraPadding = ImVec2(0, 0);
		style.IndentSpacing = 21.0f;
		style.ColumnsMinSpacing = 3.0f;
		style.ScrollbarSize = 6.0f;
		style.ScrollbarRounding = 16.0f; //16.0
		style.GrabMinSize = 0.1f;
		style.GrabRounding = 16.0f; //16.00
		style.ButtonTextAlign = ImVec2(0.5f, 0.5f);
		style.DisplayWindowPadding = ImVec2(22, 22);
		style.DisplaySafeAreaPadding = ImVec2(4, 4);
		style.AntiAliasedLines = true;
		style.CurveTessellationTol = 1.25f;
	}

	void BeginInput(HWND hWindow)
	{
		oWndProc = (WNDPROC)SetWindowLongPtr(hWindow, GWLP_WNDPROC, (LONG)WndProc);
	}

	enum PlayerColumn
	{
		PlayerColumn_Army,
		PlayerColumn_Name,
		PlayerColumn_MP,
		PlayerColumn_SP,
		PlayerColumn_CP,
		PlayerColumn_Action,
	};

	void ColoredTextValue(float value, float warning = 160.0f, bool descend = true, float max = 100.0f)
	{
		ImU32 clr = IM_COL32_WHITE;

		if (descend)
		{
			if (value < 0)
				clr = IM_COL32(255, 0, 0, 255);
			else if (value < warning)
				clr = IM_COL32(255, 255, 0, 255);
		}
		else
		{
			if (value > max)
				clr = IM_COL32(255, 0, 0, 255);
			else if (value > warning)
				clr = IM_COL32(255, 255, 0, 255);
		}

		ImGui::PushStyleColor(ImGuiCol_Text, clr);
		ImGui::Text("%.1f", value);
		ImGui::PopStyleColor();
	}

	void DrawPlayers(bool targetTeam)
	{
		static ImGuiTableFlags flags = ImGuiTableFlags_Reorderable | ImGuiTableFlags_NoBordersInBody | ImGuiTableFlags_SizingFixedFit;

		if (ImGui::BeginTable(targetTeam ? "team_a" : "team_b", 6, flags))
		{
			ImGui::TableSetupColumn("Army", ImGuiTableColumnFlags_NoSort | ImGuiTableColumnFlags_WidthFixed, 0.0f, PlayerColumn_Army);
			ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_NoSort, 0.0f, PlayerColumn_Name);
			ImGui::TableSetupColumn("MP", ImGuiTableColumnFlags_NoSort, 0.0f, PlayerColumn_MP);
			ImGui::TableSetupColumn("SP", ImGuiTableColumnFlags_NoSort, 0.0f, PlayerColumn_SP);
			ImGui::TableSetupColumn("CP", ImGuiTableColumnFlags_NoSort, 0.0f, PlayerColumn_CP);
			ImGui::TableSetupColumn(" ", ImGuiTableColumnFlags_NoSort | ImGuiTableColumnFlags_WidthFixed, 0.0f, PlayerColumn_Action);
			ImGui::TableSetupScrollFreeze(1, 1);

			ImGui::TableHeadersRow();

			ImGui::PushButtonRepeat(true);
			{
				for (const auto& [teamA, Player] : Hack::PlayerCache)
				{
					if (teamA != targetTeam)
						continue;

					ImGui::PushID(Player.id);
					ImGui::TableNextRow(ImGuiTableRowFlags_None);

					ImGui::TableSetColumnIndex(0);

					ImGui::TextUnformatted(Player.army);

					if (ImGui::TableSetColumnIndex(1))
						ImGui::TextUnformatted(Player.name);

					if (ImGui::TableSetColumnIndex(2))
						ColoredTextValue(Player.mp);
						
					if (ImGui::TableSetColumnIndex(3))
						ColoredTextValue(Player.sp, 5);

					if (ImGui::TableSetColumnIndex(4))
						ColoredTextValue(Player.cp, (Player.cp_max * 0.75), false, Player.cp_max);

					if (ImGui::TableSetColumnIndex(5))
					{
						if (ImGui::SmallButton(Hack::GetActivePlayer() == Player.id ? "YOU" : "SEAT")) {
							Hack::SwitchToPlayer(Player.id);
						}
					}

					ImGui::PopID();
				}
			}
			ImGui::PopButtonRepeat();
			ImGui::EndTable();
		}
	}

	void Draw()
	{
		if (GetAsyncKeyState(VK_END) & 1)
		{
			Settings::FreezeScope = !Settings::FreezeScope;

			if (Settings::FreezeScope)
				Hack::SaveScope();
		}

		ImGui::PushFont(m_pFont);
		ImGui::SetNextWindowContentSize(ImVec2(200.0f, 0.0f));
		ImGui::SetNextWindowBgAlpha(0.25f);
		ImGui::Begin("Stierlitz 7.0", NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
		{
			ImGui::Checkbox("No Fog of War", &Settings::NoFog);
			ImGui::Checkbox("Infinite Range", &Settings::NoRange);

			if (ImGui::Checkbox("Infinite Resources", &Settings::InfResources))
				Hack::SwitchInfiniteResources(Settings::InfResources);

			ImGui::Spacing();

			if (ImGui::Checkbox("Freeze Scope [END]", &Settings::FreezeScope));
				Hack::SaveScope();

			ImGui::Checkbox("Free Camera", &Settings::FreeCamera);
			ImGui::Spacing();

			if (!Hack::minimap_was_patched && !Hack::bIsInGame)
				if (ImGui::Button("PATCH MINIMAP"))
					Hack::PatchMinimap();

			ImGui::End();
		}

		ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x - 400, 0));
		ImGui::SetNextWindowContentSize(ImVec2(400, 0.0f));
		ImGui::SetNextWindowBgAlpha(0.25f);
		ImGui::Begin("Overflow", NULL, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove);
		{
			ImGui::Text("Team A");
			DrawPlayers(true);

			ImGui::Text("Team B");
			DrawPlayers(false);
			
			ImGui::End();
		}
		ImGui::PopFont();
	}
}