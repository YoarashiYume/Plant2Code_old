#include "PumlProceeder.hpp"
#include "../Common/Def.hpp"
#include "../Common/StringFunction.hpp"
#include "../Common/CSVReader.hpp"
#include "../Common/CSVReader.hpp"

#include <iterator>
#include <deque>

static std::set<char> acceptableCharSymbol
{
	'+',
	'-',
	'*',
	'/',
	'^',

	'|',
	'&',
	'>',
	'<',
	'!',
	'=',

	'?',
	':',
	'(',
	')'
};

static std::set<std::string> acceptableSymbol
{
	"+",
	"-",
	"*",
	"/",
	"^",

	"+=",
	"-=",
	"*=",
	"|=",
	"&=",
	"/=",

	"==",
	"!=",
	">=",
	"<=",
	">",
	"<",
	">>",
	"<<",

	"|",
	"&",

	"||",
	"&&",

	"?",
	":",
	"(",
	")"
};
static std::vector<std::string> acceptableAssign
{

	"+=",
	"-=",
	"*=",
	"/=",
	"|=",
	"&=",
	"="

};
static std::vector<std::string> includedFunction
{
	"sqrt",
	"exp"
};


void PumlProceeder::fixFunctionCallArgs(const algorithm_data_type& algorithm, FunctionAlgorithmCommand& functionCall) const
{
	for (decltype(algorithm.outerParam)::size_type i = 0; i < algorithm.outerParam.size(); ++i)
	{
		if (algorithm.outerParam.at(i).unique())
			continue;
		if (algorithm.outerParam.at(i)->name.front() == PLANTUML_INPUT_PAR_CH)
			functionCall.outputFunctionArgs.at(i).clear();
	}
	functionCall.outputFunctionArgs.erase(std::remove_if(
		functionCall.outputFunctionArgs.begin(), functionCall.outputFunctionArgs.end(), [](const FunctionAlgorithmCommand::args_type& arg)
		{
			return arg.empty();
		}), functionCall.outputFunctionArgs.end());
}

PumlProceeder::algorithm_data_type::command_store_data PumlProceeder::unspinAlgorithm(const algorithm_data_type::command_store_data& algorithmData, const ref_info& gotoData) const
{
	algorithm_data_type::command_store_data newIteration;
	for (auto iter = algorithmData.cbegin(); iter < algorithmData.cend(); iter = std::next(iter))
	{
		auto& step = *iter->get();
		if (step.command == COMMAND_TYPE::GOTO_MARKER)
			break;
		if (step.command == COMMAND_TYPE::GOTO)
		{
			auto gotoInfo = gotoData.find(step.line);
			for (auto markerIter = gotoInfo->second.first + 1; markerIter != gotoInfo->second.second; markerIter = std::next(markerIter))
				newIteration.emplace_back(*markerIter);
		}
		else
			newIteration.emplace_back(*iter);
		if (step.command == COMMAND_TYPE::END_MARKER)
			break;
	}
	return newIteration;
}

bool PumlProceeder::prepareForAlgorithmExpand(algorithm_type::element_type::value_type& algorithmData, ref_info& gotoData) const
{
	std::unordered_map < std::string, std::pair<bool, bool> > existedGoto;		//label, isGOTO, isMARKER
	auto& algorithmInfo = algorithmData.second->algorithmData;
	for (auto iter = algorithmInfo.begin(); iter < algorithmInfo.end(); iter = std::next(iter))
	{
		auto& step = *iter->get();
		if (step.command == COMMAND_TYPE::GOTO)
		{
			auto checkIter = existedGoto.find(step.line);
			if (checkIter == existedGoto.end())
				existedGoto.emplace(step.line, std::make_pair(true, false));
			else
				checkIter->second.first = true;
		}
		if (step.line.empty() || (step.command != COMMAND_TYPE::GOTO_MARKER && step.command != COMMAND_TYPE::END_MARKER))
			continue;

		auto insertPlace = gotoData.find(step.line);
		if (step.command == COMMAND_TYPE::END_MARKER)
		{
			if (insertPlace == gotoData.end())
			{
				log("Отсутствует вход в блок алгоритма \"" + step.line + "\". Алгоритм: " + algorithmData.first + ".");
				return false;
			}
			if (insertPlace->second.second != algorithmInfo.end())
			{
				log("Повторный выход из блока алгоритма \"" + step.line + "\". Алгоритм: " + algorithmData.first + ".");
				return false;
			}
			insertPlace->second.second = iter;
		}
		else
		{
			if (insertPlace != gotoData.end())
			{
				log("Повторный вход в блок алгоритма \"" + step.line + "\". Алгоритм: " + algorithmData.first + ".");
				return false;
			}
			gotoData.emplace(std::make_pair(step.line, ref_pair_type{ iter,algorithmInfo.end() }));
			auto checkIter = existedGoto.find(step.line);
			if (checkIter == existedGoto.end())
				existedGoto.emplace(step.line, std::make_pair(false, true));
			else
				checkIter->second.second = true;
		}
	}
	bool isOk{ true };
	for (auto& el : existedGoto)
	{
		if (el.second.second != el.second.first)
		{
			if (el.second.first)
				log("Отсутствует вход в блок \"" + el.first + "\". Алгоритм: " + algorithmData.first + ".");
			else
				log("Отсутствует ссылка на блок \"" + el.first + "\". Алгоритм: " + algorithmData.first + ".");
			isOk = false;
		}
	}
	for (auto& el : gotoData)
	{
		if (el.second.second == algorithmInfo.end())
		{
			log("Отсутствует выход из блока \"" + el.first + "\". Алгоритм: " + algorithmData.first + ".");
			isOk = false;
		}
	}
	return isOk;
}

