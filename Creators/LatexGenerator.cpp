#include "LatexGenerator.hpp"
#include "../Common/StringFunction.hpp"
#include "../Common/CSVReader.hpp"
#include <regex>
#include <thread>



bool LatexGenerator::genImageSection(stream_type& stream, const LatexAlgorithmData & algorithmInfo)
{
	auto algo = algorithms->at(algorithmInfo.name);
	auto pos = algo->path.find_last_of('\\');
	std::string path;
	if (pos != std::string::npos)
		path = algo->path.substr(pos + 1);
	else
		path = algo->path;
	path = path.substr(0, path.size() - strlen(".puml"));
	for (auto i = 0; i < algo->refs.size(); ++i)
	{
		std::string index = std::string(3 - std::to_string(i).size(), '0') + std::to_string(i);
		if (i == 0)
		{
			if (algorithmInfo.isLandscape.at(i))
				stream.genWithOffset(FileReading::OffsetInfo{ "\\begin{landscape}",1,false });
		}
		else
		{
			if (algorithmInfo.isLandscape.at(i) != algorithmInfo.isLandscape.at(i - 1))
			{
				if (algorithmInfo.isLandscape.at(i))
					stream.genWithOffset(FileReading::OffsetInfo{ "\\begin{landscape}",1,false });
				else
					stream.genWithOffset(FileReading::OffsetInfo{ "\\end{landscape}",-1,true });
			}
		}
		stream.genWithOffset(FileReading::OffsetInfo{ "\\begin{figure}[H]",1,false });

		stream.genWithoutOffset( "\\center{\\includegraphics[scale = " 
			+ algorithmInfo.sizeKoef.at(i)+	"]{"
			+ path + (i != 0 ? "_" + index : "")+"}}");
		stream.genWithoutOffset( "\\caption{Блок-схема алгоритма <<" + algorithmInfo.changedName +
			(algo->refs.at(i) .size() ?+" ("+ algo->refs.at(i) +")>>}" : ">>}"));
		stream.genWithoutOffset( "\\label{img: " + nameConverer(algorithmInfo.name)+ algo->refs.at(i) + " }");
		stream.genWithOffset(FileReading::OffsetInfo{ "\\end{figure}",-1,true });
	}	
	if (algorithmInfo.isLandscape.back())
		stream.genWithOffset(FileReading::OffsetInfo{ "\\end{landscape}",-1,true });
	return true;
}
static std::string useImgRef(const std::string ref)
{
	return "\\ref{img:" + ref + "}";
}
static FileReading::OffsetInfo genSections(const std::string name, const std::string section)
{
	return FileReading::OffsetInfo{ "\\" + section + " {" + name + "}{" ,1, false };
}
static FileReading::OffsetInfo genEndSection()
{
	return FileReading::OffsetInfo{ "}" ,-1, true };
}
static FileReading::OffsetInfo genSection(const std::string name)
{
	return genSections(name, "section");
}

static FileReading::OffsetInfo genSubSectionName(const std::string name)
{
	return genSections(name, "subsection");
}
static FileReading::OffsetInfo genSubSubSectionName(const std::string name)
{
	return genSections(name, "subsubsection");
}
static std::string genLabel(const std::string ref)
{
	return "\\label{" + ref + "}";
}
static FileReading::OffsetInfo genParagSectionName(const std::string name, const std::string ref)
{
	return genSections(name + " " +genLabel(ref), "customParag");
}
std::string LatexGenerator::useParagRef(const UsedAlgorithm data)
{
	return "<<" + data.changedName + ">> " + (data.extraData.size() ? ", " + data.extraData + ", " : "") +"в соответствии с п.~\\ref{" + nameConverer(data.name) + "}";
}
void LatexGenerator::createUsedAlgoList(LatexGenerator::stream_type& stream, std::vector<UsedAlgorithm> list)
{
	stream.genWithOffset(FileReading::OffsetInfo{ "\\begin{itemize}",1,false });
	for (auto i = 0u; i < list.size(); ++i)
		stream.genWithoutOffset("\\item " + useParagRef(list.at(i)) + (i == (list.size() - 1) ? "." : ";"));
	stream.genWithOffset(FileReading::OffsetInfo{ "\\end{itemize}",-1,true });
}

