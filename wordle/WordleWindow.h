#pragma once

#include <array>
#include <thread>
#include <chrono>
#include <vector>
#include <fstream>
#include <algorithm>
#include <Windows.h>
#include <ObjIdl.h>
#include <gdiplus.h>

#define Sleep() std::this_thread::sleep_for(std::chrono::milliseconds(10));

wchar_t dif_str[10] = L"EASY";

enum DIFFICULTY { EASY = 6, MEDIUM = 8, HARD = 10 };

// class for words
class WordDictionary
{
public:
	std::vector<std::string> words;

	WordDictionary()
	{
		std::string word;
		std::string words_(6, '\0');
		std::ifstream file;
		file.open("Wordle.txt");
		if (!file.is_open())
			throw EXCEPTION_ACCESS_VIOLATION;
		while (file >> word)
		{
			for (int i = 0; i < word.length(); i++)
				words_[i] = toupper(word[i]);
			words.push_back(words_);
		}
			
		file.close();
	}

	// returns a word from words vector
	std::string Word(int index)
	{
		return words[index];
	}

	// Search for str in words vector
	bool IsWord(std::string str)
	{
		for (int i = 0; i < words.size(); i++)
			if (words[i].compare(str) == 0)
				return true;
		return false;
	}

};

// Possible colors for the game
class Colors {
public:
	const static COLORREF White = RGB(255, 255, 255);
	const static COLORREF Gray = RGB(164, 174, 196);
	const static COLORREF Green = RGB(121, 184, 81);
	const static COLORREF Yellow = RGB(243, 194, 55);
	const static COLORREF Red = RGB(200, 0, 0);
};

// Game Window class
class GameRectangles
{
private:
	std::array<char, 10 * 5 + 1> _words = { '0' };

	int _work_rect;
	int cur_row;

	HDC hdc;
	
public:
	bool guessed;
	bool lastWord = false;
	bool lastAnimated = false;
	DIFFICULTY _difficulty;
	std::string _chosen_word;
	std::array<RECT, 10 * 5 + 1> _rect;
	HWND& Window;
	HFONT font;
	// constructor
	GameRectangles(HWND& Wind, DIFFICULTY dif, WordDictionary* w) : Window(Wind)
	{
		guessed = false;
		_chosen_word = w->Word(rand() % w->words.size());
		switch (dif)
		{
		case EASY:
		{
			for (int i = 0; i < 6; i++)
				for (int j = 0; j < 5; j++)
				{
					_rect[5 * i + j] = { 6 * (j + 1) + 55 * j, 6 * (i + 1) + i * 55,
						61 * (j + 1), 61 * (i + 1) };
				}
		}
		break;
		case MEDIUM:
		{
			for (int i = 0; i < 8; i++)
				for (int j = 0; j < 5; j++)
				{
					_rect[5 * i + j] = { 6 * (j + 1) + 55 * j, 6 * (i + 1) + i * 55,
				61 * (j + 1), 61 * (i + 1) };
				}

		}
		break;
		case HARD:
		{
			for (int i = 0; i < 10; i++)
				for (int j = 0; j < 5; j++)
				{
					_rect[5 * i + j] = { 6 * (j + 1) + 55 * j, 6 * (i + 1) + i * 55,
				61 * (j + 1), 61 * (i + 1) };
				}
		}
		break;
		default:
			throw ERROR_DBG_EXCEPTION_NOT_HANDLED;
			break;

		}
		
		_work_rect = 0;
		cur_row = 1;
		_difficulty = dif;
		hdc = GetDC(Window);
	}
	
	// destructor
	~GameRectangles()
	{
		DeleteObject(font);
	}

	// Increases current working row
	void IncreaseRow()
	{
		if (GuessedWordIsChose())
			return;
		if (cur_row < int(_difficulty))
		{
			cur_row++;
			return;
		}
		lastWord = true;
		return;
	}

	// Sets a pointer to next Rectangle
	void Next_Rect()
	{
		if (GuessedWordIsChose())
			return;
		if (cur_row*5+1 > (_work_rect + 1) / 5)
			_work_rect == 50 ? 50 : _work_rect++;
	}

	// Sets a pointer to previous Rectangle
	void Previus_Rect()
	{
		if (_work_rect % 5 != 0 || _work_rect/5 == cur_row)
			if (_work_rect != 0)
			{
				_words[_work_rect] = '0';
				_work_rect--;
			}
	}

