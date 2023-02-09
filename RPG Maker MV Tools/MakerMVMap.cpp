#include "MakerMVMap.h"

MakerMVMap::MakerMVMap(std::size_t x, std::size_t y) {
	for (int j = 0; j < MakerMVLayers.size(); ++j) {
		MakerMVLayers[j] = std::make_unique< std::unique_ptr<unsigned int[]>[]>(x);
		for (int i = 0; i < x; ++i)
			MakerMVLayers[j][i] = std::make_unique< unsigned int[]>(y);
	}
	for (int j = 0; j < 2; ++j) {
		MakerMVBordures[j] = std::make_unique< std::unique_ptr<unsigned int[]>[]>(x);
		for (int i = 0; i < x; ++i)
			MakerMVBordures[j][i] = std::make_unique< unsigned int[]>(y);
	}
	type = 0;
	sizeX = x; sizeY = y;
}

std::size_t MakerMVMap::Width() { return sizeX; }
std::size_t MakerMVMap::Heigth() { return sizeY; }

void MakerMVMap::setType(unsigned int _type) {
	type = _type > 3 ? 0 : _type;
}
unsigned int MakerMVMap::getType() {
	return type;
}

void MakerMVMap::doRPGMakerMVMap() {
	setLayers();
	setBorders();
	//std::ofstream file("MapXXX.json", std::fstream::binary);
	std::ofstream file("C:\\Users\\Jean-Claude\\Documents\\Games\\Project1\\data\\Map021.json", std::fstream::binary);
	if (file.is_open()) {
		std::string outstr = "{\n\"autoplayBgm\":false,\"autoplayBgs\":false,\"battleback1Name\":\"\",\"battleback2Name\":\"\",\"bgm\":{\"name\":\"\",\"pan\":0,\"pitch\":100,\"volume\":90},\"bgs\":{\"name\":\"\",\"pan\":0,\"pitch\":100,\"volume\":90},\"disableDashing\":false,\"displayName\":\"\",\"encounterList\":[],\"encounterStep\":30,\"height\":";
		outstr += std::to_string(sizeX);
		outstr += ",\"note\":\"\",\"parallaxLoopX\":false,\"parallaxLoopY\":false,\"parallaxName\":\"\",\"parallaxShow\":true,\"parallaxSx\":0,\"parallaxSy\":0,\"scrollType\":";
		outstr += std::to_string(type); // 0 : aucun, 1 : vertical, 2 : horizontal, 3 : les deux.
		outstr += ",\"specifyBattleback\":false,\"tilesetId\":";
		outstr += std::to_string(1);
		outstr += ",\"width\":";
		outstr += std::to_string(sizeY);
		outstr += ",\n\"data\":[";
		bool virgule = false;
		for (auto i = 0; i < sizeX; ++i) {
			for (auto j = 0; j < sizeY; ++j) {
				outstr += (virgule ? std::string(",") : std::string(""));
				outstr += MakerMVLayers[0][i][j] ? std::to_string(MakerMVLayers[0][i][j] + MakerMVBordures[0][i][j]) : "0";
				virgule = true;
			}
		}
		// Deuxième couche : océan profond, herbe dense, ...
		// Case de base + 48.
		for (auto i = 0; i < sizeX; ++i) {
			for (auto j = 0; j < sizeY; ++j) {
				outstr += (virgule ? std::string(",") : std::string(""));
				outstr += MakerMVLayers[1][i][j] ? std::to_string(MakerMVLayers[1][i][j] + MakerMVBordures[1][i][j]) : "0";
			}
		}
		outstr += "],\n\"events\":[\n]\n}";
		//file.write(outstr.c_str(), outstr.length());
		file << outstr;
	}
}

