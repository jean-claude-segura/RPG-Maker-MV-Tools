#pragma once

#include <map>
#include <fstream>
#include <memory>
#include <string>
#include <array>

class MakerMVMap {
protected:
	unsigned int type; // 0 : aucun, 1 : vertical, 2 : horizontal, 3 : les deux.
	unsigned int tilesetid; // 1 : zone commune, 2 : extérieur, 3 : intérieur, 4: oubliettes, 5 : SF extérieur, 6 : SF intérieur, 7 : caverne
	std::array<std::unique_ptr <std::unique_ptr<unsigned int[]>[]>, 6> MakerMVLayers;
	std::array<std::unique_ptr <std::unique_ptr<unsigned int[]>[]>, 2> MakerMVBordures;
	std::size_t sizeX, sizeY;
public:
	MakerMVMap(std::size_t, std::size_t);
	std::size_t Width();
	std::size_t Heigth();
	virtual void setLayers() = 0;
	virtual void setBorders() = 0;
	void setType(unsigned int);
	unsigned int getType();
	void doRPGMakerMVMap();
};

class MakerMVMapCommon : public MakerMVMap {
private:
	static const std::map<unsigned int, unsigned int> mBordures;
	std::unique_ptr <std::unique_ptr<int[]>[]> MakerMVHeigth;
	// Affectation des tiles par niveau :
	const std::array < std::array<unsigned int, 10>, 2> Layers = {
		2048, 2048, 3584, 2816, 2816,3200, 3200, 3680, 3968, 3968,
		2048 + 48, 0, 0, 0, 2816 + 48, 0, 3200 + 48, 0, 0, 3968 + 48 };
public:
	MakerMVMapCommon(std::size_t x, std::size_t y);
	std::unique_ptr <std::unique_ptr<int[]>[]>& getMakerMVHeigth();
	void setBorders();
	void setLayers();
};

class MakerMVMapInside : public MakerMVMap {
	MakerMVMapInside(std::size_t x, std::size_t y) : MakerMVMap(x, y) {}
	void setLayers(){}
	void setBorders(){}
};

class MakerMVMapOutside : public MakerMVMap {
	MakerMVMapOutside(std::size_t x, std::size_t y) : MakerMVMap(x, y) {}
	void setLayers() {}
	void setBorders() {}
};
