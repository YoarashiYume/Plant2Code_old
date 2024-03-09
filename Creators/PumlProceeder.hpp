#pragma once
#include <memory>

#include "../Common/Loggable.hpp"
#include "../EditableData/Dictionary.hpp"
#include "../EditableData/Generators.hpp"
#include "../Struct/Algorithm/AlgorithmStruct.hpp"
#include "../Struct/Signals/SignalStruct.hpp"

class PumlProceeder : public Loggable
{
public:
	using algorithm_data_type = AlgorithmInfo;
	using store_algorithm_data_type = std::shared_ptr<algorithm_data_type>;
	using algorithm_type = std::shared_ptr<std::unordered_map<std::string, store_algorithm_data_type>>;
	using signal_type = std::shared_ptr<const InfoTable>;
	using term_type = std::pair<std::string, TERM>;
	using term_list = std::vector<term_type>;

	using ref = algorithm_data_type::command_store_data::iterator;
	using ref_pair_type = std::pair<ref, ref>;
	using ref_info = std::unordered_map<std::string, ref_pair_type>;
private:
	algorithm_type currentAlgorithmList;			///< - Список исправляемых алгоритмов
	signal_type currentSignals;						///< - Список используемых сигналов

	
	/*
	///\brief - Метод исправляющий аргументы в вызове функций
	///\param [in] algorithm - Данные алгоритма
	///\param [in, out] functionCall - Данные вызываемой функции
	*/
	void fixFunctionCallArgs(const algorithm_data_type& algorithm, FunctionAlgorithmCommand& functionCall) const;
	/*
	///\brief - Метод исправляющий аргументы алгоритмов
	///\return Если исправление было успешно - true
	*/
	bool fixArgsInFunction();
	/*
	///\brief - Метод раскручивающий алгоритм
	///\param [in, out] algorithmData - Данные алгоритма
	///\param [in, out] gotoData - Информация о переходах
	///\return Новая последовательность шагов алгоритма
	*/
	algorithm_data_type::command_store_data unspinAlgorithm(const algorithm_data_type::command_store_data& algorithmData, const ref_info& gotoData)const;
	/*
	///\brief - Метод собирающий информацию о ссылках
	///\param [in, out] algorithmData - Данные алгоритма
	///\param [in, out] gotoData - Информация о переходах
	///\return Если сбор был успешен - true
	*/
	bool prepareForAlgorithmExpand(algorithm_type::element_type::value_type& algorithmData, ref_info& gotoData) const;
	/*
	///\brief - Метод удаляющий goto ссылки из алгоритма
	///\param [in, out] algorithmData - Данные алгоритма
	///\return Если удаление ссылок было успешно - true
	*/
	bool expandAlgorithm(algorithm_type::element_type::value_type& algorithmData) const;
	/*
	///\brief - Метод исключения пустого блока else
	///\param [in, out] algorithmData - Проверяемый алгоритм
	///\param [in] currentInstruction - endif-инструкция
	///\param [in] algorithmName - Название алгоритма
	///\return Если исключение было успешно - true
	*/
	bool checkEmptyElse(store_algorithm_data_type algorithmData, algorithm_data_type::command_store_data::iterator& currentInstruction, const std::string& algorithmName) const;
	/*
	///\brief - Метод для проверки корректности аргументов вызова
	///\param [in] args - Используемые аргументы
	///\param [in] storage - Переменная для записи аргументов
	///\param [in] algorithmName - Название алгоритма
	///\param [in] calledAlgorithm - Вызываемый алгоритм
	///\return Если проверка была успешна - true
	*/
	bool collectFunctionArgs(const std::vector<std::string> args, FunctionAlgorithmCommand::args_store_type& storage, const std::string& algorithmName, const std::string & calledAlgorithm) const;
	/*
	///\brief - Метод для проверки аргументов, используемых в вызове функции
	///\param [in] currentAlgorithm - Данные о текущем алгоритме
	///\param [in] args - Данные об аргументах алгоритма
	///\param [in] usedArgs - Используемые аргументы
	///\return Если проверка была успешна - true
	*/
	bool checkArgs(const algorithm_data_type& algorithmData, const AlgorithmInfo::param_storage_type& args, const std::string& usedArgs) const;
	/*
	///\brief - Метод для проверки вызова функции
	///\param [in] currentAlgorithm - Данные о текущем алгоритме
	///\param [in, out] commandData - Данные о строке
	///\param [in] algorithmName - Название алгоритма
	///\return Если проверка была успешна - true
	*/
	bool checkFunctionSignature(const algorithm_data_type& currentAlgorithm, algorithm_data_type::command_type currentInstruction, const std::string& algorithmName) const;
	/*
	///\brief - Метод добавляющий термы массива к сигналу
	///\param [in, out] src - Терм сигнала
	///\param [in] dst - Добавляемый массив термов
	*/
	void addArrayTermToWordTerm(term_list & src, const term_list & dst) const;
	/*
	///\brief - Метод для проверки вычислений
	///\param [in, out] currentAlgorithm - Данные о текущем алгоритме
	///\param [in, out] lineTerms - Данные о строке
	///\param [in] algorithmName - Название алгоритма
	///\param [in] currentLine - Текущая строка
	///\return Если проверка успешна - true
	*/
	bool checkCalculation(algorithm_data_type currentAlgorithm, const term_list& lineTerms, const std::string& algorithmName, const std::string& currentLine) const;
	/*
	///\brief - Метод для проверки строки с вычислениями
	///\param [in, out] currentAlgorithm - Данные о текущем алгоритме
	///\param [in, out] lineTerms - Данные о строке
	///\param [in] algorithmName - Название алгоритма
	///\param [in] currentLine - Текущая строка
	///\return Если проверка успешна - true
	*/
	bool checkCalculationInLine(const algorithm_data_type & currentAlgorithm, const term_list & lineTerms, const std::string & algorithmName, const std::string& currentLine) const;
	/*
	///\brief - Метод для проверки строки с логическими вычислениями
	///\param [in, out] currentAlgorithm - Данные о текущем алгоритме
	///\param [in, out] lineTerms - Данные о строке
	///\param [in] algorithmName - Название алгоритма
	///\param [in] currentLine - Текущая строка
	///\return Если проверка успешна - true
	*/
	bool checkLogicSignalsInLine(const algorithm_data_type& currentAlgorithm, const term_list& lineTerms, const std::string& algorithmName, const std::string & currentLine) const;
	/*
	///\brief - Метод для case'ов
	///\param [in] terms - Текущие термы
	///\return Если проверка успешна - true
	*/
	bool checkCase(const term_list& terms) const;
	/*
	///\brief - Метод для проверки строки
	///\param [in] currentAlgorithm - Данные о текущем алгоритме
	///\param [in, out] commandData - Данные о строке
	///\param [in] algorithmName - Название алгоритма
	///\return Если проверка и преобразования успешны - true
	*/
	bool checkSignalsInLine(const algorithm_data_type& currentAlgorithm, algorithm_data_type::command_type commandData, const std::string& algorithmName) const;
	/*
	///\brief - Метод находящий вложенный алгоритм
	///\param [in] terms - Проверяемая строка
	///\param [in] algorithmName - Название алгоритма
	///\param [in] start - Начало поиска
	///\return Если маркировка успешна - true
	*/
	term_list getArrayIndexTerms(const term_list & terms, const std::string& algorithmName, const term_list::size_type startIndex = 0) const;
	/*
	///\brief - Метод определяющий терм
	///\param [in] word - Слово для определения терма
	///\return В случае успеха - терм, иначе TERM::UNKNOWN
	*/
	TERM defineTermByWord(const std::string& word) const;
	/*
	///\brief - Метод разделения строки на слова
	///\param [in] line - Разделяемая
	///\return Слова
	*/
	std::vector<std::string> splitLine(const std::string& line) const;
	/*
	///\brief - Метод маркировки модуля в строке
	///\param [in, out] line - Проверяемая строка
	///\param [in] algorithmName - Название алгоритма
	///\return Если маркировка успешна - true
	*/
	bool markAbsInLine(std::string& line, const std::string& algorithmName) const;
	/*
	///\brief - Метод разбития строки на термы
	///\param [in] line - Разбиваемая строка
	///\param [in] algorithmName - Название алгоритма
	///\return В случае успеха - строка разбитая на термы, иначе TERM::UNKNOWN
	*/
	term_list splitLineOnTerm(const std::string& line, const std::string& algorithmName) const;
	/*
	///\brief - Метод проверки блока switch
	///\param [in, out] algorithmData - Проверяемый алгоритм
	///\param [in, out] currentInstruction - switch-инструкция
	///\param [in] algorithmName - Название алгоритма
	///\return Если проверка успешна - true
	*/
	bool checkSwitchLine(store_algorithm_data_type algorithmData, algorithm_data_type::command_type currentInstruction, const std::string& algorithmName) const;
public:
	/*
	///\brief - Метод для проверки корректности и преобразования puml-алгоритм
	///\param [in, out] newAlgorithm - Первичные данные puml-алгоритма
	///\param[in, out] newSignals - Данные об используемых сигналах
	///\return Если проверка и преобразования успешны - true
	*/
	PumlProceeder(algorithm_type newAlgorithm, signal_type newSignals);
	bool checkPuml();
	
	~PumlProceeder() = default;
};