const std::map<unsigned int, unsigned int> MakerMVMapCommon::mBordures = {
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

MakerMVMapCommon::MakerMVMapCommon(std::size_t x, std::size_t y) : MakerMVMap(x, y) {
	MakerMVHeigth = std::make_unique< std::unique_ptr<int[]>[]>(x);
	for (int i = 0; i < x; ++i)
		MakerMVHeigth.get()[i] = std::make_unique< int[]>(y);
}
std::unique_ptr <std::unique_ptr<int[]>[]>& MakerMVMapCommon::getMakerMVHeigth() {
	return MakerMVHeigth;
}

void MakerMVMapCommon::setBorders()
{
	for (std::size_t layer = 0; layer < 2; ++layer) {
		for (std::size_t i = 0; i < sizeX; ++i) {
			for (std::size_t j = 0; j < sizeY; ++j) {
				MakerMVBordures[layer][i][j] = 0;
				if (MakerMVLayers[layer][i][j]) {
					unsigned int bordure = 0;
					// Bords :
					if (j < sizeY - 1 && MakerMVLayers[layer][i][j + 1] != MakerMVLayers[layer][i][j])
						bordure |= 0b00000100; // Droite
					if (i < sizeX - 1 && MakerMVLayers[layer][i + 1][j] != MakerMVLayers[layer][i][j])
						bordure |= 0b00000010; // Bas
					if (j > 0 && MakerMVLayers[layer][i][j - 1] != MakerMVLayers[layer][i][j])
						bordure |= 0b00000001; // Gauche
					if (i > 0 && MakerMVLayers[layer][i - 1][j] != MakerMVLayers[layer][i][j])
						bordure |= 0b00001000; // Haut
					// Coins :
					if (i < sizeX - 1 && j < sizeY - 1 &&
						MakerMVLayers[layer][i + 1][j + 1] != MakerMVLayers[layer][i][j] &&
						MakerMVLayers[layer][i][j + 1] == MakerMVLayers[layer][i][j] &&
						MakerMVLayers[layer][i + 1][j] == MakerMVLayers[layer][i][j]
						)
						bordure |= 0b01000000; // Bas - Droite
					if (i < sizeX - 1 && j > 0 &&
						MakerMVLayers[layer][i + 1][j - 1] != MakerMVLayers[layer][i][j] &&
						MakerMVLayers[layer][i][j - 1] == MakerMVLayers[layer][i][j] &&
						MakerMVLayers[layer][i + 1][j] == MakerMVLayers[layer][i][j]
						)
						bordure |= 0b00100000; // Bas - Gauche
					if (i > 0 && j > 0 &&
						MakerMVLayers[layer][i - 1][j - 1] != MakerMVLayers[layer][i][j] &&
						MakerMVLayers[layer][i][j - 1] == MakerMVLayers[layer][i][j] &&
						MakerMVLayers[layer][i - 1][j] == MakerMVLayers[layer][i][j]
						)
						bordure |= 0b00010000; // Haut - Gauche
					if (i > 0 && j < sizeY - 1 &&
						MakerMVLayers[layer][i - 1][j + 1] != MakerMVLayers[layer][i][j] &&
						MakerMVLayers[layer][i][j + 1] == MakerMVLayers[layer][i][j] &&
						MakerMVLayers[layer][i - 1][j] == MakerMVLayers[layer][i][j]
						)
						bordure |= 0b10000000; // Haut - Droite
					MakerMVBordures[layer][i][j] = mBordures.find(bordure)->second;
				}
			}
		}
		// Calcul des bordures pour les cartes continues :
		if (type) {
			std::map<unsigned int, unsigned int> mReverseBorders;
			for (const auto& obj : mBordures) {
				mReverseBorders[obj.second] = obj.first;
			}
			// continuité horizontale :
			if (type & 2) {
				for (std::size_t i = 0; i < sizeX; ++i) {
					if (MakerMVLayers[layer][i][sizeY - 1]) {
						unsigned int bordure = mReverseBorders.find(MakerMVBordures[layer][i][sizeY - 1])->second;
						// Bords :
						if (MakerMVLayers[layer][i][0] != MakerMVLayers[layer][i][sizeY - 1])
							bordure |= 0b00000100; // Droite
						// coins
						if (i < sizeX - 1 &&
							MakerMVLayers[layer][i + 1][0] != MakerMVLayers[layer][i][sizeY - 1] &&
							MakerMVLayers[layer][i][0] == MakerMVLayers[layer][i][sizeY - 1] &&
							MakerMVLayers[layer][i + 1][sizeY - 1] == MakerMVLayers[layer][i][sizeY - 1]
							)
							bordure |= 0b01000000; // Bas - Droite
						if (i > 0 &&
							MakerMVLayers[layer][i - 1][0] != MakerMVLayers[layer][i][sizeY - 1] &&
							MakerMVLayers[layer][i][0] == MakerMVLayers[layer][i][sizeY - 1] &&
							MakerMVLayers[layer][i - 1][sizeY - 1] == MakerMVLayers[layer][i][sizeY - 1]
							)
							bordure |= 0b10000000; // Haut - Droite
						MakerMVBordures[layer][i][sizeY - 1] = mBordures.find(bordure)->second;
					}
					if (MakerMVLayers[layer][i][0]) {
						unsigned int bordure = mReverseBorders.find(MakerMVBordures[layer][i][0])->second;
						if (MakerMVLayers[layer][i][sizeY - 1] != MakerMVLayers[layer][i][0])
							bordure |= 0b00000001; // Gauche
						if (i < sizeX - 1 &&
							MakerMVLayers[layer][i + 1][sizeY - 1] != MakerMVLayers[layer][i][0] &&
							MakerMVLayers[layer][i][sizeY - 1] == MakerMVLayers[layer][i][0] &&
							MakerMVLayers[layer][i + 1][0] == MakerMVLayers[layer][i][0]
							)
							bordure |= 0b00100000; // Bas - Gauche
						if (i > 0 &&
							MakerMVLayers[layer][i - 1][sizeY - 1] != MakerMVLayers[layer][i][0] &&
							MakerMVLayers[layer][i][sizeY - 1] == MakerMVLayers[layer][i][0] &&
							MakerMVLayers[layer][i - 1][0] == MakerMVLayers[layer][i][0]
							)
							bordure |= 0b00010000; // Haut - Gauche
						MakerMVBordures[layer][i][0] = mBordures.find(bordure)->second;
					}
				}
			}
			// continuité verticale :
			if (type & 1) {
				for (std::size_t j = 0; j < sizeY; ++j) {
					if (MakerMVLayers[layer][sizeX - 1][j]) {
						unsigned int bordure = mReverseBorders.find(MakerMVBordures[layer][sizeX - 1][j])->second;
						// Bords :
						if (MakerMVLayers[layer][0][j] != MakerMVLayers[layer][sizeX - 1][j])
							bordure |= 0b00000010; // Bas
						// Coins :
						if (j < sizeY - 1 &&
							MakerMVLayers[layer][0][j + 1] != MakerMVLayers[layer][sizeX - 1][j] &&
							MakerMVLayers[layer][sizeX - 1][j + 1] == MakerMVLayers[layer][sizeX - 1][j] &&
							MakerMVLayers[layer][0][j] == MakerMVLayers[layer][sizeX - 1][j]
							)
							bordure |= 0b01000000; // Bas - Droite
						if (j > 0 &&
							MakerMVLayers[layer][0][j - 1] != MakerMVLayers[layer][sizeX - 1][j] &&
							MakerMVLayers[layer][sizeX - 1][j - 1] == MakerMVLayers[layer][sizeX - 1][j] &&
							MakerMVLayers[layer][0][j] == MakerMVLayers[layer][sizeX - 1][j]
							)
							bordure |= 0b00100000; // Bas - Gauche
						MakerMVBordures[layer][sizeX - 1][j] = mBordures.find(bordure)->second;
					}
					if (MakerMVLayers[layer][0][j]) {
						unsigned int bordure = mReverseBorders.find(MakerMVBordures[layer][0][j])->second;
						// Bords :
						if (MakerMVLayers[layer][sizeX - 1][j] != MakerMVLayers[layer][0][j])
							bordure |= 0b00001000; // Haut
						// Coins :
						if (j > 0 &&
							MakerMVLayers[layer][sizeX - 1][j - 1] != MakerMVLayers[layer][0][j] &&
							MakerMVLayers[layer][0][j - 1] == MakerMVLayers[layer][0][j] &&
							MakerMVLayers[layer][sizeX - 1][j] == MakerMVLayers[layer][0][j]
							)
							bordure |= 0b00010000; // Haut - Gauche
						if (j < sizeY - 1 &&
							MakerMVLayers[layer][sizeX - 1][j + 1] != MakerMVLayers[layer][0][j] &&
							MakerMVLayers[layer][0][j + 1] == MakerMVLayers[layer][0][j] &&
							MakerMVLayers[layer][sizeX - 1][j] == MakerMVLayers[layer][0][j]
							)
							bordure |= 0b10000000; // Haut - Droite
						MakerMVBordures[layer][0][j] = mBordures.find(bordure)->second;
					}
				}
			}
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
void MakerMVMapCommon::setLayers()
{
	auto size = 512;
	std::size_t level;
	for (auto i = 0; i < sizeX; ++i) {
		for (auto j = 0; j < sizeY; ++j) {
			auto value = static_cast<double>(MakerMVHeigth[i][j]);
			auto amplitude = static_cast<double>(size);
			/*if (value < 0) {
				// -255 <= Value < 0
				// 0 <= value + 255 < 255
				// 0 <= (value + 255) / 255 < 1
				// 0 <= 2. * (value + 255) / 255 < 2
				level = 2. * (value + 255.) / 255.;
			}
			else if (value > 0) {
				// 0 < Value <= 512. - 255.
				// 0 < value / (512. - 255.) <= 1
				// 0 < 7 * value / (512. - 255.) <= 7
				level = 2. + 8. * (value - 1) / (512. - 255.);
			}*/
			level = static_cast<std::size_t>(value < 0 ?
				2. * (value + 255.) / 255.1 :
				2 + 8. * value / 257.1);
			MakerMVLayers[0][i][j] = Layers[0][level];
			MakerMVLayers[1][i][j] = Layers[1][level];
		}
	}

	/*auto size = 255;
	auto amplitude = size * 1.5;
	for (auto i = 0; i < sizeX; ++i) {
		for (auto j = 0; j < sizeY; ++j) {
			std::size_t level = static_cast<std::size_t>(MakerMVHeigth[i][j] < 0 ?
				(MakerMVHeigth[i][j] + size * 1.5) / (amplitude / 2) :
				MakerMVHeigth[i][j] / (amplitude / 8) + 2);
			MakerMVLayers[0][i][j] = Layers[0][level];
			MakerMVLayers[1][i][j] = Layers[1][level];
		}
	}*/
}
