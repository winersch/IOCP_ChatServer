#pragma once

namespace ui {
	void Init(); // 옵션: 스크롤 버퍼 확대 등
	void EnqueueLog(const std::string& s);
	void DrainLogs();
	void RenderInput(const std::string& buf);
	bool PumpInput(std::string& buf, std::string& lineOut); // 논블로킹 1줄 확정
}