	// Returns current Rectangle
	RECT GetRect()
	{
		return _rect[_work_rect];
	}

	// Returns indexed Rectangle
	RECT GetRect(int index)
	{
		if (index < 0 || index > 49)
			throw FAST_FAIL_INVALID_ARG;
		return _rect[index];
	}

	// Returns current pointer
	int GetNumberRect()
	{
		return _work_rect;
	}

	// Returns current row
	int GetRow()
	{
		return cur_row;
	}

	// Indicates whether pointer is in the same row as 
	// user currently is
	bool CanWrite()
	{
		return cur_row > _work_rect / 5;
	}

	// Attach a char to a Rectangle
	void SetChar(char c)
	{
		_words[_work_rect] = c;
	}

	char GetChar(int index)
	{
		return _words[index];
	}

	// Returns a char assingned to and RECT
	char GetAssignedChar(int index)
	{
		if (index < 0 || index>49)
			throw FAST_FAIL_INVALID_ARG;
		return _words[index];
	}

	// Writes a letter passed in first argument,
	// sets the background to color
	void WriteLetter(char c, COLORREF color, RECT rc, COLORREF outline = Colors::Gray)
	{
		/*if (GuessedWordIsChose())
			return;*/
		hdc = GetDC(Window);
		// Changes char -> LPCWSTR
		char Arr[] = { c, '\0' };
		size_t len;
		wchar_t charac[2];

		mbstowcs_s(&len, charac, 2, Arr, 2);
		LPCWSTR key = charac;

		HPEN pen = CreatePen(PS_SOLID, 2, outline);
		HPEN oldPen = (HPEN)SelectObject(hdc, pen);
		HBRUSH brush = CreateSolidBrush(color);
		HBRUSH oldbrush = (HBRUSH)SelectObject(hdc, brush);

		HFONT font = CreateFont(
			-MulDiv(24,
				GetDeviceCaps(hdc, LOGPIXELSY), 72), // Height
			0, // Width
			0, // Escapement
			0, // Orientation
			FW_BOLD, // Weight
			false, // Italic
			FALSE, // Underline
			0, // StrikeOut
			EASTEUROPE_CHARSET, // CharSet
			OUT_DEFAULT_PRECIS, // OutPrecision
			CLIP_DEFAULT_PRECIS, // ClipPrecision
			DEFAULT_QUALITY, // Quality
			DEFAULT_PITCH | FF_SWISS, // PitchAndFamily
			_T(" Verdana ")); // Facename
		HFONT oldFont = (HFONT)SelectObject(hdc, font);

		SetBkColor(hdc, color);

		RoundRect(hdc, rc.left, rc.top, rc.right, rc.bottom, 5,5);

		DrawText(hdc, key, (int)_tcslen(key), &rc,
			DT_CENTER | DT_VCENTER | DT_SINGLELINE);
		
		SelectObject(hdc, oldFont);
		SelectObject(hdc, oldbrush);
		DeleteObject(brush);
		DeleteObject(font);
		DeleteObject(pen);
		ReleaseDC(Window, hdc);

		SetChar(c);
	}

	// Sets the overlay for rectangle - same rectangle
	void ClearRect(COLORREF color, RECT rc)
	{
		hdc = GetDC(Window);
		HPEN pen = CreatePen(PS_SOLID, 2, Colors::Gray);
		HPEN oldPen = (HPEN)SelectObject(hdc, pen);
		HBRUSH brush = CreateSolidBrush(color);
		HBRUSH oldbrush = (HBRUSH)SelectObject(hdc, brush);
		
		RoundRect(hdc, rc.left, rc.top, rc.right, rc.bottom, 5, 5);

		SelectObject(hdc, oldbrush);
		DeleteObject(brush);
		DeleteObject(pen);
		ReleaseDC(Window, hdc);

	}

	// Deletes Rectangle
	void DeleteRect(COLORREF color, RECT rc)
	{
		/*if (GuessedWordIsChose())
			return;*/
		hdc = GetDC(Window);
		HBRUSH brush = CreateSolidBrush(color);
		HBRUSH oldbrush = (HBRUSH)SelectObject(hdc, brush);

		RECT t = {rc.left-1, rc.top-1, rc.right+1, rc.bottom+1};
		FillRect(hdc, &t, brush);

		SelectObject(hdc, oldbrush);
		DeleteObject(brush);
		ReleaseDC(Window, hdc);

	}

