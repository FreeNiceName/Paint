#include <windows.h>
#include <cassert>
#include <vector>
#include "resource.h"

#define ROTATERIGHT 1
#define ROTATELEFT	2
#define ROTATEFLIP	3
#define MIRRORHORZ	4
#define MIRRORVERT	5
//Color buttons
#define IDB_CLR1	20001
#define IDB_CLR2	20002
#define IDB_CLR3	20003
#define IDB_CLR4	20004
#define IDB_CLR5	20005
#define IDB_CLR6	20006
#define IDB_CLR7	20007
#define IDB_CLR8	20008
#define IDB_CLR9	20009
#define IDB_CLR10	20010
#define IDB_CLR11	20011
#define IDB_CLR12	20012
#define IDB_CLR13	20013
#define IDB_CLR14	20014
#define IDB_CLR15	20015
#define IDB_CLR16	20016
#define IDB_CLR17	20017
#define IDB_CLR18	20018
#define IDB_CLR19	20019
#define IDB_CLR20	20020
#define IDB_CLR21	20021
#define IDB_CLR22	20022
#define IDB_CLR23	20023
#define IDB_CLR24	20024
#define IDB_CLR25	20025
#define IDB_CLR26	20026
#define IDB_CLR27	20027
#define IDB_CLR28	20028
#define IDB_CLR29	20029
#define IDB_CLR30	20030
#define IDB_CLR31	20031
#define IDB_CLR32	20032
//Tools
#define IDT_PENCIL				30017
#define IDT_BRUSH				30018
#define IDT_COLORPICKER			30019
#define IDT_ERASER				30020
#define IDT_BUCKET				30021
#define IDT_TEXT				30022
#define IDT_LINE				30023
#define IDT_CURVE				30024
#define IDT_RECT				30025
#define IDT_FILLEDRECT			30026
#define IDT_ROUNDEDRECT			30027
#define IDT_FILLEDROUNDEDRECT	30028
#define IDT_ELLIPSE				30029
#define IDT_FILLEDELLIPSE		30030
#define IDC_EDITPENWIDTH		30031

// Глобальні змінні:
HINSTANCE hInst; 	//Дескриптор програми	
LPCTSTR szWindowClass = "QWERTY";
LPCTSTR szTitle = "Paint";

LPCSTR szColorDialogClass = "Color Dialog";
LPCSTR szToolsDialogClass = "Tools Dialog";
OPENFILENAME ofn;
TCHAR szOpenFile[260] = { 0 };
std::vector<HBITMAP> hBitmaps;
UINT CurrentBitmap;
float Scale;
COLORREF arrCustomColors[16];
COLORREF arrDefaultColors[16];
HWND hColorDlg;
HWND hToolsDlg;
SHORT xStartPos, yStartPos, xFinalPos, yFinalPos, xCurrentPos, yCurrentPos;
HPEN hPen = NULL;
HBRUSH hOldBrush;
SHORT penWidth = 0;
POINT arrCurvePoints[4];
SHORT nCurvePoints = 2;

HDC hCompatibleDC;
HFONT hCurrentFont = NULL;
DWORD rgbCurrent;
UINT idCurrentTool = 0;
HWND hEditText;
RECT EditTextRect;
RECT ClientRect;
BITMAP Bitmap;
HBITMAP hCompatibleBitmap;
BOOL fBlt;           
BOOL fScroll;         
BOOL fSize;        
BOOL fDraw;
BOOL fDrawing;
BOOL fInstantDrawing;
BOOL fActiveEdit;
BOOL fDrawingCurve;

SCROLLINFO si;

int xMinScroll;      
int xCurrentScroll;  
int xMaxScroll;      

int yMinScroll;       
int yCurrentScroll;   
int yMaxScroll;      
// Попередній опис функцій

ATOM MyRegisterClass(HINSTANCE hInstance);
BOOL InitInstance(HINSTANCE, int);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

PBITMAPINFO CreateBitmapInfoStruct(HBITMAP);
void CreateBMPFile(LPTSTR, HBITMAP);
void ReadBitmapByHandle(HWND, HBITMAP, BOOL);
INT_PTR CALLBACK InputScale(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK Parameters(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK ColorDialog(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK ToolsDialog(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK HelpDialog(HWND, UINT, WPARAM, LPARAM);
VOID Drawing(HDC);
VOID SelectPen(HDC);
VOID Draw(HWND, UINT);

void BitmapPushback(HWND, HBITMAP, BOOL);
void CopyBitmap();
void RotateBitmap(HWND, UINT);
LPRECT GetBitmapRect();

// Основна програма 
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	MSG msg;

	// Реєстрація класу вікна 
	MyRegisterClass(hInstance);

	// Створення вікна програми
	if (!InitInstance(hInstance, nCmdShow))
	{
		return FALSE;
	}
	// Цикл обробки повідомлень
	HACCEL hAccel = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDR_ACCELERATOR));
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAcceleratorW(msg.hwnd, hAccel, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	return msg.wParam;
}

ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW; 		//стиль вікна
	wcex.lpfnWndProc = (WNDPROC)WndProc; 		//віконна процедура
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance; 			//дескриптор програми
	wcex.hIcon = LoadIcon(NULL, IDI_APPLICATION); 		//визначення іконки
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW); 	//визначення курсору
	wcex.hbrBackground = CreateSolidBrush(RGB(210, 210, 210)); //установка фону
	wcex.lpszMenuName = MAKEINTRESOURCE(IDR_MENU); 				//визначення меню
	wcex.lpszClassName = szWindowClass; 		//ім’я класу
	wcex.hIconSm = NULL;

	return RegisterClassEx(&wcex); 			//реєстрація класу вікна
}

// FUNCTION: InitInstance (HANDLE, int)
// Створює вікно програми і зберігає дескриптор програми в змінній hInst

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	HWND hWnd;
	hInst = hInstance;
	hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW | WS_HSCROLL | WS_VSCROLL, 
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, hInstance, NULL); 				

	if (!hWnd) 	//Якщо вікно не творилось, функція повертає FALSE
	{
		return FALSE;
	}
	ShowWindow(hWnd, nCmdShow); 		//Показати вікно
	UpdateWindow(hWnd); 				//Оновити вікно
	return TRUE;
}

// FUNCTION: WndProc (HWND, unsigned, WORD, LONG)
// Віконна процедура. Приймає і обробляє всі повідомлення, що приходять в додаток


LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	HDC hDC;
	switch (message)
	{
	case WM_CREATE:
		Scale = 1;

		hCurrentFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
		hDC = GetDC(hWnd);
		hCompatibleDC = CreateCompatibleDC(hDC);

		GetClientRect(hWnd, &ClientRect);

		Bitmap.bmBitsPixel = (BYTE)GetDeviceCaps(hDC, BITSPIXEL);
		Bitmap.bmPlanes = (BYTE)GetDeviceCaps(hDC, PLANES);
		Bitmap.bmWidth = ClientRect.right;
		Bitmap.bmHeight = ClientRect.bottom;

		Bitmap.bmWidthBytes = ((Bitmap.bmWidth + 15) &~15) / 8;

		CurrentBitmap = 0;
		hCompatibleBitmap = CreateBitmap(Bitmap.bmWidth, Bitmap.bmHeight, Bitmap.bmPlanes, Bitmap.bmBitsPixel, (CONST VOID *) NULL);
		SelectObject(hCompatibleDC, hCompatibleBitmap);
		FillRect(hCompatibleDC, &ClientRect, CreateSolidBrush(RGB(255, 255, 255)));
		hBitmaps.push_back(hCompatibleBitmap);
		ReadBitmapByHandle(hWnd, hBitmaps[CurrentBitmap], FALSE);

		fBlt = TRUE;
		fScroll = FALSE;
		fSize = FALSE;
		fDraw = FALSE;
		fInstantDrawing = FALSE;
		fActiveEdit = FALSE;
		fDrawingCurve = FALSE;
		fDrawing = FALSE;

		xMinScroll = 0;
		xCurrentScroll = 0;
		xMaxScroll = 0;

		yMinScroll = 0;
		yCurrentScroll = 0;
		yMaxScroll = 0;

		ZeroMemory(&ofn, sizeof(ofn));
		ofn.lStructSize = sizeof(ofn);
		ofn.hwndOwner = hWnd;
		ofn.lpstrFile = szOpenFile;
		ofn.nMaxFile = sizeof(szOpenFile);
		ofn.lpstrFilter = TEXT("Bitmap(*.bmp)\0*.bmp\0Все файлы(*.*)\0*.*\0");
		ofn.nFilterIndex = 1;
		ofn.lpstrFileTitle = NULL;
		ofn.nMaxFileTitle = 0;
		ofn.lpstrInitialDir = NULL;
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

		WNDCLASSEX  wc;
		wc.cbSize = sizeof(WNDCLASSEX);
		wc.style = 0;
		wc.lpfnWndProc = (WNDPROC)ColorDialog;
		wc.cbClsExtra = 0;
		wc.cbWndExtra = 0;
		wc.hInstance = (HINSTANCE)GetWindowLong(hWnd, GWL_HINSTANCE);
		wc.hIcon = NULL;
		wc.hIconSm = NULL;
		wc.hCursor = LoadCursor(NULL, IDC_ARROW);
		wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
		wc.lpszMenuName = NULL;
		wc.lpszClassName = szColorDialogClass;
		if (!RegisterClassEx(&wc))
		{
			MessageBox(NULL, "Failed To Register The Dialog Class.", "Error", MB_OK | MB_ICONERROR);
			return FALSE;
		}
		hColorDlg = CreateWindowExA(WS_EX_PALETTEWINDOW, szColorDialogClass, "Палітра", WS_VISIBLE, 0, 150, 221, 144, hWnd, NULL, hInst, NULL);

		arrDefaultColors[0] = RGB(0, 255, 255);
		arrDefaultColors[1] = RGB(0, 0, 0);
		arrDefaultColors[2] = RGB(0, 0, 255);
		arrDefaultColors[3] = RGB(255, 0, 255);
		arrDefaultColors[4] = RGB(128, 128, 128);
		arrDefaultColors[5] = RGB(0, 128, 0);
		arrDefaultColors[6] = RGB(0, 255, 0);
		arrDefaultColors[7] = RGB(128, 0, 0);
		arrDefaultColors[8] = RGB(0, 0, 128);
		arrDefaultColors[9] = RGB(128, 128, 0);
		arrDefaultColors[10] = RGB(128, 0, 128);
		arrDefaultColors[11] = RGB(255, 0, 0);
		arrDefaultColors[12] = RGB(192, 192, 192);
		arrDefaultColors[13] = RGB(0, 128, 128);
		arrDefaultColors[14] = RGB(255, 255, 255);
		arrDefaultColors[15] = RGB(255, 255, 0);

		WNDCLASSEX  wct;
		wct.cbSize = sizeof(WNDCLASSEX);
		wct.style = 0;
		wct.lpfnWndProc = (WNDPROC)ToolsDialog;
		wct.cbClsExtra = 0;
		wct.cbWndExtra = 0;
		wct.hInstance = (HINSTANCE)GetWindowLong(hWnd, GWL_HINSTANCE);
		wct.hIcon = NULL;
		wct.hIconSm = NULL;
		wct.hCursor = LoadCursor(NULL, IDC_ARROW);
		wct.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
		wct.lpszMenuName = NULL;
		wct.lpszClassName = szToolsDialogClass;
		if (!RegisterClassEx(&wct))
		{
			MessageBox(NULL, "Failed To Register The Dialog Class.", "Error", MB_OK | MB_ICONERROR);
			return FALSE;
		}
		hToolsDlg = CreateWindowExA(WS_EX_TOOLWINDOW, szToolsDialogClass, "Інструменти", WS_VISIBLE, 0, 300, 91, 314, hWnd, NULL, hInst, NULL);
		break;
	case WM_LBUTTONDOWN:
	{
		if (idCurrentTool)
		{
			if (fActiveEdit && (!(EditTextRect.left < LOWORD(lParam)) || !(LOWORD(lParam) < EditTextRect.right)
				|| !(EditTextRect.top < HIWORD(lParam)) || !(HIWORD(lParam) < EditTextRect.bottom)))
			{
				EditTextRect.bottom = (EditTextRect.bottom + yCurrentScroll)/Scale;
				EditTextRect.left = (EditTextRect.left + xCurrentScroll)/Scale;
				EditTextRect.top = (EditTextRect.top + yCurrentScroll)/Scale;
				EditTextRect.right = (EditTextRect.right + xCurrentScroll)/Scale;
				LOGFONT newFont;
				GetObject(hCurrentFont, sizeof(LOGFONT), &newFont);
				newFont.lfHeight /= Scale;
				HFONT hNewFont;
				hNewFont = CreateFontIndirect(&newFont);
				SelectObject(hCompatibleDC, hNewFont);
				CHAR Text[256];
				GetWindowText(hEditText, Text, 255);
				SetTextColor(hCompatibleDC, rgbCurrent);
				SetBkMode(hCompatibleDC, TRANSPARENT);
				DrawText(hCompatibleDC, Text, -1, &EditTextRect, DT_EDITCONTROL | DT_WORDBREAK);
				BitmapPushback(hWnd, hCompatibleBitmap, FALSE);
				DestroyWindow(hEditText);
				SelectObject(hCompatibleDC, hCurrentFont);
				DeleteObject(hNewFont);
				fActiveEdit = FALSE;
				break;
			}
			xStartPos = LOWORD(lParam);
			yStartPos = HIWORD(lParam);
			if (xStartPos > Bitmap.bmWidth*Scale || yStartPos > Bitmap.bmHeight*Scale)
				break;
			if (idCurrentTool == IDT_BUCKET)
			{
				CopyBitmap();
				COLORREF color;
				color = GetPixel(hCompatibleDC, (xStartPos + xCurrentScroll)/Scale, (yStartPos + yCurrentScroll)/Scale);
				HBRUSH hBrush;
				hBrush = CreateSolidBrush(rgbCurrent);
				SelectObject(hCompatibleDC, hBrush);
				ExtFloodFill(hCompatibleDC, (xStartPos + xCurrentScroll)/Scale, (yStartPos + yCurrentScroll)/Scale, color, FLOODFILLSURFACE);
				BitmapPushback(hWnd, hCompatibleBitmap, FALSE);
				break;
			}
			if (idCurrentTool == IDT_COLORPICKER)
			{
				rgbCurrent = GetPixel(hCompatibleDC, (xStartPos + xCurrentScroll)/Scale, (yStartPos + yCurrentScroll)/Scale);
				break;
			}
			if (idCurrentTool == IDT_TEXT)
			{
				SetCapture(hWnd);
				fDraw = TRUE;
				break;
			}
			SetCapture(hWnd);
			if (fInstantDrawing)
				CopyBitmap();
			fDraw = TRUE;
		}
		break;
	}
	case WM_LBUTTONUP:
	{
		if (fDraw)
		{
			if (fDrawingCurve && nCurvePoints < 4)
			{
				nCurvePoints++;
				fDraw = FALSE;
				ReleaseCapture();
				break;
			}
			xFinalPos = LOWORD(lParam);
			yFinalPos = HIWORD(lParam);
			fDraw = FALSE;
			if (!fInstantDrawing)
			{
				CopyBitmap();
				SelectPen(hCompatibleDC);
				Draw(hWnd, idCurrentTool);
			}
			else
				BitmapPushback(hWnd, hCompatibleBitmap, FALSE);
			ReleaseCapture();
		}
		break;
	}
	case WM_MOUSEMOVE:
	{
		if (fDraw)
		{
			xCurrentPos = LOWORD(lParam);
			yCurrentPos = HIWORD(lParam);
			InvalidateRect(hWnd, NULL, FALSE);
		}
		break;
	}
	case WM_CTLCOLOREDIT:
		if ((HWND)lParam == hEditText)
		{
			HDC hdc;
			hdc = (HDC)wParam;
			SetTextColor(hdc, rgbCurrent);
			SetBkMode(hdc, TRANSPARENT);
			return (LRESULT)GetStockObject(NULL_BRUSH);
			break;
		}
	case WM_SIZE:
	{
		GetClientRect(hWnd, &ClientRect);

		int xNewSize;
		int yNewSize;

		xNewSize = LOWORD(lParam);
		yNewSize = HIWORD(lParam);

		if (fBlt)
			fSize = TRUE;

		xMaxScroll = max(Bitmap.bmWidth*Scale - xNewSize, 0);
		xCurrentScroll = min(xCurrentScroll, xMaxScroll);
		si.cbSize = sizeof(si);
		si.fMask = SIF_RANGE | SIF_PAGE | SIF_POS;
		si.nMin = xMinScroll;
		si.nMax = Bitmap.bmWidth*Scale;
		si.nPage = xNewSize;
		si.nPos = xCurrentScroll;
		SetScrollInfo(hWnd, SB_HORZ, &si, TRUE);

		yMaxScroll = max(Bitmap.bmHeight*Scale - yNewSize, 0);
		yCurrentScroll = min(yCurrentScroll, yMaxScroll);
		si.cbSize = sizeof(si);
		si.fMask = SIF_RANGE | SIF_PAGE | SIF_POS;
		si.nMin = yMinScroll;
		si.nMax = Bitmap.bmHeight*Scale;
		si.nPage = yNewSize;
		si.nPos = yCurrentScroll;
		SetScrollInfo(hWnd, SB_VERT, &si, TRUE);

		break;
	}
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDM_READ:
			ofn.lpstrTitle = TEXT("Выберите файл для открытия");
			if (GetOpenFileName(&ofn) == TRUE)
				BitmapPushback(hWnd, (HBITMAP)LoadImage(NULL, ofn.lpstrFile, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE), TRUE);
			else
				MessageBoxA(hWnd, "Не удалось открыть файл", "Ошибка", MB_OK);
			break;
		case IDM_WRITE:
			ofn.lpstrTitle = TEXT("Выберите место для сохранения файла");
			ofn.lpstrDefExt = TEXT("bmp");
			if (GetSaveFileName(&ofn) == TRUE)
				CreateBMPFile(ofn.lpstrFile, hBitmaps[CurrentBitmap]);
			break;
		case IDM_READFROMCLIPBOARD:
			OpenClipboard(hWnd);
			HBITMAP hBM;
			if (hBM = (HBITMAP)GetClipboardData(CF_BITMAP))
			{
				GetObject(hBM, sizeof(BITMAP), &Bitmap);
				hCompatibleBitmap = (HBITMAP)CopyImage(hBM, IMAGE_BITMAP, Bitmap.bmWidth, Bitmap.bmHeight, LR_CREATEDIBSECTION);
				BitmapPushback(hWnd, hCompatibleBitmap, TRUE);
				ReadBitmapByHandle(hWnd, hBitmaps[CurrentBitmap], TRUE);
			}
			CloseClipboard();
			break;
		case IDM_WRITETOCLIPBOARD:
			OpenClipboard(hWnd);
			DIBSECTION ds;
			GetObject(hBitmaps[CurrentBitmap], sizeof(DIBSECTION), &ds);
			ds.dsBmih.biCompression = BI_RGB;
			HDC hdc;
			hdc = GetDC(NULL);
			HBITMAP hbitmap_ddb;
			hbitmap_ddb = CreateDIBitmap(hdc, &ds.dsBmih, CBM_INIT, ds.dsBm.bmBits, (BITMAPINFO*)&ds.dsBmih, DIB_RGB_COLORS);
			ReleaseDC(NULL, hdc);
			EmptyClipboard();
			SetClipboardData(CF_BITMAP, hbitmap_ddb);
			CloseClipboard();
			break;
		case IDM_UNDO:
			if (CurrentBitmap > 0)
			{
				CurrentBitmap--;
				ReadBitmapByHandle(hWnd, hBitmaps[CurrentBitmap], TRUE);
			}
			break;
		case IDM_REDO:
			if (CurrentBitmap < (hBitmaps.size() - 1))
			{
				CurrentBitmap++;
				ReadBitmapByHandle(hWnd, hBitmaps[CurrentBitmap], TRUE);
			}
			break;
		case IDM_TEXT:
			CHOOSEFONT cf;
			static LOGFONT lf;
			ZeroMemory(&cf, sizeof(cf));
			cf.lStructSize = sizeof(cf);
			cf.hwndOwner = hWnd;
			cf.lpLogFont = &lf;
			cf.Flags = CF_SCREENFONTS;
			if (ChooseFont(&cf) == TRUE)
			{
				if (hCurrentFont)
					DeleteObject(hCurrentFont);
				hCurrentFont = CreateFontIndirect(cf.lpLogFont);
				if(hEditText)
					SendMessage(hEditText, WM_SETFONT, (WPARAM)hCurrentFont, (LPARAM)TRUE);
			}
			break;
		case IDM_SCALE:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_INPUTSCALE), hWnd, InputScale);
			ReadBitmapByHandle(hWnd, hBitmaps[CurrentBitmap], TRUE);
			break;
		case IDM_ROTATELEFT:
			RotateBitmap(hWnd, ROTATELEFT);
			break;
		case IDM_ROTATERIGHT:
			RotateBitmap(hWnd, ROTATERIGHT);
			break;
		case IDM_ROTATEFLIP:
			RotateBitmap(hWnd, ROTATEFLIP);
			break;
		case IDM_MIRRORHORZ:
			RotateBitmap(hWnd, MIRRORHORZ);
			break;
		case IDM_MIRRORVERT:
			RotateBitmap(hWnd, MIRRORVERT);
			break;
		case IDM_INVERSION:
			CopyBitmap();
			InvertRect(hCompatibleDC, GetBitmapRect());
			BitmapPushback(hWnd, hCompatibleBitmap, FALSE);
			break;
		case IDM_CLEAR:
			CopyBitmap();
			FillRect(hCompatibleDC, GetBitmapRect(), CreateSolidBrush(RGB(255, 255, 255)));
			BitmapPushback(hWnd, hCompatibleBitmap, TRUE);
			break;
		case IDM_CHANGEPARAMETERS:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_PARAMETERS), hWnd, Parameters);
			break;
		case IDM_PALETTESET:
			CHOOSECOLOR cc;           
			ZeroMemory(&cc, sizeof(cc));
			cc.lStructSize = sizeof(cc);
			cc.hwndOwner = hWnd;
			cc.lpCustColors = (LPDWORD)arrCustomColors;
			cc.rgbResult = rgbCurrent;
			cc.Flags = CC_FULLOPEN | CC_RGBINIT;

			if (ChooseColor(&cc) == TRUE)
			{
				rgbCurrent = cc.rgbResult;
				InvalidateRect(hColorDlg, NULL, TRUE);
				SelectPen(hCompatibleDC);
			}
			break;
		case IDM_HELP:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_HELP), hWnd, HelpDialog);
			break;
		case IDM_ABOUT:
			MessageBoxA(hWnd, "Paint", "Про програму", MB_OK);
			break;
		}
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		PRECT prect;

		hDC = BeginPaint(hWnd, &ps);
		if (fBlt)
		{
			StretchBlt(ps.hdc, 0, 0, Bitmap.bmWidth*Scale, Bitmap.bmHeight*Scale,
				hCompatibleDC, xCurrentScroll / Scale, yCurrentScroll / Scale, Bitmap.bmWidth, Bitmap.bmHeight, SRCCOPY);
		}

		if (fSize)
		{
			StretchBlt(ps.hdc, 0, 0, Bitmap.bmWidth*Scale, Bitmap.bmHeight*Scale,
				hCompatibleDC, xCurrentScroll/Scale, yCurrentScroll/Scale, Bitmap.bmWidth, Bitmap.bmHeight, SRCCOPY);
			fSize = FALSE;
		}

		if (fScroll)
		{
			prect = &ps.rcPaint;
			StretchBlt(ps.hdc, prect->left, prect->top, (prect->right - prect->left), (prect->bottom - prect->top),
				hCompatibleDC, (prect->left + xCurrentScroll)/Scale, (prect->top + yCurrentScroll)/Scale, 
				(prect->right - prect->left)/Scale, (prect->bottom - prect->top)/Scale, SRCCOPY);
			fScroll = FALSE;
		}

		if (fDraw)
			Drawing(hDC);

		EndPaint(hWnd, &ps);

		break;
	}
	case WM_HSCROLL:
	{
		int xDelta;    
		int xNewPos;   
		int yDelta = 0;

		switch (LOWORD(wParam))
		{
		case SB_PAGEUP:
			xNewPos = xCurrentScroll - 50;
			break;

		case SB_PAGEDOWN:
			xNewPos = xCurrentScroll + 50;
			break;

		case SB_LINEUP:
			xNewPos = xCurrentScroll - 5;
			break;

		case SB_LINEDOWN:
			xNewPos = xCurrentScroll + 5;
			break;

		case SB_THUMBPOSITION:
			xNewPos = HIWORD(wParam);
			break;

		default:
			xNewPos = xCurrentScroll;
		}

		xNewPos = max(0, xNewPos);
		xNewPos = min(xMaxScroll, xNewPos);

		if (xNewPos == xCurrentScroll)
			break;

		fScroll = TRUE;

		xDelta = xNewPos - xCurrentScroll;

		xCurrentScroll = xNewPos;

		ScrollWindowEx(hWnd, -xDelta, -yDelta, (CONST RECT *) NULL,
			(CONST RECT *) NULL, (HRGN)NULL, (PRECT)NULL,
			SW_INVALIDATE);
		UpdateWindow(hWnd);

		si.cbSize = sizeof(si);
		si.fMask = SIF_POS;
		si.nPos = xCurrentScroll;
		SetScrollInfo(hWnd, SB_HORZ, &si, TRUE);

		break;
	}

	case WM_VSCROLL:
	{
		int xDelta = 0;
		int yDelta;    
		int yNewPos; 

		switch (LOWORD(wParam))
		{
		case SB_PAGEUP:
			yNewPos = yCurrentScroll - 50;
			break;

		case SB_PAGEDOWN:
			yNewPos = yCurrentScroll + 50;
			break;

		case SB_LINEUP:
			yNewPos = yCurrentScroll - 5;
			break;

		case SB_LINEDOWN:
			yNewPos = yCurrentScroll + 5;
			break;

		case SB_THUMBPOSITION:
			yNewPos = HIWORD(wParam);
			break;

		default:
			yNewPos = yCurrentScroll;
		}

		yNewPos = max(0, yNewPos);
		yNewPos = min(yMaxScroll, yNewPos);

		if (yNewPos == yCurrentScroll)
			break;

		fScroll = TRUE;

		yDelta = yNewPos - yCurrentScroll;

		yCurrentScroll = yNewPos;

		ScrollWindowEx(hWnd, -xDelta, -yDelta, (CONST RECT *) NULL,
			(CONST RECT *) NULL, (HRGN)NULL, (PRECT)NULL,
			SW_INVALIDATE);
		UpdateWindow(hWnd);

		si.cbSize = sizeof(si);
		si.fMask = SIF_POS;
		si.nPos = yCurrentScroll;
		SetScrollInfo(hWnd, SB_VERT, &si, TRUE);

		break;
	}
	case WM_DESTROY: 				
		while (!hBitmaps.empty())
		{
			DeleteObject(hBitmaps.back());
			hBitmaps.pop_back();
		}
		DestroyWindow(hColorDlg);
		if(hCurrentFont)
			DeleteObject(hCurrentFont);
		if (hPen)
			DeleteObject(hPen);
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