bool LatexGenerator::genAlgorithmSection(stream_type& stream, const LatexAlgorithmData & algorithmInfo)
{
	auto algo = algorithms->at(algorithmInfo.name);
	stream.genWithOffset(genParagSectionName(algorithmInfo.name, nameConverer(algorithmInfo.name)));
	std::string refs;
	if (algo->refs.size() == 1)
		refs = "рисунком~" + useImgRef(nameConverer(algorithmInfo.name));
	else
	{
		refs = "рисунками~" + useImgRef(nameConverer(algorithmInfo.name)) ;
		refs += algo->refs.size() == 2 ? " и~" : "...";
		refs += useImgRef(nameConverer(algorithmInfo.name) + algo->refs.back());
	}
	stream.genWithoutOffset( actor + "должен" + onna + " выполнять алгоритм <<" +algorithmInfo.changedName + ">> "
	+ (algorithmInfo.extraData.size() ? "," + algorithmInfo.extraData + "," : "") + "в соответствии " + refs + ".");
	if (algorithmInfo.usedAlgorithm.size())
	{
		if (algorithmInfo.usedAlgorithm.size() == 1)
		{
			stream.genWithoutOffset( "При выполнении алгоритма <<" + algorithmInfo.changedName + 
				">> выполняется алгоритм " 
				+ useParagRef(algorithmInfo.usedAlgorithm.front()) + ".");
		}
		else
		{
			stream.genWithoutOffset( "При выполнении алгоритма <<" + algorithmInfo.changedName + ">> выполняются алгоритмы:");
			createUsedAlgoList(stream, algorithmInfo.usedAlgorithm);
		}
	}
	genImageSection(stream, algorithmInfo);
	stream.genWithOffset(genEndSection());
	return true;
}

void LatexGenerator::genInitData(stream_type & stream)
{
	stream.genWithoutOffset( "\\documentclass[a4paper,14pt]{extarticle}");

	stream.genWithoutOffset( "\\let\\counterwithout\\relax");
	stream.genWithoutOffset( "\\let\\counterwithin\\relax");
	stream.genWithoutOffset( "\\usepackage[T2A]{ fontenc }");
	stream.genWithoutOffset( "\\usepackage[utf8]{ inputenc }");
	stream.genWithoutOffset( "\\usepackage{ lastpage }");
	stream.genWithoutOffset( "\\usepackage[russian]{ babel }");
	stream.genWithoutOffset( "\\usepackage{ common / uspd_hard }");
	stream.genWithoutOffset( "\\usepackage{ ifthen }");
	stream.genWithoutOffset( "\\usepackage{ bookmark }");
	stream.genWithoutOffset( "\\usepackage{ common / UE }");
	stream.genWithoutOffset( "\\usepackage{ graphicx }");
	stream.genWithoutOffset( "\\usepackage{ dcolumn, longtable, booktabs}");
	stream.genWithoutOffset( "\\usepackage{ float }");
	stream.genWithoutOffset( "\\usepackage{ wrapfig }");
	stream.genWithoutOffset( "\\usepackage{ booktabs }");
	stream.genWithoutOffset( "\\usepackage{ colortbl }");
	stream.genWithoutOffset( "\\pdfmapfile{ = pdftex35.map }");
	stream.genWithoutOffset( "\\graphicspath{ images/ }");
	if (isGlossary)
		stream.genWithoutOffset( "\\input{common/glossary}");

	stream.genWithoutOffset( "\\newboolean{ needtableofcontents }"); 
	stream.genWithoutOffset( "\\setboolean{ needtableofcontents }{true}");

	stream.genWithoutOffset( "\\newcounter{customParagCounter}");
	stream.genWithoutOffset( "\\newcommand{ \\customParag }[1]{ \\stepcounter{ customParagCounter }\\paragraph[#1]{ HR - \\arabic{ customParagCounter }: #1 } }");
}