bool PumlProceeder::expandAlgorithm(algorithm_type::element_type::value_type& algorithmData) const
{
	ref_info gotoData;
	
	if (prepareForAlgorithmExpand(algorithmData, gotoData))
	{
		if (gotoData.empty())
			return true;
		auto newSteps = unspinAlgorithm(algorithmData.second->algorithmData, gotoData);
		for (auto i = 0u; i < MAX_UNSPIN_ITERATION; ++i)
		{
			auto nextStep = unspinAlgorithm(newSteps, gotoData);
			if (nextStep == newSteps)
			{
				algorithmData.second->algorithmData = std::move(newSteps);
				return true;
			}
			newSteps = std::move(nextStep);
		}
		log("Не удалось раскрутить алгоритм \"" + algorithmData.first + "\" за " +std::to_string(MAX_UNSPIN_ITERATION)+ " шагов. Возможно присутствуют циклические ссылки.");
	}
	return false;
}

bool PumlProceeder::checkEmptyElse(store_algorithm_data_type algorithmData, algorithm_data_type::command_store_data::iterator& currentInstruction, const std::string & algorithmName) const
{
	bool isOk{ true };
	if (currentInstruction == algorithmData->algorithmData.begin())
	{
		log("Алгоритм \"" + algorithmName + "\" имеет некорректную инструкцию endif");
		isOk = false;
	}
	else
	{
		auto prevInstruction = std::prev(currentInstruction);
		if (prevInstruction->get()->command == COMMAND_TYPE::ELSE_KEY)
		{
			auto index = std::distance(algorithmData->algorithmData.begin(), prevInstruction);
			algorithmData->algorithmData.erase(prevInstruction);
			currentInstruction = algorithmData->algorithmData.begin() + index;
		}
	}
	return isOk;
}

bool PumlProceeder::collectFunctionArgs(const std::vector<std::string> args, FunctionAlgorithmCommand::args_store_type & storage, const std::string & algorithmName, const std::string & calledAlgorithm) const
{
	for (auto& el : args)
	{
		auto split = splitLineOnTerm(el, algorithmName);
		if (split.front().second == TERM::UNKNOWN)
		{
			log("Некорректный аргумент\"" + el + "\" для вызова алгоритма \""+ calledAlgorithm+"\" в алгоритме: " + algorithmName +".");
			return false;
		}
		for (auto& iel : split)
			StringFunction::removeExtraSpacesFromCodeText(iel.first);
		storage.emplace_back(split);
	}
	return true;
}

bool PumlProceeder::checkArgs(const algorithm_data_type& algorithmData, const AlgorithmInfo::param_storage_type & args, const std::string & usedArgs) const
{
	auto calledArgs =  Readers::CSVLineReader(usedArgs, PLANTUML_FUNCTION_ARGS_SEPARATOR, true);
	if (args.size() != calledArgs.size())
		return false;
	bool isOk{ true };
	for (AlgorithmInfo::param_storage_type::size_type index = 0u; index < args.size(); ++index)
	{
		if (calledArgs.at(index).empty())
			return false;

		if (StringFunction::_isdigit(calledArgs.at(index).front()))
			if (std::all_of(calledArgs.at(index).begin(), calledArgs.at(index).end(), [](const char ch)
			{return StringFunction::_isdigit(ch) || ch == CORRECT_FLOAT_CHAR; }))
				continue;

		StringFunction::removeExtraSpacesFromCodeText(calledArgs.at(index));
		auto signalInfo = currentSignals->getSignalInfo(calledArgs.at(index), algorithmData);
		
		if (signalInfo.isData == false)
		{
			bool isDigit{ true };
			try
			{
				std::ignore = std::stof(calledArgs.at(index));
			}
			catch (...)
			{
				try
				{
					std::ignore = std::stoi(calledArgs.at(index));
				}
				catch (...)
				{
					isDigit = false;
				}
			}
			if (isDigit)
			{
				if (args.at(index)->pumlType.empty())
				{
					log("Значение \"" + calledArgs.at(index) + "\" не может передаваться как таймер.");
					isOk = false;
				}
				else if (args.at(index)->isPtrType)
				{
					log("Значение \"" + calledArgs.at(index) + "\" не может передаваться как указатель.");
					isOk = false;
				}
			}
			else
				isOk = false;
		}
		else
		{
			if (args.at(index)->pumlType.empty())
			{
				if (currentSignals->timers.find(calledArgs.at(index)) == currentSignals->timers.end())
					if (currentSignals->constValues.find(calledArgs.at(index)) == currentSignals->constValues.end())
					{
						if (signalInfo.pumlType.size() || signalInfo.type != PUML_SIGNAL_TYPE::INPUT_ARG)
							isOk = false;
					}
			}
			else
			{
				if (args.at(index)->elementCount != signalInfo.elementCount ||
					args.at(index)->isPtrType != signalInfo.isPtrType)
					isOk = false;
				if (args.at(index)->pumlType != signalInfo.pumlType)
					if (signalInfo.type != PUML_SIGNAL_TYPE::CONST_SIGN)
						isOk = false;
			}
		}
	}

	return isOk;
}