void ReadBitmapByHandle(HWND hWnd, HBITMAP hBitmap, BOOL fResetScroll)
{
	GetClientRect(hWnd, &ClientRect);
	GetObject(hBitmap, sizeof(BITMAP), &Bitmap);
	SelectObject(hCompatibleDC, hBitmap);
	fBlt = TRUE;

	if (fResetScroll)
	{
		xMaxScroll = Bitmap.bmWidth*Scale - ClientRect.right;
		xCurrentScroll = 0;
		si.cbSize = sizeof(si);
		si.fMask = SIF_RANGE | SIF_PAGE | SIF_POS;
		si.nMin = xMinScroll;
		si.nMax = Bitmap.bmWidth*Scale;
		si.nPage = ClientRect.right;
		si.nPos = xCurrentScroll;
		SetScrollInfo(hWnd, SB_HORZ, &si, TRUE);

		yMaxScroll = Bitmap.bmHeight*Scale - ClientRect.bottom;
		yCurrentScroll = 0;
		si.cbSize = sizeof(si);
		si.fMask = SIF_RANGE | SIF_PAGE | SIF_POS;
		si.nMin = yMinScroll;
		si.nMax = Bitmap.bmHeight*Scale;
		si.nPage = ClientRect.bottom;
		si.nPos = yCurrentScroll;
		SetScrollInfo(hWnd, SB_VERT, &si, TRUE);
	}
	ScrollWindowEx(hWnd, -xCurrentScroll, -yCurrentScroll, (CONST RECT *) NULL,
		(CONST RECT *) NULL, (HRGN)NULL, (PRECT)NULL,
		SW_INVALIDATE);

	InvalidateRect(hWnd, NULL, TRUE);
	UpdateWindow(hWnd);
}

