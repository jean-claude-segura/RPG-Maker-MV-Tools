#pragma once
#include "Resource.h"
#include <array>
#include <memory>
#include <fstream>
#include "Includes\FreeImage.h"
#include <map>
#include <string>

static const std::map<unsigned int, unsigned int> mBordures = {
	{ 0b00000000, 0}, // None
	{ 0b00010000, 1}, // tlc
	{ 0b10000000, 2}, // trc
	{ 0b00010000 | 0b10000000, 3}, // tlc, trc
	{ 0b01000000, 4}, // brc
	{ 0b00010000 | 0b01000000, 5}, // tlc, brc
	{ 0b10000000 | 0b01000000, 6}, // trc, brc
	{ 0b10000000 | 0b00010000 | 0b01000000, 7}, // trc, tlc, brc
	{ 0b00100000, 8}, // blc
	{ 0b00010000 | 0b00100000, 9}, // tlc, blc
	{ 0b00100000 | 0b10000000, 10}, // blc, trc
	{ 0b00100000 | 0b00010000 | 0b10000000, 11}, // blc, tlc, trc
	{ 0b00100000 | 0b01000000, 12}, // blc, brc /**/
	{ 0b00100000 | 0b00010000 | 0b01000000, 13}, // blc, tlc, brc
	{ 0b00100000 | 0b10000000 | 0b01000000, 14}, // blc, trc, brc
	{ 0b00100000 | 0b10000000 | 0b00010000 | 0b01000000, 15}, // blc, trc, tlc, brc
	{ 0b00000001, 16}, // LS
	{ 0b00000001 | 0b10000000, 17}, // LS, trc
	{ 0b00000001 | 0b01000000, 18}, // LS, brc
	{ 0b00000001 | 0b10000000 | 0b01000000, 19}, // LS, trc, brc
	{ 0b00001000, 20}, // TS
	{ 0b00001000 | 0b01000000, 21}, // TS, brc
	{ 0b00001000 | 0b00100000, 22}, // TS, blc
	{ 0b00001000 | 0b00100000 | 0b01000000, 23}, // TS, blc, brc
	{ 0b00000100, 24}, // RS
	{ 0b00000100 | 0b00100000, 25}, // RS, blc
	{ 0b00000100 | 0b00010000, 26}, // RS, tlc
	{ 0b00000100 | 0b00100000 | 0b00010000, 27}, // RS, blc, tlc
	{ 0b00000010, 28}, // BS
	{ 0b00000010 | 0b00010000, 29}, // BS, tlc
	{ 0b00000010 | 0b10000000, 30}, // BS, trc
	{ 0b00000010 | 0b00010000 | 0b10000000, 31}, // BS, tlc, trc
	{ 0b00000100 | 0b00000001, 32}, // RS, LS
	{ 0b00001000 | 0b00000010, 33}, // TS, BS
	{ 0b00001000 | 0b00000001, 34}, // TS, LS
	{ 0b00001000 | 0b00000001 | 0b01000000, 35}, // TS, LS, brc /**/
	{ 0b00001000 | 0b00000100, 36}, // TS, RS
	{ 0b00001000 | 0b00000100 | 0b00100000, 37}, // TS, RS, blc
	{ 0b00000010 | 0b00000100, 38}, // BS, RS
	{ 0b00000010 | 0b00000100 | 0b00010000, 39}, // BS, RS, tlc
	{ 0b00000001 | 0b00000010, 40}, // LS, BS
	{ 0b00000001 | 0b00000010 | 0b10000000, 41}, // LS, BS, trc
	{ 0b00000001 | 0b00000100 | 0b00001000, 42}, // LS, RS, TS
	{ 0b00000010 | 0b00000001 | 0b00001000, 43}, // BS, LS, TS
	{ 0b00000001 | 0b00000100 | 0b00000010, 44}, // LS, RS, BS
	{ 0b00001000 | 0b00000010 | 0b00000100, 45}, // TS, BS, RS
	{ 0b00000100 | 0b00000001 | 0b00001000 | 0b00000010, 46}, // RS, LS, TS, BS
	//{ 0b00000100 | 0b00000001 | 0b00001000 | 0b00000010,47} //  RS, LS, TS, BS
};