bool PumlProceeder::checkFunctionSignature(const algorithm_data_type& currentAlgorithm, algorithm_data_type::command_type currentInstruction, const std::string & algorithmName) const
{
	auto functionInstruction = std::dynamic_pointer_cast<FunctionAlgorithmCommand>(currentInstruction);
	if (functionInstruction == nullptr)
	{
		log("Некорректно считана линия \"" + currentInstruction->line + "\". Алгоритма: " + algorithmName);
		return false;
	}
	functionInstruction->calledFunction = functionInstruction->line;
	auto argPos = functionInstruction->calledFunction.find(LEFT_SIMPLE_BRACKET);
	if (argPos != std::string::npos)
		functionInstruction->calledFunction = 
			StringFunction::removeExtraSpacesFromCodeText(functionInstruction->calledFunction.substr(0, argPos));
	
	auto calledAlgorithm = currentAlgorithmList->find(functionInstruction->calledFunction);
	if (calledAlgorithm == currentAlgorithmList->end())
	{
		log("Невозможно определить вызываемый алгоритм \"" + functionInstruction->calledFunction + "\" в алгоритме \"" + algorithmName + "\"");
		return false;
	
	}
	if (argPos == std::string::npos)
	{
		auto isArgs = calledAlgorithm->second->inputParam.empty() &&
			calledAlgorithm->second->outerParam.empty();
		if (isArgs == false)
			log("Отсутствуют аргументы для вызова ф-ии \"" + functionInstruction->calledFunction + "\" в алгоритме \"" + algorithmName + "\"");
		return isArgs;
	}
	auto inputArgs = functionInstruction->line.substr(argPos + 1,
		functionInstruction->line.find(RIGHT_SIMPLE_BRACKET) - (argPos + 1));
	argPos = functionInstruction->line.find(LEFT_SIMPLE_BRACKET,
		functionInstruction->line.find(RIGHT_SIMPLE_BRACKET));
	auto outputArgs = functionInstruction->line.substr(argPos + 1,
		functionInstruction->line.find(RIGHT_SIMPLE_BRACKET,
			functionInstruction->line.find(RIGHT_SIMPLE_BRACKET) + 1) - (argPos + 1));

	bool mainCompare = checkArgs(currentAlgorithm, calledAlgorithm->second->inputParam, inputArgs);
	if (mainCompare == false)
		log("Входные аргументы \"(" + inputArgs + ")\" в вызове ф-ии \""
			+ functionInstruction->calledFunction + "\" не совпадают со входными аргументами функции \"(" +
			StringFunction::argsToString(calledAlgorithm->second->inputParam, ",")
			+ ")\". Алгоритм \"" + algorithmName + "\"");
	if (checkArgs(currentAlgorithm, calledAlgorithm->second->outerParam, outputArgs) == false)
	{
		log("Выходные аргументы \"(" + outputArgs + ")\" в вызове ф-ии \"" 
			+ functionInstruction->calledFunction + "\" не совпадают со входными аргументами функции \"(" 
			+ StringFunction::argsToString(calledAlgorithm->second->outerParam, ",")
			+ ")\". Алгоритм \"" + algorithmName + "\"");
		mainCompare = false;
	}
	if (mainCompare)
	{
		auto inputArgs_ = StringFunction::removeExtraSpacesFromCodeText( Readers::CSVLineReader(inputArgs, ','));
		auto outputArgs_ = StringFunction::removeExtraSpacesFromCodeText(Readers::CSVLineReader(outputArgs, ','));
		auto mainIndex{ 0 };
		for (auto& el : calledAlgorithm->second->inputParam)
		{
			if (el.use_count() != 1)
			{
				auto subIndex{ 0 };
				for (auto& outputEl : calledAlgorithm->second->outerParam)
				{
					if (outputEl->name == el->name)
					{
						if (inputArgs_.at(mainIndex) != outputArgs_.at(subIndex))
						{
							log("Некорректно заданы аргументы для вызова ф-ии \"" + functionInstruction->calledFunction + "\" в алгоритме \""
								+ algorithmName + "\". Аргумент \"" + inputArgs_.at(mainIndex) + "\" должен соответствовать аргументу \"" + outputArgs_.at(subIndex) + "\".");
							return false;
						}

					}
					++subIndex;
				}
			}
			++mainIndex;
		}
		if (collectFunctionArgs(inputArgs_,
			functionInstruction->inputFunctionArgs, algorithmName,
			functionInstruction->calledFunction) == false ||
			collectFunctionArgs(outputArgs_,
				functionInstruction->outputFunctionArgs, algorithmName,
				functionInstruction->calledFunction) == false)
			return false;
	}
	return true;
}

void PumlProceeder::addArrayTermToWordTerm(term_list & src, const term_list & dst) const
{

	bool canAddP = std::all_of(dst.begin(), dst.end(), [](const term_list::value_type & el)
	{
		switch (el.second)
		{
		case TERM::ARRAY_INDEX_CLOSE:
		case TERM::ARRAY_INDEX_OPEN:
		case TERM::VALUE:
		case TERM::SIGNAL:
		case TERM::PART_SIGNAL:
			return true;
		default:
			return false;
		}
	});
	src.emplace_back(std::make_pair(std::string{ "[" }, TERM::ARRAY_INDEX_OPEN));
	if (canAddP)
		std::copy(dst.begin(), dst.end(), std::back_inserter(src));
	else
		src.emplace_back(std::make_pair(std::string{ "$" }, TERM::UNKNOWN));
	src.emplace_back(std::make_pair(std::string{ "]" }, TERM::ARRAY_INDEX_CLOSE));
}

bool PumlProceeder::checkCalculation(algorithm_data_type currentAlgorithm, const term_list & lineTerms, const std::string & algorithmName, const std::string & currentLine) const
{
	if (lineTerms.empty())
	{
		log("Есть некорректная строка: \"" + currentLine + "\".В алгоритме \"" + algorithmName + "\"");
		return false;
	}
	else if (lineTerms.size() == 1)
	{
		if (lineTerms.front().second != TERM::SIGNAL && lineTerms.front().second != TERM::VALUE)
		{
			log("Есть некорректная строка: \"" + currentLine + "\".В алгоритме \"" + algorithmName + "\"");
			return false;
		}
		return true;
	}
	return checkLogicSignalsInLine(currentAlgorithm, lineTerms, algorithmName, currentLine);
}

bool PumlProceeder::checkCalculationInLine(const algorithm_data_type & currentAlgorithm, const term_list & lineTerms, const std::string & algorithmName, const std::string& currentLine) const
{
	auto pos = std::find_if(lineTerms.begin(), lineTerms.end(), [](const term_list::value_type & el)
	{
		return el.second == TERM::ASSIGN;
	});
	if (pos == lineTerms.end())
	{
		log("Строка \"" + currentLine + "\" не содержит символа \"=\". Алгоритм: " + algorithmName);
		return false;
	}
	else
	{
		if (checkCalculation(currentAlgorithm, { lineTerms.begin(), pos },  algorithmName, currentLine) == false)
			return false;
		auto index = std::distance(lineTerms.begin(), pos) + 1;
		for (term_list::const_iterator iter = lineTerms.cbegin(); iter != lineTerms.cend(); )
		{
			iter = std::find_if(lineTerms.begin() + index, lineTerms.end(), [](const term_list::value_type termPair)
			{
				return termPair.second == TERM::TERNAR_SEPARATOR || termPair.second == TERM::TERNAR_START;
			});
			term_list termCopy{ lineTerms.cbegin() + index, iter };
			index = std::distance(lineTerms.cbegin(), iter) + 1;
			if (checkCalculation(currentAlgorithm, termCopy, algorithmName, currentLine) == false)
				return false;
		}
	}
	return true;
}

