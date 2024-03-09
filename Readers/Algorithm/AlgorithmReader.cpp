#include "AlgorithmReader.hpp"



#include "../../Common/FileReading.hpp"
#include "../../Common/StringFunction.hpp"
#include "../../Common/CSVReader.hpp"

#include "../../Common/Def.hpp"

static auto servSymbols = SERVICE_SYMBOLS;
static auto terminateSymbols = TERMINATE_SYMBOLS;
static auto keyWordSymbols = PLANTUML_START_KEY_WORDS;

static auto fullKeyWords =
{
	PLANTUML_ELSE_IF,
	PLANTUML_ELSE_IF_SEP,
	PLANTUML_IF ,
	PLANTUML_ELSE,
	PLANTUML_WHILE,
	PLANTUML_SWITCH,
	PLANTUML_CASE,
	PLANTUML_END_WORD_IF,
	PLANTUML_END_WORD_WHILE,
	PLANTUML_END_WORD_SWITCH
};
static auto argsLetter =
{
	INNER_SIGNAL,
	INPUT_SIGNAL,
	OUTPUT_SIGNAL,
	TIMER_SIGNAL,
	CONST_SIGNAL,
	PLANTUML_INPUT_PAR_CH,
	_PLANTUML_OUTPUT_PAR_CH,
	PLANTUML_LOCAL_PAR_CH
};

static auto calculatableSignal =
{
	INNER_SIGNAL,
	INPUT_SIGNAL,
	OUTPUT_SIGNAL,
	TIMER_SIGNAL,
	PLANTUML_INPUT_PAR_CH,
	_PLANTUML_OUTPUT_PAR_CH,
	PLANTUML_LOCAL_PAR_CH
};

static std::vector<std::string> acceptableAssign
{

	"+=",
	"-=",
	"*=",
	"/=",
	"%=",
	"|=",
	"&=",
	"="

};

static std::regex noteRegex = std::regex{ "(\\w+):{1}\\s*(\\w+)\\s*(\\**)?\\s*(\\[\\d+\\])?" };
static std::regex noteTimerRegex = std::regex{ "([PLR]{1}\\d+)" };

static inline void getAssignInfo(const std::string& line, const std::string::size_type offset, std::string::size_type& pos, std::string::size_type& size)
{
	std::string::size_type currentPos = std::string::npos;
	std::string::size_type currentSize = size;
	for (auto& el : acceptableAssign)
	{
		pos = line.find(el, offset);
		if (pos < currentPos)
		{
			currentPos = pos;
			currentSize = el.size();
		}
	}
	pos = currentPos;
	size = currentSize;
}
static inline bool checkPartOfArgs(const std::string& argPart)
{
	if (!std::all_of(std::next(argPart.begin()), argPart.end(), [](const char ch)
		{
			return (StringFunction::_isdigit(ch) || ch == PLANTUML_STRUCT_ACCESS_SIGN);
		}))
		return false;
	if (StringFunction::_isupper(argPart.front()))
	{
		if (argPart.size() < 2)
			return false;
		if (std::find(argsLetter.begin(), argsLetter.end(), argPart.front()) == argsLetter.end())
			return false;
	}
	return true;
}
static inline std::string::const_iterator getLinePos(const std::string & line, std::size_t offset)
{
	if (line.size() <= offset)
		return line.end();
	return std::find_if_not(line.begin() + offset, line.end(), [](char ch) {return StringFunction::_isalnum(ch) || ch == '.'; });
}