LatexGenerator::LatexGenerator()
{
}


LatexGenerator::~LatexGenerator()
{
}

void LatexGenerator::setOutputPath(const std::string path)
{
	outputPath = path;
}

std::string LatexGenerator::getTablePathFolder() const
{
	return outputPath + "\\tables";
}

std::pair<std::uint32_t, std::uint32_t> LatexGenerator::getWorkListSize() const
{
	return std::pair<std::uint32_t, std::uint32_t>(180, 247);
}

void LatexGenerator::setAlgorithm(storage_type algorithm)
{
	algorithms = algorithm;
}

void LatexGenerator::setNameConverter(const function_converter_type converter)
{
	nameConverer = converter;
}
#include <iostream>
static std::vector<std::string> getPdfImageSize(std::string path, std::uint32_t count)
{
	std::vector<std::string> result;

	//TODO: Испарвить директорию генерации
	std::string _path = "C:\\Users\\fd.nadezhnyy\\Desktop\\Git\\UDesigner\\Source\\Plant2Code\\x64\\Debug\\img\\" + path.substr(path.find_last_of('\\') + 1);
	_path = _path.substr(0,_path.find_last_of('.') == std::string::npos ? 0 : _path.find_last_of('.'));
	for (std::uint32_t i = 0; i < count; ++i)
	{
		std::string currentPath = _path;
		if (i != 0)
			currentPath += '_' + std::string(3 - std::to_string(i).size(), '0') + std::to_string(i);
		currentPath += ".pdf";

		FileReading::unique_ifstream_type file(new FileReading::unique_ifstream_type::element_type{
			currentPath, std::ios::binary },
			FileReading::closeFunction);
		std::string contents((std::istreambuf_iterator<char>(*file)), std::istreambuf_iterator<char>());
		std::regex pattern(R"(/MediaBox\s*\[\s*([0-9.]+)\s+([0-9.]+)\s+([0-9.]+)\s+([0-9.]+)\s*\])");
		std::smatch match;
		if (std::regex_search(contents, match, pattern))
		{
#define KOEF (25.4f / 72)
			double width = KOEF * std::stod(match[3]) - std::stod(match[1]);
			double height = KOEF * std::stod(match[4]) - std::stod(match[2]);
#undef KOEF
			result.emplace_back(std::to_string(width) + ";" + std::to_string(height));
		}
		else
			result.emplace_back();
		std::cout << path << '\t' << result.back() << std::endl;
	}
	return result;

}

std::vector<LatexAlgorithmData> LatexGenerator::genLatexAlgorithmData(std::vector<std::string> order)
{
	std::vector<LatexAlgorithmData> rawData;
	for (auto& key : order)
	{
		LatexAlgorithmData nextData;
		nextData.name = key;
		nextData.pdfSize = getPdfImageSize(algorithms->at(key)->path, static_cast<std::uint32_t>(algorithms->at(key)->refs.size()));
		nextData.changedName = key;
		nextData.sizeKoef = std::vector<std::string>
		( algorithms->at(key)->refs.size(), std::string{ "1.0" });
		nextData.isLandscape = std::vector<std::uint8_t>
			(algorithms->at(key)->refs.size(), false);
		for (auto& usedKeys : algorithms->at(key)->usedAlgorithm)
			nextData.usedAlgorithm.emplace_back(UsedAlgorithm{ usedKeys, usedKeys });
		rawData.emplace_back(std::move(nextData));
	}
	return rawData;
}

