// wordle.cpp : Defines the entry point for the application.
//

#include "pch.h"
#include "framework.h"
#include "wordle.h"
#include <time.h>
#include <string>
#include <future>

#define MAX_LOADSTRING 100
#define IDM_APP_MESS (WM_APP+1)
#define IDM_Animate (WM_APP+2)

DIFFICULTY curDif;

LPCWSTR fileName = L"\\cfg.ini";

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szTitleGame[MAX_LOADSTRING];
WCHAR szWindowClass[MAX_LOADSTRING]; // the main window class name
WCHAR LVL[10];

const int sizeX = 650;
const int sizeY = 250;

const int sizeXEasy = 61*2 + 66*3 + 6;
const int sizeYEasy = 66*6+12;

const int sizeXMedium = 61 * 2 + 66 * 3 + 6;
const int sizeYMedium = 66 * 8 + 6;

const int sizeXHard = 61 * 2 + 66 * 3 + 6;
const int sizeYHard = 66 * 10;

int cmdSho;

bool MouseButtonPressed = false;
POINT WindowPoint;

HWND* Overlay = new HWND[4];
GameRectangles* GameRect[4];
Frame* frames[11];
Keyboard* keyboard;
WordDictionary* Word_dic;

clock_t now = clock();
WCHAR timer[MAX_LOADSTRING];
int miliseconds = 0;

HWND EasyWindow[4];

// Deletes All objects from Main
void DeleteRect(int amount)
{
    for (int i = 0; i < amount; i++)
        delete GameRect[i];
}

// Writes to cfg.ini file which difficulty is currently selected
void WriteToIni() {
    if (GetFileAttributes(fileName) == INVALID_FILE_ATTRIBUTES)
    {
        HANDLE newfile = CreateFile(fileName, GENERIC_WRITE | GENERIC_READ, 0, NULL,
            CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
        CloseHandle(newfile);
    }
    WritePrivateProfileString(L"Wordle", L"LVL", DifToWChar(curDif), fileName);
}

void KillAllTimers(HWND);

// sets colors for overlay
void ColorBackground(HWND, COLORREF, std::string);
void SendAnimationMessageToAllWindows(UINT, int);


// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
ATOM                MyRegisterClassEasy(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK    WndProcEasy(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

// Main function
int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    GetPrivateProfileString(L"Wordle", L"LVL", L"EASY", LVL, 9, fileName);

    curDif = LToDiff(LVL);
    cmdSho = nCmdShow;
    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDS_TITLE, szTitleGame, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_WORDLE, szWindowClass, MAX_LOADSTRING);

    // Utworzenie klas dla poszczegolnych okienek
    MyRegisterClass(hInstance);
    MyRegisterClassEasy(hInstance);

    using namespace Gdiplus;
    GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR           gdiplusToken;

    // Initialize GDI+.
    GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_WORDLE));

    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    GdiplusShutdown(gdiplusToken);

    return (int) msg.wParam;
}

//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_WORDLE));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = CreateSolidBrush(RGB(255, 255, 255));
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_WORDLE);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_WORDLE));

    return RegisterClassExW(&wcex);
}

//
//  FUNCTION: MyRegisterClassEasy()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClassEasy(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProcEasy;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = NULL;
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = CreateSolidBrush(RGB(255, 255, 255));
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = L"Easy";
    wcex.hIconSm = NULL;

    return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      GetSystemMetrics(SM_CXSCREEN)/2 - sizeX/2, GetSystemMetrics(SM_CYSCREEN) - sizeY-100, sizeX, sizeY, nullptr, nullptr, hInstance, nullptr);

   keyboard = new Keyboard(hWnd);
   Word_dic = new WordDictionary();

   HMENU menu = GetMenu(hWnd);
   
   switch (curDif)
   {
   case EASY:
       CheckMenuItem(menu, IDM_EASY, MF_CHECKED);
       SendMessage(hWnd,WM_COMMAND,IDM_EASY, 0);
       break;
   case MEDIUM:
       CheckMenuItem(menu, IDM_MEDIUM, MF_CHECKED);
       SendMessage(hWnd, WM_COMMAND, IDM_MEDIUM, 0);
       break;
   case HARD:
       CheckMenuItem(menu, IDM_MEDIUM, MF_CHECKED);
       SendMessage(hWnd, WM_COMMAND, IDM_HARD, 0);
       break;
   }


   if (!hWnd)
   {
      return FALSE;
   }

   SetWindowLong(hWnd, GWL_EXSTYLE,
       GetWindowLong(hWnd, GWL_EXSTYLE) | WS_EX_LAYERED);
   // Make this window 50% alpha
   SetLayeredWindowAttributes(hWnd, 0, (255 * 80) / 100, LWA_ALPHA);
   // Show this window
   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//

