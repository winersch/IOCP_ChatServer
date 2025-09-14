#include "pch.h"
#include "ConsoleUI.h"
#include <Windows.h>
#include <conio.h>


namespace { 
	short BottomLineY() {
		CONSOLE_SCREEN_BUFFER_INFO csbi{};
		GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
		return csbi.srWindow.Bottom;
	}

	void SetCursorPosXY(short x, short y) {
		SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), { x, y });
	}

	void ClearLine(short y) {
		HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
		CONSOLE_SCREEN_BUFFER_INFO csbi{};
		GetConsoleScreenBufferInfo(h, &csbi);
		DWORD written = 0;
		const DWORD cols = csbi.dwSize.X;
		COORD start{ 0, y };
		FillConsoleOutputCharacterA(h, ' ', cols, start, &written);
		FillConsoleOutputAttribute(h, csbi.wAttributes, cols, start, &written);
	}
	std::mutex g_logMtx;
	std::deque<std::string> g_logQ;
}

namespace ui {
	void Init() {
		// 스크롤 버퍼 키워서 로그가 위로 쌓이도록
		HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
		CONSOLE_SCREEN_BUFFER_INFO csbi{};
		GetConsoleScreenBufferInfo(h, &csbi);
		csbi.dwSize.Y = std::max<LONG>(csbi.dwSize.Y, 2000);
		SetConsoleScreenBufferSize(h, csbi.dwSize);
	}

	void EnqueueLog(const std::string& s) {
		std::lock_guard<std::mutex> lg(g_logMtx);
		g_logQ.push_back(s);
	}

	// 메인 루프에서만 호출 (UI 스레드 전용)
	void DrainLogs() {
		std::deque<std::string> local;
		{
			std::lock_guard<std::mutex> lg(g_logMtx);
			local.swap(g_logQ);
		}
		for (auto& s : local) {
			std::cout << s << "\n";
		}
	}

	void RenderInput(const std::string& buf) {
		const short y = BottomLineY();
		ClearLine(y);
		SetCursorPosXY(0, y);
		std::cout << "> " << buf;
	}

	bool PumpInput(std::string& buf, std::string& out) {
		bool submitted = false;
		while (_kbhit()) {
			int ch = _getch();
			if (ch == '\r' || ch == '\n') {
				out = buf;
				buf.clear();
				submitted = true;
			}
			else if (ch == 8) { // Backspace
				if (!buf.empty()) buf.pop_back();
			}
			else if (ch >= 32 && ch <= 126) {
				buf.push_back(static_cast<char>(ch));
			}
		}
		return submitted;
	}

}