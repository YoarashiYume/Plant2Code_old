#include "API.hpp"

wrapper_type* __stdcall createWrapper()
{
	return wrapper_type::create<SimpleDictionaryC, LibGeneratorC>();
}

EXPORT_VOID destroyWrapper(wrapper_type* ptr)
{
	if (ptr != nullptr)
	{
		delete ptr;
		ptr = nullptr;
	}
}

EXPORT_VOID configureLog(wrapper_type* ptr, Loggable::log_type logFunction)
{
	if (ptr != nullptr)
		ptr->setLog(logFunction);
}
EXPORT_VOID setOutputLocation(wrapper_type * ptr, char * path)
{
	if (ptr != nullptr)
	{
		ptr->setOutputLocation(std::string{ path , strlen(path) });
	}
}
EXPORT_VOID setOutputLocation(wrapper_type* ptr, std::string path)
{
	if (ptr != nullptr)
		ptr->setOutputLocation(path);
}
EXPORT_BOOL readSignalFiles(wrapper_type* ptr, char** paths, std::uint32_t elementCount)
{
	if (ptr != nullptr && elementCount != 0)
	{
		std::vector<std::string> arrayOfPaths;
		for (auto i = 0u; i < elementCount; ++i)
			arrayOfPaths.emplace_back(std::string{ *(paths + i) ,*(paths + i) + strlen(*(paths + i))});	
		return ptr->readSignalFiles(arrayOfPaths);
	}
	return false;
}
EXPORT_BOOL readPumlFiles(wrapper_type* ptr, char** paths, std::uint32_t elementCount)
{
	if (ptr != nullptr && elementCount != 0)
	{
		std::vector<std::string> arrayOfPaths;
		for (auto i = 0u; i < elementCount; ++i)
			arrayOfPaths.emplace_back(std::string{ *(paths + i) ,*(paths + i) + strlen(*(paths + i)) });
		return ptr->readPUMLFiles(arrayOfPaths);
	}
	return false;
}
EXPORT_BOOL compareSignals(wrapper_type* ptr)
{
	if (ptr != nullptr)
	{
		return ptr->compareSignals();
	}
	return false;
}
EXPORT_BOOL forceToCompareSignals(wrapper_type* ptr)
{
	if (ptr != nullptr)
	{
		return ptr->forceToCompareSignals();
	}
	return false;
}
EXPORT_VOID clear(wrapper_type* ptr)
{
	if (ptr != nullptr)
		ptr->clear();
}

EXPORT_BOOL checkPumlData(wrapper_type* ptr)
{
	if (ptr != nullptr)
	{
		return ptr->checkPumlData();
	}
	return false;
}

EXPORT_BOOL forceToCheckPumlData(wrapper_type* ptr)
{
	if (ptr != nullptr)
	{
		return ptr->forceToCheckPumlData();
	}
	return false;
}
EXPORT_BOOL buildCode(wrapper_type* ptr)
{
	if (ptr != nullptr)
	{
		return ptr->buildCode();
	}
	return false;
}
EXPORT_BOOL forceToBuildCode(wrapper_type* ptr)
{
	if (ptr != nullptr)
	{
		return ptr->forceToBuildCode();
	}
	return false;
}
EXPORT_BOOL buildLibrary(wrapper_type* ptr)
{
	if (ptr != nullptr)
	{
		return ptr->buildLibrary();
	}
	return false;
}
EXPORT_BOOL forceToBuildLibrary(wrapper_type* ptr)
{
	if (ptr != nullptr)
	{
		return ptr->forceToBuildLibrary();
	}
	return false;
}
#ifdef EXTENDED
char* string2char(const char* str, const std::size_t size)
{
	char* result = (char*)malloc((size + 1) * sizeof(char));
	strcpy(result, str);
	return result;
}
char* string2char(const std::string& str)
{
	return string2char(str.c_str(), str.size());
}
template<typename T>
T* arr2arr(const T* str, const std::uint32_t size)
{
	T* result = (T*)malloc((size) * sizeof(T));
	memcpy(result, str, sizeof(T) * size);
	return result; 
}
EXPORT_CHAR_PTR getLib(wrapper_type* ptr)
{
	if (ptr != nullptr)
		return string2char(ptr->getLibPath().c_str(), static_cast<int>(ptr->getLibPath().size()));
	else
		return nullptr;
}

