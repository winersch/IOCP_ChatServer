#pragma once

class ServerSession;

enum class ClientState { Connecting, WaitingNickname, Lobby, InRoom };

struct ClientApp {
	Atomic<ClientState> state{ ClientState::Connecting };
	string nickname;
	uint64 myId = 0;
	uint64 currentRoomId = 0;
	std::weak_ptr<ServerSession> session;

	void SetState(ClientState s) { state.store(s, std::memory_order_relaxed); }
	ClientState GetState() const { return state.load(std::memory_order_relaxed); }

	void Log(const std::string& s);
};
// 전역 접근이 필요하면
extern ClientApp GClient;


void HandleInputLine(const std::string& line);