PBITMAPINFO CreateBitmapInfoStruct(HBITMAP hBmp)
{
	BITMAP bmp;
	PBITMAPINFO pbmi;
	WORD    cClrBits;

	assert(GetObject(hBmp, sizeof(BITMAP), (LPSTR)&bmp));

	cClrBits = (WORD)(bmp.bmPlanes * bmp.bmBitsPixel);
	if (cClrBits == 1)
		cClrBits = 1;
	else if (cClrBits <= 4)
		cClrBits = 4;
	else if (cClrBits <= 8)
		cClrBits = 8;
	else if (cClrBits <= 16)
		cClrBits = 16;
	else if (cClrBits <= 24)
		cClrBits = 24;
	else cClrBits = 32;

	if (cClrBits < 24)
		pbmi = (PBITMAPINFO)LocalAlloc(LPTR,
			sizeof(BITMAPINFOHEADER) +
			sizeof(RGBQUAD) * (1 << cClrBits));

	else
		pbmi = (PBITMAPINFO)LocalAlloc(LPTR,
			sizeof(BITMAPINFOHEADER));

	pbmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	pbmi->bmiHeader.biWidth = bmp.bmWidth;
	pbmi->bmiHeader.biHeight = bmp.bmHeight;
	pbmi->bmiHeader.biPlanes = bmp.bmPlanes;
	pbmi->bmiHeader.biBitCount = bmp.bmBitsPixel;
	if (cClrBits < 24)
		pbmi->bmiHeader.biClrUsed = (1 << cClrBits);

	pbmi->bmiHeader.biCompression = BI_RGB;

	pbmi->bmiHeader.biSizeImage = ((pbmi->bmiHeader.biWidth * cClrBits + 31) & ~31) / 8
		* pbmi->bmiHeader.biHeight;

	pbmi->bmiHeader.biClrImportant = 0;
	return pbmi;
}