void AlgorithmReader::checkStructInArgs(InfoTableLight & signalStorage, const AlgorithmInfo::param_storage_type & args) const
{
	for (auto& container : args)
		if (container->pumlType.size() && container->pumlType.front() == STRUCT_SIGNAL)
			signalStorage.structValues.emplace(container->pumlType);
}
void AlgorithmReader::checkSignalsInLine(InfoTableLight & signalStorage, const BaseAlgorithmCommand::string_type & line) const
{
	auto pos = getLinePos(line, 0);
	auto prevOffset{ line.begin() };
	bool isNotOver{true};
	do
	{
		if (pos == line.end() && prevOffset != line.begin())
			isNotOver = false;


		auto iter = std::find_if(prevOffset, pos, StringFunction::_isalnum);
		if (iter != pos)
		{

			auto signal = std::string{ iter, pos };
			if (pos != line.end())
			{
				auto structPosInSignal = signal.find(STRUCT_FIELD_SYMBOL);
				if (structPosInSignal != std::string::npos)
				{
					if (signal.front() == INNER_SIGNAL || signal.front() == INPUT_SIGNAL || signal.front() == OUTPUT_SIGNAL)
						signalStorage.structValues.emplace(signal);					
					signal = signal.substr(0, structPosInSignal);

				}
				else if (*pos == '[')
					signal += '[';
			}

			auto isSignal = [&signal]() -> bool
			{
				if (signal.empty() || !StringFunction::_isupper(signal.front()))
					return false;
				return std::all_of(std::next(signal.begin()), signal.end(), [](const char ch)
				{
					return (StringFunction::_isdigit(ch) ||
						ch == '.' ||
						ch == '[' ||
						ch == ']' ||
						ch == '*');
				});
			}();
			if (isSignal)
			{
				switch (signal.front())
				{
				case INNER_SIGNAL:
				case INPUT_SIGNAL:
				case OUTPUT_SIGNAL:
				{
					FieldInfo data;
					if (signal.find(STRUCT_FIELD_SYMBOL) != std::string::npos)
					{
						//TODO: Нужна ли эта часть?
						auto part = Readers::CSVLineReader(signal, STRUCT_FIELD_SYMBOL, true);
						auto* iData = &data;
						for (decltype(part)::size_type i = 0; i < part.size(); ++i)
						{
							if (part.at(i).empty())
							{
								signalStorage.unknownSignals.emplace(signal);
								break;
							}
							iData->isPtr = (part.at(i).find('[') != std::string::npos) ||
								(part.at(i).find('*') != std::string::npos);
							if ((i + 1) < part.size())
							{
								try
								{
									auto brPos = part.at(i + 1).find('*');
									if (brPos == std::string::npos)
										brPos = part.at(i + 1).find('[');
									 auto index{static_cast<decltype(iData->usedField)::key_type>
										 (std::stoul(part.at(i+1).substr(0, brPos)))};

									if (iData->usedField.find(index) == iData->usedField.end())
										iData->usedField.emplace(index, FieldInfo{});
									else
										iData->usedField.at(static_cast<decltype(iData->usedField)::key_type>(i)) += data;
									iData = &iData->usedField.at(index);
								}
								catch (...)
								{
									signalStorage.unknownSignals.emplace(signal);
									break;
								}
							}
						}
						signalStorage.structValues.emplace(signal);
						signal = part.front();
					}
					data.isPtr = (signal.find('[') != std::string::npos) || (signal.find('*') != std::string::npos);
					if (data.isPtr == false)
					{
						std::string tempString{ iter, line.cend() };
						if (tempString.size() > signal.size())
							if (tempString.at(signal.size()) == '[')
								data.isPtr = true;
					}
					InfoTableLight::signal_type* container{ &signalStorage.outputSignals };
					if (signal.front() == INNER_SIGNAL)
						container = &signalStorage.innerValues;
					else if (signal.front() == INPUT_SIGNAL)
						container = &signalStorage.inputSignals;

					if (data.isPtr)
					{
						auto brPos = signal.find('*');
						if (brPos == std::string::npos)
							brPos = signal.find('[');
						auto iter = container->find(signal.substr(0, brPos));
						if (iter != container->end())
							iter->second.isPtr |= data.isPtr;
						else
							container->emplace(signal.substr(0, brPos), data);
					}
					else
						container->emplace(signal, data);
					break;
				}
				case TIMER_SIGNAL:
				case CONST_SIGNAL:
					if (signal.find('.') != std::string::npos || signal.find('[') != std::string::npos ||
						signal.find(']') != std::string::npos || signal.find('*') != std::string::npos)
						signalStorage.unknownSignals.emplace(signal);
					else
					{
						if (signal.front() == TIMER_SIGNAL)
							signalStorage.timers.emplace(signal);
						else
							signalStorage.constValues.emplace(signal);
					}
					break;
				case PLANTUML_LOCAL_PAR_CH:
				case _PLANTUML_OUTPUT_PAR_CH:
				case PLANTUML_INPUT_PAR_CH:
					break;
				case STRUCT_SIGNAL:
				default:
					signalStorage.unknownSignals.emplace(signal);
					break;
				}
			}

		}

		prevOffset = pos;
		pos = getLinePos(line, std::distance(line.begin(), pos) + 1);
	} while (isNotOver);
}
bool AlgorithmReader::parseKeyWordLine(const std::string& data, const COMMAND_TYPE command, const std::string& path)
{
	std::string::size_type index = 0;
	auto balanceInfo = StringFunction::checkStringBalance(data, LEFT_SIMPLE_BRACKET, RIGHT_SIMPLE_BRACKET, &index);
	if (balanceInfo != 0)
	{
		log("В строке \"" + data + "\" различное кол-во круглых скобок (лишние символы \"" + std::string{1, balanceInfo > 0 ? LEFT_SIMPLE_BRACKET : RIGHT_SIMPLE_BRACKET} + "\"). Файл: " + path + ".");
		return false;
	}
	std::string ignoredPart;
	if (index != std::string::npos)
		ignoredPart = data.substr(index + 1);
	StringFunction::removeExtraSpacesFromCodeText(ignoredPart);
	auto dataPart = StringFunction::removeExtraSpacesFromCodeText(data.substr(0, index));
	balanceInfo = StringFunction::checkStringBalance(data, LEFT_SQARE_BRACKET, RIGHT_SQARE_BRACKET);
	if (balanceInfo != 0)
	{
		log("В строке \"" + data + "\") различное кол-во квадратных скобок (лишние символы \"" + std::string{1, balanceInfo > 0 ? LEFT_SQARE_BRACKET : RIGHT_SQARE_BRACKET } + "\"). Файл: " + path + ".");
		return false;
	}
	switch (command)
	{
	case COMMAND_TYPE::SWITCH_KEY:
		currentAlgorithm->algorithmData.emplace_back(new BaseAlgorithmCommand{ dataPart.substr(dataPart.find('(') + 1),
			COMMAND_TYPE::SWITCH_COMMENT });
		break;
	case COMMAND_TYPE::CASE_KEY:
		if (currentAlgorithm->algorithmData.empty())
		{
			log("Некорректная инструкция \"case\". Файл: " + path + ".");
			return false;
		}
		if (data.find('?') == std::string::npos)
		{
			if (parseKeyWordLine(data.substr(0, data.find('(') + 1) + '?' + data.substr(data.find('(') + 1),
				COMMAND_TYPE::CASE_KEY, path) == false)
				return false;
			break;
		}
	case COMMAND_TYPE::WHILE_KEY:
	case COMMAND_TYPE::ELIF_KEY:
	case COMMAND_TYPE::ELIF_SEP_KEY:
	case COMMAND_TYPE::IF_KEY:
	{
		bool isCorrectEnd{ true };
		if (ignoredPart.empty() || ignoredPart.size() && (ignoredPart.find(PLANTUML_THEN) != 0 && ignoredPart.find(PLANTUML_IS) != 0))
			isCorrectEnd = false;
		else
		{
			auto pos = ignoredPart.find(LEFT_SIMPLE_BRACKET);
			if (pos == std::string::npos)
				isCorrectEnd = false;
			else
			{
				ignoredPart = StringFunction::removeExtraSpacesFromCodeText(ignoredPart.substr(pos + 1, ignoredPart.size() - pos - 2));
				isCorrectEnd = ignoredPart == LANGUAGE_YES;
			}
		}
		if (isCorrectEnd == false)
		{
			log("Некорректное завершение строки: \"" + data + "\" в файле: \"" + path + "\".");
			return false;
		}

		auto splitPos = dataPart.find('?');
		if (splitPos == std::string::npos)
		{
			log("В строке: \"" + data + "\" отсутствует символ '?'. Файл: \"" + path + "\".");
			return false;
		}
		if (command == COMMAND_TYPE::ELIF_KEY || command == COMMAND_TYPE::ELIF_SEP_KEY)
			currentAlgorithm->algorithmData.emplace_back(new BaseAlgorithmCommand{ std::string{},  COMMAND_TYPE::END_IF_KEY });
		auto comment = dataPart.substr(dataPart.find('(') + 1, splitPos - dataPart.find('('));
		if (comment.size() && comment != "?")
			currentAlgorithm->algorithmData.emplace_back(new BaseAlgorithmCommand{comment, COMMAND_TYPE::COMMENT });
		auto ifString = StringFunction::removeExtraSpacesFromCodeText(dataPart.substr(splitPos + 1));
		if (command == COMMAND_TYPE::CASE_KEY)
		{
			for (auto iter = currentAlgorithm->algorithmData.rbegin(); iter != currentAlgorithm->algorithmData.rend(); ++iter)
			{
				if (iter->get()->command == COMMAND_TYPE::CASE_KEY)
				{
					currentAlgorithm->algorithmData.emplace_back(new BaseAlgorithmCommand{ std::string{}, COMMAND_TYPE::END_CASE_KEY });
					break;
				}
				if (iter->get()->command == COMMAND_TYPE::SWITCH_COMMENT)
					break;
			}

		}
		currentAlgorithm->algorithmData.emplace_back(new AlgorithmCommand{ ifString ,
			command == COMMAND_TYPE::ELIF_SEP_KEY ? COMMAND_TYPE::ELIF_KEY : command });
		break;
	}
	case COMMAND_TYPE::END_IF_KEY:
	case COMMAND_TYPE::END_CASE_KEY:
	case COMMAND_TYPE::END_SWITCH_KEY:
	case COMMAND_TYPE::END_WHILE_KEY:
	case COMMAND_TYPE::ELSE_KEY:
		if (ignoredPart.size())
		{
			log("Некорректная строка: \"" + data + "\" в файле: \"" + path + "\". Не ожидалось информации после закрытия скобок.");
			return false;
		}
		if (command == COMMAND_TYPE::END_SWITCH_KEY)
			currentAlgorithm->algorithmData.emplace_back(new BaseAlgorithmCommand{ std::string{}, COMMAND_TYPE::END_CASE_KEY });
		this->currentAlgorithm->algorithmData.emplace_back(new BaseAlgorithmCommand{ std::string{},  command });
		if (command == COMMAND_TYPE::ELSE_KEY || command == COMMAND_TYPE::END_WHILE_KEY)
		{
			bool isCorrectEnd{ true };
			auto pos = dataPart.find(LEFT_SIMPLE_BRACKET);
			if (pos == std::string::npos)
				isCorrectEnd = false;
			else
			{
				ignoredPart = StringFunction::removeExtraSpacesFromCodeText(dataPart.substr(pos + 1));
				isCorrectEnd = ignoredPart == LANGUAGE_NO;
			}
			if (isCorrectEnd == false)
			{
				log("Некорректное завершение строки: \"" + data + "\" в файле: \"" + path + "\".");
				return false;
			}
		}
		
		break;
	default:
		log("Некорректная строка: \"" + data + "\" в файле: \"" + path + "\". Имеет некорректный тип.");
		return false;
	}
	return true;
}
bool AlgorithmReader::parseBuffers(std::string& buffer, std::string& keywordBuffer, std::string& line, COMMAND_TYPE& command, const std::size_t currentKeyWordIndex, const std::string& path)
{
	if (buffer.size())
	{
		std::stringstream codeStream{ buffer };
		if (parse(codeStream, path) == false)
			return false;
		buffer.clear();
	}
	if (keywordBuffer.size())
	{
		if (parseKeyWordLine(keywordBuffer, command, path) == false)
			return false;
		keywordBuffer.clear();
	}
	command = static_cast<COMMAND_TYPE>(currentKeyWordIndex);
	
	return true;
}
bool AlgorithmReader::parseKeyWords(const std::string& data, const std::string& path)
{
	std::stringstream stream{ data };
	std::string line, buffer, keywordBuffer;
	COMMAND_TYPE command = COMMAND_TYPE::COMMENT;
	while (std::getline(stream, line))
	{
		std::size_t keyWordIndex;
		if (isWordKeyWord(line,&keyWordIndex))
		{
			if (parseBuffers(buffer, keywordBuffer, line, command, keyWordIndex, path) == false)
				return false;
			keywordBuffer = line;
		}
		else
		{
			if (line.empty() || line.front() == PLANTUML_PAR_MARKER)
			{
				if (keywordBuffer.size())
				{
					if (parseKeyWordLine(keywordBuffer, command, path) == false)
						return false;
					keywordBuffer.clear();
				}
				if (line.empty() == false)
				{
					if (buffer.size() && buffer.back() == PUML_CALCULATION_END)
					{
						if (parseBuffers(buffer, keywordBuffer, line, command, keyWordIndex, path) == false)
							return false;
						buffer = line;
					}
					else
						StringFunction::appendString(line, buffer, '\n');
						//buffer += buffer.empty() ? line : '\n' + line;
				}
			}
			else
			{
				if (keywordBuffer.empty() && buffer.empty())
				{
					log("Файл :" + path + ". Ошибка в строке \"" + line + "\".");
					return false;
				}
				if (keywordBuffer.empty())
				{
					if (buffer.size() && buffer.back() == PUML_CALCULATION_END)
					{
						if (parseBuffers(buffer, keywordBuffer, line, command, keyWordIndex, path) == false)
							return false;
						buffer = line;
					}
					else
						StringFunction::appendString(line, buffer);
						//buffer += line;
				}
				else if (buffer.empty())
					StringFunction::appendString(line, keywordBuffer);
					//keywordBuffer += line;
				else
				{
					log("Файл :" + path + ". Ошибка в строке \"" + line + "\".");
					return false;
				}
			}
		}
	}
	return parseBuffers(buffer, keywordBuffer, line, command, 0, path);
}
bool AlgorithmReader::checkFunctionArgs(const std::string& args) const
{
	if (args.empty())
		return true;
	auto spitedArgs = Readers::CSVLineReader(args, PLANTUML_FUNCTION_ARGS_SEPARATOR, true);
	for (auto& arg : spitedArgs)
	{
		StringFunction::removeExtraSpacesFromCodeText(arg);
		if (arg.size() > 1)
			for (auto i = 0u; i < arg.size() - 1; ++i)
				if ((arg.at(i) == PLANTUML_STRUCT_ACCESS_SIGN && arg.at(i + 1) == PLANTUML_STRUCT_ACCESS_SIGN) ||
					(arg.at(i) == LEFT_SQARE_BRACKET && arg.at(i + 1) == RIGHT_SQARE_BRACKET))
					return false;

		auto split = Readers::CSVLineReader(arg, LEFT_SQARE_BRACKET);
		auto pos = std::find(split.back().begin(), split.back().end(), RIGHT_SQARE_BRACKET);
		if (pos != split.back().end())
			split.back().erase(pos, split.back().end());

		for (auto& splitEl : split)
		{
			StringFunction::removeExtraSpacesFromCodeText(splitEl);
			if (checkPartOfArgs(splitEl))
				continue;			
			return false;
		}
	}
	return true;
}

