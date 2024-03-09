#include "Wrapper.hpp"
#include "../Creators/PumlProceeder.hpp"
#ifdef EXTENDED
#include "../Common/StringFunction.hpp"
#endif

#if _WIN32 || _WIN64
#if _WIN64
#define ENVIRONMENT64
#else
#define ENVIRONMENT32
#endif
#endif

#if __GNUC__
#if __x86_64__ || __ppc64__
#define ENVIRONMENT64
#else
#define ENVIRONMENT32
#endif
#endif
static bool isFile(const std::string& path)
{
	std::ifstream stream{ path };
	bool isFile_ = stream.is_open();
	stream.close();
	return isFile_;
}
static void clearFile(const std::string& path)
{
	if (isFile(path))
		remove(path.data());
}
bool Wrapper::tryToExecute(const std::string& command, const std::string& path)
{
	auto cmd = command + " >nul 2>nul";
	auto result = system(cmd.c_str());
	if (result != 0)
	{
		cmd = command + " 2>&1";
		std::unique_ptr<FILE, decltype(&_pclose)> pipe{ _popen(cmd.data(), "r"), _pclose };
		if (!pipe)
		{
			if (isFile(path))
				log("Возникли предупреждения. Предупреждения неизвестны - нет возможности открыть pipe.");
			else
				log("Компиляция dll не выполнена. Ошибки неизвестны - нет возможности открыть pipe.");
		}
		else
		{
			std::string buffer;
			char psBuffer[128];
			while (fgets(psBuffer, 128, pipe.get()))
				buffer += psBuffer;
			
			if (isFile(path))
				log(std::string{ "Возникли предупреждения. :\n" }.append(buffer).c_str());
			else
				log(std::string{ "Компиляция dll не выполнена. Ошибки:\n" }.append(buffer).c_str());
		}
	}
	return result == 0 || isFile(path);
}

Wrapper::Wrapper()
{
	signalReader.reset(new signal_reader_type);
	pumlReader.reset(new puml_reader_type);
#ifdef EXTENDED
	latexGenerator.reset(new latex_generator_type);
	latexGenerator->setNameConverter([this](const std::string& name)
		{
			auto newName = codeGenerator->nameConvert(name);
			newName.erase(std::remove_if(newName.begin(), newName.end(), StringFunction::_notAlpha), newName.end());
			return newName;
		});
#endif
}

Wrapper::~Wrapper() = default;

bool Wrapper::buildLibrary()
{
	if (wasGenerationSuccess)
		return forceToBuildLibrary();
	else
		log("Сборка библиотеки не выполнена. Т.к. этап генерации кода был пропущен/не выполнен/выполнен с ошибкой.");
	return false;
}

bool Wrapper::forceToBuildLibrary()
{
	if (generatorType >= GENERATOR_TYPE::LIBRARY)
	{
#ifdef ENVIRONMENT32
#define DLL_ADDITIONAL + " -m32"
#else
#define DLL_ADDITIONAL
#endif // ENVIRONMENT32
		auto command = "gcc -c -o \"" + codeGenerator->getLibFile(false, false) + "o\" \"" +
			codeGenerator->getLibFile(true, false) + "\" -I\"" + codeGenerator->getHeaderFile() + "\" -fPIC " DLL_ADDITIONAL;
		clearFile(codeGenerator->getLibFile(false, false) + "o");
		if (tryToExecute(command, codeGenerator->getLibFile(false, false) + "o"))
		{
			command = "gcc -c -o \"" + codeGenerator->getSourceFile(false) + "o\" \"" +
				codeGenerator->getSourceFile() + "\" -I\"" + codeGenerator->getHeaderFile() + "\" -fPIC" DLL_ADDITIONAL;
			clearFile(codeGenerator->getSourceFile(false) + "o");
			if (tryToExecute(command, codeGenerator->getSourceFile(false) + "o"))
			{
				command = "gcc -o \"" + codeGenerator->getOutput() + "outputLib.dll\" -s -shared \"" 
					+ codeGenerator->getSourceFile(false) + "o\" \"" +
					codeGenerator->getLibFile(false, false) + "o\" -I\"" + codeGenerator->getHeaderFile() + "\" -fPIC" DLL_ADDITIONAL;
				clearFile(codeGenerator->getOutput() + "outputLib.dll");
				return tryToExecute(command, codeGenerator->getOutput() + "outputLib.dll");
			}
		}
	}
	else
		log("Сборка библиотеки не выполнена. Т.к. установленный тип генератора этого не предусматривает.");
#undef DLL_ADDITIONAL
	return false;
}

bool Wrapper::buildCode()
{
	if (wasCheckSuccess)
		return forceToBuildCode();
	else
		log("Генерация кода не выполнена. Т.к. этап проверки PUML-алгоритмов был пропущен/не выполнен/выполнен с ошибкой.");
	return false;
}

