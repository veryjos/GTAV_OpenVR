#include "adminCheck.hpp"
#include <string>

int failure();

std::string getRegKey(const std::string& location, const std::string& name) {
	HKEY key;
	TCHAR value[1024];
	DWORD bufLen = 1024 * sizeof(TCHAR);
	long ret;
	ret = RegOpenKeyExA(HKEY_LOCAL_MACHINE, location.c_str(), 0, KEY_QUERY_VALUE, &key);
	if (ret != ERROR_SUCCESS) {
		return std::string();
	}
	ret = RegQueryValueExA(key, name.c_str(), 0, 0, (LPBYTE)value, &bufLen);
	RegCloseKey(key);
	if ((ret != ERROR_SUCCESS) || (bufLen > 1024 * sizeof(TCHAR))) {
		return std::string();
	}
	std::wstring wstringValue = std::wstring(value, (size_t)bufLen - 1);
	std::string stringValue(wstringValue.begin(), wstringValue.end());

	size_t i = stringValue.length();
	while (i > 0 && stringValue[i - 1] == '\0') {
		--i;
	}
	return stringValue.substr(0, i);
}

HKEY key;

void ClearRegex()
{
	// Attempt to clear registry values
	RegCloseKey(key);

	HKEY key;

	if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
		L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Windows",
		0, KEY_SET_VALUE | KEY_WOW64_64KEY, &key) == ERROR_SUCCESS)
	{
		std::string dllPath = "";

		RegSetValueEx(key,
			L"AppInit_DLLs", 0, REG_SZ,
			(const BYTE *)dllPath.c_str(), dllPath.length() + 1);

		DWORD value = 0;

		RegSetValueEx(key,
			L"LoadAppInit_DLLs", 0, REG_DWORD,
			(const BYTE *)&value, sizeof(value));
	}
}

int failure(char* err)
{
	ClearRegex();

	fprintf(stderr, err);

	printf("Press enter to close..\n");

	getchar();

	return 1;
}

int main()
{
	// Check if user has admin rights
	if (!IsCurrentUserLocalAdministrator())
	{
		fprintf(stderr, "You must have admin rights to launch GTA: V OpenVR\n");

		return 1;
	}

	// Use global hook in regex to hook GTA: V
	// Might be a better way, let's keep this temporary.

	if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
		L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Windows",
		0, KEY_SET_VALUE | KEY_WOW64_64KEY, &key) == ERROR_SUCCESS)
	{
		printf("Installing global hook for GTA: V..\n");

		std::wstring dllPath = L"F:\\moddedgta\\OVRInjectShim.dll";

		if (RegSetValueEx(key,
			L"AppInit_DLLs", 0, REG_SZ,
			(const BYTE *)dllPath.c_str(), dllPath.size() * 2 + 1) != ERROR_SUCCESS)
			return failure("Failed to set AppInit_DLLs\n");

		DWORD value = 1;

		if (RegSetValueEx(key,
			L"LoadAppInit_DLLs", 0, REG_DWORD,
			(const BYTE *)&value, sizeof(value)) != ERROR_SUCCESS)
			return failure("Failed to set LoadAppInit_DLLs\n");

		printf("Press enter to close..\n");

		RegCloseKey(key);

		getchar();
	}
	else
		return failure("Failed to open registry key at SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Windows");

	ClearRegex();

	return 0;
}