template<std::size_t SIZEY, std::size_t SIZEX, typename T>
void getBordures(std::array<std::array<T, SIZEY>, SIZEX>& vLevels, std::array<std::array<unsigned int, SIZEY>, SIZEX>& vBordures)
{
	for (std::size_t i = 0; i < SIZEX; ++i) {
		for (std::size_t j = 0; j < SIZEY; ++j) {
			vBordures[i][j] = 0;
			if (vLevels[i][j]) {
				unsigned int bordure = 0;
				// Bords :
				if (j < SIZEY - 1 && vLevels[i][j + 1] != vLevels[i][j])
					bordure |= 0b00000100; // Droite
				if (i < SIZEX - 1 && vLevels[i + 1][j] != vLevels[i][j])
					bordure |= 0b00000010; // Bas
				if (j > 0 && vLevels[i][j - 1] != vLevels[i][j])
					bordure |= 0b00000001; // Gauche
				if (i > 0 && vLevels[i - 1][j] != vLevels[i][j])
					bordure |= 0b00001000; // Haut
				// Coins :
				if (i < SIZEX - 1 && j < SIZEY - 1 &&
					vLevels[i + 1][j + 1] != vLevels[i][j] &&
					vLevels[i][j + 1] == vLevels[i][j] &&
					vLevels[i + 1][j] == vLevels[i][j]
					)
					bordure |= 0b01000000; // Bas - Droite
				if (i < SIZEX - 1 && j > 0 &&
					vLevels[i + 1][j - 1] != vLevels[i][j] &&
					vLevels[i][j - 1] == vLevels[i][j] &&
					vLevels[i + 1][j] == vLevels[i][j]
					)
					bordure |= 0b00100000; // Bas - Gauche
				if (i > 0 && j > 0 &&
					vLevels[i - 1][j - 1] != vLevels[i][j] &&
					vLevels[i][j - 1] == vLevels[i][j] &&
					vLevels[i - 1][j] == vLevels[i][j]
					)
					bordure |= 0b00010000; // Haut - Gauche
				if (i > 0 && j < SIZEY - 1 &&
					vLevels[i - 1][j + 1] != vLevels[i][j] &&
					vLevels[i][j + 1] == vLevels[i][j] &&
					vLevels[i - 1][j] == vLevels[i][j]
					)
					bordure |= 0b10000000; // Haut - Droite
				vBordures[i][j] = mBordures.find(bordure)->second;
			}
		}
	}
}

template<typename T>
unsigned int getLevel(T& vHeights, unsigned int size, int i, int j, signed int levels)
{
	auto amplitude = size * 2 * 1.5;
	auto value = vHeights[i][j];
	auto level = (value + size * 1.5) / (amplitude / levels);
	return level;
}

template<typename T>
unsigned int getLevelLand(T& vHeights, unsigned int size, int i, int j)
{
	unsigned int level;
	auto value = vHeights[i][j];
	auto amplitude = size * 1.5;
	if (value < 0)
		level = (value + size * 1.5) / (amplitude / 2);
	else
		level = value / (amplitude / 8) + 2;
	return level;
}

template<typename T, typename U>
void getLevelLand(T& vHeights, U& vLevels, unsigned int size, int width, int heigth)
{
	auto amplitude = size * 1.5;
	for (auto i = 0; i < width; ++i) {
		for (auto j = 0; j < heigth; ++j) {
			vLevels[i][j] = vHeights[i][j] < 0 ?
				(vHeights[i][j] + size * 1.5) / (amplitude / 2) :
				vHeights[i][j] / (amplitude / 8) + 2;
		}
	}
}