	// Animate revealing position of a letter in a word
	void AnimateRec(RECT rc, int index)
	{
		if (GuessedWordIsChose())
			return;
		COLORREF colorto = Colors::Gray;
		char c = GetAssignedChar(index);

		if (IsInWord(c))
			colorto = Colors::Yellow;
		if (IsOnPosition(c, index % 5))
			colorto = Colors::Green;

		// current drawn rectangle
		RECT temp = rc;

		// Animate to center
		for (int i = 1; i <= 17; i = i+2)
		{
			// Clears current rectangle
			DeleteRect(Colors::White, rc);

			temp = { rc.left, rc.top + i, rc.right, rc.bottom - i };

			WriteLetter(c, Colors::White, temp, Colors::Gray);

			Sleep();
		}

		//Animating a Rect from center in dff color
		RECT des = temp;
			
		for (int i = 1; i <= 17; i = i+2)
		{
			// Clears current rectangle
			DeleteRect(Colors::White, temp);

			temp = { des.left, des.top - i, des.right, des.bottom + i };

			WriteLetter(c, colorto, temp, colorto);

			Sleep();
		}
		DeleteRect(Colors::White, rc);
		WriteLetter(c, colorto, rc, colorto);
	}

	// Creates a string from last 5 characters
	std::string CreateString()
	{
		std::string word(6, '0');
		word[0] = _words[_work_rect - 5];
		word[1] = _words[_work_rect - 4];
		word[2] = _words[_work_rect - 3];
		word[3] = _words[_work_rect - 2];
		word[4] = _words[_work_rect - 1];
		word[5] = '\0';
		return word;
	}

	// indicates whether char is in the word
	bool IsInWord(char c)
	{
		for (int i = 0; i < 5; i++)
			if (_chosen_word[i] == c)
				return true;
		return false;
	}

	// indicates whether char is on the right position in a word
	bool IsOnPosition(char c, int position)
	{
		if (_chosen_word[position] == c)
			return true;
		return false;
	}

	// check if the word written is guessed
	bool IsWordGuessed()
	{
		std::string word = CreateString();
		if (word.compare(_chosen_word) == 0)		
			guessed = true;		
		return guessed;
	}

	// check if already word is guessed
	bool GuessedWordIsChose()
	{
		return guessed;
	}

	// Creates overlay after the game
	void CreateOverlay(COLORREF Background)
	{
		std::wstring t = GuessedWordIsChose() ? L"" : std::wstring(_chosen_word.begin(), _chosen_word.end());
		LPCWSTR text = t.c_str();

		RECT temp;
		GetWindowRect(Window, &temp);

		RECT rc = { 0,0,temp.right - temp.left, temp.bottom - temp.top };

		using namespace Gdiplus;

		HDC hWdc = GetDC(Window);

		Graphics g(hWdc);
		
		if (Background == Colors::Green)
		{
			SolidBrush brush(Color(130, 121, 184, 81));
			g.FillRectangle(&brush, 0, 0, rc.right, rc.bottom);
		}
		else
		{
			SolidBrush brush(Color(130, 200, 0, 0));
			g.FillRectangle(&brush, 0, 0, rc.right, rc.bottom);
		}

		HDC hdc = GetDC(Window);
		HPEN pen = CreatePen(PS_SOLID, 2, Background);
		HPEN oldPen = (HPEN)SelectObject(hdc, pen);

		HFONT font = CreateFont(
			-MulDiv(24,
				GetDeviceCaps(hdc, LOGPIXELSY), 72), // Height
			0, // Width
			0, // Escapement
			0, // Orientation
			FW_BOLD, // Weight
			false, // Italic
			FALSE, // Underline
			0, // StrikeOut
			EASTEUROPE_CHARSET, // CharSet
			OUT_DEFAULT_PRECIS, // OutPrecision
			CLIP_DEFAULT_PRECIS, // ClipPrecision
			DEFAULT_QUALITY, // Quality
			DEFAULT_PITCH | FF_SWISS, // PitchAndFamily
			_T(" Verdana ")); // Facename
		HFONT oldFont = (HFONT)SelectObject(hdc, font);

		SetBkMode(hdc, TRANSPARENT);

		DrawText(hdc, text, (int)_tcslen(text), &rc,
			DT_CENTER | DT_VCENTER | DT_SINGLELINE);

		SelectObject(hdc, oldFont);
		DeleteObject(font);
		DeleteObject(pen);

		ReleaseDC(Window, hdc);

	}

