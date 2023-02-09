#include "framework.h"
#include "Exterieur.h"
#include "d2d1.h"

/* X, Y*/
#define SIZEX 256
#define SIZEY 256

static std::array<std::array<signed int, SIZEY>, SIZEX> vHeights;

/* --------------------------------------------------------------------
	hCtl = 0 :
		wmCode = 0 : menu
		wmCode = 1 : accélérateur
   -------------------------------------------------------------------- */
static INT_PTR WmCommand(HWND hDlg, HWND hCtl, int wmId, int wmCode)
{
	INT_PTR lresult = (INT_PTR)FALSE;
	switch (wmId) {
	case IDC_RADIO1:
	case IDC_RADIO2:
	case IDC_RADIO3:
	case IDC_RADIO4:
	case IDC_RADIO5:
		//CheckRadioButton(hDlg, IDC_RADIO1, IDC_RADIO4, wmId);
		//PostMessage(hDlg, WM_COMMAND, IDOK, WPARAM(GetDlgItem(hDlg, IDOK)));
		dispLandscape(hDlg, vHeights);
		break;
	case IDC_GENERER:
		SetCursor(LoadCursor(NULL, IDC_WAIT));
		CreateLandscape(vHeights);
	case IDC_CHECK1:
		dispLandscape(hDlg, vHeights);
		SetCursor(LoadCursor(NULL, IDC_ARROW));
		break;
	case IDC_EXPORTER:
		SetCursor(LoadCursor(NULL, IDC_WAIT));
		doRPGMakerMVMap(vHeights);
		SetCursor(LoadCursor(NULL, IDC_ARROW));
		break;
	case IDCANCEL:
		EndDialog(hDlg, wmId);
		lresult = (INT_PTR)TRUE;
		break;
	default:
		break;
	}
	return lresult;
}

static INT_PTR WmNotify(HWND hDlg, HWND hCtrl, int wmId, int wmCode)
{
	INT_PTR lresult = (INT_PTR)FALSE;
	/*switch (wmId)
	{
	default:
		break;
	}*/
	return lresult;
}

static INT_PTR WmInitDialog(HWND hDlg, WPARAM wParam, LPARAM lParam)
{
	CheckRadioButton(hDlg, IDC_RADIO1, IDC_RADIO5, IDC_RADIO1);
	
	RECT vClientRect, vOwnerRect;

	GetWindowRect(hDlg, &vOwnerRect);
	MoveWindow(hDlg, vOwnerRect.left, vOwnerRect.top, vOwnerRect.right - vOwnerRect.left + 256 * 2, vOwnerRect.bottom - vOwnerRect.top + 256 * 2, TRUE);

	GetWindowRect(GetDlgItem(hDlg, IDCANCEL), &vClientRect);
	POINT vPoint;
	vPoint.x = vClientRect.left;
	vPoint.y = vClientRect.top;
	ScreenToClient(hDlg, &vPoint);
	GetClientRect(GetDlgItem(hDlg, IDCANCEL), &vClientRect);
	MoveWindow(GetDlgItem(hDlg, IDCANCEL), vPoint.x + 256 * 2, vPoint.y + 256 * 2, vClientRect.right, vClientRect.bottom, TRUE);

	GetWindowRect(GetDlgItem(hDlg, IDC_PICTURE), &vClientRect);
	vPoint;
	vPoint.x = vClientRect.left;
	vPoint.y = vClientRect.top;
	ScreenToClient(hDlg, &vPoint);
	MoveWindow(GetDlgItem(hDlg, IDC_PICTURE), vPoint.x, vPoint.y, 256*3, 256*3, TRUE);
	
	//PostMessage(hDlg, WM_COMMAND, IDOK, WPARAM(GetDlgItem(hDlg, IDOK)));
	return (INT_PTR)TRUE;
}

static INT_PTR WmContextMenu(HWND hDlg, HWND hCtl, int x, int y)
{
	return (INT_PTR)TRUE;
}

static INT_PTR WmCtlColorEdit(HWND hDlg, HDC hDc, HWND hCtl)
{
	unsigned int index = 0;
	int wmId = GetDlgCtrlID(hCtl);
	/*switch (wmId) {
	default:
		return (INT_PTR)FALSE;
	}*/
	return (INT_PTR)FALSE;
}

INT_PTR CALLBACK Exterieur(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
	case WM_NOTIFY:
		return WmNotify(hDlg, ((NMHDR*)lParam)->hwndFrom, ((NMHDR*)lParam)->idFrom, ((NMHDR*)lParam)->code);
	case WM_INITDIALOG:
		return WmInitDialog(hDlg, wParam, lParam);
	case WM_COMMAND:
		return WmCommand(hDlg, (HWND)lParam, LOWORD(wParam), HIWORD(wParam));
	case WM_CONTEXTMENU:
		return WmContextMenu(hDlg, (HWND)wParam, LOWORD(lParam), HIWORD(lParam));
	case WM_CTLCOLOREDIT:
		//case WM_CTLCOLORSTATIC: // Pour les controls inactifs ou en lecture seule.
		return WmCtlColorEdit(hDlg, (HDC)wParam, (HWND)lParam);
    }
    return (INT_PTR)FALSE;
}