/*
	Pour les contacts de même nature eau profonde/eau, herbe dense/herbe :
	-la case de base doit être l'index de base;
	-la profondeur est reportée sur la deuxième couche en incrémentant la valeur de la case de la valeur du type
	de contact.
	Pour les contacts de nature différente eau/herbe :
	-la case de base est incrémentée de la valeur du type de contact sur la première couche.
	Pour les contacts de nature différente eau profonde/herbe :
	-la case de base est incrémentée de la valeur du type de contact sur la première couche;
	-la case de base + 48 est incrémentée de la valeur du type de contact sur la deuxième couche.

	// Première couche :
	std::array<unsigned int, 9> tiles = {
		2048, // Eau + 48
		2816, // Herbe + 48
		3200, // Terre sèche + 48
		3584, // Sable + 48
		3968, // Neige + 48
		2912, // Herbe dense + 48
		3296, // Sable humide + 48
		3680, // Roche + 48
		4064 // Neige + 48
	};
*/
template<typename T, typename U>
void getLayers(T& vHeights, U& arrLayers, unsigned int size, int width, int heigth)
{
	// Affectation des tiles par niveau :
	std::array < std::array<unsigned int, 10>, 2> Layers = {
		2048, 2048, 3584, 2816, 2816,3200, 3200, 3680, 3968, 3968,
		2048 + 48, 0, 0, 0, 2816 + 48, 0, 3200 + 48, 0, 0, 3968 + 48 };
	auto amplitude = size * 1.5;
	for (auto i = 0; i < width; ++i) {
		for (auto j = 0; j < heigth; ++j) {
			auto level = vHeights[i][j] < 0 ?
				(vHeights[i][j] + size * 1.5) / (amplitude / 2) :
				vHeights[i][j] / (amplitude / 8) + 2;
			arrLayers[0][i][j] = Layers[0][level];
			arrLayers[1][i][j] = Layers[1][level];
		}
	}
}

template<std::size_t SIZEY, std::size_t SIZEX>
void doRPGMakerMVMap(std::array<std::array<signed int, SIZEY>, SIZEX>& arrHeights)
{
	auto arrLayers = std::make_unique<std::array<std::array<unsigned int, SIZEY>, SIZEX>[]>(6);
	getLayers(arrHeights, arrLayers, 256, SIZEX, SIZEY);
	// Calcul des bordures :
	auto vBordures = std::make_unique< std::array<std::array<unsigned int, SIZEY>, SIZEX>[]>(2);
	getBordures(arrLayers[0], vBordures[0]);
	getBordures(arrLayers[1], vBordures[1]);
	std::ofstream file("MapXXX.json", std::fstream::binary);
	//std::ofstream file("C:\\Users\\Jean-Claude\\Documents\\Games\\Project1\\data\\Map021.json", std::fstream::binary);
	if (file.is_open()) {
		std::string outstr = "{\n\"autoplayBgm\":false,\"autoplayBgs\":false,\"battleback1Name\":\"\",\"battleback2Name\":\"\",\"bgm\":{\"name\":\"\",\"pan\":0,\"pitch\":100,\"volume\":90},\"bgs\":{\"name\":\"\",\"pan\":0,\"pitch\":100,\"volume\":90},\"disableDashing\":false,\"displayName\":\"\",\"encounterList\":[],\"encounterStep\":30,\"height\":256,\"note\":\"\",\"parallaxLoopX\":false,\"parallaxLoopY\":false,\"parallaxName\":\"\",\"parallaxShow\":true,\"parallaxSx\":0,\"parallaxSy\":0,\"scrollType\":0,\"specifyBattleback\":false,\"tilesetId\":1,\"width\":256,\n\"data\":[";
		bool virgule = false;
		for (auto i = 0; i < min(256, SIZEX); ++i) {
			for (auto j = 0; j < min(256, SIZEY); ++j) {
				outstr += (virgule ? std::string(",") : std::string(""));
				outstr += arrLayers[0][i][j] ? std::to_string(arrLayers[0][i][j] + vBordures[0][i][j]) : "0";
				virgule = true;
			}
		}
		// Deuxième couche : océan profond, herbe dense, ...
		// Case de base + 48.
		for (auto i = 0; i < min(256, SIZEX); ++i) {
			for (auto j = 0; j < min(256, SIZEY); ++j) {
				outstr += (virgule ? std::string(",") : std::string(""));
				outstr += arrLayers[1][i][j] ? std::to_string(arrLayers[1][i][j] + vBordures[1][i][j]) : "0";
			}
		}
		outstr += "],\n\"events\":[\n]\n}";
		//file.write(outstr.c_str(), outstr.length());
		file << outstr;
	}
}

template <typename T>
T random_between(T _min, T _max)
{
	const T diff = _max - _min;
	T answer = rand();
	answer %= diff;
	answer += _min;
	return answer;
}

