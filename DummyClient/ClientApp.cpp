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
                GClient.Log("닉네임은 비울 수 없습니다.");
                return;
            }
            GClient.Log("닉네임 전송 중...");
            net::SetNickname(line);
            return;

        case ClientState::Lobby:
            if (line == "list") {
                net::RequestRoomList();
                return;
            }
            if (StartsWith(line, "create ")) {
                const std::string name = line.substr(7);
                if (name.empty()) { GClient.Log("방 이름을 입력하세요. 예) create myroom"); return; }
                net::CreateRoom(name);
                return;
            }
            if (StartsWith(line, "join ")) {
                const std::string idstr = line.substr(5);
                if (idstr.empty()) { GClient.Log("입장할 방 ID를 입력하세요. 예) join 1"); return; }
                const uint64_t id = strtoull(idstr.c_str(), nullptr, 10);
                if (id == 0) { GClient.Log("올바른 방 ID가 아닙니다."); return; }
                net::EnterRoomById(id);
                return;
            }
            if (line == "help") {
                GClient.Log("명령어: list | create <name> | join <roomId>");
                return;
            }
            GClient.Log("알 수 없는 명령어. help를 입력해보세요.");
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
            GClient.Log("서버 응답을 기다리는 중입니다...");
            return;
    }
}