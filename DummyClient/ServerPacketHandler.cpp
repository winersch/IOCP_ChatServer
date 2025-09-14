#include "pch.h"
#include "ServerPacketHandler.h"
#include "ServerSession.h"
#include "ClientApp.h"
#include <sstream>

PacketHandlerFunc GPacketHandler[UINT16_MAX];

// 직접 컨텐츠 작업자
bool Handle_INVALID(PacketSessionRef& session, BYTE* buffer, int32 len) {
	PacketHeader* header = reinterpret_cast<PacketHeader*>(buffer);
	// TOOD : Log
	return false;
}

bool Handle_S_NICKNAME_PROMPT(PacketSessionRef& session, Protocol::S_NICKNAME_PROMPT& pkt) {
	GClient.SetState(ClientState::WaitingNickname);
	GClient.Log(pkt.prompt());
	GClient.session = static_pointer_cast<ServerSession>(session);
	return true;
}

bool Handle_S_NICKNAME_RESULT(PacketSessionRef& session, Protocol::S_NICKNAME_RESULT& pkt) {

	if (pkt.error() != Protocol::ErrorCode::OK) {
		GClient.Log("Duplicate or invalid nickname\n");
		GClient.SetState(ClientState::WaitingNickname);
		return true;
	}
	GClient.Log("Nickname generated successfully");

	ServerSessionRef serverSession = static_pointer_cast<ServerSession>(session);
	serverSession->_player = MakeShared<Player>();
	serverSession->_player->SetId(pkt.self().id());
	serverSession->_player->SetName(pkt.self().name());

	GClient.myId = pkt.self().id();
	GClient.nickname = pkt.self().name();

	GClient.SetState(ClientState::Lobby);
	GClient.Log("로비입니다. 명령: list | create <name> | join <roomId>");

	return true;
}

bool Handle_S_ROOM_LIST(PacketSessionRef& session, Protocol::S_ROOM_LIST& pkt) {

	if (pkt.rooms().empty()) {
		GClient.Log("No room created! Create a new room by entering a room name\n");
		return true;
	}
	ostringstream oss;
	
	for (const auto& room : pkt.rooms()) {
		oss << "- " << room.roomid() << ". Room : " << room.name() << " (" << room.playercount() << " / " << room.maxplayers() << ")\n";
	}
	GClient.Log(oss.str());
	return true;
}

bool Handle_S_CREATE_ROOM(PacketSessionRef& session, Protocol::S_CREATE_ROOM& pkt) {
	if (pkt.error() != Protocol::ErrorCode::OK) {
		GClient.Log("Failed to create room");
		return true;
	}
	GClient.Log("A room has been created");
	return true;
}

bool Handle_S_ENTER_ROOM(PacketSessionRef& session, Protocol::S_ENTER_ROOM& pkt) {

	if (pkt.error() != Protocol::ErrorCode::OK) {
		GClient.Log("You can't enter the room\n");
		return true;
	}
	std::ostringstream oss;
	oss << "Entered room " << pkt.room().info().roomid();
	GClient.Log(oss.str());

	ServerSessionRef serverSession = static_pointer_cast<ServerSession>(session);
	serverSession->_room = MakeShared<Room>();

	for (auto& player : pkt.room().players()) {
		PlayerRef playerRef = MakeShared<Player>();
		playerRef->SetId(player.id());
		playerRef->SetName(player.name());
		serverSession->_room->AddPlayer(playerRef);
	}
	GClient.currentRoomId = pkt.room().info().roomid();
	GClient.SetState(ClientState::InRoom);
	GClient.Log("You have entered the room. Enter the chat and it will be sent. (/Leave to leave)");
	return true;
}


bool Handle_S_ROOM_EVENT(PacketSessionRef& session, Protocol::S_ROOM_EVENT& pkt) {
	ServerSessionRef serverSession = static_pointer_cast<ServerSession>(session);
	if (serverSession->_room == nullptr) {
		serverSession->_room = MakeShared<Room>();
	}
	PlayerRef playerRef = MakeShared<Player>();
	std::ostringstream oss;
	switch (pkt.type()) {
		case Protocol::PLAYER_JOINED:
			oss << pkt.nickname() << " has entered.";
			GClient.Log(oss.str());
			playerRef->SetId(pkt.playerid());
			playerRef->SetName(pkt.nickname());
			serverSession->_room->AddPlayer(playerRef);
			break;
		case Protocol::PLAYER_LEFT:
			oss << pkt.nickname() << " leaved";
			GClient.Log(oss.str());
			serverSession->_room->DeletePlayerByName(pkt.nickname());
			break;
		default:
			break;
	}

	return true;
}

bool Handle_S_CHAT_ROOM(PacketSessionRef& session, Protocol::S_CHAT_ROOM& pkt) {
	std::ostringstream oss;
	oss << pkt.chat().nickname() << " : " << pkt.chat().msg();
	GClient.Log(oss.str());
	return true;
}