#pragma once

namespace net {
	void SetNickname(const std::string& name);
	void RequestRoomList();
	void CreateRoom(const std::string& name);
	void EnterRoomById(uint64_t roomId);
	void ChatToRoom(const std::string& msg);
	void LeaveRoom();
}