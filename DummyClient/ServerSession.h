#pragma once
#include "Session.h"
#include "BufferReader.h"
#include "ServerPacketHandler.h"

class Player;
class Room;

using PlayerRef = shared_ptr<Player>;
using RoomRef = shared_ptr<Room>;

class Player {
public:
	Player(uint64 id, string name)
		: _id(id), _name(name) {
	}
	Player(){}
	~Player() {}
	
	uint64 GetId() { return _id; }
	string GetName() { return _name; }

	void SetId(uint64 id) { _id = id; }
	void SetName(string name) { _name = name; }
private:
	uint64 _id;
	string _name;
};

class Room {
public:

	void AddPlayer(PlayerRef player) { _players.push_back(player); }
	bool DeletePlayer(PlayerRef player) {
		auto it = std::find(_players.begin(), _players.end(), player);
		if (it != _players.end()) {
			_players.erase(it);
			return true;
		}
		return false; 
	}
	bool DeletePlayerByName(const std::string& name) {
		auto it = std::find_if(_players.begin(), _players.end(),
			[&](const PlayerRef& p) {
				return p && p->GetName() == name;
			});
		if (it != _players.end()) {
			_players.erase(it);
			return true;
		}
		return false;
	}
	void SetId(uint64 id) { _id = id; }
	void SetName(string name) { _name = name; }

	Vector<PlayerRef>& GetPlayer() { return _players; }
	uint64 GetId() { return _id; }
	string GetName() { return _name; }

private:
	uint64 _id;
	string _name;
	Vector<PlayerRef> _players;
};

class ServerSession : public PacketSession {

public:
	~ServerSession() {
		cout << "~ServerSession" << endl;
	}

	virtual void OnConnected() override {
		cout << "Connected To Server\n";

	}

	virtual void OnRecvPacket(BYTE* buffer, int32 len) override {
		PacketSessionRef session = GetPacketSessionRef();
		PacketHeader* header = reinterpret_cast<PacketHeader*>(buffer);

		ServerPacketHandler::HandlePacket(session, buffer, len);

	}

	virtual void OnSend(int32 len) override {
		
	}

	virtual void OnDisconnected() override {
		cout << "Disconnected\n";
	}

public:
	PlayerRef _player;
	RoomRef _room;
};
