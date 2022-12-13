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
			"1 - вывод списка дисков\n"
			"2 - вывод информации о диске и размер свободного пространства\n"
			"3 - создание и удаление заданных каталогов\n"
			"4 - создание файлов в новых каталогах\n"
			"5 - копирование и перемещение файлов между каталогами\n"
			"6 - анализ и изменение атрибутов файлов\n"
			"7 - копирование файла с помощью операций перекрывающегося ввода-вывода\n"
			"0 - выход из программы\n");
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
		printf("Нажмите любую кнопку...");
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
		printf("Архивный файл или каталог\n");
	if (fileAttr & FILE_ATTRIBUTE_COMPRESSED)
		printf("Сжатый файл или каталог\n");
	if (fileAttr & FILE_ATTRIBUTE_DIRECTORY)
		printf("Каталог\n");
	if (fileAttr & FILE_ATTRIBUTE_ENCRYPTED)
		printf("Зашифрованный файл или каталог\n");
	if (fileAttr & FILE_ATTRIBUTE_HIDDEN)
		printf("Файл или каталог скрыт\n");
	if (fileAttr & FILE_ATTRIBUTE_INTEGRITY_STREAM)
		printf("Каталог или поток данных пользователя настраивается с целостностью (поддерживается только в томах ReFS)\n");
	if (fileAttr & FILE_ATTRIBUTE_NORMAL)
		printf("Файл, который не имеет других атрибутов\n");
	if (fileAttr & FILE_ATTRIBUTE_NOT_CONTENT_INDEXED)
		printf("Файл или каталог не индексируются службой индексирования содержимого\n");
	if (fileAttr & FILE_ATTRIBUTE_NO_SCRUB_DATA)
		printf("Поток данных пользователя, который не будет считываться с помощью средства проверки целостности фоновых данных (SCRUBBER AKA)\n");
	if (fileAttr & FILE_ATTRIBUTE_OFFLINE)
		printf("Данные файла недоступны немедленно\n");
	if (fileAttr & FILE_ATTRIBUTE_READONLY)
		printf("Файл, доступный только для чтения\n");
	if (fileAttr & FILE_ATTRIBUTE_RECALL_ON_DATA_ACCESS)
		printf("Файл или каталог не полностью присутствует локально\n");
	if (fileAttr & FILE_ATTRIBUTE_RECALL_ON_OPEN)
		printf("Файл или каталог не имеет физического представления в локальной системе; элемент является виртуальным\n");
	if (fileAttr & FILE_ATTRIBUTE_REPARSE_POINT)
		printf("Файл или каталог с связанной точкой повторного измерения или файлом, который является символьной ссылкой\n");
	if (fileAttr & FILE_ATTRIBUTE_SPARSE_FILE)
		printf("Файл, который является разреженным файлом\n");
	if (fileAttr & FILE_ATTRIBUTE_SYSTEM)
		printf("Файл или каталог, который операционная система использует частично или полностью\n");
	if (fileAttr & FILE_ATTRIBUTE_TEMPORARY)
		printf("Файл, используемый для временного хранилища\n");
	if (fileAttr & FILE_ATTRIBUTE_PINNED)
		printf("Файл или каталог должны быть полностью представлены локально, даже если доступ к ней не выполняется активно\n");
	if (fileAttr & FILE_ATTRIBUTE_UNPINNED)
		printf("Файл или каталог не должны храниться локально, за исключением случаев активного доступа\n");
}
// GetLogicalDrives, GetLogicalDriveStrings
void DiskList() {
	int val, disks, n;
	string tmp;
	DWORD Ddrives = 255; // Max buffer length
	char lpBuffer[255];
	printf(
		"Список дисков: \n"
		"1 - используя функцию GetLogicalDrives\n"
		"2 - используя функцию GetLogicalDriveStrings\n");
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
				if (n) cout << "Найден диск " << char(65 + i) << ":\\\n";
			}
		}
		break;
	case 2:
		n = GetLogicalDriveStrings(Ddrives, (LPWSTR)lpBuffer);
		printf("Найдены диски: ");
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
		"Информация о дисках: \n"
		"1 - получить тип диска\n"
		"2 - получить информацию о разделе\n"
		"3 - получить информацию о свободном месте на диске\n");
	cin >> tmp;
	val = _strtoi64(tmp.c_str(), nullptr, 10); // convert string to int
	printf("Введите букву диска: ");
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
				"Название раздела: %s\n"
				"Имя файловой системы: %s\n"
				"Серийный номер: %u\n"
				"Максимальная длина файла: %u\n"
				"Опции файловой системы: \n",
				lpVolumeNameBuffer,
				lpFileSystemNameBuffer,
				lpVolumeSerialNumber,
				lpMaximumComponentsLength);
			// decode system flags (from microsoft documentation)
			if (lpFileSystemFlags & FILE_CASE_SENSITIVE_SEARCH)
				printf("Указанный том поддерживает имена файлов с учетом регистра\n");
			if (lpFileSystemFlags & FILE_CASE_PRESERVED_NAMES)
				printf("Указанный том поддерживает сохраненный регистр имен файлов при расположении имени на диске\n");
			if (lpFileSystemFlags & FILE_UNICODE_ON_DISK)
				printf("Указанный том поддерживает Юникод в именах файлов, как они отображаются на диске\n");
			if (lpFileSystemFlags & FILE_PERSISTENT_ACLS)
				printf("Указанный том сохраняет и применяет списки управления доступом (ACL)\n");
			if (lpFileSystemFlags & FILE_FILE_COMPRESSION)
				printf("Указанный том поддерживает сжатие на основе файлов\n");
			if (lpFileSystemFlags & FILE_VOLUME_QUOTAS)
				printf("Указанный том поддерживает квоты дисков\n");
			if (lpFileSystemFlags & FILE_SUPPORTS_SPARSE_FILES)
				printf("Указанный том поддерживает разреженные файлы\n");
			if (lpFileSystemFlags & FILE_SUPPORTS_REPARSE_POINTS)
				printf("Указанный том поддерживает точки повторного анализа\n");
			if (lpFileSystemFlags & FILE_SUPPORTS_REMOTE_STORAGE)
				printf("Указанный том поддерживает удаленное хранилище\n");
			if (lpFileSystemFlags & FILE_VOLUME_IS_COMPRESSED)
				printf("Указанный том является сжатым томом\n");
			if (lpFileSystemFlags & FILE_SUPPORTS_OBJECT_IDS)
				printf("Указанный том поддерживает идентификаторы объектов\n");
			if (lpFileSystemFlags & FILE_SUPPORTS_ENCRYPTION)
				printf("Указанный том поддерживает зашифрованную файловую систему (EFS)\n");
			if (lpFileSystemFlags & FILE_NAMED_STREAMS)
				printf("Указанный том поддерживает именованные потоки\n");
			if (lpFileSystemFlags & FILE_READ_ONLY_VOLUME)
				printf("Указанный том доступен только для чтения\n");
			if (lpFileSystemFlags & FILE_SEQUENTIAL_WRITE_ONCE)
				printf("Указанный том поддерживает одну последовательную запись\n");
			if (lpFileSystemFlags & FILE_SUPPORTS_TRANSACTIONS)
				printf("Указанный том поддерживает транзакции\n");
			if (lpFileSystemFlags & FILE_SUPPORTS_HARD_LINKS)
				printf("Указанный том поддерживает жесткие связи\n");
			if (lpFileSystemFlags & FILE_SUPPORTS_EXTENDED_ATTRIBUTES)
				printf("Указанный том поддерживает расширенные атрибуты\n");
			if (lpFileSystemFlags & FILE_SUPPORTS_OPEN_BY_FILE_ID)
				printf("Файловая система поддерживает open by FileID\n");
			if (lpFileSystemFlags & FILE_SUPPORTS_USN_JOURNAL)
				printf("Указанный том поддерживает журналы обновления порядкового номера (USN)\n");
			if (lpFileSystemFlags & FILE_SUPPORTS_BLOCK_REFCOUNTING)
				printf("Указанный том поддерживает совместное использование логических кластеров между файлами на одном томе\n");
			if (lpFileSystemFlags & FILE_DAX_VOLUME)
				printf("Указанный том является прямым доступом (DAX)\n");
			//
		}
		else printf("Ошибка запроса\n");
		break;
	case 3:
		res = GetDiskFreeSpace(l, &lpSectorsPerCluster, &lpBytesPerSector, &lpNumberOfFreeClusters, &lpTotalNumberOfClusters);
		if (res) {
			printf(
				"Количество секторов в кластере: %u\n"
				"Количество байт в секторе: %u\n"
				"Количество свободных кластеров: %u\n"
				"Количество кластеров: %u\n",
				lpSectorsPerCluster,
				lpBytesPerSector,
				lpNumberOfFreeClusters,
				lpTotalNumberOfClusters);
		}
		else printf("Ошибка запроса\n");
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
		"Создание и удаление каталогов: \n"
		"1 - создать каталог\n"
		"2 - удалить каталог\n");
	cin >> tmp;
	val = _strtoi64(tmp.c_str(), nullptr, 10); // convert string to int
	switch (val)
	{
	case 1:
		printf("Введите полное название каталога (C:\\folder_name): ");
		cin >> tmp;
		// convert string to LPCWSTR
		wtmp = wstring(tmp.begin(), tmp.end());
		l = wtmp.c_str();
		//
		res = CreateDirectory(l, nullptr);
		if (res) {
			printf("Каталог успешно создан\n");
		}
		else printf("Ошибка создания каталога\n");
		break;
	case 2:
		printf("Введите полное название каталога (C:\\folder_name): ");
		cin >> tmp;
		// convert string to LPCWSTR
		wtmp = wstring(tmp.begin(), tmp.end());
		l = wtmp.c_str();
		//
		res = RemoveDirectory(l);
		if (res) {
			printf("Каталог успешно удален\n");
		}
		else printf("Ошибка удаления каталога\n");
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
	printf("Создание файла: \n");
	printf("Введите полное название файла (C:\\folder_name\\file_name.extension): ");
	cin >> tmp;
	// convert string to LPCWSTR
	wtmp = wstring(tmp.begin(), tmp.end());
	l = wtmp.c_str();
	//
	res = CreateFile(l, GENERIC_ALL, FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
	if (res) {
		printf("Файл успешно создан\n");
	}
	else printf("Ошибка создания файла\n");
}
// CopyFile, MoveFile, MoveFileEx
void CopyMoveFile() {
	string tmp, firsttmp, secondtmp;
	int val;
	wstring wfirsttmp, wsecondtmp;
	LPCWSTR first, second;
	DWORD res;
	printf(
		"Копирование и перемещение файлов: \n"
		"1 - копировать файл\n"
		"2 - переместить файл\n"
		"3 - переместить файл с проверкой на имена\n");
	cin >> tmp;
	val = _strtoi64(tmp.c_str(), nullptr, 10); // convert string to int
	printf("Введите полный исходный путь (C:\\folder_name\\file_name.extention): ");
	cin >> firsttmp;
	// convert string to LPCWSTR
	wfirsttmp = wstring(firsttmp.begin(), firsttmp.end());
	first = wfirsttmp.c_str();
	//
	printf("Введите полный конечный путь: ");
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
			printf("Файл успешно скопирован\n");
		}
		else printf("Ошибка копирования файла\n");
		break;
	case 2:
		res = MoveFile(first, second);
		if (res) {
			printf("Файл успешно перемещен\n");
		}
		else printf("Ошибка перемещения файла\n");
		break;
	case 3:
		res = MoveFileEx(first, second, NULL);
		if (res) {
			printf("Файл успешно перемещен\n");
		}
		else {
			res = GetLastError();
			if (GetLastError() == ERROR_ALREADY_EXISTS) {
				printf("Файл существует, заменить его? (Y/N): ");
				while (tmp != "Y" && tmp != "N") {
					cin >> tmp;
					if (tmp != "Y" && tmp != "N") printf("Введите Y или N\n");
				}
				if (tmp == "Y") {
					res = MoveFileEx(first, second, MOVEFILE_REPLACE_EXISTING);
					if (res) {
						printf("Файл успешно перемещен\n");
					}
					else printf("Ошибка перемещения файла\n");
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
		"Атрибуты файлов: \n"
		"1 - получить атрибуты файла\n"
		"2 - установить атрибуты файла\n"
		"3 - получить информацию о файле по дескриптору\n"
		"4 - информация о времени файла\n"
		"5 - изменить атрибуты времени файла\n");
	cin >> tmp;
	val = _strtoi64(tmp.c_str(), nullptr, 10); // convert string to int
	printf("Введите полное название файла (C:\\folder_name\\file_name.extention): ");
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
			printf("Атрибуты файла: \n");
			FileAttrOut(res);
		}
		else printf("Ошибка в получении атрибутов файла\n");
		break;
	case 2:
		printf(
			"Установить атрибуты. Введите последовательность через пробел, оканчивающуюся на 0 (1 2 5 0): \n"
			"1 - Архивный файл или каталог\n"
			"2 - Файл или каталог скрыт\n"
			"3 - Файл, который не имеет других атрибутов\n"
			"4 - Файл или каталог не индексируются службой индексирования содержимого\n"
			"5 - Данные файла недоступны немедленно\n"
			"6 - Файл, доступный только для чтения\n"
			"7 - Файл или каталог, который операционная система использует частично или полностью\n"
			"8 - Файл, используемый для временного хранилища\n");
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
				printf("Ошибка, введено неверное число\n");
				val = 0;
				res = 0;
				break;
			}
		}
		res = SetFileAttributes(filename, res);
		if (res) {
			printf("Атрибуты успешно установлены\n");
		}
		else printf("Ошибка установления атрибутов\n");
		break;
	case 3:
		handler = CreateFile(filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, NULL, NULL);
		res = GetFileInformationByHandle(handler, &handlerRes);
		if (res) {
			fileAttr = handlerRes.dwFileAttributes;
			printf("Атрибуты файла по дескриптору: \n");
			FileAttrOut(fileAttr);
			printf(
				"Информация о файле: \n"
				"Серийный номер тома, содержащего файл: %u\n"
				"Количество ссылок на этот файл: %u\n"
				"Нижний порядок части размера файла: %u\n"
				"Верхний порядок части размера файла: %u\n"
				"Нижний порядок идентификатора, связанного с файлом: %u\n"
				"Верхний порядок идентификатора, связанного с файлом: %u\n",
				handlerRes.dwVolumeSerialNumber,
				handlerRes.nNumberOfLinks,
				handlerRes.nFileSizeLow,
				handlerRes.nFileSizeHigh,
				handlerRes.nFileIndexLow,
				handlerRes.nFileIndexHigh);
		}
		else printf("Ошибка чтения атрибутов файла\n");
		CloseHandle(handler);
		break;
	case 4:
		handler = CreateFile(filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, NULL, NULL);
		res = GetFileTime(handler, &create, &lastAccess, &lastWrite);
		if (res) {
			FileTimeToSystemTime(&create, &sysTime);
			printf("Время создания файла: %i:%i:%i.%i %i.%i.%i\n", sysTime.wHour, sysTime.wMinute, sysTime.wSecond, sysTime.wMilliseconds, sysTime.wDay, sysTime.wMonth, sysTime.wYear);
			FileTimeToSystemTime(&lastAccess, &sysTime);
			printf("Время последнего доступа к файлу: %i:%i:%i.%i %i.%i.%i\n", sysTime.wHour, sysTime.wMinute, sysTime.wSecond, sysTime.wMilliseconds, sysTime.wDay, sysTime.wMonth, sysTime.wYear);
			FileTimeToSystemTime(&lastWrite, &sysTime);
			printf("Время последнего изменения файла: %i:%i:%i.%i %i.%i.%i\n", sysTime.wHour, sysTime.wMinute, sysTime.wSecond, sysTime.wMilliseconds, sysTime.wDay, sysTime.wMonth, sysTime.wYear);
		}
		else printf("Ошибка в чтении времени файла\n");
		break;
	case 5:
		handler = CreateFile(filename, FILE_WRITE_ATTRIBUTES, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, NULL, NULL);
		printf(
			"Изменение атрибута времени файла: \n"
			"1 - изменить время создания файла\n"
			"2 - изменить время последнего доступа к файлу\n"
			"3 - изменить время последнего изменения файла\n");
		cin >> tmp;
		val = _strtoi64(tmp.c_str(), nullptr, 10); // convert string to int
		printf("Введите год: ");
		cin >> tmp;
		year = _strtoi64(tmp.c_str(), nullptr, 10); // convert string to int
		printf("Введите месяц: ");
		cin >> tmp;
		month = _strtoi64(tmp.c_str(), nullptr, 10); // convert string to int
		printf("Введите день: ");
		cin >> tmp;
		day = _strtoi64(tmp.c_str(), nullptr, 10); // convert string to int
		printf("Введите час: ");
		cin >> tmp;
		hour = _strtoi64(tmp.c_str(), nullptr, 10); // convert string to int
		printf("Введите минуту: ");
		cin >> tmp;
		minute = _strtoi64(tmp.c_str(), nullptr, 10); // convert string to int
		printf("Введите секунду: ");
		cin >> tmp;
		second = _strtoi64(tmp.c_str(), nullptr, 10); // convert string to int
		printf("Введите миллисекунду: ");
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
			printf("Время файла успешно изменено\n");
		}
		else printf("Ошибка в изменении времени файла\n");
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
	printf("Введите размер блока: 4096 * "); // disk has sector size = 512, number of sectors in cluster = 8, 512 * 8 = 4096
	cin >> blockSize;
	printf("Введите число операций: ");
	cin >> operations;
	printf("Введите полный путь первого файла (C:\\folder\\filename.extention): ");
	cin >> tmp1;
	// convert string to LPCWSTR
	wpath1 = wstring(tmp1.begin(), tmp1.end());
	path1 = wpath1.c_str();
	//
	printf("Введите полный путь второго файла (C:\\folder\\filename.extention): ");
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
		printf("Файл успешно скопирован\n");
	else
		printf("Копирование файла завершилось с ошибкой: %i\n", err);
	//
	printf("Затрачено времени: %.3f\n", ((float)(timeEnd - timeStart)/1000));
	CloseHandle(handle1);
	CloseHandle(handle2);
}