	COLORREF ColorForRect(int index)
	{
		COLORREF colorto = Colors::Gray;
		char c = GetAssignedChar(index);

		if (IsInWord(c))
			colorto = Colors::Yellow;
		if (IsOnPosition(c, index % 5))
			colorto = Colors::Green;
		return colorto;
	}
};

// KeyboardClass
class Keyboard
{
public:
	const std::array<char, 26> _chars =
	{ 'Q','W','E','R','T','Y','U','I','O','P',
		'A','S','D','F','G','H','J','K','L',
			'Z','X','C','V','B','N','M'};
	
	std::array<RECT, 26> _rect;
	int _pointer = 0;

	// constructor
	Keyboard(HWND& wind)
	{
		for (int i = 0; i < 26; i++)
			_rect[i] = {0,0,0,0};
	}

	// Creates KeyBoard in a Window, needs to be good size
	void CreateBoard(HWND hWnd)
	{
		int offset_x = 8;
		int pointer_temp = _pointer;
		// Pierwszy wiersz
		for (int j = 0; j < 10; j++)
		{
			_rect[pointer_temp] = {6 * (j + 1) + offset_x + 55 * j, 6, 61 * (j + 1) + offset_x, 61};
			WriteLetter(hWnd, _chars[pointer_temp], Colors::White, _rect[pointer_temp]);
			pointer_temp++;
		}

		offset_x += round(55 / 2);
		// Drugi wiersz
		for (int j = 0; j < 9; j++)
		{
			_rect[pointer_temp] = { 6 * (j + 1) + 55 * j + offset_x,
				66, 
				61 * (j + 1) + offset_x,
				121 };
			WriteLetter(hWnd, _chars[pointer_temp], Colors::White, _rect[pointer_temp]);
			pointer_temp++;
		}

		offset_x += 61;
		// Trzeci wiersz
		for (int j = 0; j < 7; j++)
		{
			_rect[pointer_temp] = { 6 * (j + 1) + 55 * j + offset_x,
				127,
				61 * (j + 1) + offset_x,
				182 };
			WriteLetter(hWnd, _chars[pointer_temp], Colors::White, _rect[pointer_temp]);
			pointer_temp++;
		}
	}

	// Writes a letter in specified rectangle
	void WriteLetter(HWND hWnd, char c, COLORREF color, RECT rc, COLORREF outline = Colors::Gray, RECT offset = {0,0,0,0})
	{
		HDC hdc = GetDC(hWnd);
		// Changes char -> LPCWSTR
		char Arr[] = { c, '\0' };
		size_t len;
		wchar_t charac[2];

		mbstowcs_s(&len, charac, 2, Arr, 2);
		LPCWSTR key = charac;

		HPEN pen = CreatePen(PS_SOLID, 2, outline);
		HPEN oldPen = (HPEN)SelectObject(hdc, pen);
		HBRUSH brush = CreateSolidBrush(color);
		HBRUSH oldbrush = (HBRUSH)SelectObject(hdc, brush);

		HFONT font = CreateFont(
			-MulDiv(24,
				GetDeviceCaps(hdc, LOGPIXELSY), 72), // Height
			0, // Width
			0, // Escapement
			0, // Orientation
			FW_BOLD, // Weight
			false, // Italic
			FALSE, // Underline
			0, // StrikeOut
			EASTEUROPE_CHARSET, // CharSet
			OUT_DEFAULT_PRECIS, // OutPrecision
			CLIP_DEFAULT_PRECIS, // ClipPrecision
			DEFAULT_QUALITY, // Quality
			DEFAULT_PITCH | FF_SWISS, // PitchAndFamily
			_T(" Verdana ")); // Facename
		HFONT oldFont = (HFONT)SelectObject(hdc, font);

		SetBkMode(hdc, TRANSPARENT);

		RECT temp = { rc.left + offset.left,
			rc.top + offset.top,
			rc.right + offset.right,
			rc.bottom + offset.bottom };

		RoundRect(hdc, temp.left,
			temp.top,
			temp.right,
			temp.bottom, 5, 5);

		DrawText(hdc, key, (int)_tcslen(key), &rc,
			DT_CENTER | DT_VCENTER | DT_SINGLELINE);

		SelectObject(hdc, oldFont);
		SelectObject(hdc, oldbrush);
		DeleteObject(brush);
		DeleteObject(font);
		DeleteObject(pen);
		ReleaseDC(hWnd, hdc);
	}