bool AlgorithmReader::checkFunctionSignature(const std::string& signature, const std::string& path) const
{
	auto functionSignaturePos = signature.find('(');
	auto argSplitterPos = signature.find(PUML_SINGNATURE_ARGS_SPLIT);
	if (((argSplitterPos != std::string::npos) && (functionSignaturePos == std::string::npos)) ||
		((argSplitterPos == std::string::npos) && (functionSignaturePos != std::string::npos)))
	{
		log("В сигнатуре ф-ии \"" + signature + "\" символы \"(\" и \"" PUML_SINGNATURE_ARGS_SPLIT "\" не могут присутствовать(или отсутствовать) вместе. Файл: " + path + ".");
		return false;
	}
	if (functionSignaturePos != std::string::npos)
	{
		if (argSplitterPos < functionSignaturePos)
		{
			log("В сигнатуре ф-ии \"" + signature + "\" отсутствуют входные аргументы. Файл: " + path + ".");
			return false;
		}
		auto inputArgs = signature.substr(functionSignaturePos + 1, signature.find(')')
			- (functionSignaturePos + 1));
		functionSignaturePos = signature.find('(', signature.find(')'));
		if (functionSignaturePos == std::string::npos)
		{
			log("В сигнатуре ф-ии \"" + signature + "\" отсутствуют выходные аргументы. Файл: " + path + ".");
			return false;
		}
		if (argSplitterPos > functionSignaturePos)
		{
			log("В сигнатуре ф-ии \"" + signature + "\" некорректно расположены выходные аргументы. Файл: " + path + ".");
			return false;
		}
		auto outputArgs = signature.substr(functionSignaturePos + 1,
			signature.find(')', signature.find(')') + 1)
			- (functionSignaturePos + 1));
		bool isOk = checkFunctionArgs(inputArgs);
		if (isOk == false)
			log("В сигнатуре ф-ии \"" + signature + "\" некорректно заданы входные аргументы. Файл: " + path + ".");
		if (checkFunctionArgs(outputArgs) == false)
		{
			log("В сигнатуре ф-ии \"" + signature + "\" некорректно заданы выходные аргументы. Файл: " + path + ".");
			isOk = false;
		}
		return isOk;
	}
	return true;
}

bool AlgorithmReader::parseFunction(const std::string& functionData, const std::string& path)
{
	auto pos = functionData.find(PLANTUML_FUNCTION_WRAP);
	if (pos == std::string::npos)
	{
		log("В блоке ф-ии \"" + functionData + "\" отсутствует выделение вызываемой ф-ии символами \". Файл: " + path + ".");
		return false;
	}
	auto comment = StringFunction::removeExtraSpacesFromCodeText(functionData.substr(1, pos - 1));
	if (comment.empty() == false)
		currentAlgorithm->algorithmData.emplace_back(new BaseAlgorithmCommand{ std::move(comment), COMMAND_TYPE::COMMENT });

	auto function = StringFunction::removeExtraSpacesFromCodeText(functionData.substr(pos + 1));
	auto functionLine = StringFunction::removeExtraSpacesFromCodeText(function.substr(0, function.size() - 2));

	auto balanceInfo = StringFunction::checkStringBalance(functionData, LEFT_SIMPLE_BRACKET, RIGHT_SIMPLE_BRACKET);
	if (balanceInfo != 0)
	{
		log("В строке \"" + functionData + "\"(в вызове ф-ии) различное кол-во круглых скобок (лишние символы \"" + (balanceInfo > 0 ? LEFT_SIMPLE_BRACKET : RIGHT_SIMPLE_BRACKET) + "\"). Файл: " + path + ".");
		return false;
	}
	balanceInfo = StringFunction::checkStringBalance(functionData, LEFT_SQARE_BRACKET, RIGHT_SQARE_BRACKET);
	if (balanceInfo != 0)
	{
		log("В строке \"" + functionData + "\"(в вызове ф-ии) различное кол-во квадратных скобок (лишние символы \"" + (balanceInfo > 0 ? LEFT_SQARE_BRACKET : RIGHT_SQARE_BRACKET ) + "\"). Файл: " + path + ".");
		return false;
	}
	if (checkFunctionSignature(functionLine, path))
	{
		currentAlgorithm->algorithmData.emplace_back(new FunctionAlgorithmCommand{ std::move(functionLine) , COMMAND_TYPE::FUNCTION });
		auto functionNamePos = function.find('(');
		if (functionNamePos == std::string::npos)
			functionNamePos = function.size() - 2;
		auto functionName = StringFunction::removeExtraSpacesFromCodeText(function.substr(0, functionNamePos));
		if (functionName.empty() == false)
		{
			if (std::find(currentAlgorithm->usedAlgorithm.begin(), currentAlgorithm->usedAlgorithm.end(), functionName) == currentAlgorithm->usedAlgorithm.end())
				currentAlgorithm->usedAlgorithm.emplace_back(std::move(functionName));
			return true;
		}
		log("В строке \"" + functionData + "\" отсутствует название вызываемой функции. Файл: " + path + ".");
	}
	return false;
}