// handler dla okienka z wordle puzzle
LRESULT CALLBACK WndProcEasy(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{

    switch (message)
    {
    case WM_LBUTTONDOWN:
    {
        MouseButtonPressed = true;
        GetCursorPos(&WindowPoint);
        ScreenToClient(hWnd, &WindowPoint);
    }
    break;
    case WM_LBUTTONUP:
    {
        MouseButtonPressed = false;
    }
    break;
    case WM_MOUSEMOVE:
    {
        if (MouseButtonPressed == false)
            break;

        POINT p;
        GetCursorPos(&p);
        RECT rec;
        GetWindowRect(hWnd, &rec);
        MoveWindow(hWnd, p.x-WindowPoint.x-10, p.y-WindowPoint.y-35,rec.right-rec.left, rec.bottom-rec.top, false);
    }
    break;
    case IDM_Animate:
    {
        int i = int(wParam);
        int index = int(lParam);
        GameRect[i]->AnimateRec(GameRect[i]->GetRect(index),
            index);
    }
    break;
    case IDM_APP_MESS:
    {
        int WindowNumber = int(wParam);
        COLORREF background = COLORREF(lParam);

        GameRect[WindowNumber]->CreateOverlay(background);
    }
    break;
    case WM_COMMAND:
    {
        int wmId = LOWORD(wParam);
        // Parse the menu selections:
        switch (wmId)
        {
        case IDM_ABOUT:
            DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
            break;
        case IDM_EXIT:
            DestroyWindow(hWnd);
            break;
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
    }
    break;
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        // TODO: Add any drawing code that uses hdc here...

        HPEN pen = CreatePen(PS_SOLID, 2, Colors::Gray);
        HPEN oldPen = (HPEN)SelectObject(hdc, pen);
        HBRUSH brush = CreateSolidBrush(Colors::White);
        HBRUSH oldbrush = (HBRUSH)SelectObject(hdc, brush);
        srand(time(NULL));
        DeleteRect(DiffToWindowAmount(curDif));

        for (int i = 0; i < int(curDif); i++)
        {
            for (int j = 0; j < 5; j++)
            {
                RoundRect( hdc, 6 * (j + 1) + 55 * j, 6 * (i + 1) + i * 55,
                    61 * (j + 1), 61*(i+1), 5,5);
            }
        }
        SelectObject(hdc, oldbrush);
        DeleteObject(brush);
        DeleteObject(pen);
        for(int i=0; i<DiffToWindowAmount(curDif); i++)
            GameRect[i] = new GameRectangles(EasyWindow[i], curDif, Word_dic);

        EndPaint(hWnd, &ps);
    }
    break;
    case WM_DESTROY:
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Handler for WholeApp
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    srand(time(NULL));

    switch (message)
    {
    #pragma region Timer

    case WM_TIMER:
    {
        if (int(wParam) - 100 >= 0)
        {
            if(int(wParam) - 200 >= 0)
                GameRect[int(wParam) - 200]->CreateOverlay(Colors::Red);
            else
                GameRect[int(wParam) - 100]->CreateOverlay(Colors::Green);
            KillTimer(hWnd, wParam);
            break;
        }

        int row = wParam;
        if (!frames[row]->AnimateFrame(DiffToWindowAmount(curDif), GameRect))
        {
            KillTimer(hWnd, row);
            delete frames[row];
            frames[row] = nullptr;            
        }
        
    }
    break;

#pragma endregion
    
    #pragma region WindowSameSize


    case WM_GETMINMAXINFO:
    {
        MINMAXINFO* min = (MINMAXINFO*)lParam;
        min->ptMinTrackSize.x = min->ptMaxTrackSize.x = min->ptMaxSize.x = sizeX;
        min->ptMinTrackSize.y = min->ptMaxTrackSize.y = min->ptMaxSize.y = sizeY;      
    }
    break;

#pragma endregion

    #pragma region KeyBoard
    case WM_KEYDOWN:
    {
        for (int i = 0; i < DiffToWindowAmount(curDif); i++)
        {
            if (GameRect[i]->GuessedWordIsChose())
                continue;
            if (GameRect[i]->lastWord)
                continue;
            HDC hdc = GetDC(GameRect[i]->Window);

            char code = char(wParam);
            // ENTER KEY
            if (int(code) == VK_RETURN)
            {
                if (GameRect[i]->GetNumberRect() / 5 == GameRect[i]->GetRow())
                {
                    // if words does not exist...
                    if (!Word_dic->IsWord(GameRect[i]->CreateString()))
                    {
                        // delete last 5 letters
                        for (int i = 0; i < 5; i++)
                            SendMessage(hWnd, WM_KEYDOWN, VK_BACK, i);
                        break;
                    }
                    // coloring keyboard
                    for (int j = 5; j >= 1; j--)
                    {
                        char letter = GameRect[i]->GetChar(GameRect[i]->GetNumberRect() - j);

                        COLORREF color = Colors::Gray;
                        if (GameRect[i]->IsInWord(letter))
                            color = Colors::Yellow;
                        if (GameRect[i]->IsOnPosition(letter, 5 - j))
                            color = Colors::Green;

                        RECT rec = keyboard->CharToRect(letter);
                        RECT offset = { 0,0,0,0 };

                        // coloring half
                        if (curDif == MEDIUM)
                        {
                            if (i == 0)
                                offset = { 0,0,-27,0 };
                            else
                                offset = { 27,0,0,0 };
                        }
                        if (curDif == HARD)
                        {
                            if (i == 0)
                                // left upp corner
                                offset = { 0,0,-27,-27 };
                            else if (i == 1)
                                // left bottom
                                offset = { 0,27,-27,0 };
                            else if (i == 2)
                                // right upp
                                offset = { 27,0,0,-27 };
                            else
                                // right bottom
                                offset = { 27,27,0,0 };
                        }
                        // execute color
                        keyboard->WriteLetter(hWnd, letter, color, rec, color, offset);

                    }
                    // Animataing letters
                    //for (int j = 5; j >= 1; j--)
                    //{
                    //    int index = GameRect[i]->GetNumberRect() - j;
                    //    //SendMessage(GameRect[i]->Window, IDM_Animate, i, index);
                    //    GameRect[i]->AnimateRec(GameRect[i]->GetRect(index),
                    //        index);
                    //    

                    //    /*if(frames[GameRect[i]->GetRow()]==nullptr)
                    //        frames[GameRect[i]->GetRow()] = new Frame(GameRect[i]->GetRect(index),
                    //        GameRect[i]->GetChar(index),)*/
                    //    //SetTimer(hWnd, GameRect[i]->GetRow(), 100, NULL);
                    //                       
                    //}
                    if (frames[GameRect[i]->GetRow()] == nullptr)
                    {
                        frames[GameRect[i]->GetRow()] = new Frame(GameRect[i]->GetRow());
                        SetTimer(hWnd, GameRect[i]->GetRow(), 5, NULL);
                    }
                    if (GameRect[i]->IsWordGuessed())
                    {
                        SetTimer(hWnd, i + 100, 8 * 5 * 55, NULL);
                        //GameRect[i]->CreateOverlay(Colors::Green);
                    }
                    if(GameRect[i]->GetRow() == int(curDif) && !GameRect[i]->IsWordGuessed())
                        //SendMessage(GameRect[i]->Window, IDM_APP_MESS, i, Colors::Red);
                        SetTimer(hWnd, i + 200, 8 * 5 * 55, NULL);
                    GameRect[i]->IncreaseRow();
                }
                continue;
            }
            // Backspace
            if (int(code) == VK_BACK)
            {
                if (GameRect[i]->GetNumberRect() % 5 != 0 ||
                    GameRect[i]->GetNumberRect() / 5 == GameRect[i]->GetRow())
                {
                    GameRect[i]->Previus_Rect();
                    GameRect[i]->ClearRect(Colors::White, GameRect[i]->GetRect());
                }
                if (i == DiffToWindowAmount(curDif) - 1)
                    break;
                else
                    continue;
            }
            if (int(code) < 65 || int(code) > 90)
            {
                break;
            }
            if (!GameRect[i]->CanWrite() || GameRect[i]->GuessedWordIsChose())
                break;

            GameRect[i]->WriteLetter(code, Colors::White, GameRect[i]->GetRect());
            GameRect[i]->Next_Rect();
        }
    }
    break;
#pragma endregion

    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId)
            {
            #pragma region ChangeDifficulty
            case IDM_EASY:
            {
                curDif = EASY;
                HMENU menu = GetMenu(hWnd);
                CheckMenuItem(menu, IDM_MEDIUM, MF_UNCHECKED);
                CheckMenuItem(menu, IDM_HARD, MF_UNCHECKED);
                CheckMenuItem(menu, IDM_EASY, MF_CHECKED);
                
                CloseAllWindows(EasyWindow[0], EasyWindow[1], EasyWindow[2], EasyWindow[3]);

                EasyWindow[0] = CreateWindowW(L"Easy", szTitleGame, WS_VISIBLE,
                    GetSystemMetrics(SM_CXSCREEN) / 2 - sizeXEasy / 2, GetSystemMetrics(SM_CYSCREEN) / 2 - sizeYEasy,
                    sizeXEasy, sizeYEasy, hWnd, nullptr, hInst, nullptr);
                
                keyboard->CreateBoard(hWnd);
                
                WriteToIni();
            }
            break;
            case IDM_MEDIUM:
            {
                curDif = MEDIUM;
                HMENU menu = GetMenu(hWnd);
                CheckMenuItem(menu, IDM_MEDIUM, MF_CHECKED);
                CheckMenuItem(menu, IDM_HARD, MF_UNCHECKED);
                CheckMenuItem(menu, IDM_EASY, MF_UNCHECKED);

                CloseAllWindows(EasyWindow[0], EasyWindow[1], EasyWindow[2], EasyWindow[3]);

                EasyWindow[0] = CreateWindowW(L"Easy", szTitleGame, WS_VISIBLE,
                    GetSystemMetrics(SM_CXSCREEN) / 4 - sizeXMedium, GetSystemMetrics(SM_CYSCREEN) / 2 - sizeYMedium/2,
                    sizeXMedium, sizeYMedium, hWnd, nullptr, hInst, nullptr);
                EasyWindow[1] = CreateWindowW(L"Easy", szTitleGame, WS_VISIBLE,
                    GetSystemMetrics(SM_CXSCREEN) / 2 + sizeXMedium, GetSystemMetrics(SM_CYSCREEN) / 2 - sizeYMedium/2,
                    sizeXMedium, sizeYMedium, hWnd, nullptr, hInst, nullptr);
                SetActiveWindow(hWnd);

                keyboard->CreateBoard(hWnd);

                WriteToIni();

            }
            break;
            case IDM_HARD:
            {
                curDif = HARD;
                HMENU menu = GetMenu(hWnd);
                CheckMenuItem(menu, IDM_MEDIUM, MF_UNCHECKED);
                CheckMenuItem(menu, IDM_HARD, MF_CHECKED);
                CheckMenuItem(menu, IDM_EASY, MF_UNCHECKED);

                CloseAllWindows(EasyWindow[0], EasyWindow[1], EasyWindow[2], EasyWindow[3]);

                EasyWindow[0] = CreateWindowW(L"Easy", szTitleGame, WS_VISIBLE,
                    GetSystemMetrics(SM_CXSCREEN) / 4 - sizeXHard, 0,
                    sizeXHard, sizeYHard, hWnd, nullptr, hInst, nullptr);

                EasyWindow[1] = CreateWindowW(L"Easy", szTitleGame, WS_VISIBLE,
                    GetSystemMetrics(SM_CXSCREEN) / 4 - sizeXHard, GetSystemMetrics(SM_CYSCREEN) - sizeYHard,
                    sizeXHard, sizeYHard, hWnd, nullptr, hInst, nullptr);

                EasyWindow[2] = CreateWindowW(L"Easy", szTitleGame, WS_VISIBLE,
                    GetSystemMetrics(SM_CXSCREEN) / 2 + sizeXHard, 0,
                    sizeXHard, sizeYHard, hWnd, nullptr, hInst, nullptr);

                EasyWindow[3] = CreateWindowW(L"Easy", szTitleGame, WS_VISIBLE,
                    GetSystemMetrics(SM_CXSCREEN) / 2 + sizeXHard, GetSystemMetrics(SM_CYSCREEN) - sizeYHard,
                    sizeXHard, sizeYHard, hWnd, nullptr, hInst, nullptr);
                SetActiveWindow(hWnd);

                keyboard->CreateBoard(hWnd);

                WriteToIni();


            }
            break;
#pragma endregion
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);

            keyboard->CreateBoard(hWnd);

            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

void SendAnimationMessageToAllWindows(UINT message, int index)
{
    for (int i = 0; i < DiffToWindowAmount(curDif); i++)
        SendMessage(GameRect[i]->Window, message, i, index);
}

void KillAllTimers(HWND hWnd)
{
    for (int i = 0; i < 11; i++)
        KillTimer(hWnd, i);
}