EXPORT_VOID clearString(char* string)
{
	if (string)
		free(string);
}

EXPORT_VOID clearUint8_t(std::uint8_t* arr)
{
	if (arr)
		free(arr);
}

#include "../Common/CSVReader.hpp"
#include "../Common/StringFunction.hpp"

static std::vector<LatexPersonData> from(char* names, char* pos, std::uint32_t count)
{
	std::vector<LatexPersonData> result;
	if (count != 0)
	{
		auto name = Readers::CSVLineReader(std::string{names}, '\n', true);
		auto pos_ = Readers::CSVLineReader(std::string{pos}, '\n', true);
		for (size_t i = 0; i < static_cast<size_t>(count); ++i)
			result.emplace_back(LatexPersonData{ name.at(i), pos_.at(i) });
	}
	return result;
}
static void from(const LatexPersonDataExpandedC& cData, LatexPersonDataExpanded& cppData)
{
	cppData.name = std::string{ cData.name ? cData.name : ""};
	cppData.position = std::string{ cData.position ? cData.position : "" };
	cppData.workPlace = std::string{ cData.workPlace ? cData.workPlace : "" };
}
static void from(const LatexTitleDataC& cData, LatexTitleData& cppData)
{
	from(cData.military, cppData.military);
	from(cData.approve, cppData.approve);
	cppData.firstLine = from(cData.fname,cData.fposition , cData.firstLineCount);
	cppData.secondLine = from(cData.sname,cData.sposition , cData.secondLineCount);
}
static LatexDataMain from(const LatexDataMainC& cData)
{
	LatexDataMain cppData;
	from(cData.title, cppData.title);
	cppData.documentName = std::string{ cData.documentName };
	cppData.documentNumber = std::string{ cData.documentNumber };
	cppData.fullProgramName = std::string{ cData.fullProgramName };
	cppData.programShort = std::string{ cData.programShort };
	cppData.documentVersion = cData.documentVersion;
	cppData.needMilitary = cData.needMilitary;
	cppData.objectCode = cData.objectCode;
	return cppData;
}

