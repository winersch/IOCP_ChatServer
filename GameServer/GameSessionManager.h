#pragma once
#include "Player.h"
#include "Room.h"

class GameSession;

using GameSessionRef = shared_ptr<GameSession>;

class GameSessionManager {

public:
	void Add(GameSessionRef session);
	void Remove(GameSessionRef session);
	void Broadcast(SendBufferRef sendBuffer);

	bool AddPlayer(PlayerRef player);
	bool RemovePlayer(PlayerRef player);
	PlayerRef GetPlayer(string name);
	PlayerRef GetPlayer(uint64 id);

	bool AddRoom(RoomRef room);
	bool RemoveRoom(RoomRef room);
	RoomRef GetRoom(uint64 id);
	RoomRef GetRoom(string name);
	RoomRef GetRoomFromPlayerId(uint64 id);
	Vector<RoomRef> GetAllRooms();

private:
	USE_LOCK;
	Set<GameSessionRef> _sessions;

	unordered_map<string, uint64> _playerNameToId;
	unordered_map<uint64, PlayerRef> _playerIdToPlayer;
	Atomic<uint64> _playerId = 1;
	Atomic<uint64> _roomId = 1;
	unordered_map<string, uint64> _roomNameToId;
	unordered_map<uint64, RoomRef> _roomIdToRoom;
};

extern GameSessionManager GSessionManager;