std::string::size_type AlgorithmReader::getAssignPos(const std::string& line, const std::string::size_type offset) const
{
	auto pos = std::string::npos;
	std::string::size_type size{};
	getAssignInfo(line, offset, pos, size);
	while (pos != std::string::npos)
	{
		if (pos == 0)
		{
			if ((StringFunction::_isspace(line.at(pos + size)) || StringFunction::_isalnum(line.at(pos + size))))
				break;
		}
		else if (pos == line.size() - 1)
		{
			if (StringFunction::_isspace(line.at(pos - 1)) || StringFunction::_isalnum(line.at(pos - 1)))
				break;
		}
		else
		{
			if ((StringFunction::_isspace(line.at(pos - 1)) || StringFunction::_isalnum(line.at(pos - 1))) &&
				(StringFunction::_isspace(line.at(pos + size)) || StringFunction::_isalnum(line.at(pos + size))))
				break;
		}
		getAssignInfo(line, pos + size, pos, size);
	}
	return pos;
}
static std::string getComment(const std::string& line, std::string::size_type pos, bool isCommentLine = false, std::string::size_type* prevSpacePos = nullptr)
{
	std::string comment;
	if (pos != std::string::npos)
	{
		auto prevPos = line.find_last_of(SPACE_SIGN, (line.at(pos - 1) == SPACE_SIGN ? pos - 2 : pos - 3));
		if (prevSpacePos)
			*prevSpacePos = prevPos;
		comment = line.substr(static_cast<decltype(prevPos)>(isCommentLine), prevPos - static_cast<decltype(prevPos)>(isCommentLine));
		auto oldSize = comment.size();
		bool wasErase{ false };
		if ((line.front() == PLANTUML_START_COMMAND_BLOCK && comment != line.substr(1)) ||
			(line.front() != PLANTUML_START_COMMAND_BLOCK && comment != line))
		{
			while (StringFunction::checkStringBalance(comment, LEFT_SQARE_BRACKET, RIGHT_SQARE_BRACKET) != 0)
			{

				comment.erase(comment.find_last_of(LEFT_SQARE_BRACKET));
				wasErase = true;
			}
			if (wasErase)
			{
				comment.erase(comment.find_last_of(SPACE_SIGN));
				if (prevSpacePos)
					*prevSpacePos -= oldSize - comment.size();
			}
		}
	}
	return comment;
}
bool AlgorithmReader::parseCalculationWithComment(const std::string& calculationData, const std::string& path)
{
	auto pos = calculationData.find(PLANTUML_CODE_COMMENT_START_);
	if (pos != std::string::npos)
	{
		std::string beforeComment = StringFunction::removeExtraSpacesFromCodeText(calculationData.substr(0, pos) + calculationData.back());
		std::string afterComment = calculationData.front() + calculationData.substr(pos + std::strlen(PLANTUML_CODE_COMMENT_START_));
		pos = afterComment.find(PLANTUML_CODE_COMMENT_END_);
		if (pos == std::string::npos)
		{
			log("Алгоритм не имеет закрывающего символа комментария \""  PLANTUML_CODE_COMMENT_END_  "\" в строке \"" + calculationData + "\". Файл: " + path + ".");
			return false;
		}
		afterComment.erase(pos, std::strlen(PLANTUML_CODE_COMMENT_END_));
		StringFunction::removeExtraSpacesFromCodeText(afterComment);
		return parseCalculationWithComment(beforeComment, path) && parseCalculationWithComment(afterComment, path);
	}
	pos = calculationData.find(PLANTUML_CODE_COMMENT);
	if (pos != std::string::npos)
	{
		std::string beforeComment = calculationData.substr(0, pos);
		std::string afterComment = calculationData.substr(pos + std::strlen(PLANTUML_CODE_COMMENT));
		pos = getAssignPos(afterComment);
		std::string comment;
		if (pos != std::string::npos)
			comment = getComment(afterComment, pos);
		else
			comment = afterComment;
		afterComment.erase(0, comment.size());
		StringFunction::removeExtraSpacesFromCodeText(afterComment);
		if (afterComment.size() && afterComment.front() != calculationData.back())
			afterComment = calculationData.front() + afterComment;
		else
			afterComment.clear();
		auto beforeString = getComment(beforeComment, getAssignPos(beforeComment, getAssignPos(beforeComment)+1));
		if (beforeString.size() && afterComment.back() != calculationData.front())
		{
			beforeComment = calculationData.front() + comment + " " + 
				StringFunction::removeExtraSpacesFromCodeText(beforeComment.substr(beforeString.size())) + calculationData.back();
			beforeString = beforeString + calculationData.back();
		}
		else
		{
			beforeString.clear();
			beforeComment = calculationData.front() + comment + " " +
				StringFunction::removeExtraSpacesFromCodeText(beforeComment.substr(1)) + calculationData.back();
		}
		return (beforeString.empty() || parseCalculationWithComment(beforeString, path)) &&
			parseCalculationWithComment(beforeComment, path) &&
			(afterComment.empty() || parseCalculationWithComment(afterComment, path));
	}

	return parseCalculation(calculationData, path);
}

bool AlgorithmReader::parseCalculation(const std::string& calculationData, const std::string& path)
{
	auto balanceInfo = StringFunction::checkStringBalance(calculationData, LEFT_SIMPLE_BRACKET, RIGHT_SIMPLE_BRACKET);
	if (balanceInfo != 0)
	{
		log("В строке \"" + calculationData + "\" различное кол-во круглых скобок (лишние символы \"" + (balanceInfo > 0 ? LEFT_SIMPLE_BRACKET : RIGHT_SIMPLE_BRACKET) + "\"). Файл: " + path + ".");
		return false;
	}
	if (calculationData.find(PLANTUML_CODE_COMMENT) != std::string::npos ||
		calculationData.find(PLANTUML_CODE_COMMENT_START_) != std::string::npos)
	{
		return parseCalculationWithComment(calculationData, path);
	}
	else
	{
		auto pos = getAssignPos(calculationData);
		if (pos == std::string::npos)
		{
			currentAlgorithm->algorithmData.emplace_back(new BaseAlgorithmCommand{ calculationData.substr(1,calculationData.size() - 2), COMMAND_TYPE::COMMENT });
			return true;
		}

		auto workLine = calculationData;
		bool isCommentLine{ true };

		while (pos != std::string::npos || workLine.size())
		{
			std::string::size_type prevSpacePos;
			std::string workString = getComment(workLine, pos, isCommentLine, &prevSpacePos);
			if (workString.empty())
				workString = workLine;
			StringFunction::removeExtraSpacesFromCodeText(workString);

			if (isCommentLine == false)
			{
				if (std::find(calculatableSignal.begin(), calculatableSignal.end(), workString.front()) == calculatableSignal.end())
				{
					log("Строка \"" + workLine + "\" начинается с недопустимого сигнала - " + std::string{workString.front()} +
						".Полная строка\"" + calculationData + "\". Файл: " + path + ".");
					return false;
				}
				balanceInfo = StringFunction::checkStringBalance(workString, LEFT_SQARE_BRACKET, RIGHT_SQARE_BRACKET);
				if (balanceInfo != 0)
				{
					log("В строке \"" + workString + "\" различное кол-во квадратных скобок (лишние символы \"" + (balanceInfo > 0 ? LEFT_SQARE_BRACKET : RIGHT_SQARE_BRACKET) + "\"). Файл: " + path + ".");
					return false;
				}
				currentAlgorithm->algorithmData.emplace_back(new AlgorithmCommand{ std::move(workString) , COMMAND_TYPE::CALCULATION });
				if (pos != std::string::npos)
				{
					workLine = workLine.substr(prevSpacePos + 1);
					pos = getAssignPos(workLine);
				}
				else
					workLine.clear();
			}
			else
			{
				isCommentLine = false;
				if (prevSpacePos != std::string::npos)
				{
					if (workString.size())
						currentAlgorithm->algorithmData.emplace_back(new BaseAlgorithmCommand{ std::move(workString) , COMMAND_TYPE::COMMENT });
					workLine = workLine.substr(prevSpacePos + 1, workLine.size() - (prevSpacePos + 2));
					pos = getAssignPos(workLine);
				}
				else
				{
					workLine = workLine.substr(1, workLine.size() - 2);
					--pos;
				}
			}
			pos = getAssignPos(workLine, pos + 1);
		}
	}
	return true;
}

