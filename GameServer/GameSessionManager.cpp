#include "pch.h"
#include "GameSessionManager.h"
#include "GameSession.h"

GameSessionManager GSessionManager;

void GameSessionManager::Add(GameSessionRef session) {
	WRITE_LOCK;
	_sessions.insert(session);
}

void GameSessionManager::Remove(GameSessionRef session) {
	WRITE_LOCK;
	_sessions.erase(session);
}

void GameSessionManager::Broadcast(SendBufferRef sendBuffer) {
	WRITE_LOCK;
	for (GameSessionRef session : _sessions) {
		session->Send(sendBuffer);
	}
}

bool GameSessionManager::AddPlayer(PlayerRef player) {
	if (!player) {
		return false;
	}
	WRITE_LOCK;

	if (_playerNameToId.find(player->name) != _playerNameToId.end()) {
		return false;
	}

	const uint64 id = _playerId.fetch_add(1);
	player->playerId = id;
	_playerNameToId[player->name] = player->playerId;
	_playerIdToPlayer[player->playerId] = player;

	return true;
}

bool GameSessionManager::RemovePlayer(PlayerRef player) {
	WRITE_LOCK;
	auto itName = _playerIdToPlayer.find(player->playerId);
	if (itName == _playerIdToPlayer.end()){
		return false;
	}
	_playerNameToId.erase(player->name);
	_playerIdToPlayer.erase(player->playerId);
	
	return true;
}

PlayerRef GameSessionManager::GetPlayer(string name) {
	READ_LOCK;
	auto itId = _playerNameToId.find(name);
	if (itId == _playerNameToId.end()) {
		return nullptr;
	}
	auto itP = _playerIdToPlayer.find(itId->second);
	if (itP == _playerIdToPlayer.end()) {
		return nullptr;
	}
	return itP->second;
}

PlayerRef GameSessionManager::GetPlayer(uint64 id) {
	READ_LOCK;
	auto it = _playerIdToPlayer.find(id);
	if (it == _playerIdToPlayer.end()) {
		return nullptr;
	}
	return it->second;
}

bool GameSessionManager::AddRoom(RoomRef room) {
	WRITE_LOCK;

	const auto key = (room->GetName());
	if (_roomNameToId.find(key) != _roomNameToId.end()){
		return false;
	}
	const uint64 id = _roomId.fetch_add(1);
	room->SetId(id);
	_roomNameToId[key] = id;
	_roomIdToRoom[id] = room;
	return true;
}

bool GameSessionManager::RemoveRoom(RoomRef room) {
	if (!room) return false;
	WRITE_LOCK;

	auto it = _roomIdToRoom.find(room->GetId());
	if (it == _roomIdToRoom.end()) return false;

	_roomIdToRoom.erase(it);
	_roomNameToId.erase(room->GetName());
	return true;
}


RoomRef GameSessionManager::GetRoom(uint64 id) {
	READ_LOCK;

	auto it = _roomIdToRoom.find(id);
	if (it == _roomIdToRoom.end()) {
		return nullptr;
	}

	return it->second;
}

RoomRef GameSessionManager::GetRoom(string name) {
	READ_LOCK;

	auto itId = _roomNameToId.find(name);
	if (itId == _roomNameToId.end()) {
		return nullptr;
	}

	auto itR = _roomIdToRoom.find(itId->second);
	if (itR == _roomIdToRoom.end()) {
		return nullptr;
	}

	return itR->second;
}

RoomRef GameSessionManager::GetRoomFromPlayerId(uint64 id) {
	READ_LOCK;
	auto player = GetPlayer(id);
	if (player->roomId == -1) {
		return nullptr;
	}
	auto room = GetRoom(player->roomId);

	return room;
}

Vector<RoomRef> GameSessionManager::GetAllRooms() {
	Vector<RoomRef> rooms;
	{
		READ_LOCK;
		rooms.reserve(_roomIdToRoom.size());
		for (const auto& kv : _roomIdToRoom) {
			rooms.push_back(kv.second);
		}
	}
	return rooms;
}