bool PumlProceeder::checkLogicSignalsInLine(const algorithm_data_type & currentAlgorithm, const term_list & lineTerms, const std::string & algorithmName, const std::string & currentLine) const
{
	std::string currentValue;
	bool isIncorDec{ false };
	term_list words;
	auto beforeClearSignal = [&]() -> bool
	{
		if (words.size())
		{
			std::string signal;
			for (auto& el : words)
				signal.append(el.first);
			words.clear();
			if (currentSignals->getSignalInfo(signal, currentAlgorithm).isData == false)
			{
				log("Строка \"" + currentLine + "\" алгоритма \"" + algorithmName +
					"\" содержит неизвестный сигнал \"" + signal + "\".");
				return false;
			}
			return true;
		}
		return true;
	};
	for (term_list::size_type index = 0; index < lineTerms.size(); ++index)
	{
		auto & term = lineTerms.at(index).second;
		auto prevTerm = index == 0 ? TERM::NONE : lineTerms.at(index - 1).second;
		switch (term)
		{
		case TERM::BRACKET_CLOSE:
		case TERM::ABS_CLOSE:
			if (prevTerm != TERM::SIGNAL &&	prevTerm != TERM::PART_SIGNAL &&
				prevTerm != TERM::VALUE && prevTerm != TERM::ARRAY_INDEX_CLOSE && 
				prevTerm != TERM::ABS_CLOSE && prevTerm != TERM::BRACKET_CLOSE)
			{
				log("Строка \"" + currentLine + "\" алгоритма \"" + algorithmName +
					"\" содержит некорректное закрытие " + (term == TERM::BRACKET_CLOSE? "скобки." : "модуля."));
				return false;
			}
			if (beforeClearSignal() == false)
				return false;
			continue;
		case TERM::SQRT:
		case TERM::EXP:
		case TERM::ABS_OPEN:
		case TERM::BRACKET_OPEN:
			if (prevTerm != TERM::SYMBOL && prevTerm != TERM::ABS_OPEN &&
				prevTerm != TERM::BRACKET_OPEN && prevTerm != TERM::NONE &&
				prevTerm != TERM::ASSIGN && prevTerm != TERM::POW &&
				prevTerm != TERM::TERNAR_START && prevTerm != TERM::TERNAR_SEPARATOR)
			{
				if (term != TERM::SQRT || term != TERM::EXP)
					if (prevTerm == TERM::SQRT || prevTerm == TERM::EXP)
						continue;
				log("Строка \"" + currentLine + "\" алгоритма \"" + algorithmName +
					"\" содержит некорректный терм " + (term == TERM::SQRT ? "корня." :
						term == TERM::EXP ? "экспоненты." : 
						term == TERM::ABS_OPEN ? "открытия модуля." : "открытия скобки."));
				return false;
			}
			continue;
		case TERM::PART_SIGNAL:
			if (words.empty())
			{
				log("Строка \"" + currentLine + "\" алгоритма \"" + algorithmName +
					"\" содержит некорректное обращение к структуре.");
				return false;
			}
			words.emplace_back(lineTerms.at(index));
			continue;
		case TERM::SIGNAL:
		case TERM::VALUE:
			if (prevTerm == TERM::SIGNAL || prevTerm == TERM::VALUE ||
				prevTerm == TERM::PART_SIGNAL || prevTerm == TERM::ABS_CLOSE ||
				prevTerm == TERM::BRACKET_CLOSE || prevTerm == TERM::ARRAY_INDEX_CLOSE ||
				prevTerm == TERM::SQRT || prevTerm == TERM::EXP || 
				(term == TERM::VALUE && prevTerm == TERM::INC_OR_DEC))
			{
				log("Строка \"" + currentLine + "\" алгоритма \"" + algorithmName +
					"\" содержит некорректн" + (term == TERM::SIGNAL ? "ый сигнал." : "ое значение."));
				return false;
			}
			if (term == TERM::SIGNAL)
				words.emplace_back(lineTerms.at(index));
			continue;
		case TERM::ASSIGN:
			if (words.empty() || lineTerms.front().second == TERM::INC_OR_DEC)
			{
				log("Строка \"" + currentLine + "\" алгоритма \"" + algorithmName +
					"\" содержит некорректное присваивание.");
				return false;
			}
			if (beforeClearSignal() == false)
				return false;
			continue;
		case TERM::POW:
			if (words.empty() && prevTerm != TERM::BRACKET_CLOSE && prevTerm != TERM::ABS_CLOSE)
			{
				log("Строка \"" + currentLine + "\" алгоритма \"" + algorithmName +
					"\" содержит некорректную степень.");
				return false;
			}
			if (beforeClearSignal() == false)
				return false;
			continue;
		case TERM::INC_OR_DEC:
			if (prevTerm != TERM::SYMBOL && prevTerm != TERM::BRACKET_OPEN &&
				prevTerm != TERM::ABS_OPEN && prevTerm != TERM::ARRAY_INDEX_OPEN &&
				prevTerm != TERM::NONE && prevTerm != TERM::ASSIGN &&
				prevTerm != TERM::TERNAR_START && prevTerm != TERM::TERNAR_SEPARATOR && 
				prevTerm != TERM::POW)
			{
				log("Строка \"" + currentLine + "\" алгоритма \"" + algorithmName +
					"\" содержит некорректный инкремент/декремент.");
				return false;
			}
			continue;
		case TERM::SYMBOL:
			if (prevTerm != TERM::VALUE && prevTerm != TERM::SIGNAL &&
				prevTerm != TERM::PART_SIGNAL && prevTerm != TERM::ABS_CLOSE &&
				prevTerm != TERM::ARRAY_INDEX_CLOSE && prevTerm != TERM::BRACKET_CLOSE)
			{
				if ((prevTerm == TERM::SYMBOL || prevTerm == TERM::ABS_OPEN ||
					prevTerm == TERM::ARRAY_INDEX_OPEN || prevTerm == TERM::BRACKET_OPEN) && lineTerms.at(index).first == MINUS_SYMBOL)
				{
					if (index == 0 || lineTerms.at(index - 1).first == MINUS_SYMBOL)
					{
						log("Строка \"" + currentLine + "\" алгоритма \"" + algorithmName +
							"\" содержит некорректное использование символа '-'");
						return false;
					}
				}
				else
				{
					if (index == 0 && lineTerms.at(index).first == MINUS_SYMBOL)
					{
					}
					else
					{
						log("Строка \"" + currentLine + "\" алгоритма \"" + algorithmName +
							"\" содержит некорректное вычисление " + lineTerms.at(index).first);
						return false;
					}
				}
			}
			if (beforeClearSignal() == false)
				return false;
			continue;
		case TERM::ARRAY_INDEX_OPEN:
			if (prevTerm != TERM::SIGNAL && prevTerm != TERM::PART_SIGNAL&&
				prevTerm != TERM::ARRAY_INDEX_CLOSE)
			{
				log("Строка \"" + currentLine + "\" алгоритма \"" + algorithmName + "\" содержит некорректное обращение к массиву");
				return false;
			}
			else
			{
				auto newArray = getArrayIndexTerms(lineTerms, algorithmName, index);
				addArrayTermToWordTerm(words, newArray);
				if (checkCalculation(currentAlgorithm, newArray, algorithmName, currentLine) == false)
					return false;
				index += newArray.size() + 1;
			}
			continue;
		default:
			break;
		}
	}
	return beforeClearSignal();
}

