#include "pch.h"
#include "ClientCommands.h"
#include "ServerPacketHandler.h"
#include "ServerSession.h"
#include "ClientApp.h"


namespace {
	ServerSessionRef AcquireSession() { return GClient.session.lock(); }
}

namespace net {
    void SetNickname(const string& name) {
        if (auto s = AcquireSession()) {
            Protocol::C_SET_NICKNAME pkt;
            pkt.set_nickname(name);
            auto sb = ServerPacketHandler::MakeSendBuffer(pkt);
            s->Send(sb);
        }
    }

    void RequestRoomList() {
        if (auto s = AcquireSession()) {
            Protocol::C_REQUEST_ROOM_LIST pkt;
            auto sb = ServerPacketHandler::MakeSendBuffer(pkt);
            s->Send(sb);
        }
    }

    void CreateRoom(const string& name) {
        if (auto s = AcquireSession()) {
            Protocol::C_CREATE_ROOM pkt;
            pkt.set_name(name);
            auto sb = ServerPacketHandler::MakeSendBuffer(pkt);
            s->Send(sb);
        }
    }

    void EnterRoomById(uint64_t roomId) {
        if (auto s = AcquireSession()) {
            Protocol::C_JOIN_ROOM pkt;
            Protocol::Player* player = new Protocol::Player();

            auto* self = pkt.mutable_self();
            self->set_id(GClient.myId);
            self->set_name(GClient.nickname);

            pkt.set_roomid(roomId);
            auto sb = ServerPacketHandler::MakeSendBuffer(pkt);
            s->Send(sb);
        }
    }

    void ChatToRoom(const string& msg) {
        if (auto s = AcquireSession()) {
            Protocol::C_CHAT_ROOM pkt;
            Protocol::Player* player = new Protocol::Player();

            auto* self = pkt.mutable_self();
            self->set_id(GClient.myId);
            self->set_name(GClient.nickname);
            pkt.set_msg(msg);

            auto sb = ServerPacketHandler::MakeSendBuffer(pkt);
            s->Send(sb);
        }
    }

    void LeaveRoom() {
        if (auto s = AcquireSession()) {
            Protocol::C_LEAVE_ROOM pkt;
            auto* self = pkt.mutable_self();
            self->set_id(GClient.myId);
            self->set_name(GClient.nickname);
            pkt.set_roomid(GClient.currentRoomId);
            auto sb = ServerPacketHandler::MakeSendBuffer(pkt);
            s->Send(sb);
            GClient.SetState(ClientState::Lobby);
        }
    }
}