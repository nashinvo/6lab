#include <windows.h>
#include <ctime>
#include <iostream>

// Семафоры
static HANDLE mutex_semaphore; // Взаимоисключение на критических участках
static HANDLE full_semaphore;  // Буфер пуст, потребитель ждёт
static HANDLE empty_semaphore; // Буфер заполнен, производитель ждёт
// Общий круговой буфер и указатели 
static int in = 0;
static int out = 0;
static int count = 0;
// Объект, который используют процессы.
static int object = 0;
DWORD Buffer[4]; // Общая область буфера 

DWORD WINAPI ConsumerThread(LPVOID lpArg) { // Потребитель
	int threadnumber = (int)lpArg;
	// Потребитель 
	while (1) {
		WaitForSingleObject(full_semaphore, INFINITE); // Ожидаем объект в буфере 
		WaitForSingleObject(mutex_semaphore, INFINITE); // Общие глобальные переменные - используем мьютекс
		count--;
		if (count == 0) // Проверяем, не пустой ли буфер 
			printf("Потребитель считал объект %d из %d-го элемента буфера. Буфер пуст, потребитель ждёт появления новых объектов.\n", Buffer[out], out);
		else
			printf("Потребитель считал объект %d из %d-го элемента буфера.\n", Buffer[out], out);
		out = (out + 1) % 4; // для кольцевого переноса указателей, когда они достигают конца буфера.
		// Увеличиваем значение семафоров на 1
		ReleaseSemaphore(mutex_semaphore, 1, NULL);
		ReleaseSemaphore(empty_semaphore, 1, NULL);
		Sleep(rand() % 2000); // Случайная задержка для моделирования процесса считывания объекта из буфера 
	}
	return 0;
}

int main(int argc, TCHAR* argv[], TCHAR* envp[])
{
	setlocale(LC_ALL, "Russian");
	srand(time(NULL));
	HANDLE hThread1;
	DWORD dwThread1ID = 0;
	int nParameter = 1, i;
	// задание семафоров и их начальных и максимальных значений 
	mutex_semaphore = CreateSemaphore(NULL, 1, 1, TEXT("mutex")); // 1 для блокировки мьютекса	
	full_semaphore = CreateSemaphore(NULL, 0, 4, TEXT("full")); // 0 объектов в буфере 
	empty_semaphore = CreateSemaphore(NULL, 4, 4, TEXT("empty")); // 4 максимальное количество объектов в буфере 
	for (i = 0; i < 4; i++) Buffer[i] = 0;
	hThread1 = CreateThread(NULL, 0, ConsumerThread, (LPVOID)nParameter, 0, &dwThread1ID);
	// Производитель 
	while (1) {
		WaitForSingleObject(empty_semaphore, INFINITE); // Ожидание свободного пространства в буфере 
		WaitForSingleObject(mutex_semaphore, INFINITE); // Общие глобальные переменные - используем мьютекс 
		Buffer[in] = object; object++; // Добавляем новый объект в буфер 
		count++;
		if (count >= 4) // Проверяем, не полный ли буфер
			printf("Производитель поместил объект %d в %d-й элемент буфера. Буфер заполнен, производитель ждёт освобождения места.\n", Buffer[in], in);
		else
			printf("Производитель поместил объект %d в %d-й элемент буфера.\n", Buffer[in], in);
		in = (in + 1) % 4; // для кольцевого переноса указателей, когда они достигают конца буфера.
		// Увеличиваем значение семафоров на 1
		ReleaseSemaphore(mutex_semaphore, 1, NULL);
		ReleaseSemaphore(full_semaphore, 1, NULL);
		Sleep(rand() % 1000); // Случайная задержка для моделирования процесса производства нового объекта
	}
	CloseHandle(hThread1);
	CloseHandle(mutex_semaphore);
	CloseHandle(full_semaphore);
	CloseHandle(empty_semaphore);
	return 0;
}