bool AlgorithmReader::configureOutputArgs(const std::string& path)
{
	for (auto& args : currentAlgorithm->outerParam)
	{
		AlgorithmReader::algorithm_data_type::param_storage_type::iterator referenceParam;
		AlgorithmReader::algorithm_data_type::param_storage_type* storage{nullptr};
		switch (args->name.front())
		{
		case _PLANTUML_OUTPUT_PAR_CH:
			continue;
		case PLANTUML_INPUT_PAR_CH:
			storage = &currentAlgorithm->inputParam;
		case PLANTUML_LOCAL_PAR_CH:
			if (storage == nullptr)
				storage = &currentAlgorithm->localParam;

			referenceParam = std::find_if(storage->begin(),	storage->end(),
				[&args](const AlgorithmReader::algorithm_data_type::param_type& ref) {return ref->name == args->name; });
			break;

		default:
			return false;
		}
		if (storage == nullptr || referenceParam == storage->end() || *args != *(referenceParam->get()))
		{
			log("Некорректный выходной аргумент \"" + args->name + "\" в алгоритме \"" + algorithmName + "\". Файл: " + path);
			return false;
		}
		else
			*referenceParam = args;

	}
	return true;
}

AlgorithmReader::algorithm_data_type::param_storage_type* AlgorithmReader::defineArgsType(const std::string& line, std::string& signalType, std::vector<char>& possibleNames)
{
	if (line.find(PLANTUML_INPUT_PAR) != std::string::npos)
	{
		possibleNames = std::vector<char>{ PLANTUML_INPUT_PAR_CH };
		signalType = "входные";
		return &currentAlgorithm->inputParam;
	}
	else if (line.find(PLANTUML_LOCAL_PAR) != std::string::npos)
	{
		possibleNames = std::vector<char>{ PLANTUML_LOCAL_PAR_CH };
		signalType = "локальные";
		return &currentAlgorithm->localParam;
	}
	else if (line.find(PLANTUML_OUTPUT_PAR) != std::string::npos)
	{
		possibleNames = std::vector<char>{ _PLANTUML_OUTPUT_PAR_CH, PLANTUML_LOCAL_PAR_CH, PLANTUML_INPUT_PAR_CH };
		signalType = "выходные";
		return &currentAlgorithm->outerParam;

	}
	return nullptr;
}

AlgorithmInfo::param_type AlgorithmReader::parseRegexArg(std::smatch& matches) const
{
	AlgorithmInfo::param_type arg{new AlgorithmInfo::param_type::element_type};
	std::uint32_t currentIndex{ 1 };
	arg->name = matches[currentIndex++].str();
	arg->pumlType = matches[currentIndex++].str();
	arg->title = StringFunction::removeExtraSpacesFromCodeText(matches.suffix().str());	
	if (arg->pumlType.empty() && arg->name.size())
	{
		if (matches.size() == 2)
			return arg;
		else
			return nullptr;
	}
	if (arg->pumlType.empty() || arg->name.empty())
		return nullptr;
	arg->isPtrType = matches[currentIndex++].str().size() || matches[currentIndex].str().size();
	if (matches[currentIndex - 1].str().size() && matches[currentIndex].str().size())
		arg->pumlType += matches[currentIndex - 1].str();
	if (arg->isPtrType)
	{
		auto regexResult{ matches[currentIndex].str() };
		if (regexResult.empty())
			arg->elementCount = 1;
		else
		{
			regexResult.pop_back();
			try
			{
				arg->elementCount = static_cast<std::uint32_t>(std::stoul(regexResult.substr(1)));
			}
			catch (...)
			{
				return nullptr;
			}
		}
	}
	return arg;
}

bool AlgorithmReader::parseAlgorithmArgs(std::istream& stream, algorithm_data_type::param_storage_type& storage, const std::vector<char>& possibleNames)
{
	std::string line;
	std::string::size_type prevFilePos{static_cast<std::string::size_type>(stream.tellg())};
	while (std::getline(stream, line))
	{
		StringFunction::removeExtraSpacesFromCodeText(line);
		if (line.empty())
			continue;
		std::smatch m;
		if (std::regex_search(line, m, noteRegex))
		{
			auto argsData = parseRegexArg(m);
			if (argsData == nullptr)
				return false;
			storage.emplace_back(std::move(argsData));
		}
		else
		{
			if (std::regex_search(line, m, noteTimerRegex) == false)
				break;
			auto argsData = parseRegexArg(m);
			if (argsData == nullptr)
				return false;
			storage.emplace_back(std::move(argsData));
		}
		prevFilePos = static_cast<decltype(prevFilePos)>(stream.tellg());
	}
	stream.seekg(prevFilePos);
	
	if (std::all_of(storage.begin(), storage.end(), [&possibleNames](const algorithm_data_type::param_storage_type::value_type& args)
		{
			return std::find(possibleNames.begin(), possibleNames.end(), args->name.front()) != possibleNames.end();
		}) == false)
	{
		return false;
	}
	auto copy = storage;
	std::sort(copy.begin(), copy.end(), [](algorithm_data_type::param_storage_type::value_type& l, algorithm_data_type::param_storage_type::value_type& r)
		{
			return l->name < r->name;
		});
	return std::adjacent_find(copy.begin(), copy.end(), [](const algorithm_data_type::param_storage_type::value_type& l, const  algorithm_data_type::param_storage_type::value_type& r)
		{
			return l->name == r->name;
		}) == copy.end();
}

bool AlgorithmReader::isWordKeyWord(const std::string& word, std::string* keyWord) const
{
	auto iter = std::find_if(keyWordSymbols.begin(), keyWordSymbols.end(), [&word](const std::string& str)
		{
			return word.find(str) == 0;
		});
	if (keyWord != nullptr)
		if (iter != keyWordSymbols.end())
			*keyWord = *iter;
		else
			keyWord->clear();
	return iter != keyWordSymbols.end();
}

bool AlgorithmReader::isWordKeyWord(const std::string& word, std::size_t* keyWordIndex) const
{
	auto iter = std::find_if(fullKeyWords.begin(), fullKeyWords.end(), [&word](const std::string& str)
		{
			return word.find(str) == 0;
		});
	if (keyWordIndex != nullptr)
		if (iter != fullKeyWords.end())
			*keyWordIndex = static_cast<std::size_t>(std::distance(fullKeyWords.begin(), iter));
		else
			*keyWordIndex = 0;
	return iter != fullKeyWords.end();
}
static void removeNewlinrFromNote(std::string& line)
{
	auto pos = line.find(PLANTUML_NODE_NEW_LINE);
	while (pos != std::string::npos)
	{
		line.erase(pos, strlen(PLANTUML_NODE_NEW_LINE));
		pos = line.find(PLANTUML_NODE_NEW_LINE);
	}
	StringFunction::removeExtraSpacesFromCodeText(line);
}
bool AlgorithmReader::parseNote(std::istream& stream, const std::string& path, const bool isInitNote)
{
	std::string line;
	bool needIgnoreData{ false };
	bool wasNote{ false };
	auto prevFilePos = stream.tellg();
	while (std::getline(stream, line))
	{
		StringFunction::removeExtraSpacesFromCodeText(line);
		if (line.empty())
			continue;
		wasNote |= line.find(PLANTUML_NOTE_START) != std::string::npos;
		if (line.find(PLANTUML_NOTE_END) == std::string::npos)
		{
			if (line.find(PLANTUML_NOTE_START) != std::string::npos)
			{
				auto isEnterPos = line.find(PLANTUML_NOTE_START) +
					std::strlen(PLANTUML_NOTE_START) + 1;
				bool isEnter{ false };
				for (auto& el : PLANTUML_NOTE_START_WhERE)
					isEnter |= line.find(el, isEnterPos) != std::string::npos;
				if (currentRef.empty() && isEnter == false)
				{
					log("Точка входа в алгоритм файла \"" + path + "\" не определена.");
					return false;
				}
				if (line.find(PLANTUML_NODE_NAME_SIMPLE_CH) != std::string::npos)
				{
					algorithmName += line.substr(line.find(PLANTUML_NODE_NAME_SIMPLE_CH) + 1);
					StringFunction::removeExtraSpacesFromCodeText(algorithmName);
					break;
				}
				else
					continue;
			}
		}
		else
		{
			if (wasNote && line.find(PLANTUML_NOTE_END) == std::string::npos)
				algorithmName += line;
			break;
		}
		if (needIgnoreData)
			continue;
		if (line.find(PLANTUML_DETALED_PAR) != std::string::npos ||
			line.find(PLANTUML_DETALED_PAR_2) != std::string::npos)
		{
			needIgnoreData = true;
			continue;
		}
		if (isInitNote == false)
		{
			if (line.find(PLANTUML_LOCAL_PAR) != std::string::npos)
			{
				if (parseAlgorithmArgs(stream, currentAlgorithm->localParam, { PLANTUML_LOCAL_PAR_CH }) == false)
				{
					log("Файл: " + path + ". Некорректно прописаны локальные аргументы в составных частях алгоритма.");
					return false;
				}
			}
			else if (wasNote == false && (line.front() == ':' || isWordKeyWord(line, (std::string*)(nullptr))))
			{
				stream.seekg(prevFilePos);
				break;
			}
			prevFilePos = static_cast<std::string::size_type>(stream.tellg());
			continue;
		}

		if (line.find(PLANTUML_PAR_MARKER) == std::string::npos)
		{
			if (currentRef.empty())
			{
				if (currentAlgorithm->inputParam.empty() &&
					currentAlgorithm->localParam.empty() && currentAlgorithm->outerParam.empty())
				{
					if (algorithmName.size())
						algorithmName += ' ';
					algorithmName += line;
				}
				else
				{
					if (isInitNote == false || algorithmName.size())
					{
						log("Файл: " + path + ". Некорректно прописан главный вход в алгоритм.");
						return false;
					}
				}
			}
			continue;
		}
		std::vector<char> letters;
		std::string whereMessage;
		auto storagePtr = defineArgsType(line, whereMessage, letters);
		if (storagePtr != nullptr)
		{
			if (parseAlgorithmArgs(stream, *storagePtr, letters) == false)
			{
				log("Файл: " + path + ". Некорректно прописаны " + whereMessage + " аргументы в начале алгоритма.");
				return false;
			}
		}
		else
		{
			log("Файл: " + path + ". Неожиданная строка в объявлении алгоритма \"" + line + "\".");
			return false;
		}


	}
	for (auto& el : this->currentAlgorithm->inputParam)
		removeNewlinrFromNote(el->title);
	for (auto& el : this->currentAlgorithm->localParam)
		removeNewlinrFromNote(el->title);
	for (auto& el : this->currentAlgorithm->outerParam)
		removeNewlinrFromNote(el->title);
	removeNewlinrFromNote(algorithmName);
	return true;
}