void CreateBMPFile(LPTSTR pszFile, HBITMAP hBMP)
{
	HANDLE hf;                 
	BITMAPFILEHEADER hdr;      
	PBITMAPINFOHEADER pbih;    
	LPBYTE lpBits;              
	DWORD dwTotal;              
	DWORD cb;                  
	BYTE *hp;                   
	DWORD dwTmp;
	PBITMAPINFO pbi;
	HDC hDC;

	hDC = CreateCompatibleDC(GetWindowDC(GetDesktopWindow()));
	SelectObject(hDC, hBMP);

	pbi = CreateBitmapInfoStruct(hBMP);

	pbih = (PBITMAPINFOHEADER)pbi;
	lpBits = (LPBYTE)GlobalAlloc(GMEM_FIXED, pbih->biSizeImage);

	assert(lpBits);

	assert(GetDIBits(hDC, hBMP, 0, (WORD)pbih->biHeight, lpBits, pbi,
		DIB_RGB_COLORS));

	hf = CreateFile(pszFile,
		GENERIC_READ | GENERIC_WRITE,
		(DWORD)0,
		NULL,
		CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL,
		(HANDLE)NULL);
	assert(hf != INVALID_HANDLE_VALUE);

	hdr.bfType = 0x4d42;         
	hdr.bfSize = (DWORD)(sizeof(BITMAPFILEHEADER) +
		pbih->biSize + pbih->biClrUsed
		* sizeof(RGBQUAD) + pbih->biSizeImage);
	hdr.bfReserved1 = 0;
	hdr.bfReserved2 = 0;

	hdr.bfOffBits = (DWORD) sizeof(BITMAPFILEHEADER) + pbih->biSize + pbih->biClrUsed * sizeof(RGBQUAD);

	assert(WriteFile(hf, (LPVOID)&hdr, sizeof(BITMAPFILEHEADER), (LPDWORD)&dwTmp, NULL));

	assert(WriteFile(hf, (LPVOID)pbih, sizeof(BITMAPINFOHEADER) + pbih->biClrUsed * sizeof(RGBQUAD), (LPDWORD)&dwTmp, (NULL)));
 
	dwTotal = cb = pbih->biSizeImage;
	hp = lpBits;
	assert(WriteFile(hf, (LPSTR)hp, (int)cb, (LPDWORD)&dwTmp, NULL));

	assert(CloseHandle(hf));

	GlobalFree((HGLOBAL)lpBits);
}

INT_PTR CALLBACK InputScale(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	int newScale;
	BOOL Translated;
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		SetDlgItemInt(hDlg, IDC_EDITSCALE, Scale * 100, FALSE);
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK)
		{
			newScale = GetDlgItemInt(hDlg, IDC_EDITSCALE, &Translated, FALSE);
			if (Translated)
				Scale = (float)newScale / 100;
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		if (LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}

void BitmapPushback(HWND hWnd, HBITMAP hBitmap, BOOL fResetScroll)
{
	CurrentBitmap++;
	while (CurrentBitmap < hBitmaps.size())
	{
		DeleteObject(hBitmaps.back());
		hBitmaps.pop_back();
	}
	hBitmaps.push_back(hBitmap);
	ReadBitmapByHandle(hWnd, hBitmap, fResetScroll);
}

void CopyBitmap()
{
	hCompatibleBitmap = (HBITMAP)CopyImage(hBitmaps[CurrentBitmap], IMAGE_BITMAP, Bitmap.bmWidth, Bitmap.bmHeight, LR_CREATEDIBSECTION);
	SelectObject(hCompatibleDC, hCompatibleBitmap);
}

void RotateBitmap(HWND hWnd, UINT TypeOfRotation)
{
	HBITMAP hDestBitmap;
	HDC hDestDC = CreateCompatibleDC(GetWindowDC(GetDesktopWindow()));
	if((TypeOfRotation == ROTATELEFT) || (TypeOfRotation == ROTATERIGHT))
	hDestBitmap = CreateBitmap(Bitmap.bmHeight, Bitmap.bmWidth, Bitmap.bmPlanes, Bitmap.bmBitsPixel, Bitmap.bmBits);
	else
		hDestBitmap = CreateBitmap(Bitmap.bmWidth, Bitmap.bmHeight, Bitmap.bmPlanes, Bitmap.bmBitsPixel, Bitmap.bmBits);
	SelectObject(hDestDC, hDestBitmap);
	POINT UpLeft, UpRight, LowLeft;
	switch (TypeOfRotation)
	{
	case ROTATERIGHT:
		LowLeft.x = LowLeft.y = UpLeft.y = 0;
		UpLeft.x = UpRight.x = Bitmap.bmHeight;
		UpRight.y = Bitmap.bmWidth;
		break;
	case ROTATELEFT:
		UpRight.x = UpRight.y = UpLeft.x = 0;
		LowLeft.x = Bitmap.bmHeight;
		UpLeft.y = LowLeft.y = Bitmap.bmWidth;
		break;
	case ROTATEFLIP:
		LowLeft.y = UpRight.x = 0;
		UpLeft.y = UpRight.y = Bitmap.bmHeight;
		LowLeft.x = UpLeft.x = Bitmap.bmWidth;
		break;
	case MIRRORHORZ:
		UpRight.x = UpRight.y = UpLeft.y = 0;
		LowLeft.y = Bitmap.bmHeight;
		UpLeft.x = LowLeft.x = Bitmap.bmWidth;
		break;
	case MIRRORVERT:
		LowLeft.x = LowLeft.y = UpLeft.x = 0;
		UpLeft.y = UpRight.y = Bitmap.bmHeight;
		UpRight.x = Bitmap.bmWidth;
		break;
	}
	POINT ArrPoints[3] = { UpLeft, UpRight, LowLeft };
	PlgBlt(hDestDC, ArrPoints, hCompatibleDC, 0, 0, Bitmap.bmWidth, Bitmap.bmHeight, NULL, NULL, NULL);
	HBITMAP hNewBitmap;
	if ((TypeOfRotation == ROTATELEFT) || (TypeOfRotation == ROTATERIGHT))
		hNewBitmap = (HBITMAP)CopyImage(hDestBitmap, IMAGE_BITMAP, Bitmap.bmHeight, Bitmap.bmWidth, LR_CREATEDIBSECTION);
	else 
		hNewBitmap = (HBITMAP)CopyImage(hDestBitmap, IMAGE_BITMAP, Bitmap.bmWidth, Bitmap.bmHeight, LR_CREATEDIBSECTION);
	BitmapPushback(hWnd, hNewBitmap, TRUE);
	DeleteObject(hDestBitmap);
	DeleteDC(hDestDC);
}

LPRECT GetBitmapRect()
{
	RECT rt;
	rt.bottom = Bitmap.bmHeight;
	rt.right = Bitmap.bmWidth;
	return &rt;
}

LRESULT CALLBACK ColorDialog(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	static HWND hColorButtons[32];
	static HBRUSH hBrush = NULL;
	switch (message)
	{
	case WM_CREATE:
		for (int i = 0; i < 16; i++)
			arrCustomColors[i] = RGB(255, 255, 255);

		for (int i = 0, y = 5, id = IDB_CLR1; y < 100; y += 25)
		{
			for (int x = 5; x < 200; x += 25, i++, id++)
			{
				hColorButtons[i] = CreateWindow("BUTTON", NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | BS_OWNERDRAW, x, y, 20, 20,
					hDlg, (HMENU)id, (HINSTANCE)GetWindowLong(hDlg, GWL_HINSTANCE), NULL);
			}
		}
		break;
	case WM_CTLCOLORBTN:
	{
		for (int i = 0; i < 32; i++)
		{
			if ((HWND)lParam == hColorButtons[i])
			{
				if(hBrush)
					DeleteObject(hBrush);
				if (i < 16)
					hBrush = CreateSolidBrush(arrDefaultColors[i]);
				else
					hBrush = CreateSolidBrush(arrCustomColors[i - 16]);
				return (LRESULT)hBrush;
			}
		}
		break;
	}
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDB_CLR1:
			rgbCurrent = arrDefaultColors[0];
			break;
		case IDB_CLR2:
			rgbCurrent = arrDefaultColors[1];
			break;
		case IDB_CLR3:
			rgbCurrent = arrDefaultColors[2];
			break;
		case IDB_CLR4:
			rgbCurrent = arrDefaultColors[3];
			break;
		case IDB_CLR5:
			rgbCurrent = arrDefaultColors[4];
			break;
		case IDB_CLR6:
			rgbCurrent = arrDefaultColors[5];
			break;
		case IDB_CLR7:
			rgbCurrent = arrDefaultColors[6];
			break;
		case IDB_CLR8:
			rgbCurrent = arrDefaultColors[7];
			break;
		case IDB_CLR9:
			rgbCurrent = arrDefaultColors[8];
			break;
		case IDB_CLR10:
			rgbCurrent = arrDefaultColors[9];
			break;
		case IDB_CLR11:
			rgbCurrent = arrDefaultColors[10];
			break;
		case IDB_CLR12:
			rgbCurrent = arrDefaultColors[11];
			break;
		case IDB_CLR13:
			rgbCurrent = arrDefaultColors[12];
			break;
		case IDB_CLR14:
			rgbCurrent = arrDefaultColors[13];
			break;
		case IDB_CLR15:
			rgbCurrent = arrDefaultColors[14];
			break;
		case IDB_CLR16:
			rgbCurrent = arrDefaultColors[15];
			break;
		case IDB_CLR17:
			rgbCurrent = arrCustomColors[0];
			break;
		case IDB_CLR18:
			rgbCurrent = arrCustomColors[1];
			break;
		case IDB_CLR19:
			rgbCurrent = arrCustomColors[2];
			break;
		case IDB_CLR20:
			rgbCurrent = arrCustomColors[3];
			break;
		case IDB_CLR21:
			rgbCurrent = arrCustomColors[4];
			break;
		case IDB_CLR22:
			rgbCurrent = arrCustomColors[5];
			break;
		case IDB_CLR23:
			rgbCurrent = arrCustomColors[6];
			break;
		case IDB_CLR24:
			rgbCurrent = arrCustomColors[7];
			break;
		case IDB_CLR25:
			rgbCurrent = arrCustomColors[8];
			break;
		case IDB_CLR26:
			rgbCurrent = arrCustomColors[9];
			break;
		case IDB_CLR27:
			rgbCurrent = arrCustomColors[10];
			break;
		case IDB_CLR28:
			rgbCurrent = arrCustomColors[11];
			break;
		case IDB_CLR29:
			rgbCurrent = arrCustomColors[12];
			break;
		case IDB_CLR30:
			rgbCurrent = arrCustomColors[13];
			break;
		case IDB_CLR31:
			rgbCurrent = arrCustomColors[14];
			break;
		case IDB_CLR32:
			rgbCurrent = arrCustomColors[15];
			break;
		}
		SelectPen(hCompatibleDC);
		break;
	case WM_DESTROY:
		for (int i = 0; i < 32; i++)
			DestroyWindow(hColorButtons[i]);
		DeleteObject(hBrush);
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hDlg, message, wParam, lParam);
	}
	return 0;
}