template<typename T>
// Carré de la forme 2^n + 1
void CreateLandscape(T& vHeights, unsigned int size)
{
	signed int h = size;// vHeights.size();//t.coté()
	/* initialisation des coins */
	vHeights[0][0] = random_between(-h, h);
	vHeights[0][h - 1] = random_between(-h, h);
	vHeights[h - 1][h - 1] = random_between(-h, h);
	vHeights[h - 1][0] = random_between(-h, h);
	signed int i = h - 1;
	while (i > 1) {
		signed int id = i / 2;
		// Diamant : le centre de chaque carré prend pour valeur la moyenne des 4 coins
		// à laquelle on ajoute une valeur aléatoire.
		for (signed int x = id; x < h; x += i) {  /* début de la phase du diamant */
			for (signed int y = id; y < h; y += i) {
				signed int moyenne = (vHeights[x - id][y - id] + vHeights[x - id][y + id] + vHeights[x + id][y + id] + vHeights[x + id][y - id]) / 4;
				vHeights[x][y] = moyenne + random_between(-id, id);
			}
		}
		signed int décalage = 0;
		// Carré : chaque milieu des segments du carré, prend pour valeur la moyenne des 4 points
		// formant ce diamant ajoutée d'une valeur aléatoire.
		for (signed int x = 0; x < h; x += id) {  /* début de la phase du carré */
			if (décalage == 0) {
				décalage = id;
			}
			else {
				décalage = 0;
			}
			for (signed int y = décalage; y < h; y += i) {
				signed int somme = 0;
				signed int n = 0;
				if (x >= id) {
					somme = somme + vHeights[x - id][y];
					n = n + 1;
				}
				if (x + id < h) {
					somme = somme + vHeights[x + id][y];
					n = n + 1;
				}
				if (y >= id) {
					somme = somme + vHeights[x][y - id];
					n = n + 1;
				}
				if (y + id < h) {
					somme = somme + vHeights[x][y + id];
					n = n + 1;
				}
				vHeights[x][y] = somme / n + random_between(-id, id);
			}
		}
		i = id;
	}
}

template <typename T>
std::size_t newSide(T left, T right)
{
	auto maxsize = max(left, right);
	int exp = log2(maxsize);
	auto newsize = exp2(exp2(exp) + 1 >= maxsize ? exp : exp + 1) + 1;
	return newsize;
}

template<std::size_t SIZEY, std::size_t SIZEX>
// Carré de la forme 2^n + 1
void CreateLandscape(std::array<std::array<signed int, SIZEY>, SIZEX>& arrHeights)
{
	auto side = newSide(SIZEY, SIZEX);
	auto vHeights = std::make_unique <std::unique_ptr<int[]>[]>(side);
	for (int i = 0; i < side; ++i) {
		vHeights.get()[i] = std::make_unique<int[]>(side);
	}
	CreateLandscape(vHeights, side);
	for (int i = 0; i < SIZEX; ++i) {
		for (int j = 0; j < SIZEY; ++j) {
			arrHeights[i][j] = vHeights[i][j];
		}
	}
}