bool AlgorithmReader::parseReference(std::istream& stream, std::string& referenceData, const std::string& path)
{
	auto refPos = referenceData.find(PLANTUML_REF_WORD);
	if (refPos == std::string::npos)
	{
		refPos = referenceData.find(PLANTUML_REF_OUT_WORD);
		if (refPos == std::string::npos)
		{
			if (referenceData.size() < PLANTUML_MIN_REF_SIZE || algorithmName.empty())
			{
				log("Некорректная ссылка \"" + referenceData + "\"" + (algorithmName.size() ? " в алгоритме \"" + algorithmName + "\"" : "") +
					".Файл: " + path);
				return false;
			}
			currentAlgorithm->algorithmData.emplace_back(
				new BaseAlgorithmCommand{ referenceData.substr(1,referenceData.size() - 2),COMMAND_TYPE::GOTO});
		}
		else
		{
			auto currentOutName = referenceData.substr(1 + std::strlen(PLANTUML_REF_OUT_WORD), 
				referenceData.size() - 1 + std::strlen(PLANTUML_REF_OUT_WORD) -1);
			currentOutName.pop_back();
			StringFunction::removeExtraSpacesFromCodeText(currentOutName);

			if (currentRef.empty())
			{
				if (currentOutName.size())
				{
					log("В файле: \"" + path + "\" некорректное кол-во входов и выходов алгоритма.");
					return false;
				}
			}
			currentRef.clear();
			currentAlgorithm->algorithmData.emplace_back(new BaseAlgorithmCommand{
			std::move(currentOutName), COMMAND_TYPE::END_MARKER });
		}
	}
	else
	{
		auto refName = StringFunction::removeExtraSpacesFromCodeText(referenceData.substr(refPos + strlen(PLANTUML_REF_WORD),
			referenceData.size() - refPos - strlen(PLANTUML_REF_WORD) - 1));
		if (!refName.empty())
		{
			if (currentRef.empty())
			{
				currentAlgorithm->algorithmData.emplace_back(new BaseAlgorithmCommand{ refName,COMMAND_TYPE::GOTO_MARKER });
				currentRef = refName;
#ifdef EXTENDED
				currentAlgorithm->refs.emplace_back(refName);
#endif // EXPANDED
			}
			else
			{
				log("Некорректное расположение ссылки \"" + refName + "\"" + 
					(algorithmName.size() ? " в алгоритме \"" + algorithmName + "\"" : "") +
				".Файл: " +path);
				return false;
			}
		}
		return parseNote(stream, path, refName.empty());

	}
	return true;
}

std::pair<bool, std::int32_t>AlgorithmReader::useDelta(std::vector<std::int32_t>& array) const
{
	std::int32_t delta{0};
	for (auto& el : array)
	{
		if (el == 0)
			continue;
		if (delta == 0)
			delta = el;
		else if (el != delta)
			return std::make_pair(false, std::int32_t{});
		el = std::abs(el / delta);
	}
	return std::make_pair(true, delta);
}

std::vector<std::int32_t> AlgorithmReader::getBlockExpandDelta(const std::vector<std::string>& begin, const std::vector<std::string>& end) const
{
	std::vector<std::int32_t> deltas;
	if (begin.size() == end.size())
	{
		for (std::vector<std::int32_t>::size_type index = 0; index < begin.size(); ++index)
		{
			auto &firsVar = begin.at(index);
			auto &secondVar = end.at(index);
			if ((StringFunction::_isdigit(firsVar.front()) || firsVar.front() == *MINUS_SYMBOL) != 
				(secondVar.front() == *MINUS_SYMBOL || StringFunction::_isdigit(secondVar.front())))
			{
				deltas.clear();
				break;
			}
			if (StringFunction::_isdigit(firsVar.front()))
			{
				try
				{
					deltas.emplace_back(std::stoi(secondVar) - std::stoi(firsVar));
				}
				catch (...)
				{
					deltas.clear();
					break;
				}
			}
			else
				if (firsVar != secondVar)
				{
					deltas.clear();
					break;
				}
		}

	}
	return deltas;
}

std::vector<std::string> AlgorithmReader::splitBlockData(std::string line) const
{
	StringFunction::removeExtraSpacesFromCodeText(line);
	std::vector<std::string> result(1, std::string{});
	if (!line.empty())
	{
		bool isNum{ StringFunction::_isdigit(line.front())  || line.front() == *MINUS_SYMBOL };
		for (auto& ch : line)
		{
			if ((StringFunction::_isdigit(ch) || line.front() == *MINUS_SYMBOL) != isNum)
			{
				result.emplace_back(std::string{ ch });
				isNum = !isNum;
			}
			else
				result.back().push_back(ch);
		}
	}
	for (auto& el : result)
	{
		auto minusCount = std::count(el.begin(), el.end(), *MINUS_SYMBOL);
		if (minusCount == 0 || (minusCount == 1 || el.front() == *MINUS_SYMBOL))
			continue;
		else
			return {};
	}
	return result;
}