bool PumlProceeder::checkCase(const term_list & terms) const
{
	std::int32_t arrayTermCount{ 0 };
	std::uint32_t termCount{ 0 };
	for (auto& el : terms)
	{
		if (el.second == TERM::ARRAY_INDEX_OPEN)
			++arrayTermCount;
		else if (el.second == TERM::ARRAY_INDEX_CLOSE)
			if (--arrayTermCount < 0)
				return false;
		if (arrayTermCount == 0)
			if (el.second == TERM::SIGNAL ||
				el.second == TERM::SYMBOL ||
				el.second == TERM::VALUE)
				++termCount;
	}
	return termCount == CASE_WORD_COUNT;
}

bool PumlProceeder::checkSignalsInLine(const algorithm_data_type & currentAlgorithm, algorithm_data_type::command_type commandData, const std::string & algorithmName) const
{
	auto termList = splitLineOnTerm(commandData->line, algorithmName);
	if (termList.size() == 1 && commandData->command != COMMAND_TYPE::CASE_KEY)
		return false;
	bool isTernar{false};
	std::int32_t ternarCount{ 0 };
	for (auto& el : termList)
	{
		if (el.second == TERM::TERNAR_START)
		{
			++ternarCount;
			isTernar = true;
		}
		else if (el.second == TERM::TERNAR_SEPARATOR)
			if (--ternarCount < 0)
				break;
	}
	if (ternarCount != 0)
	{
		log("Невозможно корректно разбить тернарный оператор в строке \"" + commandData->line + "\". Алгоритм: " + algorithmName);
		return false;
	}
	switch (commandData->command)
	{
	case COMMAND_TYPE::CASE_KEY:
		if (isTernar)
		{
			log("Тернарный оператор не может использоваться в логических блоках: " + commandData->line + "\". Алгоритма: " + algorithmName);
			return false;
		}
		return checkCase(termList);
	case COMMAND_TYPE::IF_KEY:
	case COMMAND_TYPE::ELIF_KEY:
	case COMMAND_TYPE::WHILE_KEY:
	{
		if (isTernar)
		{
			log("Тернарный оператор не может использоваться в логических блоках: " + commandData->line + "\". Алгоритма: " + algorithmName);
			return false;
		}
		if (commandData->command == COMMAND_TYPE::CASE_KEY)
			return checkCase(termList);
		else
		{
			auto result = checkLogicSignalsInLine(currentAlgorithm, termList, algorithmName, commandData->line);
			auto* ptr = dynamic_cast<AlgorithmCommand*>(commandData.get());
			if (ptr == nullptr)
			{
				log("Некорректно считана линия \"" + commandData->line + "\". Алгоритма: " + algorithmName);
				result = false;
			}
			if (result)
				ptr->terms.emplace_back(std::move(termList));
			return result;
		}
	}
	case COMMAND_TYPE::CALCULATION:
	{
		std::vector<term_list> data;
		auto iter = std::find_if(termList.cbegin(), termList.cend(), [](const term_list::value_type& el)
		{
			return el.second == TERM::ASSIGN;
		});
		do
		{
			iter = std::find_if(iter + 1, termList.cend(), [](const term_list::value_type& el)
			{
				return el.second == TERM::ASSIGN;
			});
			if (iter == termList.end())
				data.emplace_back(term_list{ termList.begin(), termList.end() });
			else
			{
				data.emplace_back(term_list{ termList.cbegin(), std::prev(iter) });
				termList.erase(termList.begin(), std::prev(iter));
				iter = std::find_if(termList.cbegin(), termList.cend(), [](const term_list::value_type& el)
				{
					return el.second == TERM::ASSIGN;
				});
			}
		} while (iter != termList.end());
		bool result{ true };
		for (auto& line : data)
			result &= checkCalculationInLine(currentAlgorithm, line, algorithmName, commandData->line);
		auto* ptr = dynamic_cast<AlgorithmCommand*>(commandData.get());
		if (ptr == nullptr)
		{
			log("Некорректно считана линия \"" + commandData->line + "\". Алгоритма: " + algorithmName);
			result = false;
		}
		if (result)
			ptr->terms = std::move(data);
		return result;
	}
	default:
		log("Некорректная строка \"" + commandData->line + "\". Алгоритм: " + algorithmName);
		return false;
	}
	return true;
}