static FileReading::OffsetInfo startTable(const std::vector<std::string>& sizes)
{
	std::string stringSize{"{|"};
	for (auto& el : sizes)
		stringSize += "p{" + el + "mm}|";
	return FileReading::OffsetInfo{ "\\begin{longtable}"+ stringSize+"}" ,1, false };
}
static FileReading::OffsetInfo endTable()
{
	return FileReading::OffsetInfo{ "\\end{longtable}" ,-1, true };
}
std::string LatexGenerator::formTableLine(std::string line, const LatexTableData & tableInfo) const
{
	auto split = Readers::CSVLineReader(line, ';', true);
	bool isSame = true;
	std::string tableLine;
	for (auto& el : split)
	{
		StringFunction::appendString(el, tableLine, '&');
		isSame &= split.front() == el;
	}
	if (isSame)
		return "\\multicolumn{" + std::to_string(tableInfo.size.size()) +
			"}{|p{" + std::to_string(tableInfo.isLandscape ? getWorkListSize().first : getWorkListSize().second) + "}" + +"mm}|}{" + tableLine + "}\\\\";

	else
		return  tableLine + "\\\\";
}
static FileReading::OffsetInfo genCommand(const std::string name)
{
	return FileReading::OffsetInfo{ "\\newcommand{\\" + name + "}{" ,1, false };
}
static FileReading::OffsetInfo genEndCommand()
{
	return FileReading::OffsetInfo{ "}" ,-1, true };
}
static std::string genFullCommand(const std::string& name, const std::string& value)
{
	return "\\newcommand{\\" + name + "}{" + value + "}";
}
static std::string createBool(const std::string& name, bool value)
{
	return "\\newboolean{" + name + "}\n\\setboolean{" + name + "}{" + (value ? "true" : "false") + "}";
}
inline static std::string fixNewLine(const std::string& line)
{
	return StringFunction::escapeRemove(line, "\\n", "\\newline{}");
}