std::pair<bool, std::string> AlgorithmReader::textBlockExpand(const std::string& begin, const std::string& end, const std::string& line, const std::string& path) const
{
	auto firstLineData = Readers::CSVLineReader(begin, ASSIGN_SIGN);
	auto secondLineData = Readers::CSVLineReader(end, ASSIGN_SIGN);

	std::vector<std::vector<std::string>> splittedFirstData(firstLineData.size(), std::vector<std::string>{}),
		splittedSecondData(secondLineData.size(), std::vector<std::string>{});

	for (decltype(firstLineData)::size_type i = 0; i < firstLineData.size(); ++i)
		splittedFirstData.at(i) = splitBlockData(firstLineData.at(i));
	for (decltype(secondLineData)::size_type i = 0; i < secondLineData.size(); ++i)
		splittedSecondData.at(i) = splitBlockData(secondLineData.at(i));
	if (splittedSecondData.size() == splittedFirstData.size() && splittedFirstData.size() == 2)
	{
		auto leftDelta = getBlockExpandDelta(splittedFirstData.front(), splittedSecondData.front());
		if (!leftDelta.empty())
		{
			auto rightDelta = getBlockExpandDelta(splittedFirstData.back(), splittedSecondData.back());
			if (!rightDelta.empty())
			{
				auto lDelta = useDelta(leftDelta);
				auto rDelta = useDelta(rightDelta);
				auto minPosDelta = std::min(std::abs(lDelta.second), std::abs(rDelta.second));
				auto maxPosDelta = std::max(std::abs(lDelta.second), std::abs(rDelta.second));
				if (lDelta.first == true && rDelta.first == true && (minPosDelta == 0 ||
					maxPosDelta % minPosDelta == 0))
				{
					std::string expandedLine(1, SPACE_SIGN);
					auto count = static_cast<std::uint32_t>(minPosDelta == 0 ? maxPosDelta : minPosDelta);
					auto firstUseDelta = lDelta.second / count;
					auto secondUseDelta = rDelta.second / count;
					for (std::uint32_t i = 1u, j = 0u, k = 0u; i < count; ++i, j = 0, k = 0)
					{
						std::string addLine;
						for (auto& el : splittedFirstData.front())
						{
							if (StringFunction::_isdigit(el.front()))
								addLine += std::to_string(static_cast<std::int32_t>(std::stoi(el) + i * firstUseDelta * leftDelta.at(j++)));
							else
								addLine += el;
						}
						addLine += " = ";
						for (auto& el : splittedFirstData.back())
						{
							if (StringFunction::_isdigit(el.front()))
								addLine += std::to_string(static_cast<std::int32_t>(std::stoi(el) + i * secondUseDelta * rightDelta.at(k++)));
							else
								addLine += el;
						}
						expandedLine += addLine + SPACE_SIGN;
					}
					return std::make_pair(true, expandedLine);
				}
				else
					log("Невозможно раскрыть строку (Несоразмерные дельты): \"" + line + "\". Файл: " + path);
			}
			else
				log("Невозможно раскрыть строку (Некорректная r-дельта): \"" + line + "\". Файл: " + path);
		}
		else
			log("Невозможно раскрыть строку (Некорректная l-дельта): \"" + line + "\". Файл: " + path);
	}
	else
		log("Невозможно раскрыть строку (Некорректный размер блоков начала и конца): \"" + line + "\". Файл: " + path);
	return std::pair<bool, std::string>{false, std::string{}};
}

bool AlgorithmReader::blockExpansion(std::string& line, const std::string& path) const
{
	auto workLine = line;
	auto expansionPos = workLine.find(EXPANSION_SYMBOL);
	while (expansionPos != std::string::npos)
	{
		auto startExpansionPos = workLine.find_last_of(ASSIGN_LINE_SIGN, expansionPos);
		if (startExpansionPos == std::string::npos)
		{
			log("Файл: " + path + ". Отсутствует символ \"=\" перед символами \"" EXPANSION_SYMBOL "\" В строке: \"" + line + "\".");
			return false;
		}
		auto endExpansionPos = workLine.find(ASSIGN_LINE_SIGN, expansionPos);
		if (endExpansionPos == std::string::npos)
		{
			log("Файл: " + path + ". Отсутствует символ \"=\" после символов \"" EXPANSION_SYMBOL "\" В строке: \"" + line + "\".");
			return false;
		}

		auto iter = std::find(std::next(workLine.rbegin(), workLine.size() - startExpansionPos + 1), workLine.rend(), SPACE_SIGN);
		auto distance = std::distance(workLine.begin(), iter == workLine.rend() ? (iter - 1).base() : iter.base());
		auto firstAssignment = workLine.substr(distance, expansionPos - distance - 1);
		decltype(firstAssignment) secondAssignment;
		
		auto afterExpansionPos = workLine.find(ASSIGN_SIGN, endExpansionPos + 1);
			
		if (afterExpansionPos == std::string::npos)
		{
			secondAssignment = workLine.substr(expansionPos + 4, workLine.size() - (expansionPos + 4) - 1);
			if (StringFunction::checkStringBalance(secondAssignment, LEFT_SQARE_BRACKET, RIGHT_SQARE_BRACKET) != 0)
			{
				std::int32_t nonCloseBlockCount{};
				for (std::string::size_type i = secondAssignment.find(ASSIGN_SIGN); i < secondAssignment.size(); ++i)
				{
					if (secondAssignment.at(i) == LEFT_SQARE_BRACKET)
						++nonCloseBlockCount;
					else if (secondAssignment.at(i) == RIGHT_SQARE_BRACKET)
					{
						if (--nonCloseBlockCount < 0)
						{
							secondAssignment = secondAssignment.substr(0, i);
							break;
						}

					}
				}
			}
		}
		else
		{
			iter = std::find(std::next(workLine.rbegin(), workLine.size() - afterExpansionPos + 1), workLine.rend(), SPACE_SIGN);
			distance = std::distance(workLine.begin(), iter.base() - 1);
			secondAssignment = workLine.substr(expansionPos + 4, distance - (expansionPos + 4));
			if (secondAssignment.find('\n') != std::string::npos)
			{
				secondAssignment = StringFunction::removeExtraSpacesFromCodeText(secondAssignment.substr(0, secondAssignment.find('\n')));
				if (secondAssignment.back() == PUML_CALCULATION_END)
					secondAssignment.pop_back();
			}
			afterExpansionPos = secondAssignment.find(ASSIGN_SIGN, secondAssignment.find(ASSIGN_SIGN) + 1);
			if (afterExpansionPos != std::string::npos)
			{
				//TODO: Нет реализации, необходим пример. Не уверен, что подобный случай может существовать
				log("Нет реализации для случая раскрытия строки \"" + line + "\". Файл: " + path);
			}
		}
		auto blockData = textBlockExpand(firstAssignment, secondAssignment, line, path);
		if (blockData.first == false)
			return false;
		workLine = workLine.substr(0, expansionPos - 1) + blockData.second + workLine.substr(expansionPos + 4);
		expansionPos = workLine.find(EXPANSION_SYMBOL);
	}
	line = std::move(workLine);
	return true;
}

bool AlgorithmReader::readKeyWordBlock(std::istream & stream, std::string & line, const std::string & keyWord, const std::string & path) const
{
	std::string newLine;
	std::string endWord{ PLANTUML_END_WORD + keyWord };
	while (std::getline(stream, newLine))
	{
		StringFunction::removeExtraSpacesFromCodeText(newLine);
		if (newLine.empty() || newLine.front() == PLANTUML_SETTING_SIGN1 ||
			isCommentLine(stream, newLine))
			continue;
		if (newLine.front() == PLANTUML_SETTING_SIGN2)
			return false;
		for (auto& servSymbol : servSymbols)
			if (std::find(newLine.begin(), newLine.end(), servSymbol) != newLine.end())
			{
				this->log("Строка \"" + newLine + "\" содержит зарезервированные символы " MAKE_STR_(SERVICE_SYMBOLS) ".");
				return false;
			}
		std::string kewWord;
		if (isWordKeyWord(newLine, &kewWord))
		{
			if (readKeyWordBlock(stream, newLine, kewWord, path) == false)
				return false;
			StringFunction::appendString(newLine, line, '\n');
		}
		else
		{
			StringFunction::appendString(newLine, line, '\n');
			if (newLine.find(endWord) == 0)
				return true;
		}
	}
	this->log("Ошибка в считывания \"Scope\"-блока. Файл: " + path + ". Блок: " + line);
	return false;
}

