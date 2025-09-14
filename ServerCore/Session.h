#pragma once
#include "IocpCore.h"
#include "IocpEvent.h"
#include "NetAddress.h"
#include "RecvBuffer.h"

class Service;

/*------------------
	 Session
-------------------*/

class Session : public IocpObject{
	friend class Listener;
	friend class IocpCore;
	friend class Service;

	enum {
		BUFFER_SIZE = 0x10000, // 64KB
	};

public:
	Session();
	virtual ~Session();

public:
						/* �ܺο��� ��� */
	void				Send(SendBufferRef sendBuffer);
	bool 				Connect();
	void				Disconnect(const WCHAR* cause);

	shared_ptr<Service> GetService() { return _service.lock(); }
	void				SetService(shared_ptr<Service> service) { _service = service; }
	
public:
						/* ���� ���� */
	void				SetNetAddress(NetAddress address) { _netaddress = address; }
	NetAddress			GetAddress() { return _netaddress; }
	SOCKET				GetSocket() { return _socket; }
	bool				IsConnected() { return _connected; }
	SessionRef			GetSessionRef() { return static_pointer_cast<Session>(shared_from_this()); }

public:
						/* �������̽� ���� */
	virtual HANDLE		GetHandle() override;
	virtual void		Dispatch(class IocpEvent* iocpEvent, int32 numOfBytes = 0) override;

private:
						/* ���۰��� */
	bool				RegisterConnect();
	bool				RegisterDisconnect();
	void				RegisterRecv();
	void				RegisterSend();

	void				ProcessConnect();
	void				ProcessDisconnect();
	void				ProcessRecv(int32 numOfBytes);
	void				ProcessSend(int32 numOfBytes);

	void				HandleError(int32 errorCode);

protected:
						/* ������ �ڵ忡�� ������ */
	virtual void		OnConnected() {}
	virtual int32		OnRecv(BYTE* buffer, int32 len) { return len; }
	virtual void		OnSend(int32 len){}
	virtual void		OnDisconnected() {}


public:



private:
	weak_ptr<Service>	_service;
	SOCKET				_socket = INVALID_SOCKET;
	NetAddress			_netaddress = {};
	Atomic<bool>		_connected = false;

private:
	USE_LOCK;

							/* ���� ���� */
	RecvBuffer				_recvBuffer;

							/* �۽� ���� */
	queue<SendBufferRef>	_sendQueue;
	Atomic<bool>			_sendRegistered = false;


private:
						/* IocpEvent ���� */
	ConnectEvent		_connectEvent;
	DisconnectEvent		_disconnectEvent;
	RecvEvent			_recvEvent;
	SendEvent			_sendEvent;

};


/*------------------
	PacketSession
-------------------*/

struct PacketHeader {
	uint16 size;
	uint16 id; // ��������ID ( ex) 1 = �α���, 2 = �̵���û)
};

class PacketSession : public Session {

public:
	PacketSession();
	virtual ~PacketSession();
	 

	PacketSessionRef GetPacketSessionRef() { return static_pointer_cast<PacketSession>(shared_from_this()); }

protected:

	virtual int32		OnRecv(BYTE* buffer, int32 len) sealed;
	virtual void		OnRecvPacket(BYTE* buffer, int32 len) abstract;

private:


};