LRESULT CALLBACK ToolsDialog(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	static HWND hTools[14];
	HBITMAP hBitmap[14];
	HWND hComboBox = NULL;
	switch (message)
	{
	case WM_CREATE:
		for (int i = 0, y = 5, id = IDT_PENCIL, idb = IDB_BITMAPPENCIL; y < 220; y += 35)
		{
			for (int x = 5; x < 70; x += 35, i++, id++, idb++)
			{
				hTools[i] = CreateWindow("BUTTON", NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | BS_BITMAP, x, y, 30, 30,
					hDlg, (HMENU)id, (HINSTANCE)GetWindowLong(hDlg, GWL_HINSTANCE), NULL);
				hBitmap[i] = (HBITMAP)LoadImageA((HINSTANCE)GetWindowLong(hDlg, GWL_HINSTANCE), MAKEINTRESOURCE(idb), IMAGE_BITMAP, 25, 25, NULL);
				SendDlgItemMessage(hDlg, id, BM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)hBitmap[i]);
			}
		}
		hComboBox = CreateWindow("COMBOBOX", NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | CBS_DROPDOWN | CBS_HASSTRINGS, 5, 250, 65, 200, hDlg, 
			(HMENU)IDC_EDITPENWIDTH, (HINSTANCE)GetWindowLong(hDlg, GWL_HINSTANCE), NULL);
		for (int i = 0; i < 8; i++)
		{
			CHAR buffer[5];
			_itoa_s(pow(2, i), buffer, 10);
			SendMessage(hComboBox, (UINT)CB_ADDSTRING, (WPARAM)0, (LPARAM)buffer);
			SendMessage(hComboBox, (UINT)CB_SETITEMDATA, (WPARAM)i, (LPARAM)pow(2, i));
		}
		SendMessage(hComboBox, (UINT)CB_SETCURSEL, (WPARAM)0, (LPARAM)NULL);
		break;
	case WM_COMMAND:
		if (HIWORD(wParam) == CBN_SELCHANGE)
		{
			int ItemIndex = SendMessage((HWND)lParam, (UINT)CB_GETCURSEL, (WPARAM)0, (LPARAM)0);
			penWidth = SendMessage((HWND)lParam, (UINT)CB_GETITEMDATA, (WPARAM)ItemIndex, (LPARAM)0);
			break;
		}
		switch (LOWORD(wParam))
		{
		case IDT_PENCIL:
		case IDT_BRUSH:
		case IDT_ERASER:
			fInstantDrawing = TRUE;
			idCurrentTool = LOWORD(wParam);
			break;
		case IDT_BUCKET:
		case IDT_COLORPICKER:
		case IDT_CURVE:
		case IDT_ELLIPSE:
		case IDT_FILLEDELLIPSE:
		case IDT_LINE:
		case IDT_RECT:
		case IDT_FILLEDRECT:
		case IDT_ROUNDEDRECT:
		case IDT_FILLEDROUNDEDRECT:
		case IDT_TEXT:
			fInstantDrawing = FALSE;
			idCurrentTool = LOWORD(wParam);
			break;
		}
		if (fDrawingCurve)
		{
			CopyBitmap();
			Draw(GetParent(hDlg), IDT_CURVE);
			fDraw = FALSE;
		}
		SelectPen(hCompatibleDC);
		break;
	case WM_DESTROY:
		for (int i = 0; i < 14; i++)
		{
			DestroyWindow(hTools[i]);
			DeleteObject(hBitmap[i]);
		}
		DestroyWindow(hComboBox);
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hDlg, message, wParam, lParam);
	}
	return 0;
}