std::string LatexGenerator::genUE(const LatexDataMain& titleData)
{
	std::string path = outputPath + "\\UE.tex";
	FileReading::FileWriter ostream{ path };

	ostream.genWithoutOffset(createBool("needmilitary", titleData.needMilitary));

	genUEPerson(ostream, titleData.title, titleData.needMilitary);

	std::string workLine = std::to_string(titleData.objectCode);
	workLine = std::string(5 - workLine.size(), '0') + workLine;
	ostream.genWithoutOffset(genFullCommand("objectcode", workLine));

	workLine = titleData.programShort + workLine;
	ostream.genWithoutOffset( genFullCommand("programcodeshort", workLine));

	workLine = workLine + (titleData.documentVersion >= 10 ? "-" : "-0") + std::to_string(titleData.documentVersion);
	ostream.genWithoutOffset( genFullCommand("programcode", workLine));

	ostream.genWithoutOffset(genFullCommand("docdecimalnumber", titleData.documentNumber));
	ostream.genWithoutOffset(genFullCommand("documentname", titleData.documentName));
	ostream.genWithoutOffset( genFullCommand("programNameFull", titleData.fullProgramName));
	return path;
	
}
static inline std::vector<std::string> splitLatexPerson(const std::vector<LatexPersonData>& data, const std::size_t start)
{
	std::vector<std::string>result;
	for (std::size_t i = start; i < data.size(); ++i)
	{
		
		auto split = Readers::CSVLineReader(data.at(i).position, '\n', true);
		result.insert(result.end(), std::make_move_iterator(split.begin()),
			std::make_move_iterator(split.end()));
		result.emplace_back("\\signfield{" + data.at(i).name + "}");
		result.emplace_back("\\datefield");
		result.emplace_back("  ");
	}
	return result;
}
void LatexGenerator::genUEPerson(stream_type& stream, const LatexTitleData& title, bool needMilitary)
{
	stream.genWithoutOffset( genFullCommand("militarydep" ,
		needMilitary ? fixNewLine(title.military.position) : "" ));
	stream.genWithoutOffset( genFullCommand("militarydepby",
		needMilitary ? fixNewLine(title.military.workPlace) : ""));
	stream.genWithoutOffset( genFullCommand("militarydephead",
		needMilitary ? fixNewLine(title.military.name) : ""));
	
	stream.genWithoutOffset( genFullCommand("approvedby" , fixNewLine(title.approve.position) ));
	stream.genWithoutOffset( genFullCommand("plant" , fixNewLine(title.approve.workPlace) ));
	stream.genWithoutOffset( genFullCommand("approvedbyname" , fixNewLine(title.approve.name) ));

	stream.genWithoutOffset( genFullCommand("litera", std::string{}));

	if (!title.secondLine.empty())
	{
		stream.genWithoutOffset( genFullCommand("devhead", fixNewLine(title.secondLine.front().name)));
		stream.genWithoutOffset( genFullCommand("devheadtitle", fixNewLine(title.secondLine.front().position)));

	}
	stream.genWithOffset( genCommand("additionallapprove"));

	std::vector<std::string> secondCol{splitLatexPerson(title.secondLine, 1)}, firstCol{ splitLatexPerson(title.firstLine, 1) };
	auto maxIndex = firstCol.size() > secondCol.size() ? firstCol.size() : secondCol.size();
	for (decltype(maxIndex) i = 0; i < maxIndex; ++i)
	{
		std::string line;
		const std::string* src{nullptr};
		if (firstCol.size() > i)
			src = &firstCol.at(i);
		StringFunction::appendString(src ? *src : " ", line, ';');
		StringFunction::appendString(" ", line, ';');
		if (secondCol.size() > i)
			src = &secondCol.at(i);
		else
			src = nullptr;
		StringFunction::appendString(src ? *src : " ", line, ';');
		stream.genWithoutOffset( formTableLine(line));
	}

	stream.genWithOffset( genEndCommand());
}
std::vector < std::string > LatexGenerator::genRefsArray(std::vector<PUML_SIGNAL_TYPE>& types, std::vector<LatexTableData>& tableInfo, PUML_SIGNAL_TYPE type)
{
	std::vector<std::string> refs;
	for (std::vector<PUML_SIGNAL_TYPE>::size_type i = 0; i < types.size(); ++i)
	{
		if (types.at(i) == type)
			refs.emplace_back("\\ref{" +nameConverer(tableInfo.at(i).name) + "}");
	}
	return refs;
}
static std::string genTableText(std::vector<std::string> refs, std::string text)
{
	std::string refsText = "~";
	if (refs.empty())
		return std::string{};
	else if (refs.size() == 1)
		refsText = refs.front();
	else
	{
		refsText += refs.front();
		if (refs.size() == 2)
			refsText += " и~";
		else
			refsText += "...";
		refsText += refs.back();
	}

	return "В таблиц" + std::string{ refs.size() == 1 ? "e" : "ах" }+refsText +" приведены " + text + ".";
}
std::string LatexGenerator::createTableInclude(std::vector<PUML_SIGNAL_TYPE>& types, std::vector<LatexTableData>& tableInfo, PUML_SIGNAL_TYPE type)
{
	std::string result;
	for (std::vector<PUML_SIGNAL_TYPE>::size_type i = 0; i < types.size(); ++i)
		if (types.at(i) == type)
			StringFunction::appendString("\\input{" + getTablePathFolder() + "\\" + nameConverer(tableInfo.at(i).name) + "}", result, '\n');
	return result;
}
void LatexGenerator::genTable(std::vector<LatexTableData>& tableInfo)
{
	std::vector<PUML_SIGNAL_TYPE> types;
	std::vector<std::string> refs;
	for (auto& el : tableInfo)
		types.emplace_back(genTable(el));
	
	FileReading::FileWriter stream{ outputPath + "\\tableFile.tex" };
	if (stream.isOpen())
	{
		stream.genWithOffset(genSection("Требования к интерфейсам"));
		stream.genWithoutOffset("\\captionsetup[table]{margin=1cm}");
		stream.genWithoutOffset("\\captionsetup[longtable]{margin=1cm}");
		
		auto text = genTableText(genRefsArray(types, tableInfo, PUML_SIGNAL_TYPE::INPUT), "входные сигналы, поступающие в " + onna);
		if (text.size())
		{
			stream.genWithOffset(genSubSectionName("Входные сигналы"));
			stream.genWithoutOffset(text);
			stream.genWithoutOffset(createTableInclude(types, tableInfo, PUML_SIGNAL_TYPE::INPUT));
			stream.genWithOffset(genEndSection());
			stream.genWithoutOffset("\\newpage");
		}		

		text = genTableText(genRefsArray(types, tableInfo, PUML_SIGNAL_TYPE::OUTPUT), "выходные сигналы, генерируемые " + onna);
		if (text.size())
		{
			stream.genWithOffset(genSubSectionName("Выходные сигналы"));
			stream.genWithoutOffset(text);
			stream.genWithoutOffset(createTableInclude(types, tableInfo, PUML_SIGNAL_TYPE::OUTPUT));
			stream.genWithOffset(genEndSection());
			stream.genWithoutOffset("\\newpage");
		}
		text = genTableText(genRefsArray(types, tableInfo, PUML_SIGNAL_TYPE::UNKNOWN), "приведены используемые структуры");
		if (text.size())
		{
			stream.genWithOffset(genSubSectionName("Используемые структуры"));
			stream.genWithoutOffset(text);
			stream.genWithoutOffset(createTableInclude(types, tableInfo, PUML_SIGNAL_TYPE::UNKNOWN));
			stream.genWithOffset(genEndSection());
			stream.genWithoutOffset("\\newpage");
		}


		stream.genWithOffset(genEndSection());


		stream.genWithOffset(genSection("Требования к внутренним данным"));
		text = genTableText(genRefsArray(types, tableInfo, PUML_SIGNAL_TYPE::CONST_SIGN), "константы");
		if (text.size())
		{
			stream.genWithoutOffset(text);
			stream.genWithoutOffset(createTableInclude(types, tableInfo, PUML_SIGNAL_TYPE::CONST_SIGN));
			stream.genWithoutOffset("\\newpage");
		}
		text = genTableText(genRefsArray(types, tableInfo, PUML_SIGNAL_TYPE::TIMER), "таймеры");
		if (text.size())
		{
			stream.genWithoutOffset(text);
			stream.genWithoutOffset(createTableInclude(types, tableInfo, PUML_SIGNAL_TYPE::TIMER));
			stream.genWithoutOffset("\\newpage");
		}
		text = genTableText(genRefsArray(types, tableInfo, PUML_SIGNAL_TYPE::CONST_SIGN), "внутренние состояния");
		if (text.size())
		{
			stream.genWithoutOffset(text);
			stream.genWithoutOffset(createTableInclude(types, tableInfo, PUML_SIGNAL_TYPE::INNER));
			stream.genWithoutOffset("\\newpage");
		}
		stream.genWithOffset(genEndSection());
	}

}
PUML_SIGNAL_TYPE LatexGenerator::genTable(LatexTableData & tableInfo)
{
	FileReading::FileWriter ostream{
		 getTablePathFolder() + "\\" + nameConverer(tableInfo.name) +".tex" };
	FileReading::unique_ifstream_type istream{
		new FileReading::unique_ifstream_type::element_type{ tableInfo.path },
		FileReading::closeFunction };

	ostream.genWithOffset( startTable(tableInfo.size));
	ostream.genWithoutOffset( "\\caption{"+tableInfo.name+"}");
	tableInfo.ref = "tab:" + nameConverer(tableInfo.name);
	ostream.genWithoutOffset( "\\label{" + tableInfo.ref + "}");
	ostream.genWithoutOffset( "\\endfirsthead");
	ostream.genWithoutOffset( "\\multicolumn{ "+std::to_string(tableInfo.size.size())+ "}}{@{\\hspace{1cm}}l}{Продолжение таблицы \\thetable}\\");
	ostream.genWithoutOffset( "\\hline");
	ostream.genWithoutOffset( "\\endhead");
	ostream.genWithoutOffset( "\\hline");
	std::string line;
	PUML_SIGNAL_TYPE type = PUML_SIGNAL_TYPE::UNKNOWN;
	bool isFirst = true;
	while (std::getline(*istream, line))
	{
		auto split = Readers::CSVLineReader(line, ';',true);
		bool isSame = true;
		std::string tableLine;
		for (auto& el : split)
		{
			StringFunction::appendString(el, tableLine, '&');
			isSame &= split.front() == el;
			if (!isFirst && type != PUML_SIGNAL_TYPE::UNKNOWN)
			{
				if (el.size() > 1)
					if (StringFunction::_isupper(el.front()) && std::all_of(std::next(el.begin()), el.end(), [](char ch)
					{
						return (StringFunction::_isalpha(ch) == false) || ch == '[' || ch == ']' || ch == '.';
					}))
					{
						type = static_cast<PUML_SIGNAL_TYPE>(el.front());
						switch (type)
						{
						
						case PUML_SIGNAL_TYPE::INPUT:
						case PUML_SIGNAL_TYPE::OUTPUT:
						case PUML_SIGNAL_TYPE::INNER:
						case PUML_SIGNAL_TYPE::CONST_SIGN:
						case PUML_SIGNAL_TYPE::TIMER:
							break;
						case PUML_SIGNAL_TYPE::LOCAL_ARS:
						case PUML_SIGNAL_TYPE::INPUT_ARG:
						case PUML_SIGNAL_TYPE::OUTPUT_ARG:
						case PUML_SIGNAL_TYPE::UNKNOWN:
						default:
							type = PUML_SIGNAL_TYPE::UNKNOWN;
							break;
						}
					}
			}
		}
		if (isSame)
			ostream.genWithoutOffset( "\\multicolumn{" + std::to_string(tableInfo.size.size()) +
				"}{|p{" + std::to_string(tableInfo.isLandscape ? getWorkListSize().first : getWorkListSize().second) + "}" + +"mm}|}{" + tableLine + "}\\");

		else
			ostream.genWithoutOffset( tableLine + "\\");
		ostream.genWithoutOffset( "\\hline");
		isFirst = false;
	}

	ostream.genWithOffset( endTable());
	return type;
}



