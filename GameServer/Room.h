#pragma once
#include "ClientPacketHandler.h"

class Room;
using RoomRef = shared_ptr<Room>;

class Room {

public:

	bool Enter(PlayerRef player);
	void Leave(PlayerRef player);
	void Broadcast(SendBufferRef sendBuffer);
	Protocol::RoomSummary GetSummary();

	void SetName(string name, uint64 maxPlayerCount = 16) { _name = name; _maxPlayerCount = maxPlayerCount; }
	string GetName() { return _name; }
	void SetId(uint64 id) { _id = id; }
	uint64 GetId() { return _id; }
	uint64 GetPlayerCount() { return _players.size(); }
	uint64 GetMaxPlayerCount() { return _maxPlayerCount; }

private:
	USE_LOCK;
	map<uint64, PlayerRef> _players;
	string _name;
	uint64 _id;
	uint64 _maxPlayerCount;
};

