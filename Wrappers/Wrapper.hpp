#pragma once
#include "../Readers/Algorithm/AlgorithmReader.hpp"
#include "../Readers/Signal/SignalReader.hpp"
#include "../Creators/CodeGenerator.hpp"
#ifdef EXTENDED
#include"../Creators/LatexGenerator.hpp"
#endif
/*
///\brief - Класс-обертка для взаимодействия
*/
class Wrapper : Loggable
{
public:
	using generator_type = CodeGenerator;
	using signal_reader_type = SignalReader;
	using puml_reader_type = AlgorithmReader;
	using ptr_generator_type = std::unique_ptr<generator_type>;
	using ptr_signal_reader_type = std::unique_ptr <signal_reader_type>;
	using ptr_puml_reader_type = std::unique_ptr <puml_reader_type>;

#ifdef EXTENDED
	using latex_generator_type = LatexGenerator;
	using ptr_latex_generator_type = std::unique_ptr<LatexGenerator>;
#endif // EXTENDED


	using path_type = std::string;
	using paths_type = std::vector<std::string>;
private:
	ptr_generator_type codeGenerator;		///< - элемента для итоговой генерации кода
	ptr_signal_reader_type signalReader;	///< - элемент для считывания CSV-сигналов
	ptr_puml_reader_type pumlReader;		///< - элемент для считывания puml диаграмм активности
#ifdef EXTENDED
	ptr_latex_generator_type latexGenerator;
#endif // EXTENDED

	GENERATOR_TYPE generatorType;

	bool wasSignalReadSuccess;				///< - флаг корректности считывания сигналов
	bool wasPumlReadSuccess;				///< - флаг корректности считывания puml-алгоритмов
	bool wasCompareSuccess;					///< - флаг корректности сверки puml и CSV сигналов
	bool wasCheckSuccess;					///< - флаг корректности проверки puml-алгоритмов
	bool wasGenerationSuccess;				///< - флаг корректности генерации кода

	bool tryToExecute(const std::string& command, const std::string& path);

	Wrapper();
public:
	~Wrapper();
	/*
	///\brief - Метод для генерации библиотеки (если wasGenerationSuccess == true и generatorType >= LIBRARY)
	///\return Возвращает true, если генерация прошла успешно
	*/
	bool buildLibrary();
	/*
	///\brief - Метод для генерации библиотеки (если generatorType == LIBRARY)
	///\return Возвращает true, если генерация прошла успешно
	*/
	bool forceToBuildLibrary();
	/*
	///\brief - Метод для генерации кода (если wasCheckSuccess == true)
	///\return Возвращает true, если генерация прошла успешно
	*/
	bool buildCode();
	/*
	///\brief - Метод для генерации кода
	///\return Возвращает true, если генерация прошла успешно
	*/
	bool forceToBuildCode();
	/*
	///\brief - Метод для проверки и первичной обработки результатов считывания puml-алгоритмов (если wasCompareSuccess == true)
	///\return Возвращает true, если проверка и обработка прошли успешно (изменяет данные pumlReader`а)
	*/
	bool checkPumlData();
	/*
	///\brief - Метод для проверки и первичной обработки результатов считывания puml-алгоритмов
	///\return Возвращает true, если проверка и обработка прошли успешно (изменяет данные pumlReader`а)
	*/
	bool forceToCheckPumlData();
	/*
	///\brief - Метод для сверки сигналов (если wasSignalReadSuccess == wasPumlReadSuccess == true)
	///\return Возвращает true, если сигналы CSV соответствуют Puml-сигналам
	*/
	bool compareSignals();
	/*
	///\brief - Метод для сверки сигналов
	///\return Возвращает true, если сигналы CSV соответствуют Puml-сигналам
	*/
	bool forceToCompareSignals();
	/*
	///\brief - Метод для считывания puml-алгоритмов
	///\param[in] paths - Файлы для считывания
	///\return Возвращает true, если считывание было успешно
	*/
	bool readPUMLFiles(const paths_type& paths);
	/*
	///\brief - Метод для считывания CSV-сигналов
	///\param[in] paths - Файлы для считывания
	///\return Возвращает true, если считывание было успешно
	*/
	bool readSignalFiles(const paths_type& paths);
	/*
	///\brief - Метод для установки функции-логирования
	///\param[in] newLog - Новая ф-я логирования
	*/
	void setLog(Loggable::log_type newLog);
	/*
	///\brief - Метод для установки выходной директории
	///\param[in] path - Путь до директории
	*/
	void setOutputLocation(const path_type& path);
	/*
	///\brief - Метод очистки данных
	*/
	void clear();
	/*
	///\brief - Метод создания обертки
	///\tparam - T Словарь языка для генерации
	///\tparam - U Генератор конструкций языка
	///\return - Экземпляр генератора
	*/
	template <typename T, typename U,
		typename X = typename  std::enable_if<std::is_base_of<TypeDictionary, T>::value>,
		typename Y = typename  std::enable_if<std::is_base_of<OneFileGenerator, U>::value>>
		static Wrapper* create();
#ifdef EXTENDED
	std::string getLibPath() const;
	void setLatexSetting();
	std::string genUE(const LatexDataMain& titleData);
	std::string genTables(std::vector<LatexTableData>& tables);
	std::string getTablesOutputPath()const ;
	std::vector<LatexAlgorithmData> getLatexAlgorithmData();
	std::string genSections(const std::vector<LatexSectoinData>& sections);
#endif // EXTENDED

};

template<typename T, typename U, typename X, typename Y>
inline Wrapper* Wrapper::create()
{
	auto ptr = new Wrapper();
	ptr->codeGenerator.reset(generator_type::create<T, U>());
	if (std::is_base_of<LibGenerator, U>::value)
		ptr->generatorType = GENERATOR_TYPE::LIBRARY;
	else if (std::is_base_of<TwoFileGenerator, U>::value)
		ptr->generatorType = GENERATOR_TYPE::TWO_FILE;
	else if (std::is_base_of<OneFileGenerator, U>::value)
		ptr->generatorType = GENERATOR_TYPE::ONE_FILE;
	else
	{
		delete ptr;
		ptr = nullptr;
	}
	return ptr;
}
