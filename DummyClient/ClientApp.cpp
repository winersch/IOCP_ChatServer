#include "pch.h"
#include "ClientApp.h"
#include "ConsoleUI.h"
#include "ClientCommands.h"

ClientApp GClient;

static bool StartsWith(const std::string& s, const char* prefix) {
    const size_t n = std::strlen(prefix);
    return s.size() >= n && std::memcmp(s.data(), prefix, n) == 0;
}

void ClientApp::Log(const std::string& s) { ui::EnqueueLog(s); }

void HandleInputLine(const std::string& line) {
    switch (GClient.GetState()) {
        case ClientState::WaitingNickname:
            if (line.empty()) {
                GClient.Log("�г����� ��� �� �����ϴ�.");
                return;
            }
            GClient.Log("�г��� ���� ��...");
            net::SetNickname(line);
            return;

        case ClientState::Lobby:
            if (line == "list") {
                net::RequestRoomList();
                return;
            }
            if (StartsWith(line, "create ")) {
                const std::string name = line.substr(7);
                if (name.empty()) { GClient.Log("�� �̸��� �Է��ϼ���. ��) create myroom"); return; }
                net::CreateRoom(name);
                return;
            }
            if (StartsWith(line, "join ")) {
                const std::string idstr = line.substr(5);
                if (idstr.empty()) { GClient.Log("������ �� ID�� �Է��ϼ���. ��) join 1"); return; }
                const uint64_t id = strtoull(idstr.c_str(), nullptr, 10);
                if (id == 0) { GClient.Log("�ùٸ� �� ID�� �ƴմϴ�."); return; }
                net::EnterRoomById(id);
                return;
            }
            if (line == "help") {
                GClient.Log("��ɾ�: list | create <name> | join <roomId>");
                return;
            }
            GClient.Log("�� �� ���� ��ɾ�. help�� �Է��غ�����.");
            return;

        case ClientState::InRoom:
            if (line == "/leave" || line == "/Leave") {
                net::LeaveRoom();
                return;
            }
            if (!line.empty()) {
                net::ChatToRoom(line);
            }
            return;

        case ClientState::Connecting:
        default:
            GClient.Log("���� ������ ��ٸ��� ���Դϴ�...");
            return;
    }
}