#include "pch.h"
#include "Room.h"
#include "Player.h"
#include "GameSession.h"
#include "ClientPacketHandler.h"


bool Room::Enter(PlayerRef player) {
	WRITE_LOCK;

	if (GetPlayerCount() >= GetMaxPlayerCount()) {
		return false;
	}

	_players[player->playerId] = player;

	Protocol::S_ROOM_EVENT roomEvent;
	Protocol::RoomSummary* summary = new Protocol::RoomSummary(GetSummary());
	roomEvent.set_allocated_room(summary);

	roomEvent.set_type(Protocol::RoomEventType::PLAYER_JOINED);
	roomEvent.set_playerid(player->playerId);
	roomEvent.set_nickname(player->name);

	auto sendBuffer = ClientPacketHandler::MakeSendBuffer(roomEvent);
	for (const auto& p : _players) {
		auto session = p.second->ownerSession.lock();
		session->Send(sendBuffer);
	}
	
	return true;
}

void Room::Leave(PlayerRef player) {
	WRITE_LOCK;
	_players.erase(player->playerId);

	Protocol::S_ROOM_EVENT roomEvent;
	Protocol::RoomSummary* summary = new Protocol::RoomSummary(GetSummary());
	roomEvent.set_allocated_room(summary);

	roomEvent.set_type(Protocol::RoomEventType::PLAYER_LEFT);
	roomEvent.set_playerid(player->playerId);
	roomEvent.set_nickname(player->name);

	auto sendBuffer = ClientPacketHandler::MakeSendBuffer(roomEvent);
	if (auto session = player->ownerSession.lock()) {
		session->Send(sendBuffer);
	}
	for (const auto& p : _players) {
		auto session = p.second->ownerSession.lock();
		session->Send(sendBuffer);
	}
}

void Room::Broadcast(SendBufferRef sendBuffer) {
	WRITE_LOCK;
	for (auto& p : _players) {
		if (auto session = p.second->ownerSession.lock()) {
			session->Send(sendBuffer);
		}
	}
}

Protocol::RoomSummary Room::GetSummary() {
	Protocol::RoomSummary summary;
	summary.set_roomid(GetId());
	summary.set_name(GetName());
	summary.set_playercount(GetPlayerCount());
	summary.set_maxplayers(GetMaxPlayerCount());
	return summary;
}
