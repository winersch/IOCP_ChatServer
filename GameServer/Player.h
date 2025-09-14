#pragma once

class Player {

public:

	uint64					playerId = 0;
	string					name;
	uint64					roomId = 0;
	weak_ptr<GameSession>	ownerSession;
};