bool AlgorithmReader::isCommentLine(std::istream & stream, std::string & line) const
{
	if (line.empty() || line.front() == PLANTUML_SINGLE_COMMENT)
		return true;
	auto singlelineCommentPos = line.find(PLANTUML_SINGLE_COMMENT);
	auto multilineCommentPos = line.find(PLANTUML_BLOCK_COMMENT_OPEN);
	if (singlelineCommentPos < multilineCommentPos)
		line.erase(singlelineCommentPos);
	else if (multilineCommentPos != std::string::npos)
	{
		auto tempData = line.substr(0, multilineCommentPos);
		line.erase(0, multilineCommentPos);

		std::streampos currentFilePos{};
		do
		{
			currentFilePos = stream.tellg();
			multilineCommentPos = line.find(PLANTUML_BLOCK_COMMENT_CLOSE);
			if (multilineCommentPos != std::string::npos)
				break;
		} while (std::getline(stream, line));
		line.erase(0, multilineCommentPos + std::strlen(PLANTUML_BLOCK_COMMENT_CLOSE));
		StringFunction::appendString(line, tempData);
		std::swap(line, tempData);
	}
	StringFunction::removeExtraSpacesFromCodeText(line);
	return line.empty();
}
static std::uint32_t substringCounter(const std::string& str, const std::string& substring)
{
	std::uint32_t count = 0;
	for (auto offset = str.find(substring); 
		offset != std::string::npos; 
		offset = str.find(substring, offset + substring.length()))
		++count;
	return count;
}
bool AlgorithmReader::readPumlBlock(std::istream & stream, std::string & line, const std::string & path) const
{
	line.clear();
	std::string newLine;
	bool isEndRead{ false };
	while (std::getline(stream, newLine))
	{
		StringFunction::removeExtraSpacesFromCodeText(newLine);
		if (newLine.empty() || newLine.front() == PLANTUML_SETTING_SIGN1 ||
			 isCommentLine(stream, newLine))
			continue;
		if (newLine.front() == PLANTUML_SETTING_SIGN2)
		{
			if (newLine == PLANTUML_DIAG_END_WORD)
			{
				if (line.empty())
				{
					isEndRead = true;
					break;
					
				}
				else
				{
					this->log("Некорректное окончание файла: " + path + ". Файл должен оканчиваться " PLANTUML_DIAG_END_WORD);
					return false;
				}
			}
			else
				continue;
		}
		for (auto& servSymbol : servSymbols)
		{
			auto _symbol = std::find(newLine.begin(), newLine.end(), servSymbol);
			if (std::find(newLine.begin(), newLine.end(), servSymbol) != newLine.end())
			{
				if (*_symbol == '$')
				{
					auto substringCount{ substringCounter(newLine,PLANTUML_CODE_COMMENT_START_) +
						substringCounter(newLine,PLANTUML_CODE_COMMENT_END_) };
					if (std::count(newLine.begin(), newLine.end(), *_symbol) ==  substringCount)
						continue;
				}
				this->log("Строка \"" + newLine + "\" содержит зарезервированные символы " MAKE_STR_(SERVICE_SYMBOLS) ".");
				return false;
			}
		}
		//TODO: Возможно некорректно работает (судя по прошлой версии)
		if (newLine.find(PLANTUML_DETACH) == 0)
			continue;
		StringFunction::appendString(newLine, line);
		if (std::find(terminateSymbols.begin(), terminateSymbols.end(), line.back()) != terminateSymbols.end())
		{
			isEndRead = true;
			break;
		}
		else
		{
			auto iter = std::find_if(keyWordSymbols.begin(), keyWordSymbols.end(), [&line](const std::string& str)
			{
				return line.find(str) == 0;
			});
			if (iter == keyWordSymbols.end() || line != newLine)
				continue;
			if (readKeyWordBlock(stream, line, *iter, path))
			{
				isEndRead = true;
				break;
			}
			else
				return false;
		}
	}
	if (isEndRead)
		isEndRead = blockExpansion(line, path);
	else
		log("Возможно некорректное окончание файла \"" + path + "\". Файл должен оканчиваться строкой \"" PLANTUML_DIAG_END_WORD "\".");
	return isEndRead;
}

bool AlgorithmReader::parse(std::istream & stream, const std::string & path)
{
	std::string line;
	bool isOkRead{ true };
	while (!stream.eof() && isOkRead)
	{
		if (readPumlBlock(stream, line, path) == false)
			return false;
		if (line.empty())
			continue;
		switch (line.back())
		{
		case PUML_FUNCTION_END:
			isOkRead &= parseFunction(line, path);
			break;
		case PUML_REF_END:
			isOkRead &= parseReference(stream, line, path);
			break;
		case PUML_CALCULATION_END:
			isOkRead &= parseCalculation(line, path);
			break;
		default:
			isOkRead &= parseKeyWords(line, path);
			break;
		}
	}
	return isOkRead;
}

AlgorithmReader::AlgorithmReader()
	: algorithmList{new storage_type ::element_type}
{}

AlgorithmReader::~AlgorithmReader() = default;

void AlgorithmReader::clear()
{
	algorithmList.reset( new storage_type::element_type );
}

bool AlgorithmReader::read(const std::string & path)
{
	FileReading::unique_ifstream_type stream
	{ new FileReading::unique_ifstream_type::element_type{ path, std::ios::binary | std::ios::in },
		FileReading::closeFunction };
	if (!stream->is_open())
	{
		this->log("Невозможно открыть файл: " + path);
		return false;
	}
	currentAlgorithm.reset(new algorithm_data_type);
#ifdef  EXTENDED
	currentAlgorithm->refs.emplace_back();
	currentAlgorithm->path = path;
#endif //  EXPANDED
	auto readResult = parse(*stream, path) && configureOutputArgs(path);
	if (readResult)
	{
		readResult &= currentRef.empty();
		if (readResult)
		{
			readResult &= algorithmList->emplace(algorithmName, currentAlgorithm).second;
			if (readResult == false)
				log("Повторное использование названия алгоритма \"" + algorithmName + "\". Файл: " + path);
		}
		else
			log("В алгоритме \"" + algorithmName + "\" отсутствует выход из части алгоритма. Файл: " + path);
	}
	else
		log("Ошибка в считывании файла \"" + path + "\".");
	algorithmName.clear();
	currentAlgorithm.reset();
	currentRef.clear();
	return readResult;
}

static std::string sepArgs(const std::string& functionLine)
{
	auto pos = functionLine.find(PUML_SINGNATURE_ARGS_SPLIT);
	if (pos != std::string::npos)
	{
		std::string result = functionLine.substr(0, pos);
		std::reverse(result.begin(), result.end());
		decltype(pos) startPos{};
		auto balance = StringFunction::checkStringBalance(result, RIGHT_SIMPLE_BRACKET, LEFT_SIMPLE_BRACKET, &startPos);
		if (balance == 0)
		{
			result = result.substr(0, startPos+1);
			std::reverse(result.begin(), result.end());
			return result + functionLine.substr(pos);
		}

	}
	return {};
}

std::shared_ptr<InfoTableLight> AlgorithmReader::getSignals() const
{
	std::shared_ptr<InfoTableLight> existedSignals{ new InfoTableLight };
	for (auto& algorithm : *algorithmList)
	{
		for (auto& term : algorithm.second->algorithmData)
		{
			switch (term->command)
			{
			case COMMAND_TYPE::ELSE_KEY:
			case COMMAND_TYPE::END_IF_KEY:
			case COMMAND_TYPE::END_WHILE_KEY:
			case COMMAND_TYPE::END_SWITCH_KEY:
			case COMMAND_TYPE::COMMENT:
			case COMMAND_TYPE::SWITCH_COMMENT:
			case COMMAND_TYPE::GOTO:
			case COMMAND_TYPE::GOTO_MARKER:
			case COMMAND_TYPE::END_MARKER:
			case COMMAND_TYPE::END_CASE_KEY:
				continue;
			case COMMAND_TYPE::IF_KEY:
			case COMMAND_TYPE::ELIF_KEY:
			case COMMAND_TYPE::ELIF_SEP_KEY:
			case COMMAND_TYPE::WHILE_KEY:
			case COMMAND_TYPE::CALCULATION:
			case COMMAND_TYPE::CASE_KEY:
				checkSignalsInLine(*existedSignals, term->line);
				break;
			case COMMAND_TYPE::FUNCTION:
			{
				std::string argsLine = sepArgs(term->line);
				if (argsLine.size())
					checkSignalsInLine(*existedSignals, argsLine);
				break;
			}				
			default:
				return nullptr;
			}
		}
		checkStructInArgs(*existedSignals, algorithm.second->inputParam);
		checkStructInArgs(*existedSignals, algorithm.second->localParam);
		checkStructInArgs(*existedSignals, algorithm.second->outerParam);
	}
	return existedSignals;
}

AlgorithmReader::storage_type AlgorithmReader::getAlgorithms()
{
	return algorithmList;
}
