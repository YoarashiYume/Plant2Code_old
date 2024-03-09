#pragma once
#include <string>
#include <vector>
struct UsedAlgorithm
{
	std::string name;
	std::string changedName;
	std::string extraData;
};
struct LatexAlgorithmData : public UsedAlgorithm
{
	std::vector<std::string> sizeKoef;
	std::vector<std::string> pdfSize;//split with ;
	std::vector<std::uint8_t> isLandscape;
	std::vector<UsedAlgorithm> usedAlgorithm;
};
struct LatexSectoinData : public UsedAlgorithm
{
	std::vector<LatexAlgorithmData> sections;
};
struct LatexTableData
{
	std::string name;
	std::string path;
	std::string ref;
	bool isLandscape;
	std::vector<std::string> size;
};

struct LatexPersonData
{
	std::string name;
	std::string position;

};
struct LatexPersonDataExpanded : LatexPersonData
{
	std::string workPlace;
};

struct LatexTitleData
{
	std::vector<LatexPersonData> firstLine;
	std::vector<LatexPersonData> secondLine;
	LatexPersonDataExpanded military;
	LatexPersonDataExpanded approve;
};

struct LatexDataMain
{
	std::string documentName;
	std::string fullProgramName;
	std::string documentNumber;

	std::uint16_t documentVersion;
	std::uint16_t objectCode;

	std::string programShort;

	bool needMilitary;

	LatexTitleData title;
};