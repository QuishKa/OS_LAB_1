#include <windows.h>
#include <iostream>
#include <string.h>

using namespace std;

int completions = 0;

void DiskList();
void DiskInfo();
void CreateDeleteDirectory();
void CreateFileInDirectory();
void CopyMoveFile();
void FileAttributes();
void NormalizeOutputString(char* begin, char* end);
void FileAttrOut(DWORD fileAttr);
void CopyFileTask();
void Completion(DWORD dwErrorCode, DWORD dwNumberOfBytesTransfered, LPOVERLAPPED lpOverlapped) { ++completions; }

class Menu {
private:
	static int chosen;
	static bool work;
	static void execute(int n) {
		if (!work) return;
		switch (n)
		{
		case 1: DiskList();
			break;
		case 2:
			DiskInfo();
			break;
		case 3:
			CreateDeleteDirectory();
			break;
		case 4:
			CreateFileInDirectory();
			break;
		case 5:
			CopyMoveFile();
			break;
		case 6:
			FileAttributes();
			break;
		case 7:
			CopyFileTask();
			break;
		default:
			break;
		}
	}
public:
	static void init() { chosen = 0; work = true; }
	static void PrintList() {
		printf(
			"1 - ����� ������ ������\n"
			"2 - ����� ���������� � ����� � ������ ���������� ������������\n"
			"3 - �������� � �������� �������� ���������\n"
			"4 - �������� ������ � ����� ���������\n"
			"5 - ����������� � ����������� ������ ����� ����������\n"
			"6 - ������ � ��������� ��������� ������\n"
			"7 - ����������� ����� � ������� �������� ���������������� �����-������\n"
			"0 - ����� �� ���������\n");
	}
	static void setChosen(int value) { chosen = value; work = value == 0 ? false : true; execute(value); }
	static bool getWork() { return work; }
};
int Menu::chosen;
bool Menu::work;

