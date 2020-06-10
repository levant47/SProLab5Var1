#include <windows.h>
#include <string>
#include <vector>
#include <random>

const std::string window_class_name = "SPROLAB5 WINDOW";
const std::string window_title = "Lab 5";
const int IDM_PROCESS_OPEN = 1;
const int IDM_PROCESS_CLOSE = 2;
const int IDM_PROCESS_EXIT = 3;
const std::string notepad_path = "C:\\WINDOWS\\system32\\notepad.exe";
PROCESS_INFORMATION process_info { };
static int thread1_count = 0;
static int thread2_count = 0;
static int thread3_count = 0;
static int thread4_count = 0;
HANDLE thread1_handle = NULL;
HANDLE thread2_handle = NULL;
HANDLE thread3_handle = NULL;
HANDLE thread4_handle = NULL;
static bool mutex = false;
HANDLE realmutex = NULL;

void log(const std::string& message)
{
	OutputDebugString((message + '\n').c_str());
}

void launch_notepad()
{
	DWORD exit_code;
	GetExitCodeProcess(process_info.hProcess, &exit_code);
	if(exit_code == STILL_ACTIVE)
	{
		return;
	}

	STARTUPINFO startup_info { };
	char cmd[256] { };
	CreateProcess(notepad_path.c_str(), cmd, NULL, NULL, TRUE, 0, NULL, NULL, &startup_info, &process_info);
}

void close_editor(HWND window_handle, bool show_message = true)
{
	DWORD exit_code;
	GetExitCodeProcess(process_info.hProcess, &exit_code);
	if (exit_code != STILL_ACTIVE)
	{
		if (show_message)
		{
			MessageBox(window_handle, "Данний процес відсутній. Будь ласка, запустіть спочатку Ваш редактор!", "", MB_OK);
		}
		return;
	}

	TerminateProcess(process_info.hProcess, 0);
}

void handle_input(HWND window_handle, int message)
{
	switch (message)
	{
		case IDM_PROCESS_OPEN:
		{
			launch_notepad();
			break;
		}
		case IDM_PROCESS_CLOSE:
		{
			close_editor(window_handle);
			break;
		}
		case IDM_PROCESS_EXIT:
		{
			close_editor(window_handle, false);
			exit(0);
		}
	}
}

void grab()
{
	WaitForSingleObject(realmutex, INFINITE);
	/*
	while (mutex)
	{
		Sleep(1);
	}
	mutex = true;
	*/
}

void release()
{
	ReleaseMutex(realmutex);
	//mutex = false;
}

std::string get_naturals_message()
{
	std::string message;
	for (int i = 0; i < 25; i++)
	{
		message += std::to_string(i) + " ";
	}
	return message;
}

RECT get_window_coordinates(HWND window_handle)
{
	RECT window_coordinates;
	GetClientRect(window_handle, &window_coordinates);
	return window_coordinates;
}

POINT get_window_dimensions(HWND window_handle)
{
	RECT window_coordinates = get_window_coordinates(window_handle);
	return
	{
		window_coordinates.right - window_coordinates.left,
		window_coordinates.bottom - window_coordinates.top
	};
}

int fibonacci(int n)
{
	if (n == 0)
	{
		return 0;
	}

	if (n == 1)
	{
		return 1;
	}

	return fibonacci(n - 1) + fibonacci(n - 2);
}

std::string get_fibonacci_message()
{
	std::string message;
	for (int i = 0; i < 25; i++)
	{
		message += std::to_string(fibonacci(i)) + " ";
	}
	return message;
}

int get_random_number(int min, int max)
{
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<> rng { 0, max };
	return rng(gen);
}

void draw_thread4(HWND window_handle)
{
	POINT window_dimensions = get_window_dimensions(window_handle);

	std::string message = "thread 1: " + std::to_string(thread1_count) +
		", thread2: " +
		std::to_string(thread2_count) +
		", thread3: " +
		std::to_string(thread3_count) +
		", thread4: " +
		std::to_string(thread4_count);

	grab();

	PAINTSTRUCT paint_info;
	BeginPaint(window_handle, &paint_info);
	{
		HDC device_context_handle = GetDC(window_handle);

		TextOut(device_context_handle, window_dimensions.x / 2, window_dimensions.y / 2, message.c_str(), message.size());

		ReleaseDC(window_handle, device_context_handle);
	}
	EndPaint(window_handle, &paint_info);

	release();
}

DWORD WINAPI thread4(LPVOID parameters)
{
	HWND* window_handle = (HWND*)parameters;

	draw_thread4(*window_handle);

	thread4_count++;

	return 0;
}

void run_thread4(HWND window_handle)
{
	DWORD thread4_id = 2;
	thread4_handle = CreateThread(NULL, 0, thread4, &window_handle, 0, &thread4_id);
}

void draw_thread3(HWND window_handle)
{
	POINT window_dimensions = get_window_dimensions(window_handle);
	if (window_dimensions.x <= 0 || window_dimensions.y <= 0)
	{
		return;
	}

	grab();

	PAINTSTRUCT paint_info;
	BeginPaint(window_handle, &paint_info);
	{
		HDC device_context_handle = GetDC(window_handle);

		int x1 = get_random_number(0, window_dimensions.x / 2);
		int x2 = get_random_number(0, window_dimensions.x / 2 - x1);
		int y1 = get_random_number(0, window_dimensions.y / 2);
		int y2 = get_random_number(0, window_dimensions.y / 2 - y1);

		RECT rect;
		rect.left = x1;
		rect.right = x1 + x2;
		rect.top = window_dimensions.y / 2 + y1;
		rect.bottom = window_dimensions.y / 2 + y1 + y2;
		Rectangle(device_context_handle, rect.left, rect.top, rect.right, rect.bottom);

		ReleaseDC(window_handle, device_context_handle);
	}
	EndPaint(window_handle, &paint_info);

	release();
}

