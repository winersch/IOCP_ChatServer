#pragma once
#include "Protocol.pb.h"

using PacketHandlerFunc = std::function<bool(PacketSessionRef&, BYTE*, int32)>;
extern PacketHandlerFunc GPacketHandler[UINT16_MAX];

enum : uint16 {
	PKT_S_NICKNAME_PROMPT = 1000,
	PKT_C_SET_NICKNAME = 1001,
	PKT_S_NICKNAME_RESULT = 1002,
	PKT_S_ROOM_LIST = 1003,
	PKT_C_REQUEST_ROOM_LIST = 1004,
	PKT_C_CREATE_ROOM = 1005,
	PKT_S_CREATE_ROOM = 1006,
	PKT_C_JOIN_ROOM = 1007,
	PKT_S_ENTER_ROOM = 1008,
	PKT_S_ROOM_EVENT = 1009,
	PKT_C_CHAT_ROOM = 1010,
	PKT_S_CHAT_ROOM = 1011,
	PKT_C_LEAVE_ROOM = 1012,
};

// Custom Handlers
bool Handle_INVALID(PacketSessionRef& session, BYTE* buffer, int32 len);
bool Handle_S_NICKNAME_PROMPT(PacketSessionRef& session, Protocol::S_NICKNAME_PROMPT& pkt);
bool Handle_S_NICKNAME_RESULT(PacketSessionRef& session, Protocol::S_NICKNAME_RESULT& pkt);
bool Handle_S_ROOM_LIST(PacketSessionRef& session, Protocol::S_ROOM_LIST& pkt);
bool Handle_S_CREATE_ROOM(PacketSessionRef& session, Protocol::S_CREATE_ROOM& pkt);
bool Handle_S_ENTER_ROOM(PacketSessionRef& session, Protocol::S_ENTER_ROOM& pkt);
bool Handle_S_ROOM_EVENT(PacketSessionRef& session, Protocol::S_ROOM_EVENT& pkt);
bool Handle_S_CHAT_ROOM(PacketSessionRef& session, Protocol::S_CHAT_ROOM& pkt);


class ServerPacketHandler {

public:

	static void Init() {
		for (int32 i = 0; i < UINT16_MAX; i++) {
			GPacketHandler[i] = Handle_INVALID;
		}
		GPacketHandler[PKT_S_NICKNAME_PROMPT] = [](PacketSessionRef& session, BYTE* buffer, int32 len) {return HandlePacket<Protocol::S_NICKNAME_PROMPT>(Handle_S_NICKNAME_PROMPT, session, buffer, len); };
		GPacketHandler[PKT_S_NICKNAME_RESULT] = [](PacketSessionRef& session, BYTE* buffer, int32 len) {return HandlePacket<Protocol::S_NICKNAME_RESULT>(Handle_S_NICKNAME_RESULT, session, buffer, len); };
		GPacketHandler[PKT_S_ROOM_LIST] = [](PacketSessionRef& session, BYTE* buffer, int32 len) {return HandlePacket<Protocol::S_ROOM_LIST>(Handle_S_ROOM_LIST, session, buffer, len); };
		GPacketHandler[PKT_S_CREATE_ROOM] = [](PacketSessionRef& session, BYTE* buffer, int32 len) {return HandlePacket<Protocol::S_CREATE_ROOM>(Handle_S_CREATE_ROOM, session, buffer, len); };
		GPacketHandler[PKT_S_ENTER_ROOM] = [](PacketSessionRef& session, BYTE* buffer, int32 len) {return HandlePacket<Protocol::S_ENTER_ROOM>(Handle_S_ENTER_ROOM, session, buffer, len); };
		GPacketHandler[PKT_S_ROOM_EVENT] = [](PacketSessionRef& session, BYTE* buffer, int32 len) {return HandlePacket<Protocol::S_ROOM_EVENT>(Handle_S_ROOM_EVENT, session, buffer, len); };
		GPacketHandler[PKT_S_CHAT_ROOM] = [](PacketSessionRef& session, BYTE* buffer, int32 len) {return HandlePacket<Protocol::S_CHAT_ROOM>(Handle_S_CHAT_ROOM, session, buffer, len); };
	}

	static bool HandlePacket(PacketSessionRef& session, BYTE* buffer, int32 len) {
		PacketHeader* header = reinterpret_cast<PacketHeader*>(buffer);
		return GPacketHandler[header->id](session, buffer, len);
	}
	static SendBufferRef MakeSendBuffer(Protocol::C_SET_NICKNAME& pkt) { return MakeSendBuffer(pkt, PKT_C_SET_NICKNAME); }
	static SendBufferRef MakeSendBuffer(Protocol::C_REQUEST_ROOM_LIST& pkt) { return MakeSendBuffer(pkt, PKT_C_REQUEST_ROOM_LIST); }
	static SendBufferRef MakeSendBuffer(Protocol::C_CREATE_ROOM& pkt) { return MakeSendBuffer(pkt, PKT_C_CREATE_ROOM); }
	static SendBufferRef MakeSendBuffer(Protocol::C_JOIN_ROOM& pkt) { return MakeSendBuffer(pkt, PKT_C_JOIN_ROOM); }
	static SendBufferRef MakeSendBuffer(Protocol::C_CHAT_ROOM& pkt) { return MakeSendBuffer(pkt, PKT_C_CHAT_ROOM); }
	static SendBufferRef MakeSendBuffer(Protocol::C_LEAVE_ROOM& pkt) { return MakeSendBuffer(pkt, PKT_C_LEAVE_ROOM); }

private:

	template<typename PacketType, typename ProcessFunc>
	static bool HandlePacket(ProcessFunc func, PacketSessionRef& session, BYTE* buffer, int32 len) {
		PacketType pkt;

		if (pkt.ParseFromArray(buffer + sizeof(PacketHeader), len - sizeof(PacketHeader)) == false) {
			return false;
		}

		return func(session, pkt);
	}


	template<typename T>
	static SendBufferRef MakeSendBuffer(T& pkt, uint16 pktId) {
		const uint16 dataSize = static_cast<uint16>(pkt.ByteSizeLong());
		const uint16 packetSize = dataSize + sizeof(PacketHeader);

		SendBufferRef sendBuffer = GSendBufferManager->Open(packetSize);
		PacketHeader* header = reinterpret_cast<PacketHeader*>(sendBuffer->Buffer());
		header->size = packetSize;
		header->id = pktId;
		ASSERT_CRASH(pkt.SerializeToArray(&header[1], dataSize));
		sendBuffer->Close(packetSize);

		return sendBuffer;
	}

};
