#include <windows.h>
#include <TlHelp32.h>
#include <vector>

namespace proc {
	DWORD procId = NULL;
	HANDLE hProc = nullptr;

	DWORD GetProcId(const wchar_t* procName) {
		DWORD procId = NULL;
		HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
		if (hSnap != INVALID_HANDLE_VALUE) {
			PROCESSENTRY32 procEntry;
			procEntry.dwSize = sizeof(procEntry);
			if (Process32First(hSnap, &procEntry)) {
				do {
					if (!_wcsicmp(procEntry.szExeFile, procName)) {
						procId = procEntry.th32ProcessID;
						break;
					}
				} while (Process32Next(hSnap, &procEntry));
			}
		}
		CloseHandle(hSnap);
		return procId;
	}

	uintptr_t GetModuleBaseAddress(DWORD procId, const wchar_t* modName) {
		uintptr_t modBaseAddr = NULL;
		HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, procId);
		if (hSnap != INVALID_HANDLE_VALUE) {
			MODULEENTRY32 modEntry;
			modEntry.dwSize = sizeof(modEntry);
			if (Module32First(hSnap, &modEntry)) {
				do {
					if (!_wcsicmp(modEntry.szModule, modName)) {
						modBaseAddr = (uintptr_t)modEntry.modBaseAddr;
						break;
					}
				} while (Module32Next(hSnap, &modEntry));
			}
		}
		CloseHandle(hSnap);
		return modBaseAddr;
	}
}

namespace mem {
	template<typename T>
	T RPM(uintptr_t address) {
		T buffer;
		ReadProcessMemory(proc::hProc, (LPCVOID)address, &buffer, sizeof(buffer), nullptr);
		return buffer;
	}

	template<typename T>
	void WPM(uintptr_t address, T buffer) {
		WriteProcessMemory(proc::hProc, (LPVOID)address, &buffer, sizeof(buffer), nullptr);
	}

	template<typename T>
	T ReadChain(uintptr_t base, std::vector<unsigned int> offsets) {
		uintptr_t address = base;
		for (unsigned int i = 0; i < offsets.size(); i++) {
			address = RPM<uintptr_t>(address);
			address += offsets[i];
		}
		return RPM<T>(address);
	}
}