int main()
{
	string tmp;
	int val;
	Menu::init();
	SetConsoleCP(1251);
	SetConsoleOutputCP(1251);
	// menu loop
	while (Menu::getWork()) {
		system("cls");
		Menu::PrintList();
		cin >> tmp;
		val = _strtoi64(tmp.c_str(), nullptr, 10);
		Menu::setChosen(val);
		printf("������� ����� ������...");
		getchar();
		getchar();
	}

	return 0;
}
// deletes extra '\0' symbols from string ("N\0T\0F\0S\0\0\0" -> "NTFS\0") 
void NormalizeOutputString(char* begin, char* end) {
	bool stop = false;
	char tmp[100];
	int i = 0;
	for (char* ptr = begin; ptr != end && !stop; ++ptr) {
		if (*ptr == '\0') {
			if (*(ptr + 1) == '\0') stop = true;
		}
		else {
			tmp[i++] = *ptr;
		}
	}
	tmp[i] = '\0';
	for (char* ptr = begin, i = 0; ptr != end; ++ptr, ++i) {
		*ptr = tmp[i];
		if (tmp[i] == '\0') break;
	}
}
// reads file attributes
void FileAttrOut(DWORD fileAttr) {
	if (fileAttr & FILE_ATTRIBUTE_ARCHIVE)
		printf("�������� ���� ��� �������\n");
	if (fileAttr & FILE_ATTRIBUTE_COMPRESSED)
		printf("������ ���� ��� �������\n");
	if (fileAttr & FILE_ATTRIBUTE_DIRECTORY)
		printf("�������\n");
	if (fileAttr & FILE_ATTRIBUTE_ENCRYPTED)
		printf("������������� ���� ��� �������\n");
	if (fileAttr & FILE_ATTRIBUTE_HIDDEN)
		printf("���� ��� ������� �����\n");
	if (fileAttr & FILE_ATTRIBUTE_INTEGRITY_STREAM)
		printf("������� ��� ����� ������ ������������ ������������� � ������������ (�������������� ������ � ����� ReFS)\n");
	if (fileAttr & FILE_ATTRIBUTE_NORMAL)
		printf("����, ������� �� ����� ������ ���������\n");
	if (fileAttr & FILE_ATTRIBUTE_NOT_CONTENT_INDEXED)
		printf("���� ��� ������� �� ������������� ������� �������������� �����������\n");
	if (fileAttr & FILE_ATTRIBUTE_NO_SCRUB_DATA)
		printf("����� ������ ������������, ������� �� ����� ����������� � ������� �������� �������� ����������� ������� ������ (SCRUBBER AKA)\n");
	if (fileAttr & FILE_ATTRIBUTE_OFFLINE)
		printf("������ ����� ���������� ����������\n");
	if (fileAttr & FILE_ATTRIBUTE_READONLY)
		printf("����, ��������� ������ ��� ������\n");
	if (fileAttr & FILE_ATTRIBUTE_RECALL_ON_DATA_ACCESS)
		printf("���� ��� ������� �� ��������� ������������ ��������\n");
	if (fileAttr & FILE_ATTRIBUTE_RECALL_ON_OPEN)
		printf("���� ��� ������� �� ����� ����������� ������������� � ��������� �������; ������� �������� �����������\n");
	if (fileAttr & FILE_ATTRIBUTE_REPARSE_POINT)
		printf("���� ��� ������� � ��������� ������ ���������� ��������� ��� ������, ������� �������� ���������� �������\n");
	if (fileAttr & FILE_ATTRIBUTE_SPARSE_FILE)
		printf("����, ������� �������� ����������� ������\n");
	if (fileAttr & FILE_ATTRIBUTE_SYSTEM)
		printf("���� ��� �������, ������� ������������ ������� ���������� �������� ��� ���������\n");
	if (fileAttr & FILE_ATTRIBUTE_TEMPORARY)
		printf("����, ������������ ��� ���������� ���������\n");
	if (fileAttr & FILE_ATTRIBUTE_PINNED)
		printf("���� ��� ������� ������ ���� ��������� ������������ ��������, ���� ���� ������ � ��� �� ����������� �������\n");
	if (fileAttr & FILE_ATTRIBUTE_UNPINNED)
		printf("���� ��� ������� �� ������ ��������� ��������, �� ����������� ������� ��������� �������\n");
}
// GetLogicalDrives, GetLogicalDriveStrings
void DiskList() {
	int val, disks, n;
	string tmp;
	DWORD Ddrives = 255; // Max buffer length
	char lpBuffer[255];
	printf(
		"������ ������: \n"
		"1 - ��������� ������� GetLogicalDrives\n"
		"2 - ��������� ������� GetLogicalDriveStrings\n");
	cin >> tmp;
	val = _strtoi64(tmp.c_str(), nullptr, 10); // convert string to int
	switch (val)
	{
	case 1:
		disks = GetLogicalDrives();
		// decode answer
		if (disks != 0) {
			for (int i = 0; i < 26; i++) {
				n = ((disks >> i) & 1);
				if (n) cout << "������ ���� " << char(65 + i) << ":\\\n";
			}
		}
		break;
	case 2:
		n = GetLogicalDriveStrings(Ddrives, (LPWSTR)lpBuffer);
		printf("������� �����: ");
		// print answer w/o (int n * 2 represents number of output chars)
		for (int i = 0; i < n * 2; i++)
			printf("%c", lpBuffer[i]);
		printf("\n");
		break;
	default:
		break;
	}
}
// GetDriveType, GetVolumeInformation, GetDiskFreeSpace
void DiskInfo() {
	int val, disk;
	char diskLetter, lpVolumeNameBuffer[100], lpFileSystemNameBuffer[100];
	bool res;
	string tmp;
	wstring wtmp;
	LPCWSTR l;
	DWORD nVolumeNameSize = 100;
	unsigned long lpVolumeSerialNumber, lpMaximumComponentsLength, lpFileSystemFlags,
		lpSectorsPerCluster, lpBytesPerSector, lpNumberOfFreeClusters, lpTotalNumberOfClusters;
	printf(
		"���������� � ������: \n"
		"1 - �������� ��� �����\n"
		"2 - �������� ���������� � �������\n"
		"3 - �������� ���������� � ��������� ����� �� �����\n");
	cin >> tmp;
	val = _strtoi64(tmp.c_str(), nullptr, 10); // convert string to int
	printf("������� ����� �����: ");
	cin >> diskLetter;
	tmp = diskLetter; // save disk letter
	tmp.append(":\\"); // create path
	// convert string to LPCWSTR
	wtmp = wstring(tmp.begin(), tmp.end());
	l = wtmp.c_str();
	//
	switch (val)
	{
	case 1:
		disk = GetDriveType(l);
		// decode answer
		switch (disk)
		{
		case DRIVE_UNKNOWN:
			printf("UNKNOWN\n");
			break;
		case DRIVE_NO_ROOT_DIR:
			printf("DRIVE NO ROOT DIR\n");
			break;
		case DRIVE_REMOVABLE:
			printf("REMOVABLE\n");
			break;
		case DRIVE_FIXED:
			printf("FIXED\n");
			break;
		case DRIVE_REMOTE:
			printf("REMOTE\n");
			break;
		case DRIVE_CDROM:
			printf("CDROM\n");
			break;
		case DRIVE_RAMDISK:
			printf("RAMDISK\n");
			break;
		default:
			break;
		}
		break;
	case 2:
		res = GetVolumeInformation(l, (LPWSTR)lpVolumeNameBuffer, nVolumeNameSize, &lpVolumeSerialNumber, &lpMaximumComponentsLength, &lpFileSystemFlags, (LPWSTR)lpFileSystemNameBuffer, nVolumeNameSize);
		if (res) {
			// delete extra '\0' symbols
			NormalizeOutputString(begin(lpFileSystemNameBuffer), end(lpFileSystemNameBuffer));
			NormalizeOutputString(begin(lpVolumeNameBuffer), end(lpVolumeNameBuffer));
			//
			printf(
				"�������� �������: %s\n"
				"��� �������� �������: %s\n"
				"�������� �����: %u\n"
				"������������ ����� �����: %u\n"
				"����� �������� �������: \n",
				lpVolumeNameBuffer,
				lpFileSystemNameBuffer,
				lpVolumeSerialNumber,
				lpMaximumComponentsLength);
			// decode system flags (from microsoft documentation)
			if (lpFileSystemFlags & FILE_CASE_SENSITIVE_SEARCH)
				printf("��������� ��� ������������ ����� ������ � ������ ��������\n");
			if (lpFileSystemFlags & FILE_CASE_PRESERVED_NAMES)
				printf("��������� ��� ������������ ����������� ������� ���� ������ ��� ������������ ����� �� �����\n");
			if (lpFileSystemFlags & FILE_UNICODE_ON_DISK)
				printf("��������� ��� ������������ ������ � ������ ������, ��� ��� ������������ �� �����\n");
			if (lpFileSystemFlags & FILE_PERSISTENT_ACLS)
				printf("��������� ��� ��������� � ��������� ������ ���������� �������� (ACL)\n");
			if (lpFileSystemFlags & FILE_FILE_COMPRESSION)
				printf("��������� ��� ������������ ������ �� ������ ������\n");
			if (lpFileSystemFlags & FILE_VOLUME_QUOTAS)
				printf("��������� ��� ������������ ����� ������\n");
			if (lpFileSystemFlags & FILE_SUPPORTS_SPARSE_FILES)
				printf("��������� ��� ������������ ����������� �����\n");
			if (lpFileSystemFlags & FILE_SUPPORTS_REPARSE_POINTS)
				printf("��������� ��� ������������ ����� ���������� �������\n");
			if (lpFileSystemFlags & FILE_SUPPORTS_REMOTE_STORAGE)
				printf("��������� ��� ������������ ��������� ���������\n");
			if (lpFileSystemFlags & FILE_VOLUME_IS_COMPRESSED)
				printf("��������� ��� �������� ������ �����\n");
			if (lpFileSystemFlags & FILE_SUPPORTS_OBJECT_IDS)
				printf("��������� ��� ������������ �������������� ��������\n");
			if (lpFileSystemFlags & FILE_SUPPORTS_ENCRYPTION)
				printf("��������� ��� ������������ ������������� �������� ������� (EFS)\n");
			if (lpFileSystemFlags & FILE_NAMED_STREAMS)
				printf("��������� ��� ������������ ����������� ������\n");
			if (lpFileSystemFlags & FILE_READ_ONLY_VOLUME)
				printf("��������� ��� �������� ������ ��� ������\n");
			if (lpFileSystemFlags & FILE_SEQUENTIAL_WRITE_ONCE)
				printf("��������� ��� ������������ ���� ���������������� ������\n");
			if (lpFileSystemFlags & FILE_SUPPORTS_TRANSACTIONS)
				printf("��������� ��� ������������ ����������\n");
			if (lpFileSystemFlags & FILE_SUPPORTS_HARD_LINKS)
				printf("��������� ��� ������������ ������� �����\n");
			if (lpFileSystemFlags & FILE_SUPPORTS_EXTENDED_ATTRIBUTES)
				printf("��������� ��� ������������ ����������� ��������\n");
			if (lpFileSystemFlags & FILE_SUPPORTS_OPEN_BY_FILE_ID)
				printf("�������� ������� ������������ open by FileID\n");
			if (lpFileSystemFlags & FILE_SUPPORTS_USN_JOURNAL)
				printf("��������� ��� ������������ ������� ���������� ����������� ������ (USN)\n");
			if (lpFileSystemFlags & FILE_SUPPORTS_BLOCK_REFCOUNTING)
				printf("��������� ��� ������������ ���������� ������������� ���������� ��������� ����� ������� �� ����� ����\n");
			if (lpFileSystemFlags & FILE_DAX_VOLUME)
				printf("��������� ��� �������� ������ �������� (DAX)\n");
			//
		}
		else printf("������ �������\n");
		break;
	case 3:
		res = GetDiskFreeSpace(l, &lpSectorsPerCluster, &lpBytesPerSector, &lpNumberOfFreeClusters, &lpTotalNumberOfClusters);
		if (res) {
			printf(
				"���������� �������� � ��������: %u\n"
				"���������� ���� � �������: %u\n"
				"���������� ��������� ���������: %u\n"
				"���������� ���������: %u\n",
				lpSectorsPerCluster,
				lpBytesPerSector,
				lpNumberOfFreeClusters,
				lpTotalNumberOfClusters);
		}
		else printf("������ �������\n");
		break;
	default:
		break;
	}
}
// CreateDirectory, RemoveDirectory
void CreateDeleteDirectory() {
	string tmp;
	wstring wtmp;
	LPCWSTR l;
	bool res;
	int val;
	printf(
		"�������� � �������� ���������: \n"
		"1 - ������� �������\n"
		"2 - ������� �������\n");
	cin >> tmp;
	val = _strtoi64(tmp.c_str(), nullptr, 10); // convert string to int
	switch (val)
	{
	case 1:
		printf("������� ������ �������� �������� (C:\\folder_name): ");
		cin >> tmp;
		// convert string to LPCWSTR
		wtmp = wstring(tmp.begin(), tmp.end());
		l = wtmp.c_str();
		//
		res = CreateDirectory(l, nullptr);
		if (res) {
			printf("������� ������� ������\n");
		}
		else printf("������ �������� ��������\n");
		break;
	case 2:
		printf("������� ������ �������� �������� (C:\\folder_name): ");
		cin >> tmp;
		// convert string to LPCWSTR
		wtmp = wstring(tmp.begin(), tmp.end());
		l = wtmp.c_str();
		//
		res = RemoveDirectory(l);
		if (res) {
			printf("������� ������� ������\n");
		}
		else printf("������ �������� ��������\n");
		break;
	default:
		break;
	}
}
// CreateFile
void CreateFileInDirectory() {
	string tmp;
	wstring wtmp;
	LPCWSTR l;
	bool res;
	printf("�������� �����: \n");
	printf("������� ������ �������� ����� (C:\\folder_name\\file_name.extension): ");
	cin >> tmp;
	// convert string to LPCWSTR
	wtmp = wstring(tmp.begin(), tmp.end());
	l = wtmp.c_str();
	//
	res = CreateFile(l, GENERIC_ALL, FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
	if (res) {
		printf("���� ������� ������\n");
	}
	else printf("������ �������� �����\n");
}
// CopyFile, MoveFile, MoveFileEx
void CopyMoveFile() {
	string tmp, firsttmp, secondtmp;
	int val;
	wstring wfirsttmp, wsecondtmp;
	LPCWSTR first, second;
	DWORD res;
	printf(
		"����������� � ����������� ������: \n"
		"1 - ���������� ����\n"
		"2 - ����������� ����\n"
		"3 - ����������� ���� � ��������� �� �����\n");
	cin >> tmp;
	val = _strtoi64(tmp.c_str(), nullptr, 10); // convert string to int
	printf("������� ������ �������� ���� (C:\\folder_name\\file_name.extention): ");
	cin >> firsttmp;
	// convert string to LPCWSTR
	wfirsttmp = wstring(firsttmp.begin(), firsttmp.end());
	first = wfirsttmp.c_str();
	//
	printf("������� ������ �������� ����: ");
	cin >> secondtmp;
	// convert string to LPCWSTR
	wsecondtmp = wstring(secondtmp.begin(), secondtmp.end());
	second = wsecondtmp.c_str();
	//
	switch (val)
	{
	case 1:
		res = CopyFile(first, second, false);
		if (res) {
			printf("���� ������� ����������\n");
		}
		else printf("������ ����������� �����\n");
		break;
	case 2:
		res = MoveFile(first, second);
		if (res) {
			printf("���� ������� ���������\n");
		}
		else printf("������ ����������� �����\n");
		break;
	case 3:
		res = MoveFileEx(first, second, NULL);
		if (res) {
			printf("���� ������� ���������\n");
		}
		else {
			res = GetLastError();
			if (GetLastError() == ERROR_ALREADY_EXISTS) {
				printf("���� ����������, �������� ���? (Y/N): ");
				while (tmp != "Y" && tmp != "N") {
					cin >> tmp;
					if (tmp != "Y" && tmp != "N") printf("������� Y ��� N\n");
				}
				if (tmp == "Y") {
					res = MoveFileEx(first, second, MOVEFILE_REPLACE_EXISTING);
					if (res) {
						printf("���� ������� ���������\n");
					}
					else printf("������ ����������� �����\n");
				}
			}
		}
		break;
	default:
		break;
	}
}
// GetFileAttributes, SetFileAttributes, GetFileInformationByHandle, GetFileTime, SetFileTime
void FileAttributes() {
	string tmp, tmpfilename;
	int val, year, month, day, hour, minute, second, millisecond;
	wstring wtmpfilename;
	LPCWSTR filename;
	DWORD res, fileAttr;
	HANDLE handler;
	BY_HANDLE_FILE_INFORMATION handlerRes;
	FILETIME create, lastAccess, lastWrite;
	SYSTEMTIME sysTime;
	printf(
		"�������� ������: \n"
		"1 - �������� �������� �����\n"
		"2 - ���������� �������� �����\n"
		"3 - �������� ���������� � ����� �� �����������\n"
		"4 - ���������� � ������� �����\n"
		"5 - �������� �������� ������� �����\n");
	cin >> tmp;
	val = _strtoi64(tmp.c_str(), nullptr, 10); // convert string to int
	printf("������� ������ �������� ����� (C:\\folder_name\\file_name.extention): ");
	cin >> tmpfilename;
	// convert string to LPCWSTR
	wtmpfilename = wstring(tmpfilename.begin(), tmpfilename.end());
	filename = wtmpfilename.c_str();
	//
	switch (val)
	{
	case 1:
		res = GetFileAttributes(filename);
		if (res) {
			printf("�������� �����: \n");
			FileAttrOut(res);
		}
		else printf("������ � ��������� ��������� �����\n");
		break;
	case 2:
		printf(
			"���������� ��������. ������� ������������������ ����� ������, �������������� �� 0 (1 2 5 0): \n"
			"1 - �������� ���� ��� �������\n"
			"2 - ���� ��� ������� �����\n"
			"3 - ����, ������� �� ����� ������ ���������\n"
			"4 - ���� ��� ������� �� ������������� ������� �������������� �����������\n"
			"5 - ������ ����� ���������� ����������\n"
			"6 - ����, ��������� ������ ��� ������\n"
			"7 - ���� ��� �������, ������� ������������ ������� ���������� �������� ��� ���������\n"
			"8 - ����, ������������ ��� ���������� ���������\n");
		res = 0;
		while (val != 0)
		{
			cin >> tmp;
			val = _strtoi64(tmp.c_str(), nullptr, 10); // convert string to int
			switch (val)
			{
			case 0:
				break;
			case 1:
				res |= FILE_ATTRIBUTE_ARCHIVE;
				break;
			case 2:
				res |= FILE_ATTRIBUTE_HIDDEN;
				break;
			case 3:
				res |= FILE_ATTRIBUTE_NORMAL;
				break;
			case 4:
				res |= FILE_ATTRIBUTE_NOT_CONTENT_INDEXED;
				break;
			case 5:
				res |= FILE_ATTRIBUTE_OFFLINE;
				break;
			case 6:
				res |= FILE_ATTRIBUTE_READONLY;
				break;
			case 7:
				res |= FILE_ATTRIBUTE_SYSTEM;
				break;
			case 8:
				res |= FILE_ATTRIBUTE_TEMPORARY;
				break;
			default:
				printf("������, ������� �������� �����\n");
				val = 0;
				res = 0;
				break;
			}
		}
		res = SetFileAttributes(filename, res);
		if (res) {
			printf("�������� ������� �����������\n");
		}
		else printf("������ ������������ ���������\n");
		break;
	case 3:
		handler = CreateFile(filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, NULL, NULL);
		res = GetFileInformationByHandle(handler, &handlerRes);
		if (res) {
			fileAttr = handlerRes.dwFileAttributes;
			printf("�������� ����� �� �����������: \n");
			FileAttrOut(fileAttr);
			printf(
				"���������� � �����: \n"
				"�������� ����� ����, ����������� ����: %u\n"
				"���������� ������ �� ���� ����: %u\n"
				"������ ������� ����� ������� �����: %u\n"
				"������� ������� ����� ������� �����: %u\n"
				"������ ������� ��������������, ���������� � ������: %u\n"
				"������� ������� ��������������, ���������� � ������: %u\n",
				handlerRes.dwVolumeSerialNumber,
				handlerRes.nNumberOfLinks,
				handlerRes.nFileSizeLow,
				handlerRes.nFileSizeHigh,
				handlerRes.nFileIndexLow,
				handlerRes.nFileIndexHigh);
		}
		else printf("������ ������ ��������� �����\n");
		CloseHandle(handler);
		break;
	case 4:
		handler = CreateFile(filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, NULL, NULL);
		res = GetFileTime(handler, &create, &lastAccess, &lastWrite);
		if (res) {
			FileTimeToSystemTime(&create, &sysTime);
			printf("����� �������� �����: %i:%i:%i.%i %i.%i.%i\n", sysTime.wHour, sysTime.wMinute, sysTime.wSecond, sysTime.wMilliseconds, sysTime.wDay, sysTime.wMonth, sysTime.wYear);
			FileTimeToSystemTime(&lastAccess, &sysTime);
			printf("����� ���������� ������� � �����: %i:%i:%i.%i %i.%i.%i\n", sysTime.wHour, sysTime.wMinute, sysTime.wSecond, sysTime.wMilliseconds, sysTime.wDay, sysTime.wMonth, sysTime.wYear);
			FileTimeToSystemTime(&lastWrite, &sysTime);
			printf("����� ���������� ��������� �����: %i:%i:%i.%i %i.%i.%i\n", sysTime.wHour, sysTime.wMinute, sysTime.wSecond, sysTime.wMilliseconds, sysTime.wDay, sysTime.wMonth, sysTime.wYear);
		}
		else printf("������ � ������ ������� �����\n");
		break;
	case 5:
		handler = CreateFile(filename, FILE_WRITE_ATTRIBUTES, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, NULL, NULL);
		printf(
			"��������� �������� ������� �����: \n"
			"1 - �������� ����� �������� �����\n"
			"2 - �������� ����� ���������� ������� � �����\n"
			"3 - �������� ����� ���������� ��������� �����\n");
		cin >> tmp;
		val = _strtoi64(tmp.c_str(), nullptr, 10); // convert string to int
		printf("������� ���: ");
		cin >> tmp;
		year = _strtoi64(tmp.c_str(), nullptr, 10); // convert string to int
		printf("������� �����: ");
		cin >> tmp;
		month = _strtoi64(tmp.c_str(), nullptr, 10); // convert string to int
		printf("������� ����: ");
		cin >> tmp;
		day = _strtoi64(tmp.c_str(), nullptr, 10); // convert string to int
		printf("������� ���: ");
		cin >> tmp;
		hour = _strtoi64(tmp.c_str(), nullptr, 10); // convert string to int
		printf("������� ������: ");
		cin >> tmp;
		minute = _strtoi64(tmp.c_str(), nullptr, 10); // convert string to int
		printf("������� �������: ");
		cin >> tmp;
		second = _strtoi64(tmp.c_str(), nullptr, 10); // convert string to int
		printf("������� ������������: ");
		cin >> tmp;
		millisecond = _strtoi64(tmp.c_str(), nullptr, 10); // convert string to int
		sysTime.wYear = year;
		sysTime.wMonth = month;
		sysTime.wDay = day;
		sysTime.wHour = hour;
		sysTime.wMinute = minute;
		sysTime.wSecond = second;
		sysTime.wMilliseconds = millisecond;
		SystemTimeToFileTime(&sysTime, &create);
		switch (val) {
		case 1:
			res = SetFileTime(handler, &create, NULL, NULL);
			break;
		case 2:
			res = SetFileTime(handler, NULL, &create, NULL);
			break;
		case 3:
			res = SetFileTime(handler, NULL, NULL, &create);
			break;
		default:
			res = 0;
			break;
		}
		if (res) {
			printf("����� ����� ������� ��������\n");
		}
		else printf("������ � ��������� ������� �����\n");
		break;
	default:
		break;
	}
}
// task 2 - APC
void CopyFileTask() {
	int blockSize, operations, totalBlockSize, countIter;
	long fileSize, finalFileSize;
	char* buffer1, * buffer2, * pToBuf;
	bool flagFirstEnterRead = false, flagFirstEnterWrite = false;
	string tmp1, tmp2;
	wstring wpath1, wpath2;
	LPCWSTR path1, path2;
	HANDLE handle1, handle2;
	DWORD timeStart, timeEnd, err;
	OVERLAPPED* overlapped1, * overlapped2;
	printf("������� ������ �����: 4096 * "); // disk has sector size = 512, number of sectors in cluster = 8, 512 * 8 = 4096
	cin >> blockSize;
	printf("������� ����� ��������: ");
	cin >> operations;
	printf("������� ������ ���� ������� ����� (C:\\folder\\filename.extention): ");
	cin >> tmp1;
	// convert string to LPCWSTR
	wpath1 = wstring(tmp1.begin(), tmp1.end());
	path1 = wpath1.c_str();
	//
	printf("������� ������ ���� ������� ����� (C:\\folder\\filename.extention): ");
	cin >> tmp2;
	// convert string to LPCWSTR
	wpath2 = wstring(tmp2.begin(), tmp2.end());
	path2 = wpath2.c_str();
	//
	// main script
	timeStart = GetTickCount(); // time
	totalBlockSize = 4096 * blockSize;
	// create handlers
	handle1 = CreateFile(path1, GENERIC_READ, NULL, NULL, OPEN_EXISTING, FILE_FLAG_NO_BUFFERING | FILE_FLAG_OVERLAPPED, NULL);
	handle2 = CreateFile(path2, GENERIC_READ | GENERIC_WRITE, NULL, NULL, CREATE_ALWAYS, FILE_FLAG_NO_BUFFERING | FILE_FLAG_OVERLAPPED, NULL);
	//
	fileSize = GetFileSize(handle1, NULL);
	finalFileSize = fileSize; // save final size for later
	pToBuf = new char[totalBlockSize * operations]; // poiter to buffer
	overlapped1 = new OVERLAPPED[operations];
	overlapped2 = new OVERLAPPED[operations];
	for (int i = 0; i < operations; ++i) {
		overlapped1[i].Offset = 0;
		overlapped1[i].OffsetHigh = 0;
		overlapped1[i].hEvent = NULL;
		overlapped2[i].Offset = 0;
		overlapped2[i].OffsetHigh = 0;
		overlapped2[i].hEvent = NULL;
	}
	do {
		completions = 0; // completed Ex-operations
		countIter = 0; // count executed Ex-operations (end of file may require less operations to read)
		// return buffers to their start positions
		buffer1 = pToBuf;
		buffer2 = pToBuf;
		//
		for (int i = 0; i < operations; ++i) {
			if (fileSize > 0) {
				++countIter;
				if (flagFirstEnterRead) { // we don't need to push offsets for the first read
					for (int j = 0; j < operations; ++j) {
						overlapped1[j].Offset += totalBlockSize; // push all offsets to the number of executed Ex-operations
					}
				}
				if (i) buffer1 += totalBlockSize; // we don't need to shift buffer for first in loop read
				flagFirstEnterRead = true;
				ReadFileEx(handle1, buffer1, totalBlockSize, &overlapped1[i], Completion);
				fileSize -= totalBlockSize;
			}
		}
		while (completions < countIter) // wait for all reads
			SleepEx(-1, true);
		completions = 0;
		for (int i = 0; i < operations; ++i) {
			if (flagFirstEnterWrite) { // we don't need to push offsets for the first write
				for (int j = 0; j < operations; ++j) {
					overlapped2[j].Offset += totalBlockSize; // push all offsets to the number of executed Ex-operations
				}
			}
			if (i) buffer2 += totalBlockSize; // we don't need to shift buffer for first in loop write
			flagFirstEnterWrite = true;
			WriteFileEx(handle2, buffer2, totalBlockSize, &overlapped2[i], Completion);
		}
		while (completions < countIter) // wait for all writes
			SleepEx(-1, true);
	} while (fileSize > 0);
	SetFilePointer(handle2, finalFileSize, NULL, FILE_BEGIN);
	SetEndOfFile(handle2);
	timeEnd = GetTickCount(); // time end
	err = GetLastError();
	if (err == ERROR_SUCCESS || err == ERROR_ALREADY_EXISTS) // createfile func drops unnecessary in our case ERROR_ALREADY_EXISTS error
		printf("���� ������� ����������\n");
	else
		printf("����������� ����� ����������� � �������: %i\n", err);
	//
	printf("��������� �������: %.3f\n", ((float)(timeEnd - timeStart)/1000));
	CloseHandle(handle1);
	CloseHandle(handle2);
}