bool Wrapper::forceToBuildCode()
{
	codeGenerator->setPuml(pumlReader->getAlgorithms());
	codeGenerator->setSignals(signalReader->getSignals());
	wasGenerationSuccess = codeGenerator->createSourceFile();
	if (generatorType >= TWO_FILE)
		wasGenerationSuccess = codeGenerator->createHeaderFile();
	if (generatorType >= LIBRARY)
		wasGenerationSuccess = codeGenerator->createLibFile();
	return wasGenerationSuccess;
}

bool Wrapper::checkPumlData()
{
	if (wasCompareSuccess)
		return forceToCheckPumlData();
	else
		log("Проверка PUML-алгоритмов не выполнена. Т.к. этап сверки сигналов был пропущен/не выполнен/выполнен с ошибкой.");
	return false;
}

bool Wrapper::forceToCheckPumlData()
{
	wasGenerationSuccess = false;
	std::unique_ptr<PumlProceeder> checker{
		new PumlProceeder{ pumlReader->getAlgorithms(), signalReader->getSignals() }};
	wasCheckSuccess = checker->checkPuml();
	return wasCheckSuccess;
}

bool Wrapper::compareSignals()
{
	if (wasSignalReadSuccess && wasPumlReadSuccess)
		return forceToCompareSignals();
	else
	{
		std::string end = wasPumlReadSuccess == false && wasSignalReadSuccess == false ? "ы" : "";
		std::string step = wasPumlReadSuccess == false && wasSignalReadSuccess == false ? "CSV и PUML" :
			(wasSignalReadSuccess == false ? "CSV" : "PUML" );
		log("Сверка сигналов не выполнена. Т.к. этап" + end +
			" считывания " + step + " пропущен" + end +"/не выполнен" + end + "/выполнен" + end + " с ошибкой.");
	}
	return false;
}

bool Wrapper::forceToCompareSignals()
{
	wasCheckSuccess = wasGenerationSuccess = false;
	wasCompareSuccess = signalReader->compareSignalsWithPumlSignals(*pumlReader->getSignals());
	return wasCompareSuccess;
}

bool Wrapper::readPUMLFiles(const paths_type& paths)
{
	pumlReader->clear();
	wasCompareSuccess = wasCheckSuccess = wasGenerationSuccess = false;
	wasPumlReadSuccess = true;
	for (auto& path : paths)
		wasPumlReadSuccess &= pumlReader->read(path);
	return wasPumlReadSuccess;
}

bool Wrapper::readSignalFiles(const paths_type& paths)
{
	signalReader->clear();
	wasCompareSuccess = wasCheckSuccess = wasGenerationSuccess = false;
	wasSignalReadSuccess = signalReader->readSignals(paths);
	return wasSignalReadSuccess;
}

void Wrapper::setLog(Loggable::log_type newLog)
{
	Loggable::setLog(newLog);
}

void Wrapper::setOutputLocation(const path_type& path)
{
	codeGenerator->setOutputPath(path);
#ifdef EXTENDED
	latexGenerator->setOutputPath(path + "\\latex");
#endif
}

void Wrapper::clear()
{
	signalReader->clear();
	pumlReader->clear();
	codeGenerator->clear();
	wasSignalReadSuccess = wasPumlReadSuccess = wasCompareSuccess = wasCheckSuccess = wasGenerationSuccess = false;
}
#ifdef EXTENDED

std::string Wrapper::getLibPath() const
{
	return codeGenerator->getOutput() + "outputLib.dll";
}
void Wrapper::setLatexSetting()
{
	LatexGenerator gen;
	gen.setAlgorithm(pumlReader->getAlgorithms());	
	gen.genPdfImage();
	gen.genLatexAlgorithmData(codeGenerator->getSortedFunctionsKeys());
//	gen.genAlgorithm(gen.genLatexAlgorithmData(codeGenerator->getSortedFunctionsKeys()));
	LatexTableData data;
	data.isLandscape = false;
	data.name = "Входные аналоговые сигналы и РК";
	data.path = "E:\\environment\\Рабочий стол\\Тест Як\\SARD\\signals\\in_a.csv";
	data.size = std::vector<std::string>{ "6.5", "5.5","4.2","3.43" ,"3.93" ,"7.145" };
//	gen.genTable(data);
	
	LatexDataMain t;
	

	gen.genUE(t);
	auto a = 0;
}

std::string Wrapper::genUE(const LatexDataMain& titleData)
{
	return latexGenerator->genUE(titleData);
}

std::string Wrapper::genTables(std::vector<LatexTableData>& tables)
{
	for (auto& el : tables)
		latexGenerator->genTable(el);
	return latexGenerator->getTablePathFolder();
}

std::string Wrapper::getTablesOutputPath() const
{
	return latexGenerator->getTablePathFolder();
}

std::vector<LatexAlgorithmData> Wrapper::getLatexAlgorithmData()
{
	latexGenerator->setAlgorithm(pumlReader->getAlgorithms());
	return latexGenerator->genLatexAlgorithmData(codeGenerator->getSortedFunctionsKeys());
}

std::string Wrapper::genSections(const std::vector<LatexSectoinData>& sections)
{
	return latexGenerator->genSections(sections);
}

#endif // EXTENDED