PumlProceeder::term_list PumlProceeder::getArrayIndexTerms(const term_list & terms, const std::string & algorithmName, const term_list::size_type startIndex) const
{
	auto start = std::find_if(terms.begin() + startIndex, terms.end(), [](const term_list::value_type& term)
	{
		return term.second == TERM::ARRAY_INDEX_OPEN;
	});
	if (start == terms.cend() || start == terms.cbegin())
		return{};
	auto pos = static_cast<term_list::size_type>(std::distance(terms.begin(), start) + 1);
	std::int32_t brCount{ 1 };
	for (; pos < terms.size(); ++pos)
	{
		if (terms.at(pos).second == TERM::ARRAY_INDEX_OPEN)
			++brCount;
		else if (terms.at(pos).second == TERM::ARRAY_INDEX_CLOSE)
			if (--brCount == 0)
				break;
	}
	auto end = terms.cbegin() + pos;
	if (end >= terms.end())
		return{};
	auto term = std::prev(start);
	if (term->second != TERM::SIGNAL && term->second != TERM::PART_SIGNAL)
		return{};
	return{ std::next(start), end };
}

TERM PumlProceeder::defineTermByWord(const std::string & word) const
{
	TERM wordTerm{ TERM::UNKNOWN };
	if (word.empty())
		return wordTerm;
	if (StringFunction::_isupper(word.front()) ||
		((word.front() == TERM_ABS || word.back() == TERM_ABS) && word.size() > 2))
	{
		std::string tempWord{ word };
		if(tempWord.front() == TERM_ABS || tempWord.back() == TERM_ABS)
			if (tempWord.front() != tempWord.back())
				return wordTerm;
			else
				tempWord = tempWord.substr(1, tempWord.size() - 2);
		if (tempWord.size() > 1)
			if (std::adjacent_find(tempWord.begin(), tempWord.end(), 
				[](const char lch, const char rch) {return (lch == rch && rch == STRUCT_FIELD_SYMBOL); })
				!= tempWord.end())
				return wordTerm;
		if (std::all_of(std::next(tempWord.begin()), tempWord.end(), 
			[](const char ch) { return (StringFunction::_isdigit(ch) || ch == '.'); }))
			wordTerm = TERM::SIGNAL;
	}
	else if (word.front() == STRUCT_FIELD_SYMBOL)
	{
		if (std::all_of(word.begin(), word.end(), [](const char ch)
		{
			return StringFunction::_isdigit(ch) || ch == STRUCT_FIELD_SYMBOL;
		}))
			wordTerm = TERM::PART_SIGNAL;
	}
	else if (StringFunction::_isdigit(word.front()))
	{
		auto dotFirstPos = word.find(FLOAT_SIGN);
		auto dotLastPos = word.find_last_of(FLOAT_SIGN);
		if (dotLastPos == dotFirstPos && dotFirstPos != 0 && dotFirstPos != (word.size() - 1))
			if (std::all_of(word.begin(), word.end(), [](const char ch)
			{
				return StringFunction::_isdigit(ch) || ch == FLOAT_SIGN;
			}))
				wordTerm = TERM::VALUE;
	}
	else if (word == TERM_ARRAY_SIGN_STR)
		wordTerm = TERM::ARRAY_INDEX_OPEN;
	else if (word == TERM_ABS_START_STR)
		wordTerm = TERM::ABS_OPEN;
	else if (word == TERM_ABS_END_STR)
		wordTerm = TERM::ABS_CLOSE;
	else if (word == TERM_TERNAR_START)
		wordTerm = TERM::TERNAR_START;
	else if (word == TERM_TERNAR_SEPARATOR)
		wordTerm = TERM::TERNAR_SEPARATOR;
	else if (std::find(acceptableAssign.begin(), acceptableAssign.end(), word) != acceptableAssign.end())
		wordTerm = TERM::ASSIGN;
	else if (word == TERM_ARRAY_INC || word == TERM_ARRAY_DEC)
		wordTerm = TERM::INC_OR_DEC;
	else
	{
		if (acceptableSymbol.find(word) != acceptableSymbol.end())
		{
			if (word == TERM_LEFT_SIMPLE_BRACKET)
				wordTerm = TERM::BRACKET_OPEN;
			else if (word == TERM_RIGHT_SIMPLE_BRACKET)
				wordTerm = TERM::BRACKET_CLOSE;
			else if (word == TERM_POW)
				wordTerm = TERM::POW;
			else
				wordTerm = TERM::SYMBOL;
		}
		else
		{
			auto iter = std::find(includedFunction.begin(), includedFunction.end(), word);
			if (iter != includedFunction.end())
				wordTerm = static_cast<TERM>(
					static_cast<std::uint8_t>(TERM::SQRT) + std::distance(includedFunction.begin(), iter));
		}
	}
	return wordTerm;
}

std::vector<std::string> PumlProceeder::splitLine(const std::string & line) const
{
	std::string currentWord;
	std::vector<std::string> splittedLine;
	for (auto& ch : line)
	{
		if (ch == SPACE_SIGN || ch == LEFT_SIMPLE_BRACKET ||
			ch == RIGHT_SIMPLE_BRACKET || ch == TERM_ABS_START ||
			ch == TERM_ABS_END || ch == TERM_ARRAY_SIGN)
		{
			if (currentWord.size())
			{
				splittedLine.emplace_back();
				std::swap(splittedLine.back(), currentWord);
			}
			if (ch != SPACE_SIGN)
				splittedLine.emplace_back(std::string{ ch });
			continue;
		}
		if (acceptableCharSymbol.find(ch) == acceptableCharSymbol.end())
		{
			if (std::any_of(currentWord.begin(), currentWord.end(), [](const char ch)
			{
				return acceptableCharSymbol.find(ch) != acceptableCharSymbol.end();
			}))
			{
				if (currentWord.size())
				{
					if ((currentWord.front() != TERM_ABS) || 
						(currentWord.size() != 1 && std::all_of(std::next(currentWord.begin()), currentWord.end(), [](const char ch)
					{
						return acceptableCharSymbol.find(ch) != acceptableCharSymbol.end();
					})))
					{
						splittedLine.emplace_back();
						std::swap(splittedLine.back(), currentWord);
					}
				}
			}
		}
		else
		{
			if (std::all_of(currentWord.begin(), currentWord.end(), [](const char ch)
			{
				return acceptableCharSymbol.find(ch) == acceptableCharSymbol.end();
			}))
			{
				if (currentWord.size())
				{
					splittedLine.emplace_back();
					std::swap(splittedLine.back(), currentWord);
				}
			}
		}
		currentWord += ch;
	}
	if (currentWord.size())
		splittedLine.emplace_back(std::move(currentWord));
	return splittedLine;
}