std::string LatexGenerator::genSections(const std::vector<LatexSectoinData>& sectionInfo)
{
	FileReading::FileWriter stream{ outputPath + "\\algorithmFile.tex" };
	bool isOk{ false };
	if (isOk = stream.isOpen())
	{
		genInitData(stream);//Возможно не генерировать все, а использовать в шаблоне
		stream.genWithOffset(genSubSectionName("Требования к функционированию"));
		for (auto& section : sectionInfo)
		{
			stream.genWithOffset(genSubSubSectionName(section.changedName));
			if (section.extraData.size())
				stream.genWithoutOffset(section.extraData);
			for (auto& algorithm : section.sections)
				isOk &= genAlgorithmSection(stream, algorithm);

			stream.genWithOffset(genEndSection());
		}
		stream.genWithOffset(genEndSection());
	}
	return isOk ? stream.path() : std::string{};
}
bool LatexGenerator::tryToExecute(const std::string& command, const std::string& path)
{
	auto cmd = command + " >nul 2>nul";
	auto result = system(cmd.data());
	if (result != 0)
	{
		cmd = command + " 2>&1";
		std::unique_ptr<FILE, decltype(&_pclose)> pipe{ _popen(cmd.data(), "r"), _pclose };
		if (!pipe)
		{
			log("Нет возможности открыть pipe для просмотра ошибок при генерации из файла: " + path);
		}
		else
		{
			std::string buffer;
			char psBuffer[128];
			while (fgets(psBuffer, 128, pipe.get()))
				buffer += psBuffer;
			log(buffer.c_str());
		}
	}
	return result == 0;
}

