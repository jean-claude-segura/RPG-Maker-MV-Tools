#pragma once
#include "Resource.h"
#include "Includes\FreeImage.h"
#include "terrain.h"
#include <algorithm>

template<typename T>
unsigned int getLevel(T& vHeights, unsigned int size, int i, int j, signed int levels)
{
	auto amplitude = static_cast<double>(size) * 2. * 1.5;
	auto value = static_cast<double>(vHeights[i][j]);
	// 0 <= (value + 255. ) / 512. <= 1
	auto level = static_cast<double>(levels - 1) * (value + 255. ) / 512.;
	return static_cast<unsigned int>(level);
}

template<typename T>
unsigned int getLevelLand(T& vHeights, unsigned int size, int i, int j)
{
	double level;
	auto value = static_cast<double>(vHeights[i][j]);
	auto amplitude = static_cast<double>(size) * 1.5;
	/*if (value < 0) {
		level = 2. * (value + 255.) / 255.1;
	}
	else {
		level = 2 + 8. * value / 257.1;
	}*/
	if (value < 0) {
		// -255 <= Value < 0
		// 0 <= value + 255 < 255
		// 0 <= (value + 255) / 255 < 1
		// 0 <= 2. * (value + 255) / 255 < 2
		level = 2. * (value + 255.) / 255.;
	}
	else if(value > 0) {
		// 0 < Value <= 512. - 255.
		// 0 < value / (512. - 255.) <= 1
		// 0 < 7 * value / (512. - 255.) <= 7
		level = 2. + 8. * (value-1) / (512. - 255.);
	}
	else {
		level = 2;
	}
	return static_cast<unsigned int>(level);
}

template<typename T, typename U>
void getLevelLand(T& vHeights, U& vLevels, unsigned int size, int width, int heigth)
{
	auto amplitude = static_cast<double>(size) * 1.5;
	for (auto i = 0; i < width; ++i) {
		for (auto j = 0; j < heigth; ++j) {
			vLevels[i][j] = static_cast<unsigned int>(vHeights[i][j] < 0 ?
				static_cast<double>((vHeights[i][j]) + static_cast<double>(size) * 1.5) / (amplitude / 2.) :
				static_cast<double>(vHeights[i][j]) / (amplitude / 8.) + 2.);
		}
	}
}

const int Colours[17][3] = {
	{0x00,0x00,0xFF}, // Bleu
	// On augmente le vert :
	{0x00,0x40,0xFF}, {0x00,0x80,0xFF}, {0x00,0xBF,0xFF},
	{0x00,0xFF,0xFF},
	// On diminue le bleu :
	{0x00,0xFF,0xBF}, {0x00,0xFF,0x80}, {0x00,0xFF,0x40},
	{0x00,0xFF,0x00}, // Vert
	// On augmente le rouge :
	{0x40,0xFF,0x00}, {0x80,0xFF,0x00}, {0xBF,0xFF,0x00},
	{0xFF,0xFF,0x00}, // Jaune
	// On diminue le vert :
	{0xFF,0xFB,0x00}, {0xFF,0x80,0x00}, {0xFF,0x40,0x00},
	{0xFF,0x00,0x00} // Rouge
};
const int Colours_penta[5][3] = {
	{0x00,0x00,0xFF}, // Bleu
	{0x00,0xFF,0xFF}, // Bleu ciel
	{0x00,0xFF,0x00}, // Vert
	{0xFF,0xFF,0x00}, // Jaune
	{0xFF,0x00,0x00} // Rouge
};
const int Colours_land[10][3] = {
	{0x00,0x00,0xFF}, // Bleu
	{0x00,0xFF,0xFF}, // Bleu ciel
	{0xFF,0xFF,0x00}, // Jaune
	{0x00,0xFF,0x00}, // Vert
	{0x00,0x66,0x00}, // Vert
	//{0xA0,0x52,0x2D}, // Marron
	{0xFF,0xB2,0x66}, // Orange pastel
	{0xFF,0x99,0x33}, // Orange clair
	{0x80,0x80,0x80}, // Gris
	{0xF0,0xF0,0xF0}, // Blanc
	{0xFF,0xFF,0xFF} // Blanc
};

FIBITMAP* makeGrid(const std::size_t SIZEX, const std::size_t SIZEY, const int width, const int height);
FIBITMAP* makeGridFast(const std::size_t SIZEX, const std::size_t SIZEY, const int width, const int height);