EXPORT_CHAR_PTR genUE(wrapper_type* ptr, LatexDataMainC& data)
{
	return string2char(ptr->genUE(from(data)));
}
static std::vector<LatexTableData> from(const LatexTableDataC* cData, std::uint32_t dataCount)
{
	std::vector<LatexTableData> result;
	for (std::uint32_t i = 0u; i < dataCount; ++i)
	{
		result.emplace_back(LatexTableData{
		std::string{(cData + i)->name},
		std::string{(cData + i)->path},
		std::string{},
		(cData + i)->isLandscape,
		Readers::CSVLineReader(std::string{(cData + i)->sizes}, '\n', true) });
	}
	return result;
}
EXPORT_CHAR_PTR genLatexTables(wrapper_type* ptr, LatexTableDataC* data, std::uint32_t dataCount)
{
	auto cppData = from(data, dataCount);
	auto path = ptr->genTables(cppData);
	for (std::uint32_t i = 0u; i < dataCount; ++i)
		(data + i)->ref = string2char(cppData.at(i).ref);
	return string2char(path);
}
static void from(const UsedAlgorithmC& cData, UsedAlgorithm& cppData)
{
	cppData.name = std::string{ cData.name ? cData.name : "" };
	cppData.changedName = std::string{ cData.changedName ? cData.changedName : "" };
	cppData.extraData = std::string{ cData.extraData ? cData.extraData : "" };
}
static void from(const LatexAlgorithmDataC& cData, LatexAlgorithmData& cppData)
{
	cppData.name = std::string{ cData.name ? cData.name : "" };
	cppData.changedName = std::string{ cData.changedName ? cData.changedName : "" };
	cppData.extraData = std::string{ cData.extraData ? cData.extraData : "" };
	cppData.sizeKoef = Readers::CSVLineReader(std::string{ cData.sizeKoef }, '\n', true);
	cppData.pdfSize = Readers::CSVLineReader(std::string{ cData.pdfSize }, '\n', true);
	for (auto& el : cppData.sizeKoef)
		if (el.empty())
			el = "1.0";
	std::copy(cData.isLandscape, cData.isLandscape + cData.elementCount, std::back_inserter(cppData.isLandscape));
	cppData.usedAlgorithm = std::vector<UsedAlgorithm>(cData.usedCount, UsedAlgorithm{});
	for (auto i = 0u; i < cData.usedCount; ++i)
		from(*(cData.used + i), cppData.usedAlgorithm.back());	
}
static void from(const LatexSectoinDataC& cData, LatexSectoinData& cppData)
{
	cppData.name = std::string{ cData.name ? cData.name : "" };
	cppData.changedName = std::string{ cData.changedName ? cData.changedName : "" };
	cppData.extraData = std::string{ cData.extraData ? cData.extraData : "" };
	cppData.sections = std::vector<LatexAlgorithmData>(cData.sectionsCount, LatexAlgorithmData{});
	for (auto i = 0u; i < cData.sectionsCount; ++i)
		from(*(cData.sections + i) ,cppData.sections.back());
}
EXPORT_CHAR_PTR genLatexSections(wrapper_type* ptr, LatexSectoinDataC* sections, std::uint32_t sectionCount)
{
	std::vector<LatexSectoinData> texSections(sectionCount, LatexSectoinData{});
	for (auto i = 0u; i < sectionCount; ++i)
		from(*(sections + i), texSections.back());
	return string2char(ptr->genSections(texSections));
}
EXPORT_CHAR_PTR getTablesOutputPath(wrapper_type* ptr)
{
	return string2char(ptr->getTablesOutputPath());
}
LatexAlgorithmDataC* __stdcall getLatexAlgorithmData(wrapper_type* ptr, std::uint32_t& count)
{
	auto arr = ptr->getLatexAlgorithmData();
	count = static_cast<std::uint32_t>(arr.size());
	LatexAlgorithmDataC* cData = (LatexAlgorithmDataC*)malloc(count * sizeof(LatexAlgorithmDataC));
	for ( auto i = 0u; i < count; ++i)
	{
		(cData + i)->name = string2char(arr.at(i).name);
		(cData + i)->changedName = string2char(arr.at(i).changedName);
		(cData + i)->extraData = string2char(arr.at(i).extraData);
		(cData + i)->pdfSize = string2char(StringFunction::join(arr.at(i).pdfSize, '\n'));
		(cData + i)->elementCount = static_cast<std::uint32_t>(arr.at(i).isLandscape.size());
		(cData + i)->isLandscape = arr2arr(arr.at(i).isLandscape.data(), (cData + i)->elementCount);
		(cData + i)->sizeKoef = string2char(StringFunction::join(arr.at(i).sizeKoef, '\n'));


		(cData + i)->usedCount = static_cast<std::uint32_t>(arr.at(i).usedAlgorithm.size());
		(cData + i)->used = (UsedAlgorithmC*)malloc((cData + i)->usedCount * sizeof(UsedAlgorithmC));
		for (auto j = 0u; j < (cData + i)->usedCount; ++j)
		{
			((cData + i)->used +j)->name = string2char(arr.at(i).usedAlgorithm.at(j).name);
			((cData + i)->used +j)->changedName = string2char(arr.at(i).usedAlgorithm.at(j).changedName);
			((cData + i)->used +j)->extraData = string2char(arr.at(i).usedAlgorithm.at(j).extraData);
		}
	}
	return cData;
}
#endif