VOID Drawing(HDC hDC)
{
	fDrawing = TRUE;
	HBRUSH hBrush;
	if (xCurrentPos > Bitmap.bmWidth*Scale)
		xCurrentPos = Bitmap.bmWidth*Scale;
	if (yCurrentPos > Bitmap.bmHeight*Scale)
		yCurrentPos = Bitmap.bmHeight*Scale;
	switch (idCurrentTool)
	{
	case IDT_PENCIL:
	case IDT_BRUSH:
	case IDT_ERASER:
		MoveToEx(hCompatibleDC, (xStartPos+xCurrentScroll) / Scale, (yStartPos+yCurrentScroll) / Scale, NULL);
		LineTo(hCompatibleDC, (xCurrentPos+xCurrentScroll) / Scale, (yCurrentPos+yCurrentScroll) / Scale);
		xStartPos = xCurrentPos;
		yStartPos = yCurrentPos;
		break;
	case IDT_TEXT:
		hOldBrush = (HBRUSH)SelectObject(hDC, GetStockObject(NULL_BRUSH));
		SelectPen(hDC);
		EditTextRect.left = min(xStartPos, xCurrentPos);
		if (EditTextRect.left < 0)
			EditTextRect.left = 0;
		EditTextRect.top = min(yStartPos, yCurrentPos);
		if (EditTextRect.top < 0)
			EditTextRect.top = 0;
		EditTextRect.right = max(xStartPos, xCurrentPos);
		if (EditTextRect.right > ClientRect.right || EditTextRect.right > Bitmap.bmWidth*Scale)
			EditTextRect.right = min(ClientRect.right, Bitmap.bmWidth*Scale);
		EditTextRect.bottom = max(yStartPos, yCurrentPos);
		if (EditTextRect.bottom > ClientRect.bottom || EditTextRect.bottom > Bitmap.bmHeight*Scale)
			EditTextRect.bottom = min(ClientRect.bottom, Bitmap.bmHeight*Scale);
		Rectangle(hDC, EditTextRect.left, EditTextRect.top, EditTextRect.right, EditTextRect.bottom);
		SelectObject(hDC, hOldBrush);
		break;
	case IDT_LINE:
		SelectPen(hDC);
		MoveToEx(hDC, xStartPos, yStartPos, NULL);
		LineTo(hDC, xCurrentPos, yCurrentPos);
		break;
	case IDT_CURVE:
		fDrawingCurve = TRUE;
		if (nCurvePoints == 2)
		{
			arrCurvePoints[0].x = xStartPos;
			arrCurvePoints[0].y = yStartPos;
			arrCurvePoints[1].x = xStartPos;
			arrCurvePoints[1].y = yStartPos;
			arrCurvePoints[2].x = xCurrentPos;
			arrCurvePoints[2].y = yCurrentPos;
			arrCurvePoints[3].x = xCurrentPos;
			arrCurvePoints[3].y = yCurrentPos;
		}
		if (nCurvePoints == 3)
		{
			arrCurvePoints[1].x = xCurrentPos;
			arrCurvePoints[1].y = yCurrentPos;
			arrCurvePoints[2].x = xCurrentPos;
			arrCurvePoints[2].y = yCurrentPos;
		}
		if (nCurvePoints == 4)
		{
			arrCurvePoints[2].x = xCurrentPos;
			arrCurvePoints[2].y = yCurrentPos;
		}
		SelectPen(hDC);
		PolyBezier(hDC, arrCurvePoints, 4);
		break;
	case IDT_RECT:
		SelectPen(hDC);
		hOldBrush = (HBRUSH)SelectObject(hDC, GetStockObject(NULL_BRUSH));
		Rectangle(hDC, xStartPos, yStartPos, xCurrentPos, yCurrentPos);
		SelectObject(hDC, hOldBrush);
		break;
	case IDT_FILLEDRECT:
		SelectPen(hDC);
		hBrush = CreateSolidBrush(rgbCurrent);
		hOldBrush = (HBRUSH)SelectObject(hDC, hBrush);
		Rectangle(hDC, xStartPos, yStartPos, xCurrentPos, yCurrentPos);
		SelectObject(hDC, hOldBrush);
		DeleteObject(hBrush);
		break;
	case IDT_ROUNDEDRECT:
		SelectPen(hDC);
		hOldBrush = (HBRUSH)SelectObject(hDC, GetStockObject(NULL_BRUSH));
		RoundRect(hDC, xStartPos, yStartPos, xCurrentPos, yCurrentPos, abs(xStartPos - xCurrentPos)/2,abs(yStartPos - yCurrentPos)/2);
		SelectObject(hDC, hOldBrush);
		break;
	case IDT_FILLEDROUNDEDRECT:
		SelectPen(hDC);
		hBrush = CreateSolidBrush(rgbCurrent);
		hOldBrush = (HBRUSH)SelectObject(hDC, hBrush);
		RoundRect(hDC, xStartPos, yStartPos, xCurrentPos, yCurrentPos, abs(xStartPos - xCurrentPos) / 2, abs(yStartPos - yCurrentPos) / 2);
		SelectObject(hDC, hOldBrush);
		DeleteObject(hBrush);
		break;
	case IDT_ELLIPSE:
		SelectPen(hDC);
		hOldBrush = (HBRUSH)SelectObject(hDC, GetStockObject(NULL_BRUSH));
		Ellipse(hDC, xStartPos, yStartPos, xCurrentPos, yCurrentPos);
		SelectObject(hDC, hOldBrush);
		break;
	case IDT_FILLEDELLIPSE:
		SelectPen(hDC);
		hBrush = CreateSolidBrush(rgbCurrent);
		hOldBrush = (HBRUSH)SelectObject(hDC, hBrush);
		Ellipse(hDC, xStartPos, yStartPos, xCurrentPos, yCurrentPos);
		SelectObject(hDC, hOldBrush);
		DeleteObject(hBrush);
		break;
	}
	fDrawing = FALSE;
}

VOID SelectPen(HDC hDC)
{
	FLOAT Scaling = 1;
	if (fDrawing)
		Scaling = Scale;
	if (hPen)
		DeleteObject(hPen);
	if (idCurrentTool == IDT_PENCIL)
		hPen = CreatePen(PS_SOLID, 0, rgbCurrent);
	else if (idCurrentTool == IDT_TEXT)
		hPen = CreatePen(PS_DASH, 0, RGB(0, 0, 0));
	else if (idCurrentTool == IDT_ERASER)
		hPen = CreatePen(PS_SOLID, penWidth*Scaling, RGB(255, 255, 255));
	else
		hPen = CreatePen(PS_SOLID, penWidth*Scaling, rgbCurrent);
	SelectObject(hDC, hPen);
}

VOID Draw(HWND hWnd, UINT idTool)
{
	if (idTool != IDT_TEXT)
	{
		xStartPos += xCurrentScroll;
		yStartPos += yCurrentScroll;
		xFinalPos += xCurrentScroll;
		yFinalPos += yCurrentScroll;
	}
	HBRUSH hBrush;
	switch (idTool)
	{
	case IDT_TEXT:
		EditTextRect.left = min(xStartPos, xFinalPos);
		if (EditTextRect.left < 0)
			EditTextRect.left = 0;
		EditTextRect.top = min(yStartPos, yFinalPos);
		if (EditTextRect.top < 0)
			EditTextRect.top = 0;
		EditTextRect.right = max(xStartPos, xFinalPos);
		if (EditTextRect.right > ClientRect.right || EditTextRect.right > Bitmap.bmWidth*Scale)
			EditTextRect.right = min(ClientRect.right,Bitmap.bmWidth*Scale);
		EditTextRect.bottom = max(yStartPos, yFinalPos);
		if (EditTextRect.bottom > ClientRect.bottom || EditTextRect.bottom > Bitmap.bmHeight*Scale)
			EditTextRect.bottom = min(ClientRect.bottom, Bitmap.bmHeight*Scale);
		hEditText = CreateWindow("EDIT", NULL, WS_VISIBLE | WS_CHILD | WS_BORDER | ES_MULTILINE, EditTextRect.left, 
			EditTextRect.top, EditTextRect.right - EditTextRect.left, EditTextRect.bottom - EditTextRect.top, hWnd, NULL, hInst, NULL);
		SendMessage(hEditText, WM_SETFONT, (WPARAM)hCurrentFont, (LPARAM)TRUE);
		SelectObject(hCompatibleDC, hCurrentFont);
		fActiveEdit = TRUE;
		break;
	case IDT_LINE:
		MoveToEx(hCompatibleDC, xStartPos / Scale, yStartPos / Scale, NULL);
		LineTo(hCompatibleDC, xFinalPos / Scale, yFinalPos / Scale);
		break;
	case IDT_CURVE:
		for (int i = 0; i < 4; i++)
		{
			arrCurvePoints[i].x = (arrCurvePoints[i].x + xCurrentScroll) / Scale;
			arrCurvePoints[i].y = (arrCurvePoints[i].y + yCurrentScroll) / Scale;
		}
		PolyBezier(hCompatibleDC, arrCurvePoints, 4);
		nCurvePoints = 2;
		fDrawingCurve = FALSE;
		break;
	case IDT_RECT:
		hOldBrush = (HBRUSH)SelectObject(hCompatibleDC, GetStockObject(NULL_BRUSH));
		Rectangle(hCompatibleDC, xStartPos / Scale, yStartPos / Scale, xFinalPos / Scale, yFinalPos / Scale);
		SelectObject(hCompatibleDC, hOldBrush);
		break;
	case IDT_FILLEDRECT:
		hBrush = CreateSolidBrush(rgbCurrent);
		hOldBrush = (HBRUSH)SelectObject(hCompatibleDC, hBrush);
		Rectangle(hCompatibleDC, xStartPos / Scale, yStartPos / Scale, xFinalPos / Scale, yFinalPos / Scale);
		SelectObject(hCompatibleDC, hOldBrush);
		DeleteObject(hBrush);
		break;
	case IDT_ROUNDEDRECT:
		hOldBrush = (HBRUSH)SelectObject(hCompatibleDC, GetStockObject(NULL_BRUSH));
		RoundRect(hCompatibleDC, xStartPos / Scale, yStartPos / Scale, xFinalPos / Scale, yFinalPos / Scale, 
			abs(xStartPos - xFinalPos) / 2 / Scale, abs(yStartPos - yFinalPos) / 2 / Scale);
		SelectObject(hCompatibleDC, hOldBrush);
		break;
	case IDT_FILLEDROUNDEDRECT:
		hBrush = CreateSolidBrush(rgbCurrent);
		hOldBrush = (HBRUSH)SelectObject(hCompatibleDC, hBrush);
		RoundRect(hCompatibleDC, xStartPos / Scale, yStartPos / Scale, xFinalPos / Scale, yFinalPos / Scale,
			abs(xStartPos - xFinalPos) / 2 / Scale, abs(yStartPos - yFinalPos) / 2 / Scale);
		SelectObject(hCompatibleDC, hOldBrush);
		DeleteObject(hBrush);
		break;
	case IDT_ELLIPSE:
		hOldBrush = (HBRUSH)SelectObject(hCompatibleDC, GetStockObject(NULL_BRUSH));
		Ellipse(hCompatibleDC, xStartPos / Scale, yStartPos / Scale, xFinalPos / Scale, yFinalPos / Scale);
		SelectObject(hCompatibleDC, hOldBrush);
		break;
	case IDT_FILLEDELLIPSE:
		hBrush = CreateSolidBrush(rgbCurrent);
		hOldBrush = (HBRUSH)SelectObject(hCompatibleDC, hBrush);
		Ellipse(hCompatibleDC, xStartPos / Scale, yStartPos / Scale, xFinalPos / Scale, yFinalPos / Scale);
		SelectObject(hCompatibleDC, hOldBrush);
		DeleteObject(hBrush);
		break;
	}
	BitmapPushback(hWnd, hCompatibleBitmap, FALSE);
}

