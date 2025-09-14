#include "pch.h"
#include "ThreadManager.h"
#include "Service.h"
#include "Session.h"
#include "BufferReader.h"
#include "ServerPacketHandler.h"
#include "ServerSession.h"
#include "ConsoleUI.h"
#include "ClientApp.h"

int main() {
	ServerPacketHandler::Init();

	ClientServiceRef service = MakeShared<ClientService>(
		NetAddress(L"127.0.0.1", 7777),
		MakeShared<IocpCore>(),
		MakeShared<ServerSession>,
		1);

	ASSERT_CRASH(service->Start());
	GThreadManager->Launch([=]() {
		while (true) {
			service->GetIocpCore()->Dispatch();
		}
		});

	std::string buf, line;
	ui::DrainLogs();
	ui::RenderInput(buf);

	while (true) {
		if (ui::PumpInput(buf, line)) {
			if (!line.empty()){
				HandleInputLine(line);
			}
			line.clear();
		}
		ui::DrainLogs();
		ui::RenderInput(buf);
		std::this_thread::sleep_for(10ms);
	}
	GThreadManager->Join();
	return 0;
}