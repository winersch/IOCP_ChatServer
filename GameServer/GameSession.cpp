#include "pch.h"
#include "GameSession.h"
#include "GameSessionManager.h"
#include "ClientPacketHandler.h"

void GameSession::OnConnected() {
	GSessionManager.Add(static_pointer_cast<GameSession>(shared_from_this()));
	Protocol::S_NICKNAME_PROMPT promptPkt;
	promptPkt.set_prompt("Enter your nickname : ");

	auto sendBuffer = ClientPacketHandler::MakeSendBuffer(promptPkt);
	Send(sendBuffer);
}

void GameSession::OnDisconnected() {
	GSessionManager.Remove(static_pointer_cast<GameSession>(shared_from_this()));
}

void GameSession::OnRecvPacket(BYTE* buffer, int32 len)  {

	PacketSessionRef session = GetPacketSessionRef();
	PacketHeader* header = reinterpret_cast<PacketHeader*>(buffer);

	ClientPacketHandler::HandlePacket(session, buffer, len);
}

void GameSession::OnSend(int32 len)  {

}