#include "pch.h"
#include "ClientPacketHandler.h"
#include "Player.h"
#include "Room.h"
#include "GameSession.h"
#include "GameSessionManager.h"

PacketHandlerFunc GPacketHandler[UINT16_MAX];

bool Handle_INVALID(PacketSessionRef& session, BYTE* buffer, int32 len) {
	PacketHeader* header = reinterpret_cast<PacketHeader*>(buffer);
	return false;
}
bool Handle_C_SET_NICKNAME(PacketSessionRef& session, Protocol::C_SET_NICKNAME& pkt) {

	string name = pkt.nickname();

	PlayerRef player = MakeShared<Player>();
	player->playerId = 1;
	player->name = name;
	
	Protocol::S_NICKNAME_RESULT nicknameResultPkt;
	SendBufferRef sendBuffer;

	if (GSessionManager.AddPlayer(player) == false) {
		nicknameResultPkt.set_error(Protocol::ErrorCode::DUPLICATE_NICK);
		sendBuffer = ClientPacketHandler::MakeSendBuffer(nicknameResultPkt);
		session->Send(sendBuffer);
		Protocol::S_NICKNAME_PROMPT nicknamePrompt;
		nicknamePrompt.set_prompt("Enter your nickname : ");
		sendBuffer = ClientPacketHandler::MakeSendBuffer(nicknamePrompt);
		session->Send(sendBuffer);
	}
	else {
		player->ownerSession = static_pointer_cast<GameSession>(session);
		auto* playerPkt = nicknameResultPkt.mutable_self();
		playerPkt->set_id(GSessionManager.GetPlayer(name)->playerId);
		playerPkt->set_name(name);
		nicknameResultPkt.set_error(Protocol::ErrorCode::OK);
		sendBuffer = ClientPacketHandler::MakeSendBuffer(nicknameResultPkt);
		session->Send(sendBuffer);
	}

	return true;
}
bool Handle_C_REQUEST_ROOM_LIST(PacketSessionRef& session, Protocol::C_REQUEST_ROOM_LIST& pkt) {
	Protocol::S_ROOM_LIST createRoomPkt;
	SendBufferRef sendBuffer;
	auto rooms = GSessionManager.GetAllRooms();
	for (const auto& r : rooms) {
		auto* summary = createRoomPkt.add_rooms();
		summary->set_roomid(r->GetId());
		summary->set_name(r->GetName());
		summary->set_maxplayers(r->GetMaxPlayerCount());
		summary->set_playercount(r->GetPlayerCount());
	}

	sendBuffer = ClientPacketHandler::MakeSendBuffer(createRoomPkt);
	session->Send(sendBuffer);
	return true;
}
bool Handle_C_CREATE_ROOM(PacketSessionRef& session, Protocol::C_CREATE_ROOM& pkt) {
	RoomRef roomRef = MakeShared<Room>();
	roomRef->SetName(pkt.name());

	Protocol::S_CREATE_ROOM createRoomPkt;
	SendBufferRef sendBuffer;
	if (GSessionManager.AddRoom(roomRef) == false) {
		createRoomPkt.set_error(Protocol::ErrorCode::DUPLICATE_NICK);
	}
	else {
		createRoomPkt.set_error(Protocol::ErrorCode::OK);
		auto rooms = GSessionManager.GetAllRooms();
		for (const auto& r : rooms) {
			auto* summary = createRoomPkt.add_rooms();
			summary->set_roomid(r->GetId());
			summary->set_name(r->GetName());
			summary->set_maxplayers(r->GetMaxPlayerCount());
			summary->set_playercount(r->GetPlayerCount());
		}
	}
	sendBuffer = ClientPacketHandler::MakeSendBuffer(createRoomPkt);
	session->Send(sendBuffer);
	return true;
}
bool Handle_C_JOIN_ROOM(PacketSessionRef& session, Protocol::C_JOIN_ROOM& pkt) {

	auto room = GSessionManager.GetRoom(pkt.roomid());
	auto player = GSessionManager.GetPlayer(pkt.self().id());
	
	Protocol::S_ENTER_ROOM enterRoomPkt;
	SendBufferRef sendBuffer;

	if (room == nullptr) {
		enterRoomPkt.set_error(Protocol::ErrorCode::ROOM_NOT_FOUND);
		sendBuffer = ClientPacketHandler::MakeSendBuffer(enterRoomPkt);
	} 
	else if (room->Enter(player) == false) {
		enterRoomPkt.set_error(Protocol::ErrorCode::ROOM_FULL);
		sendBuffer = ClientPacketHandler::MakeSendBuffer(enterRoomPkt);
	}
	else {
		enterRoomPkt.set_error(Protocol::ErrorCode::OK);
		sendBuffer = ClientPacketHandler::MakeSendBuffer(enterRoomPkt);
	}

	ClientPacketHandler::MakeSendBuffer(enterRoomPkt);
	session->Send(sendBuffer);
	player->roomId = room->GetId();


	return true;
}
bool Handle_C_CHAT_ROOM(PacketSessionRef& session, Protocol::C_CHAT_ROOM& pkt) {

	auto room = GSessionManager.GetRoomFromPlayerId(pkt.self().id());
	Protocol::S_CHAT_ROOM chatRoomPkt;
	Protocol::ChatMessage* chatMessage= new Protocol::ChatMessage();
	chatMessage->set_roomid(room->GetId());
	chatMessage->set_playerid(pkt.self().id());
	chatMessage->set_nickname(pkt.self().name());
	chatMessage->set_msg(pkt.msg());
	chatRoomPkt.set_allocated_chat(chatMessage);

	SendBufferRef sendbuffer = ClientPacketHandler::MakeSendBuffer(chatRoomPkt);

	room->Broadcast(sendbuffer);
	return true;
}

bool Handle_C_LEAVE_ROOM(PacketSessionRef& session, Protocol::C_LEAVE_ROOM& pkt) {

	auto name = pkt.self().name();
	auto id = pkt.self().id();

	auto room = GSessionManager.GetRoomFromPlayerId(id);
	if (room == nullptr) {
		return false;
	}
	room->Leave(GSessionManager.GetPlayer(id));
	GSessionManager.GetPlayer(id)->roomId = -1; 

	return true;
}