template<std::size_t SIZEY, std::size_t SIZEX>
void dispLandscape(HWND hDlg, std::array<std::array<signed int, SIZEY>, SIZEX>& vHeights)
{
	//FIBITMAP* bitmap = FreeImage_Allocate(256, 256, 24);
	FIBITMAP* bitmap = FreeImage_Allocate(vHeights.size() * 3, vHeights[0].size() * 3, 24);
	if (bitmap) {
		int Colours[17][3] = {
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
		int Colours_penta[5][3] = {
			{0x00,0x00,0xFF}, // Bleu
			{0x00,0xFF,0xFF}, // Bleu ciel
			{0x00,0xFF,0x00}, // Vert
			{0xFF,0xFF,0x00}, // Jaune
			{0xFF,0x00,0x00} // Rouge
		};
		int Colours_land[10][3] = {
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
		/**/
		auto vLevels = std::make_unique<std::array<unsigned int, SIZEY>[]>(SIZEX);
		/**/
		int visualisation = 0;
		if (IsDlgButtonChecked(hDlg, IDC_RADIO2)) {
			visualisation = 1;
			getLevelLand(vHeights, vLevels, 256, SIZEX, SIZEY);
		}
		else if (IsDlgButtonChecked(hDlg, IDC_RADIO3))
			visualisation = 2;
		else if (IsDlgButtonChecked(hDlg, IDC_RADIO4))
			visualisation = 3;
		else if (IsDlgButtonChecked(hDlg, IDC_RADIO5))
			visualisation = 4;
		for (auto i = 0, ip = 0; i < vHeights.size(); ++i, ip += 3) {
			for (auto j = 0, jp = 0; j < vHeights[i].size(); ++j, jp += 3) {
				RGBQUAD pixel;
				unsigned int level;
				switch (visualisation) {
				case 0:
					level = getLevel(vHeights, 256, i, j, 255);
					pixel.rgbRed = 64;
					pixel.rgbGreen = vHeights[i][j] >= 0 ? level : 0;
					pixel.rgbBlue = vHeights[i][j] < 0 ? level : 0;
					break;
				case 1:
					//level = getLevelLand(vHeights, 256,  i, j);
					level = vLevels[i][j];
					pixel.rgbRed = Colours_land[level][0];
					pixel.rgbGreen = Colours_land[level][1];
					pixel.rgbBlue = Colours_land[level][2];
					break;
				case 2:
					level = getLevel(vHeights, 256, i, j, 255);
					pixel.rgbRed = level;
					pixel.rgbGreen = level;
					pixel.rgbBlue = level;
					break;
				case 3:
					level = getLevel(vHeights, 256, i, j, 17);
					pixel.rgbRed = Colours[level][0];
					pixel.rgbGreen = Colours[level][1];
					pixel.rgbBlue = Colours[level][2];
					break;
				case 4:
					level = getLevel(vHeights, 256, i, j, 5);
					pixel.rgbRed = Colours_penta[level][0];
					pixel.rgbGreen = Colours_penta[level][1];
					pixel.rgbBlue = Colours_penta[level][2];
					break;
				}

				FreeImage_SetPixelColor(bitmap, ip, jp, &pixel);
				FreeImage_SetPixelColor(bitmap, ip, jp + 1, &pixel);
				FreeImage_SetPixelColor(bitmap, ip + 1, jp, &pixel);
				FreeImage_SetPixelColor(bitmap, ip + 1, jp + 1, &pixel);

				if (TRUE == IsDlgButtonChecked(hDlg, IDC_CHECK1)) {
					RGBQUAD pxzero;
					pixel.rgbRed = 0;
					pixel.rgbGreen = 0;
					pixel.rgbBlue = 0;
					FreeImage_SetPixelColor(bitmap, ip + 2, jp, &pxzero);
					FreeImage_SetPixelColor(bitmap, ip + 2, jp + 1, &pxzero);
					FreeImage_SetPixelColor(bitmap, ip + 2, jp + 2, &pxzero);
					FreeImage_SetPixelColor(bitmap, ip + 1, jp + 2, &pxzero);
					FreeImage_SetPixelColor(bitmap, ip, jp + 2, &pxzero);
				}
				else {
					FreeImage_SetPixelColor(bitmap, ip + 2, jp, &pixel);
					FreeImage_SetPixelColor(bitmap, ip + 2, jp + 1, &pixel);
					FreeImage_SetPixelColor(bitmap, ip + 2, jp + 2, &pixel);
					FreeImage_SetPixelColor(bitmap, ip + 1, jp + 2, &pixel);
					FreeImage_SetPixelColor(bitmap, ip, jp + 2, &pixel);
				}
			}

		}
		//bitmap = FreeImage_Rotate(bitmap, -90);
		FIBITMAP* bitmap2 = FreeImage_Allocate(FreeImage_GetWidth(bitmap), FreeImage_GetHeight(bitmap), 24);
		//bitmap2 = FreeImage_Rescale(bitmap, 256 * 3, 256 * 3);
		bitmap2 = FreeImage_Rotate(bitmap, -90);
		FreeImage_Unload(bitmap);
		//FreeImage_Save(FIF_PNG, bitmap, "map.png", 0);
		FreeImage_Save(FIF_PNG, bitmap2, "map.png", 0);

		HDC hDC = GetWindowDC(GetDlgItem(hDlg, IDC_PICTURE));
		SetStretchBltMode(hDC, COLORONCOLOR);
		StretchDIBits(hDC, 0/*rcDest.left*/, 0/*rcDest.top*/,

			256 * 3/*rcDest.right - rcDest.left*/, 256 * 3/*rcDest.bottom - rcDest.top*/,
			0, 0, FreeImage_GetWidth(bitmap2), FreeImage_GetHeight(bitmap2),
			FreeImage_GetBits(bitmap2), FreeImage_GetInfo(bitmap2), DIB_RGB_COLORS, SRCCOPY);
		FreeImage_Unload(bitmap2);
		ReleaseDC(hDlg, hDC);
	}
}

INT_PTR CALLBACK Exterieur(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
