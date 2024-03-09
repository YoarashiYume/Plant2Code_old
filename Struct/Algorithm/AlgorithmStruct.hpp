#pragma once
#include <vector>
#include "../Signals/SignalInfo.hpp"
#include "AlgorithmInfo.hpp"
/*
///\brief - Базовый тип элемента алгоритма
*/
struct BaseAlgorithmCommand
{
	using string_type = std::string;
	
	string_type line;							///< Строка алгоритма
	COMMAND_TYPE command;						///< Инструкция, содержащаяся в строке
	BaseAlgorithmCommand() = default;
	BaseAlgorithmCommand(const string_type& currentLine, const COMMAND_TYPE currentCommand) :
		line(currentLine), command(currentCommand) {};
	BaseAlgorithmCommand(string_type&& currentLine, const COMMAND_TYPE currentCommand) :
		line(std::move(currentLine)), command(currentCommand) {};
	virtual ~BaseAlgorithmCommand() = default;
};
/*
///\brief - Элемент алгоритма, содержащий расчет
*/
struct AlgorithmCommand : BaseAlgorithmCommand
{
	using term_type = std::pair<string_type, TERM>;
	using terms_type = std::vector<term_type>;
	
	std::vector<terms_type> terms;				///< - Термы, содержащиеся в строке

	AlgorithmCommand() = default;
	AlgorithmCommand(const string_type& currentLine, const COMMAND_TYPE currentCommand) :
		BaseAlgorithmCommand(currentLine, currentCommand) {};
	AlgorithmCommand(string_type&& currentLine, const COMMAND_TYPE currentCommand) :
		BaseAlgorithmCommand(std::move(currentLine), currentCommand) {};
	virtual ~AlgorithmCommand() = default;
};
/*
///\brief - Элемент вызова функции
*/
struct FunctionAlgorithmCommand : BaseAlgorithmCommand
{
	using term_type = std::pair<string_type, TERM>;
	using args_type = std::vector<term_type>;
	using args_store_type = std::vector<args_type>;

	args_store_type inputFunctionArgs;	///< - Термы входных аргументов
	args_store_type outputFunctionArgs;	///< - Термы выходных аргументов

	string_type calledFunction;					///< - Название вызываемой функции

	FunctionAlgorithmCommand() = default;
	FunctionAlgorithmCommand(const string_type& currentLine, const COMMAND_TYPE currentCommand) :
		BaseAlgorithmCommand(currentLine, currentCommand) {};
	FunctionAlgorithmCommand(string_type&& currentLine, const COMMAND_TYPE currentCommand) :
		BaseAlgorithmCommand(std::move(currentLine), currentCommand) {};
	virtual ~FunctionAlgorithmCommand() = default;
};
/*
///\brief - Данные по Puml-алгоритму
*/
struct AlgorithmInfo
{
	using param_type = std::shared_ptr<ParamName>;
	using param_storage_type = std::vector<param_type>;
	using string_type = std::string;
	using command_type = std::shared_ptr<BaseAlgorithmCommand>;
	using command_store_data = std::vector<command_type>;

#ifdef EXTENDED
	std::string path;
	std::vector <std::string> refs;
#endif // EXPANDED

	std::vector<string_type> usedAlgorithm;	///< - Список используемых алгоритмов
	param_storage_type inputParam;		///< - Список входных параметров для алгоритма
	param_storage_type localParam;		///< - Список локальных параметров для алгоритма
	param_storage_type outerParam;		///< - Список выходных параметров для алгоритма
	command_store_data algorithmData;///< - Команды алгоритма


};