	// returns pointer to appriopriate char in a keyboard
	int CharToPointer(char c)
	{
		int point = 0;
		while (point < 26 && _chars[point] != c)
			point++;

		return point == 26 ? -1 : point;
	}

	// returns RECT to appriopriate char in a keyboard
	RECT CharToRect(char c)
	{
		return _rect[CharToPointer(c)];
	}
};

// Class dedicated for row animation
class Frame 
{
public:
	int _row;
	int _CurFrame;
	int _MaxFrame;

	Frame(int row, int currentFrame=1, int MaxFrame = 5*55)
	{
		_row = row;
		_CurFrame = currentFrame;
		_MaxFrame = MaxFrame;
	}

	bool AnimateFrame(int windowamount, GameRectangles** window)
	{		
		if (_CurFrame >= _MaxFrame)
		{
			for (int i = 0; i < windowamount; i++)
				if (window[i]->GuessedWordIsChose())
					window[i]->lastAnimated = true;
			return false;	
		}
		int rect_in_row = (_row - 1) * 5 + int(_CurFrame / 55);
		RECT offset = { 0, _CurFrame % 55 + 1, 0, -_CurFrame % 55 - 1 };

		for (int i = 0; i < windowamount; i++)
		{
			if (window[i]->lastAnimated)
				continue;
			RECT animated_rect = window[i]->GetRect(rect_in_row);

			COLORREF color_to = window[i]->ColorForRect(rect_in_row);
			char letter = window[i]->GetAssignedChar(rect_in_row);

			AnimateRect(window[i], animated_rect, letter,
				_CurFrame % 55 < 27 ? Colors::White : color_to, _CurFrame % 55 < 27 ? Colors::Gray : color_to, offset);

			if (_CurFrame % 55 == 54)
			{
				RECT rec = { animated_rect.left, animated_rect.top - 3, animated_rect.right, animated_rect.bottom + 3 };
				window[i]->DeleteRect(Colors::White, rec);
				EndAnimation(window[i], animated_rect, letter, color_to, color_to);
			}
		}		
		_CurFrame += 2;

		return true;

	}

	void AnimateRect(GameRectangles* game, RECT rec, char letter, 
		COLORREF background, COLORREF outline = Colors::Gray, RECT offset = {0,0,0,0})
	{
		game->DeleteRect(Colors::White, rec);

		RECT rect = { rec.left + offset.left,
		rec.top + offset.top,
		rec.right + offset.right,
		rec.bottom + offset.bottom, };

		game->WriteLetter(letter, background, rect, outline);
	}
	void EndAnimation(GameRectangles* game, RECT rec, char letter,
		COLORREF background, COLORREF outline = Colors::Gray)
	{
		game->WriteLetter(letter, background, rec, outline);
	}
};

// Closes all Window, without terminating app
void CloseAllWindows(HWND window1, HWND window2, HWND window3, HWND window4)
{
	DestroyWindow(window1);
	DestroyWindow(window2);
	DestroyWindow(window3);
	DestroyWindow(window4);
}

// Converts DIFFICUTLTY to LPCWSTR
LPCWSTR DifToWChar(DIFFICULTY dif)
{
	switch (dif)
	{
	case EASY:
		wcscpy_s(dif_str, L"EASY");
		break;
	case MEDIUM:
		wcscpy_s(dif_str, L"MEDIUM");
		break;
	case HARD:
		wcscpy_s(dif_str, L"HARD");
		break;
	default:
		break;
	}
	LPCWSTR c = static_cast<LPCWSTR>(dif_str);
	return c;
}

// Converts LPCWSTR to DIFFICULTY
DIFFICULTY LToDiff(LPCWSTR string)
{
	if (wcscmp(string, L"EASY") == 0)
		return EASY;
	if (wcscmp(string, L"MEDIUM") == 0)
		return MEDIUM;
	if (wcscmp(string, L"HARD") == 0)
		return HARD;
}

int DiffToWindowAmount(DIFFICULTY dif)
{
	if (dif == EASY)
		return 1;
	if (dif == MEDIUM)
		return 2;
	return 4;
}