template<typename T>
void dispLandscapeFast(HWND hDlg, T& vHeights, std::size_t SIZEX, std::size_t SIZEY)
{
	int visualisation = 0;
	if (IsDlgButtonChecked(hDlg, IDC_RADIO2))
		visualisation = 1;
	else if (IsDlgButtonChecked(hDlg, IDC_RADIO3))
		visualisation = 2;
	else if (IsDlgButtonChecked(hDlg, IDC_RADIO4))
		visualisation = 3;
	else if (IsDlgButtonChecked(hDlg, IDC_RADIO5))
		visualisation = 4;
	auto rawbitmap = std::make_unique<uint8_t[]>(SIZEX * SIZEY * 3);
	unsigned int level;
	unsigned int index = 0;
	for (auto i = 0, ip = 0; i < SIZEX; ++i) {
		for (auto j = 0, jp = 0; j < SIZEY; ++j, ++index) {
			switch (visualisation) {
			case 0:
				level = getLevel(vHeights, 256, i, j, 256);
				rawbitmap[index] = vHeights[i][j] < 0 ? level : 0;
				rawbitmap[++index] = vHeights[i][j] >= 0 ? level : 0;
				rawbitmap[++index] = 64;
				break;
			case 1:
				level = getLevelLand(vHeights, 255, i, j);
				rawbitmap[index] = Colours_land[level][2];
				rawbitmap[++index] = Colours_land[level][1];
				rawbitmap[++index] = Colours_land[level][0];
				break;
			case 2:
				level = getLevel(vHeights, 256, i, j, 256);
				rawbitmap[index] = level;
				rawbitmap[++index] = level;
				rawbitmap[++index] = level;
				break;
			case 3:
				level = getLevel(vHeights, 256, i, j, 17);
				rawbitmap[index] = Colours[level][2];
				rawbitmap[++index] = Colours[level][1];
				rawbitmap[++index] = Colours[level][0];
				break;
			case 4:
				level = getLevel(vHeights, 256, i, j, 5);
				rawbitmap[index] = Colours_penta[level][2];
				rawbitmap[++index] = Colours_penta[level][1];
				rawbitmap[++index] = Colours_penta[level][0];
				break;
			}

			// FreeImage_SetPixelColor(bitmap, i, j, &pixel);
		}
	}
	FIBITMAP* bitmap = FreeImage_ConvertFromRawBits(rawbitmap.get(), SIZEY, SIZEX, SIZEY * 3, 24, 0xFF, 0xFF, 0xFF, TRUE);
	if (bitmap) {
		RECT pictureRect;
		GetWindowRect(GetDlgItem(hDlg, IDC_PICTURE), &pictureRect);
		int ratio = std::max(SIZEX, SIZEY) / std::min(SIZEX, SIZEY);		
		HDC hDC = GetWindowDC(GetDlgItem(hDlg, IDC_PICTURE));
		SetStretchBltMode(hDC, COLORONCOLOR);
		StretchDIBits(hDC, 0, 0,
			(pictureRect.right - pictureRect.left) / (SIZEY < SIZEX ? ratio : 1),
			(pictureRect.bottom - pictureRect.top) / (SIZEY > SIZEX ? ratio : 1),
			0, 0, FreeImage_GetWidth(bitmap), FreeImage_GetHeight(bitmap),
			FreeImage_GetBits(bitmap), FreeImage_GetInfo(bitmap), DIB_RGB_COLORS, SRCCOPY);
		FreeImage_Unload(bitmap);
		if (TRUE == IsDlgButtonChecked(hDlg, IDC_CHECK1)) {
			auto grid = makeGridFast(SIZEX, SIZEY,
				(pictureRect.right - pictureRect.left) / (SIZEY < SIZEX ? ratio : 1),
				(pictureRect.bottom - pictureRect.top) / (SIZEY > SIZEX ? ratio : 1));
			StretchDIBits(hDC,
				0, 0, FreeImage_GetWidth(grid), FreeImage_GetHeight(grid),
				0, 0, FreeImage_GetWidth(grid), FreeImage_GetHeight(grid),
				FreeImage_GetBits(grid), FreeImage_GetInfo(grid), DIB_RGB_COLORS, SRCAND);
			FreeImage_Unload(grid);
		}
		ReleaseDC(hDlg, hDC);
	}
}

INT_PTR CALLBACK Exterieur(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
