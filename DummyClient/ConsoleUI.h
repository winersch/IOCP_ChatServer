#pragma once

namespace ui {
	void Init(); // �ɼ�: ��ũ�� ���� Ȯ�� ��
	void EnqueueLog(const std::string& s);
	void DrainLogs();
	void RenderInput(const std::string& buf);
	bool PumpInput(std::string& buf, std::string& lineOut); // ����ŷ 1�� Ȯ��
}