bool LatexGenerator::ceatePDF(const std::string path)
{
	std::ifstream input_file(path, std::ios::in | std::ios::binary);
	if (!input_file.is_open())
	{
		log("Невозможно открыть файл: " + path);
		return false;
	}
	else
	{
		auto offset = path.find_last_of('\\');
		std::string outPath = "C:\\Users\\fd.nadezhnyy\\Desktop\\Git\\UDesigner\\Source\\Plant2Code\\x64\\Debug\\img\\" +
			path.substr(offset == std::string::npos ? 0 : offset + 1);
		std::ofstream output_file(outPath,
			std::ios::out | std::ios::binary);

		if (!output_file.is_open())
		{
			log("Невозможно открыть файл: " + outPath);
			input_file.close();
			return false;
		}
		else
		{
			std::string line;
			while (std::getline(input_file, line))
				output_file << line;
			output_file.close();
			input_file.close();
			bool isOk = tryToExecute("java -jar plantuml2pdf\\plantuml.jar -tpdf " + outPath, outPath);
			std::remove(outPath.c_str());
			return isOk;
		}

	}
	
}

bool LatexGenerator::genPdfImage()
{
	
	//TODO: Заменить содание на копирование
	std::ofstream common_file("C:\\Users\\fd.nadezhnyy\\Desktop\\Git\\UDesigner\\Source\\Plant2Code\\x64\\Debug\\img\\common.iuml", std::ios::out | std::ios::binary);
	common_file << "@startuml\nskinparam monochrome true\nskinparam shadowing false\nskinparam DefaultFontSize 10\n"
		"skinparam DefaultFontName Verdana\nskinparam SvgLinkTarget _parent\nskinparam dpi 300\nskinparam RoundCorner 25\n"
		"@enduml";
	common_file.close();

	////TODO: буста не дало
	//std::vector<std::thread> threads;
	//std::vector<bool> isOkThreads(std::thread::hardware_concurrency(), 1);
	//for (std::uint32_t i = 0; i < static_cast<std::uint32_t>(isOkThreads.size()); ++i)
	//{
	//	threads.emplace_back(std::thread
	//	{
	//		[i,this, &isOkThreads]()
	//	{
	//		std::uint32_t currentIndex{ 0 };
	//		for (auto& el : *algorithms)
	//		{
	//			if (currentIndex++ % isOkThreads.size() == i)
	//				isOkThreads.at(i) = isOkThreads.at(i) && ceatePDF(el.second->path);
	//		}
	//	}
	//	});
	//}
	bool isOk{ true };
	//for (auto i = 0; i < isOkThreads.size(); ++i)
	//{
	//	threads.at(i).join();
	//	isOk &= isOkThreads.at(i);
	//}
	//
		


	////TODO: заменить путь
	//std::remove("C:\\Users\\fd.nadezhnyy\\Desktop\\Git\\UDesigner\\Source\\Plant2Code\\x64\\Debug\\img\\common.iuml");
	//return isOk;

	

	for (auto& el : *algorithms)
	{
		std::ifstream input_file(el.second->path, std::ios::in | std::ios::binary);
		if (!input_file.is_open())
		{
			log("Невозможно открыть файл: " + el.second->path);
			isOk = false;
		}
		else
		{
			auto offset = el.second->path.find_last_of('\\');
			std::string outPath = "C:\\Users\\fd.nadezhnyy\\Desktop\\Git\\UDesigner\\Source\\Plant2Code\\x64\\Debug\\img\\" +
				el.second->path.substr(offset == std::string::npos ? 0 : offset + 1);
			std::ofstream output_file(outPath,
				std::ios::out | std::ios::binary);
			
			if (!output_file.is_open())
			{
				log("Невозможно открыть файл: " + outPath);
				isOk = false;
			}
			else
			{
				std::string line;
				while (std::getline(input_file, line))
					output_file << line;
				output_file.close();
				isOk |= tryToExecute("java -jar plantuml2pdf\\plantuml.jar -tpdf " + outPath, outPath);
				std::remove(outPath.c_str());
			}
			
		}
		input_file.close();
		
	}
	//TODO: заменить путь
	std::remove("C:\\Users\\fd.nadezhnyy\\Desktop\\Git\\UDesigner\\Source\\Plant2Code\\x64\\Debug\\img\\common.iuml");
	return isOk;
}