bool PumlProceeder::markAbsInLine(std::string & line, const std::string & algorithmName) const
{
	if (line.size() < 3)
		return true;
	auto lineCopy = line;
	std::vector<std::pair<std::string::size_type, std::string::value_type>> indexPlaces;
	auto pos = line.find(TERM_ABS);
	while (pos != std::string::npos)
	{
		if (pos == 0)
		{
			if (StringFunction::_isalnum(line.at(pos + 1)))
				line.at(pos) = TERM_ABS_START;
		}
		else if (pos == line.size() - 1)
		{
			if (StringFunction::_isdigit(line.at(pos - 1)) ||
				line.at(pos - 1) == RIGHT_SQARE_BRACKET || line.at(pos - 1) == TERM_ABS_END)
				line.at(pos) = TERM_ABS_END;
		}
		else
		{
			if (line.at(pos - 1) != TERM_ABS && line.at(pos + 1) != TERM_ABS)
			{
				if (line.at(pos + 1) != ASSIGN_SIGN)
				{
					if (!StringFunction::_isspace(line.at(pos - 1)) && line.at(pos - 1) != LEFT_SIMPLE_BRACKET)
						line.at(pos) = TERM_ABS_END;
					else if (line.at(pos - 1) == LEFT_SIMPLE_BRACKET)
						line.at(pos) = TERM_ABS_START;
					else if (!StringFunction::_isspace(line.at(pos + 1)))
						line.at(pos) = TERM_ABS_START;
				}
			}
			else if (line.at(pos - 1) != TERM_ABS)
				if (StringFunction::_isalnum(line.at(pos - 1)) || line.at(pos - 1) == TERM_ABS_END)
					line.at(pos) = TERM_ABS_END;

		}
		pos = line.find(TERM_ABS, pos + 1);
	}
	auto balance = StringFunction::checkStringBalance(line, TERM_ABS_START, TERM_ABS_END) == 0;
	if (balance == false)
	{
		log("В строке \"" + lineCopy + "\") Некорректное кол-во символов модуля. Алгоритм: " + algorithmName);
		return false;
	}
	return balance;
}

PumlProceeder::term_list PumlProceeder::splitLineOnTerm(const std::string & line, const std::string & algorithmName) const
{
	auto workLine = line;
#define errorReturn  term_list{ std::make_pair(std::string{}, TERM::UNKNOWN) }
	if (markAbsInLine(workLine, algorithmName) == false)
		return errorReturn;
	std::deque<term_list> innerData;
	while (workLine.find(LEFT_SQARE_BRACKET) != std::string::npos)
	{
		std::string::size_type pos;
		StringFunction::checkStringBalance(workLine, LEFT_SQARE_BRACKET, RIGHT_SQARE_BRACKET, &pos);
		if (pos == std::string::npos)
		{
			log("Некорректное обозначение массива \"" + line + "\". Алгоритм: " + algorithmName);
			return errorReturn;
		}
		auto startArrayPos = workLine.find(LEFT_SQARE_BRACKET);
		auto innerBlockData = splitLineOnTerm(workLine.substr(startArrayPos +1, pos - startArrayPos - 1),
			algorithmName);
		if (innerBlockData == errorReturn)
			return errorReturn;
		workLine.replace(workLine.begin() + startArrayPos,
			workLine.begin() + pos + 1, TERM_ARRAY_HOLDER);
		innerData.emplace_back(std::move(innerBlockData));
	}
	auto splittedLine = splitLine(workLine);
	term_list result;
	for (auto& word : splittedLine)
	{
		auto term = defineTermByWord(word);
		switch (term)
		{
		case TERM::UNKNOWN:
			log("Невозможно определить тип терма \"" + word + "\" в строке \"" + line + "\". Алгоритм: " + algorithmName);
			return errorReturn;
		case TERM::ARRAY_INDEX_OPEN:
			result.emplace_back(std::string{}, TERM::ARRAY_INDEX_OPEN);
			word.clear();
			result.insert(result.end(), std::make_move_iterator(innerData.front().begin()),
				std::make_move_iterator((innerData.front().end())));
			innerData.pop_front();
			term = TERM::ARRAY_INDEX_CLOSE;
			break;
		case TERM::ABS_OPEN:
		case TERM::ABS_CLOSE:
			word = TERM_ABS_STR;
			break;
		default:
			break;
		}
		result.emplace_back(std::move(word), term);
	}
#undef errorReturn
	return result;
}

bool PumlProceeder::checkSwitchLine(store_algorithm_data_type algorithmData, algorithm_data_type::command_type currentInstruction, const std::string & algorithmName) const
{
	bool isOk{true};
	auto pos = std::find(algorithmData->algorithmData.begin(),
		algorithmData->algorithmData.end(), currentInstruction);
	term_list whatCompared, value;
	bool isValue{ false };
	for (; pos != algorithmData->algorithmData.end(); ++pos)
	{
		if (pos->get()->command != COMMAND_TYPE::CASE_KEY)
			continue;
		auto terms = splitLineOnTerm(pos->get()->line, algorithmName);
		if (terms.front().second == TERM::UNKNOWN)
		{
			isOk = false;
			break;
		}
		if (whatCompared.empty())
		{
			for (auto& el : terms)
			{
				if (isValue)
				{
					value.emplace_back(el);
				}
				else
				{
					if (el.second == TERM::ARRAY_INDEX_OPEN)
					{
						auto vec = getArrayIndexTerms(terms, algorithmName);
						whatCompared.emplace_back(std::make_pair(std::string{}, TERM::ARRAY_INDEX_OPEN));
						whatCompared.insert(whatCompared.end(), std::make_move_iterator(vec.begin()),
							std::make_move_iterator(vec.end()));
						whatCompared.emplace_back(std::make_pair(std::string{}, TERM::ARRAY_INDEX_CLOSE));
					}
					else if (el.second == TERM::SIGNAL || el.second == TERM::PART_SIGNAL)
						whatCompared.emplace_back(el);
					else
					{
						if (whatCompared.empty())
							isOk = false;
						isValue = true;
					}
				}
			}
		}
		else
		{
			if (whatCompared.size() > terms.size())
			{
				log("Была некорректно считана инструкция case. Алгоритм: " + algorithmName);
				isOk = false;
			}
			else
			{
				for (auto i = 0u; i < whatCompared.size(); ++i)
					isOk &= whatCompared.at(i) == terms.at(i);

				auto* ptr = dynamic_cast<AlgorithmCommand*>(pos->get());
				if (ptr == nullptr)
				{
					log("Была некорректно считана инструкция case. Строка " +
						pos->get()->line + ". Алгоритм: " + algorithmName);
					isOk = false;
				}
				else
				{
					ptr->line.clear();
					ptr->terms.clear();
					ptr->terms.emplace_back(AlgorithmCommand::terms_type{});
					for (auto i = whatCompared.size() + 1; i < terms.size(); ++i)
					{
						StringFunction::appendString(terms.at(i).first, ptr->line);
						ptr->terms.back().emplace_back(terms.at(i));
					}
				}
			}
		}
	}
	return isOk;
}

