#include "framework.h"
#include "Exterieur.h"
#include "MakerMVMap.h"

static std::unique_ptr<MakerMVMapCommon> currentMap;

FIBITMAP* makeGrid(const std::size_t SIZEX, const std::size_t SIZEY, const int width, const int height)
{
	double ratio = static_cast<double>(max(width, height)) / static_cast<double>(max(SIZEX, SIZEY));
	int reste = max(width, height) % max(SIZEX, SIZEY);
	FIBITMAP* grid = FreeImage_Allocate(width, height, 24);
	RGBQUAD pxzero;
	pxzero.rgbRed = 0;
	pxzero.rgbGreen = 0;
	pxzero.rgbBlue = 0;
	RGBQUAD pxwhite;
	pxwhite.rgbRed = 255;
	pxwhite.rgbGreen = 255;
	pxwhite.rgbBlue = 255;
	for (auto i = 0; i < width; ++i) {
		for (auto j = 0; j < height; ++j) {
			FreeImage_SetPixelColor(grid, i, j, &pxwhite);
		}
	}
	for (auto i = 0, ip = 0; i < SIZEY; ++i, ip = static_cast<int>(static_cast<double>(i) * ratio)) {
		for (auto j = 0, jp = 0; j < SIZEX; ++j, jp = static_cast<int>(static_cast<double>(j) * ratio)) {
			for (auto k = 0; static_cast<double>(k) < ratio; ++k) {
				FreeImage_SetPixelColor(grid, ip + k, jp, &pxzero);
				FreeImage_SetPixelColor(grid, ip, jp + k, &pxzero);
			}
		}
	}
	for (auto i = 0; i < width; ++i) {
		FreeImage_SetPixelColor(grid, i, 0, &pxwhite);
	}
	for (auto j = 0; j < height; ++j) {
		FreeImage_SetPixelColor(grid, 0, j, &pxwhite);
	}
	return grid;
}

FIBITMAP* makeGridFast(const std::size_t SIZEX, const std::size_t SIZEY, const int width, const int height)
{
	double ratio = static_cast<double>(max(width, height)) / static_cast<double>(max(SIZEX, SIZEY));
	int reste = max(width, height) % max(SIZEX, SIZEY);
	auto bmp = std::make_unique<uint8_t[]>(width * height * 3);

	memset(bmp.get(), 0xFF, width * height * 3);
	unsigned int index = 0;

	for (auto i = 0, ip = 0; i < SIZEY; ++i, ip = static_cast<int>(static_cast<double>(i)* ratio)) {
		for (int j = 1, jp = static_cast<int>(static_cast<double>(j)* ratio); j < SIZEX; ++j, jp = static_cast<int>(static_cast<double>(j)* ratio)) {
			for (auto k = 0; static_cast<double>(k) < ratio; ++k) {
				index = ((ip + k) + jp * width) * 3;
				bmp[index] = 0;
				bmp[++index] = 0;
				bmp[++index] = 0;
			}
		}
	}

	for (int i = 1, ip = static_cast<int>(static_cast<double>(i)* ratio); i < SIZEY; ++i, ip = static_cast<int>(static_cast<double>(i)* ratio)) {
		for (int j = 0, jp = 0; j < SIZEX; ++j, jp = static_cast<int>(static_cast<double>(j)* ratio)) {
			for (auto k = 0; static_cast<double>(k) < ratio; ++k) {
				index = (ip + (jp + k) * width) * 3;
				bmp.get()[index] = 0;
				bmp.get()[++index] = 0;
				bmp.get()[++index] = 0;
			}
		}
	}

	FIBITMAP* grid = FreeImage_ConvertFromRawBits(bmp.get(), width, height, width * 3, 24, 0xFF, 0xFF, 0xFF, TRUE);
	return grid;
}

void setDialog(HWND hDlg, int nWidth, int nHeigth)
{
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
	MoveWindow(GetDlgItem(hDlg, IDC_PICTURE), vPoint.x, vPoint.y, 256 * 3, 256 * 3, TRUE);
}

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
		if (currentMap) {
			//dispLandscape(hDlg, currentMap.get()->getMakerMVHeigth(), currentMap.get()->Width(), currentMap.get()->Heigth());
			dispLandscapeFast(hDlg, currentMap.get()->getMakerMVHeigth(), currentMap.get()->Width(), currentMap.get()->Heigth());
		}
		break;
	case IDC_FONCTION:
		if (wmCode == CBN_SELCHANGE) {
			terrain::generator.seed(1);
			terrain::initPerlin = true;
		}
		break;
	case IDC_GENERER:
		SetCursor(LoadCursor(NULL, IDC_WAIT));
		currentMap.reset();
		currentMap = std::make_unique<MakerMVMapCommon>(256, 256);
		terrain::CreateLandscape(currentMap.get()->getMakerMVHeigth(), currentMap.get()->Width(), currentMap.get()->Heigth(), static_cast<unsigned int>(SendDlgItemMessage(hDlg, IDC_GENERATEUR, CB_GETCURSEL, 0, 0)));
	case IDC_CHECK1:
		//dispLandscape(hDlg, currentMap.get()->getMakerMVHeigth(), currentMap.get()->Width(), currentMap.get()->Heigth());
		dispLandscapeFast(hDlg, currentMap.get()->getMakerMVHeigth(), currentMap.get()->Width(), currentMap.get()->Heigth());
		SetCursor(LoadCursor(NULL, IDC_ARROW));
		break;
	case IDC_EXPORTER:
		SetCursor(LoadCursor(NULL, IDC_WAIT));
		if(SendDlgItemMessage(hDlg, IDC_GENERATEUR, CB_GETCURSEL, 0, 0) == 2)
			currentMap.get()->setType(3);
		currentMap.get()->doRPGMakerMVMap();
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
	SendDlgItemMessage(hDlg, IDC_GENERATEUR, CB_ADDSTRING, 0, LPARAM(L"Diamant-carré"));
	SendDlgItemMessage(hDlg, IDC_GENERATEUR, CB_ADDSTRING, 0, LPARAM(L"Bruit de Perlin"));
	SendDlgItemMessage(hDlg, IDC_GENERATEUR, CB_ADDSTRING, 0, LPARAM(L"Bruit de tampon"));
	SendDlgItemMessage(hDlg, IDC_GENERATEUR, CB_ADDSTRING, 0, LPARAM(L"Aléatoire"));
	SendDlgItemMessage(hDlg, IDC_GENERATEUR, CB_ADDSTRING, 0, LPARAM(L"Tensor"));
	SendDlgItemMessage(hDlg, IDC_GENERATEUR, CB_ADDSTRING, 0, LPARAM(L"Volcano"));
	SendDlgItemMessage(hDlg, IDC_GENERATEUR, CB_ADDSTRING, 0, LPARAM(L"Sinus cardinal"));
	SendDlgItemMessage(hDlg, IDC_GENERATEUR, CB_SETCURSEL, 0, 0);
	setDialog(hDlg, 0, 0);

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
