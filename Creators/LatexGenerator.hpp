#pragma once

#include <unordered_map>
#include <functional>

#include "../Common/FileReading.hpp"

#include "../Common/Loggable.hpp"
#include "../Struct/Algorithm/AlgorithmStruct.hpp"
#include "../Struct/Latex/LatexStruct.h"


class LatexGenerator : public Loggable
{
public:
	using algorithm_data_type = AlgorithmInfo;
	using store_algorithm_data_type = std::shared_ptr<algorithm_data_type>;
	using storage_type = std::shared_ptr<std::unordered_map<std::string, store_algorithm_data_type>>;

	using function_converter_type = std::function<std::string(const std::string&)>;
	using stream_type = FileReading::FileWriter;
	

private:
	storage_type algorithms;
	std::string outputPath;

	bool isGlossary{ false };

	function_converter_type nameConverer;
	std::string actor;	//TODO:  заполнение
	std::string onna;	//TODO:  заполнение

	bool genImageSection(stream_type& stream, const LatexAlgorithmData& algorithmInfo);
	std::string useParagRef(const UsedAlgorithm data);
	void createUsedAlgoList(LatexGenerator::stream_type & stream, std::vector<UsedAlgorithm>);
	bool genAlgorithmSection(stream_type& stream, const LatexAlgorithmData& algorithmInfo);
	void genInitData(stream_type& stream);

public:
	LatexGenerator();
	~LatexGenerator();

	void setOutputPath(const std::string path);
	std::string getTablePathFolder()const ;

	std::pair<std::uint32_t, std::uint32_t> getWorkListSize() const;

	void setAlgorithm(storage_type algorithm);
	void setNameConverter(const function_converter_type converter);
	std::vector<LatexAlgorithmData> genLatexAlgorithmData(std::vector<std::string> order);
	std::string formTableLine(std::string line, const LatexTableData & tableInfo = {}) const;
	std::string genUE(const LatexDataMain& titleData);
	void genUEPerson(stream_type& stream, const LatexTitleData& title, bool needMilitary);
	std::vector<std::string> genRefsArray(std::vector<PUML_SIGNAL_TYPE>& types, std::vector<LatexTableData>& tableInfo, PUML_SIGNAL_TYPE type);
	PUML_SIGNAL_TYPE genTable(LatexTableData& tableInfo);
	std::string createTableInclude(std::vector<PUML_SIGNAL_TYPE>& types, std::vector<LatexTableData>& tableInfo, PUML_SIGNAL_TYPE type);
	void genTable(std::vector<LatexTableData>& tableInfo);

	std::string genSections(const std::vector<LatexSectoinData>& sectionInfo);

	bool tryToExecute(const std::string & command, const std::string & path);

	bool ceatePDF(const std::string path);

	bool genPdfImage();
};