PumlProceeder::PumlProceeder(algorithm_type newAlgorithm, signal_type newSignals)
{
	currentAlgorithmList = newAlgorithm;
	currentSignals = newSignals;
}

bool PumlProceeder::checkPuml()
{
	if (currentAlgorithmList == nullptr || currentSignals == nullptr)
	{
		currentAlgorithmList.reset();
		currentSignals.reset();
		log("Не передан один из компонентов для проверки puml-кода.");
		return false;
	}
	bool isOk{ true };
	for (auto& algorithm : *currentAlgorithmList)
	{
		bool isAlgorithmOk{true};
		for (auto iter = algorithm.second->algorithmData.begin();
			iter != algorithm.second->algorithmData.end(); iter = std::next(iter))
		{
			auto& instruction = *iter;
			switch (instruction->command)
			{
			case COMMAND_TYPE::SWITCH_COMMENT:
				isAlgorithmOk &= checkSwitchLine(algorithm.second, *iter, algorithm.first);
				break;
			case COMMAND_TYPE::IF_KEY:
			case COMMAND_TYPE::ELIF_KEY:
			case COMMAND_TYPE::WHILE_KEY:
			case COMMAND_TYPE::CASE_KEY:
			case COMMAND_TYPE::CALCULATION:		
				isAlgorithmOk &= checkSignalsInLine(*algorithm.second, *iter, algorithm.first);
				break;
			case COMMAND_TYPE::FUNCTION:
				isAlgorithmOk &= checkFunctionSignature(*algorithm.second, *iter, algorithm.first);
				break;
			case COMMAND_TYPE::END_IF_KEY:
				isAlgorithmOk &= checkEmptyElse(algorithm.second, iter, algorithm.first);
				break;
			default:
				continue;
			}
		}
		if (isAlgorithmOk)
			isOk &= expandAlgorithm(algorithm);
		auto iter = std::find_if(currentAlgorithmList->begin(), currentAlgorithmList->end(), [&algorithm](const algorithm_type::element_type::value_type& el)
			{
				return std::find(el.second->usedAlgorithm.begin(), el.second->usedAlgorithm.end(), algorithm.first) != el.second->usedAlgorithm.end();
			});
		if (iter == currentAlgorithmList->end())
			log("Алгоритм \"" + algorithm.first + "\" не используется (Не является критической ошибкой).");
	}
	return isOk && fixArgsInFunction();
}

static void determArg(PumlProceeder::algorithm_data_type::param_storage_type& storage, bool isReturnable = false)
{
	bool canBeReturnableOutput{ false };
	if (isReturnable)
		canBeReturnableOutput = storage.size() > 1;
	for (auto& arg : storage)
	{
		if (arg->isPtrType && !arg.unique())
			arg->argInfo = ABILITY_IN_ARGS::PTRABLE_AND_RETURNABLE;
		else if (arg->isPtrType)
			arg->argInfo = ABILITY_IN_ARGS::PTRABLE;
		else if (!arg.unique() || canBeReturnableOutput)
			arg->argInfo = ABILITY_IN_ARGS::RETURNABLE;
		else
			arg->argInfo = ABILITY_IN_ARGS::SIMPLE;
	}
}

bool PumlProceeder::fixArgsInFunction()
{
	for (auto& algorithm : *currentAlgorithmList)
	{
		determArg(algorithm.second->inputParam);
		determArg(algorithm.second->outerParam, true);
		determArg(algorithm.second->localParam);
		if (algorithm.second->inputParam.empty() && algorithm.second->outerParam.empty())
			continue;
		std::string algorithmWhere;
		try
		{
			for (auto& fixedAlgorithm : *currentAlgorithmList)
			{
				algorithmWhere = fixedAlgorithm.first;
				for (auto& instruction : fixedAlgorithm.second->algorithmData)
					if (instruction->command == COMMAND_TYPE::FUNCTION)
					{
						auto functionInstruction = std::dynamic_pointer_cast<FunctionAlgorithmCommand>(instruction);
						if (functionInstruction->calledFunction == algorithm.first)
							fixFunctionCallArgs(*algorithm.second, *functionInstruction);
					}
			}
			algorithm.second->outerParam.erase(std::remove_if(
				algorithm.second->outerParam.begin(), algorithm.second->outerParam.end(), [](const AlgorithmInfo::param_type& arg)
				{
					return (!arg.unique() && arg->name.front() == PLANTUML_INPUT_PAR_CH);
				}), algorithm.second->outerParam.end());
		}
		catch (...)
		{
			log("Ошибка при исправлении вызова функции \"" + algorithm.first + "\" в алгоритме \""+ algorithmWhere +"\". Исправление всех(возможно остались некорректные вызовы в остальных алгоритмах) вызовов не завершено.");
			return false;
		}
		
	}		
	return true;
}