INT_PTR CALLBACK Parameters(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	int newWidth, newHeight;
	BOOL Translated1, Translated2;
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		SetDlgItemInt(hDlg, IDC_EDITWIDTH, Bitmap.bmWidth, FALSE);
		SetDlgItemInt(hDlg, IDC_EDITHEIGHT, Bitmap.bmHeight, FALSE);
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK)
		{
			newWidth = GetDlgItemInt(hDlg, IDC_EDITWIDTH, &Translated1, FALSE);
			newHeight = GetDlgItemInt(hDlg, IDC_EDITHEIGHT, &Translated2, FALSE);
			if (Translated1 && Translated2)
			{
				HDC hDC;
				hDC = CreateCompatibleDC(GetDC(hDlg));
				HBITMAP hBM;
				hBM = CreateCompatibleBitmap(GetDC(hDlg), newWidth, newHeight);
				SelectObject(hDC, hBM);
				StretchBlt(hDC, 0, 0, newWidth, newHeight, hCompatibleDC, 0, 0, Bitmap.bmWidth, Bitmap.bmHeight, SRCCOPY);
				hCompatibleBitmap = (HBITMAP)CopyImage(hBM, IMAGE_BITMAP, newWidth, newHeight, LR_CREATEDIBSECTION);
				BitmapPushback(GetParent(hDlg), hCompatibleBitmap, TRUE);
				DeleteDC(hDC);
			}
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		if (LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}

INT_PTR CALLBACK HelpDialog(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	HWND hList;
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		hList = GetDlgItem(hDlg, IDC_LISTHELP);
		SendMessage(hList, LB_INSERTSTRING, -1, (LPARAM)"Файл");
		SendMessage(hList, LB_SETITEMDATA, 0, (LPARAM)"Читання - відкривається ділогове вікно перегляду каталогів диску з можливістю обрати файл для відкриття.\r\nЗапис - відкривається вікно переглду каталогів диску з можливістю обрати каталог для збереження файлу та введення ім'я нового файлу.");
		SendMessage(hList, LB_INSERTSTRING, -1, (LPARAM)"Правка");
		SendMessage(hList, LB_SETITEMDATA, 1, (LPARAM)"Буфер обміну - за допомогою цього пункту можна зчитати малюнок з буферу обміну(\"Ctrl+V\") або записати малюнок до буферу обміну(\"Ctrl+C\").\r\nСкасувати дію(\"Ctrl+Z\") - скасовує результати останньої дії над малюнком.\r\nПовторити скасовану дію(\"Ctrl+Y\") - повторює результати останньої скасованої дії.");
		SendMessage(hList, LB_INSERTSTRING, -1, (LPARAM)"Вид");
		SendMessage(hList, LB_SETITEMDATA, 2, (LPARAM)"Атрибути тексту - відкривається ділогове вікно з можливістю зміни атрибутів тексту(шрифт, розмір та ін.), що виводиться на малюнку.\r\nМасштаб - відкривається ділогове вікно з можливістю вводу нового масштабу відображення малюнку.");
		SendMessage(hList, LB_INSERTSTRING, -1, (LPARAM)"Рисунок");
		SendMessage(hList, LB_SETITEMDATA, 3, (LPARAM)"Поворот - повертає або відзеркалює зображення у відповідному до обраного пункту напрямку.\r\nІнверсія - змінює кольори зображення на протилежні.\r\nОчищення - очищує зображення.\r\nЗміна параметрів рисунка - відкривається діалогове вікно з можливістю введення нових значень ширини та висоти малюнка, при натисканні кнопки \"ОК\" зображення буде розтягнуте або стиснуте відповідно до введених значень.");
		SendMessage(hList, LB_INSERTSTRING, -1, (LPARAM)"Палітра");
		SendMessage(hList, LB_SETITEMDATA, 4, (LPARAM)"Установка - відкривається діалогове вікно з можливістю вибору кольорів та встановлення нових кольорів, що знаходяться у вікні \"Палітра\"");
		SendMessage(hList, LB_INSERTSTRING, -1, (LPARAM)"Довідка");
		SendMessage(hList, LB_SETITEMDATA, 5, (LPARAM)"Переглянути довідку(\"F1\") - вікривається ділогове вікно з інформацією про можливості програми з описом пунктів меню та інструментів.\r\nПро програму - відкривається вікно з інформацією про програму.");
		SendMessage(hList, LB_INSERTSTRING, -1, (LPARAM)"Вікно \"Інструменти\"");
		SendMessage(hList, LB_SETITEMDATA, 6, (LPARAM)"Містить доступні користувачу інструменти для малювання та можливість вибору товщини ліній для таких інструментів як пензлик, гумка, лінія, прямокутник, елліпс і т.п., вибір інструменту здійснюється натисканням на потрібний інструмент.");
		SendMessage(hList, LB_INSERTSTRING, -1, (LPARAM)"Вікно \"Палітра\"");
		SendMessage(hList, LB_SETITEMDATA, 7, (LPARAM)"Містить доступні для вибору користувачем кольори якими здійснюється малювання, вибір кольору здійснюється натисканням на необхідний колір.");
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (HIWORD(wParam) == LBN_SELCHANGE)
		{
			hList = GetDlgItem(hDlg, IDC_LISTHELP);
			int nIndex;
			nIndex = SendMessage(hList, LB_GETCURSEL, NULL, NULL);
			SetDlgItemText(hDlg, IDC_EDITHELP, (LPCSTR)SendMessage(hList, LB_GETITEMDATA, nIndex, NULL));
		}
		if (LOWORD(wParam == IDCANCEL))
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
	}
	return (INT_PTR)FALSE;
}