DWORD WINAPI thread3(LPVOID parameters)
{
	HWND* window_handle = (HWND*)parameters;

	draw_thread3(*window_handle);

	thread3_count++;

	return 0;
}

void run_thread3(HWND window_handle)
{
	DWORD thread3_id = 2;
	thread3_handle = CreateThread(NULL, 0, thread3, &window_handle, 0, &thread3_id);
}

void draw_thread2(HWND window_handle)
{
	static std::string message = get_fibonacci_message();

	grab();

	PAINTSTRUCT paint_info;
	BeginPaint(window_handle, &paint_info);
	{
		HDC device_context_handle = GetDC(window_handle);

		POINT window_dimensions = get_window_dimensions(window_handle);

		RECT area;
		area.left = window_dimensions.x / 2;
		area.right = window_dimensions.x - 10;
		area.top = 0;
		area.bottom = window_dimensions.y / 2 - 16;
		DrawText(device_context_handle, message.c_str(), message.length(), &area, DT_WORDBREAK);

		ReleaseDC(window_handle, device_context_handle);
	}
	EndPaint(window_handle, &paint_info);

	release();
}

DWORD WINAPI thread2(LPVOID parameters)
{
	HWND* window_handle = (HWND*)parameters;

	draw_thread2(*window_handle);

	thread2_count++;

	return 0;
}

void run_thread2(HWND window_handle)
{
	DWORD thread2_id = 1;
	thread2_handle = CreateThread(NULL, 0, thread2, &window_handle, 0, &thread2_id);
}

void draw_thread1(HWND window_handle)
{
	static std::string message = get_naturals_message();

	grab();

	PAINTSTRUCT paint_info;
	BeginPaint(window_handle, &paint_info);
	{
		HDC device_context_handle = GetDC(window_handle);

		POINT window_dimensions = get_window_dimensions(window_handle);

		RECT area;
		area.left = 0;
		area.right = window_dimensions.x / 2 - 10;
		area.top = 0;
		area.bottom = window_dimensions.y / 2 - 16;
		DrawText(device_context_handle, message.c_str(), message.length(), &area, DT_WORDBREAK);

		ReleaseDC(window_handle, device_context_handle);
	}
	EndPaint(window_handle, &paint_info);

	release();
}

DWORD WINAPI thread1(LPVOID parameters)
{
	HWND* window_handle = (HWND*)parameters;

	draw_thread1(*window_handle);

	thread1_count++;

	return 0;
}

void run_thread1(HWND window_handle)
{
	DWORD thread1_id = 0;
	thread1_handle = CreateThread(NULL, 0, thread1, &window_handle, 0, &thread1_id);
}

LRESULT CALLBACK window_message_handler(HWND window_handle, UINT message, WPARAM w_param, LPARAM l_param)
{
	switch (message)
	{
		case WM_COMMAND:
		{
			handle_input(window_handle, LOWORD(w_param));
			break;
		}
		case WM_CLOSE:
		{
			close_editor(window_handle, false);
			exit(0);
		}
		case WM_PAINT:
		{
			run_thread1(window_handle);
			run_thread2(window_handle);
			run_thread3(window_handle);
			run_thread4(window_handle);
			break;
		}
		default: return DefWindowProc(window_handle, message, w_param, l_param);
	}

	return 0;
}

void setup_ui(HWND window_handle)
{
	HMENU menu_bar = CreateMenu();
	HMENU menu = CreateMenu();

	AppendMenu(menu, MF_STRING, IDM_PROCESS_OPEN, "Створити процес");
	AppendMenu(menu, MF_STRING, IDM_PROCESS_CLOSE, "Завершити процес");
	AppendMenu(menu, MF_STRING, IDM_PROCESS_EXIT, "Припинити роботу");

	AppendMenu(menu_bar, MF_POPUP, (UINT_PTR)menu, "Процеси");
	SetMenu(window_handle, menu_bar);
}

int WINAPI WinMain(HINSTANCE program_instance, HINSTANCE, LPSTR, int start_options)
{
	WNDCLASS window_class { };
	window_class.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
	window_class.lpfnWndProc = (WNDPROC)window_message_handler;
	window_class.hInstance = program_instance;
	window_class.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	window_class.lpszClassName = window_class_name.c_str();

	RegisterClass(&window_class);

	HWND window_handle = CreateWindow
	(
		window_class_name.c_str(),
		window_title.c_str(),
		WS_SYSMENU | WS_THICKFRAME,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		NULL,
		NULL,
		program_instance,
		NULL
	);

	setup_ui(window_handle);

	realmutex = CreateMutex(NULL, FALSE, NULL);

	ShowWindow(window_handle, start_options);
	UpdateWindow(window_handle);

	MSG message;
	while (true)
	{
		BOOL get_message_successful = GetMessage(&message, window_handle, 0, 0);
		if (get_message_successful <= 0)
		{
			return 1;
		}

		TranslateMessage(&message);
		DispatchMessage(&message);
	}

	return 0;
}

