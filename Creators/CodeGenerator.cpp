#include "CodeGenerator.hpp"
#include "../Common/FileReading.hpp"
#include "../Common/Def.hpp"
#include "../Common/StringFunction.hpp"
#include "../Common/CSVReader.hpp"
#include <functional>
#include <iterator>

CodeGenerator::CodeGenerator(TypeDictionary*&& newDictionary, OneFileGenerator*&& newGenerator)
{
	dictionary.reset(newDictionary);
	generator.reset(newGenerator);
	inlineFunctions = 
	{
		std::make_pair(TERM::SQRT, inline_function_list_type{&OneFileGenerator::genSqrt, &OneFileGenerator::genSqrtf,&OneFileGenerator::genSqrti}),
		std::make_pair(TERM::ABS_OPEN, inline_function_list_type{&OneFileGenerator::genAbs, &OneFileGenerator::genAbsf,&OneFileGenerator::genAbsi}),
		std::make_pair(TERM::EXP, inline_function_list_type{&OneFileGenerator::genExp, &OneFileGenerator::genExpf,&OneFileGenerator::genExpi})
	};
}

std::string CodeGenerator::genPath(const std::string& fileName, bool isExtension, bool isHeader) const
{
	std::string path{getOutput() + fileName};
	if (isExtension)
	{
		if (isHeader == false)
			path += generator->sourceFileExtension();
		else
		{
			auto twoFileGenerator = dynamic_cast<TwoFileGenerator*>(generator.get());
			if (twoFileGenerator != nullptr)
				path += twoFileGenerator->headerFileExtension();
			else
			{
				log("Задан некорректный генератор - невозможно сгенерировать расширения header-файла.");
				path.clear();
			}
		}
	}
	return path;
}

void CodeGenerator::changeOffset(const std::int32_t offsetChange)
{
	if (offsetChange == 0)
		return;
	if (offsetInFile.size() == 1 && offsetChange == -1)
		offsetInFile.clear();
	else
	{
		if (offsetChange < 0 && offsetInFile.size() < static_cast<decltype(offsetInFile)::size_type>(abs(offsetChange)))
		{
			log("Отступ не может быть отрицательным - возможно некорректная настройка словаря/генератора. Отступ принят за 0.");
			offsetInFile.clear();
		}
		if (offsetChange > 0)
			offsetInFile.resize(offsetInFile.size() + offsetChange, '\t');
		else if (offsetChange < 0)
			offsetInFile.erase(offsetInFile.end() + offsetChange, offsetInFile.end());
	}
}

void CodeGenerator::genWithOffset(stream_type& stream, const generator_offset_type& dataForGen)
{
	if (dataForGen.changeOffsetBeforeGen)
		changeOffset(dataForGen.offset);
	if (dataForGen.stringReturn.size())
		genWithoutOffset(stream, dataForGen.stringReturn);
	if (dataForGen.changeOffsetBeforeGen == false)
		changeOffset(dataForGen.offset);
}

void CodeGenerator::genWithoutOffset(stream_type& stream, const generator_simple_type& dataForGen) const
{
	stream << offsetInFile << dataForGen << std::endl;
}

void CodeGenerator::genIncludeFile(stream_type& stream, const std::string& fileForInclude)
{
	auto twoFileGenerator = dynamic_cast<TwoFileGenerator*>(generator.get());
	genWithoutOffset(stream, twoFileGenerator->genInclude(fileForInclude));
}

bool CodeGenerator::genGlobalSignal(stream_type& stream)
{
	auto twoFileGenerator = dynamic_cast<TwoFileGenerator*>(generator.get());
	if (twoFileGenerator == nullptr || 
		twoFileGenerator->needGenGlobalVarInHeader() == OneFileGenerator::BOOL_ANSWER::NO)
		genWithOffset(stream, generator->beforeGenGlobalVar());
	else
		genWithOffset(stream, twoFileGenerator->beforeGenGlobalVarHeader());
	bool isOk{ genConst(stream)};
	isOk &= genTimers(stream);
	isOk &= genGeneralSignals(stream, signalInfo->inputSignals, GEN_INPUT);
	isOk &= genGeneralSignals(stream, signalInfo->innerValues, GEN_INNER);
	isOk &= genGeneralSignals(stream, signalInfo->outputSignals, GEN_OUTPUT);
	if (twoFileGenerator == nullptr ||
		twoFileGenerator->needGenGlobalVarInHeader() == OneFileGenerator::BOOL_ANSWER::NO)
		genWithOffset(stream, generator->afterGenGlobalVar());
	else
		genWithOffset(stream, twoFileGenerator->afterGenGlobalVarHeader());
	return isOk;
}

bool CodeGenerator::genStructTypes(stream_type& stream)
{
	if (signalInfo->structData.size() == 0)
		return true;
	auto order = getOrderForStructGen();
	if (order.empty())
		return false;

	for (auto& currentStructName : order)
	{
		auto& currentStruct = signalInfo->structData.at(currentStructName);

		genDoxygenBriefComment(stream, currentStruct.title);
		genWithOffset(stream, generator->startGenStruct(currentStructName));

		for (StructInfo::storage_type::size_type index = 0; index < currentStruct.structFieldType.size(); ++index)
		{
			auto fieldType = getType(*currentStruct.structFieldType.at(index), true);
			if (fieldType.empty())
				return false;
			auto defaultValue = dictionary->getDefaultValue(getType(currentStruct.structFieldType.at(index)->pumlType, false, 0, true),
				currentStruct.structFieldType.at(index)->isPtrType, currentStruct.structFieldType.at(index)->elementCount);
			auto fieldName = currentStructName + "_" + std::to_string(index);
			genDoxygenFieldComment(stream, generator->genField(fieldType, fieldName, defaultValue) , currentStruct.structFieldType.at(index)->title);
		}
		genWithOffset(stream, generator->endGenStruct(currentStructName));
	}
	return true;
}

std::vector<std::string> CodeGenerator::getOrderForStructGen() const
{
	
	std::vector<std::string> order;
	order.reserve(signalInfo->structData.size());
	while (order.size() != signalInfo->structData.size())
	{
		auto prevOrderSize = order.size();
		for (auto& structInfo : signalInfo->structData)
		{
			if (std::find(order.begin(), order.end(), structInfo.first) != order.end())
				continue;
			bool needEmplace{ true };
			for (auto& field : structInfo.second.structFieldType)
			{
				if (field->pumlType.size())
				{
					if (field->pumlType.front() == STRUCT_SIGNAL)
						if (std::find(order.begin(), order.end(), field->pumlType) == order.end())
							needEmplace = false;
				}
				else
				{
					this->log("Отсутствует тип поля структуры " + structInfo.first);
					return{};

				}
			}
			if (needEmplace)
				order.emplace_back(structInfo.first);
		}
		if (prevOrderSize == order.size())
		{
			log("Невозможно вывести порядок объявления структур. Возможно есть структуры ссылающиеся друг на друга.");
			order.clear();
			break;
		}
	}
	return order;
}

void CodeGenerator::genDoxygenBriefComment(stream_type& stream, const std::string& brief)
{
	if (brief.size())
		genWithoutOffset(stream, dictionary->getDoxygenCommentSymbol() + DOXYGEN_BRIEF + brief);
}

std::string CodeGenerator::getType(const std::string& pumlType, const bool isPtr, const std::uint32_t elementCount, const bool canBeTimer) const
{
	std::string type;
	if (pumlType.empty())
	{
		if (canBeTimer)
			type = dictionary->getTimerType();
	}
	else
	{
		if (pumlType.front() != STRUCT_SIGNAL)
		{
			if (isPtr)
				type = dictionary->getType(pumlType, elementCount);
			else
				type = dictionary->getType(pumlType);
		}
		else
		{
			if (signalInfo->structData.find(pumlType) != signalInfo->structData.end())
				if (isPtr)
					type = dictionary->makePtrable(pumlType, elementCount);
				else
					type = pumlType;
		}
	}
	if (type.empty())
		log("Невозможно определить тип для \"" + pumlType +"\".");
	return type;
}

std::string CodeGenerator::getType(const ValueInfo& valueInfo, const bool canBeTimer) const
{
	return getType(valueInfo.pumlType, valueInfo.isPtrType, valueInfo.elementCount, canBeTimer);
}

void CodeGenerator::genDoxygenFieldComment(stream_type& stream, const std::string& field, const std::string& fieldComment)
{
	std::string line {field};
	if (fieldComment.size())
		StringFunction::appendString(dictionary->getDoxygenCommentFieldSymbol() + " " + fieldComment, line);
	genWithoutOffset(stream, line);
}

bool CodeGenerator::genConst(stream_type& stream)
{
	if (signalInfo->constValues.empty())
		return true;
	bool isOk{ true };
	for (auto& key : getSortedSignalsKeys(signalInfo->constValues))
	{
		if (key.empty())
		{
			log((signalInfo->constValues.at(key).title.size() ? "Ошибка константы \"" + signalInfo->constValues.at(key).title + "\". " :
				"") + "Константа без имени, значение = " + signalInfo->constValues.at(key).value + ".");
			isOk = false;
		}
		else if (signalInfo->constValues.at(key).value.empty())
		{
			log((signalInfo->constValues.at(key).title.size() ? "Ошибка константы \"" + signalInfo->constValues.at(key).title + "\". " :
				"") + "Константа без имени, значение = " + signalInfo->constValues.at(key).value);
			isOk = false;
		}
		else
			genDoxygenFieldComment(stream, generator->genConst(key, signalInfo->constValues.at(key).value), signalInfo->constValues.at(key).title);
	}
	genWithoutOffset(stream);
	return isOk;
}

bool CodeGenerator::genTimers(stream_type & stream)
{
	if (signalInfo->timers.empty())
		return true;
	bool isOk{ true };
	for (auto& key : getSortedSignalsKeys(signalInfo->timers))
	{
		if (key.empty())
		{
			log((signalInfo->timers.at(key).size() ? "Ошибка таймера \"" + signalInfo->timers.at(key) + "\". " :
				"") + "Присутствует таймер без имени.");
			isOk = false;
		}
		else
			genDoxygenFieldComment(stream, generator->genGlobalVariable(dictionary->getTimerType(),
				key,dictionary->getDefaultValue(dictionary->getTimerType(),false, 1) ),
				signalInfo->timers.at(key));
	}
	genWithoutOffset(stream);
	return isOk;
}

bool CodeGenerator::genGeneralSignals(stream_type & stream, const InfoTable::signals_type & signal, const std::string & signalType)
{
	if (signal.empty())
		return true;
	bool isOk{ true };
	for (auto& key : getSortedSignalsKeys(signal))
	{
		auto& signalData = signal.at(key);
		if (signalData.pumlType.empty())
		{
			log(signalType + " сигнал не имеет имени." + (signalData.title.empty() ? "" : signalData.title));
			isOk = false;
		}
		else if (signalData.pumlType.empty())
		{
			log(signalType + " " + key + " сигнал не имеет типа.");
			isOk = false;
		}
		else
		{
			auto type = getType(signalData, false);
			if (type.empty())
			{
				log(signalType + " " + key + " имеет некорректный тип " + signalData.pumlType);
				isOk = false;
				continue;
			}
			auto defaultValue = dictionary->getDefaultValue(dictionary->getType(signalData.pumlType),
				signalData.isPtrType, signalData.elementCount);
			genDoxygenFieldComment(stream, generator->genGlobalVariable(type, key, defaultValue),
				signalData.title);
		}
	}
	genWithoutOffset(stream);
	return isOk;
}

bool CodeGenerator::genFunctions(stream_type & stream, const FILE_TYPE_GEN type)
{
	
	auto sortedFunctions = getSortedFunctionsKeys();
	bool isOk{ sortedFunctions.empty() == false };
	if (isOk)
	{
		for (auto& algorithmKey : sortedFunctions)
			isOk &= genFunctions(stream, *algorithmInfo->at(algorithmKey), algorithmKey, type);
		auto libGenerator = dynamic_cast<LibGenerator*>(generator.get());
		if (type != FILE_TYPE_GEN::LIB_HEADER && type != FILE_TYPE_GEN::LIB_SOURCE &&
			libGenerator != nullptr && libGenerator->needGenAccessFunction() == TwoFileGenerator::BOOL_ANSWER::YES)
		{
			isOk &= genAccessFunction(stream, signalInfo->timers, type);
			isOk &= genAccessFunction(stream, signalInfo->inputSignals, type);
			isOk &= genAccessFunction(stream, signalInfo->innerValues, type);
			isOk &= genAccessFunction(stream, signalInfo->outputSignals, type);
		}
	}
	return isOk;
}

bool CodeGenerator::genFunctions(stream_type & stream, const AlgorithmInfo & algorithmData, const std::string & algorithmName, const FILE_TYPE_GEN type, const bool isAccessFunction)
{
	if (genFunctionComment(stream, algorithmData, algorithmName, isAccessFunction))
	{
		auto nameInCode{ nameConvert(algorithmName) };
		if (genFunctionSignature(stream, algorithmData, nameInCode, type, isAccessFunction))
			if (type == FILE_TYPE_GEN::SOURCE)
			{
				if (genFunctionBody(stream, algorithmData, nameInCode, isAccessFunction))
				{
					genWithoutOffset(stream);
					return true;
				}
			}
			else
			{
				genWithoutOffset(stream);
				return true;
			}

	}
	return false;
}

bool CodeGenerator::genFunctionComment(stream_type & stream, const AlgorithmInfo & algorithmData, const std::string & algorithmName, const bool isAccessFunction)
{
	if (algorithmName.empty())
	{
		log("Присутствует анонимный алгоритм");
		return false;
	}
	genDoxygenBriefComment(stream, algorithmName);
	for (auto& arg : algorithmData.inputParam)
	{
		if (arg->name.empty())
		{
			this->log("Отсутствует имя входной переменной в алгоритме: " + algorithmName);
			return false;
		}
		genDoxygenParamComment(stream, arg->name, arg->title,
			arg->argInfo == ABILITY_IN_ARGS::PTRABLE || arg->argInfo == ABILITY_IN_ARGS::PTRABLE_AND_RETURNABLE
			|| arg->argInfo == ABILITY_IN_ARGS::RETURNABLE
			? DOXYGEN_PARAM_IN_OUT : DOXYGEN_PARAM_IN);
	}
	bool isOneReturn{ checkIsOneReturnAlgorithm(algorithmData) };
	for (auto& arg : algorithmData.localParam)
	{
		if (arg->argInfo == ABILITY_IN_ARGS::RETURNABLE)
		{
			if (arg->name.empty())
			{
				this->log("Отсутствует имя локальной переменной в алгоритме: " + algorithmName);
				return false;
			}
			genDoxygenParamComment(stream, isOneReturn ? "" : arg->name, arg->title,
				isOneReturn ? DOXYGEN_RETURN : DOXYGEN_PARAM_OUT);
		}
	}
	for (auto& arg : algorithmData.outerParam)
	{
		if (arg->name.front() == _PLANTUML_OUTPUT_PAR_CH || isAccessFunction)
		{
			if (arg->name.empty())
			{
				this->log("Отсутствует имя локальной переменной в алгоритме: " + algorithmName);
				return false;
			}
			genDoxygenParamComment(stream, isOneReturn ? "" : arg->name, arg->title,
				isOneReturn ? DOXYGEN_RETURN : DOXYGEN_PARAM_IN_OUT);
		}
	}
	return true;
}

void CodeGenerator::genDoxygenParamComment(stream_type & stream, const std::string & param, const std::string& paramCommand, const std::string& argsType) const
{
	std::string line{ dictionary->getDoxygenCommentSymbol() };
	line += argsType;
	StringFunction::appendString(param, line);
	if (paramCommand.size())
		StringFunction::appendString(paramCommand, line);
	genWithoutOffset(stream, line);
}

bool CodeGenerator::genFunctionSignature(stream_type & stream, const AlgorithmInfo & algorithmData, const std::string & algorithmName, const FILE_TYPE_GEN type, const bool isAccessFunction)
{
	
	OneFileGenerator::return_with_offset_type outputInfo;

	switch (type)
	{
	case FILE_TYPE_GEN::SOURCE:
		genWithOffset(stream, generator->beforeGenSignature());
		outputInfo = generator->afterGenSignature();
		break;
	case FILE_TYPE_GEN::HEADER:
	{
		auto twoFileGenerator = dynamic_cast<TwoFileGenerator*>(generator.get());
		if (twoFileGenerator == nullptr)
		{
			log("Установлен некорректный генератор");
			return false;
		}
		genWithOffset(stream, twoFileGenerator->beforeGenSignatureHeader());
		outputInfo = twoFileGenerator->afterGenSignatureHeader();
		break;
	}
	case FILE_TYPE_GEN::LIB_SOURCE:
	{
		auto libGenerator = dynamic_cast<LibGenerator*>(generator.get());
		if (libGenerator == nullptr)
		{
			log("Установлен некорректный генератор");
			return false;
		}
		genWithOffset(stream, libGenerator->beforeGenSignatureLib());
		outputInfo = libGenerator->afterGenSignatureLib();
		break;
	}
	default:
		log("Некорректный тип генерируемой функции");
		return false;
	}

	auto returnType{ dictionary->getVoidType() };
	if (isAccessFunction)
	{
		if (algorithmData.outerParam.front()->pumlType.empty())
			returnType = dictionary->makePtrable(dictionary->getTimerType());
		else
			returnType = dictionary->makePtrable(dictionary->getType(algorithmData.outerParam.front()->pumlType));
	}
	else
	{
		bool isOneReturn = checkIsOneReturnAlgorithm(algorithmData);
		if (isOneReturn)
		{
			for (auto& el : algorithmData.outerParam)
			{
				if (el->name.front() != PLANTUML_INPUT_PAR_CH)
				{
					returnType = returnType =
						dictionary->getType(el->pumlType);
					if (el->isPtrType)
						returnType = dictionary->makePtrable(returnType);
					break;
				}
			}
		}

		if (isOneReturn && returnType == dictionary->getVoidType())
		{
			log("Невозможно определить возвратный тип функции " + algorithmName);
			return false;
		}
	}
	auto args = buildArgsForSignature(algorithmData, algorithmName);
	

	switch (type)
	{
	case FILE_TYPE_GEN::SOURCE:
	{
		auto libGenerator = dynamic_cast<LibGenerator*>(generator.get());
		std::string signature;
		if (libGenerator != nullptr || nameConvert(generator->mainFunctionName()) != algorithmName)
			signature = generator->genSignature(returnType, algorithmName, args.first);
		else
			signature = generator->genSignatureMain(returnType, algorithmName, args.first);
		genWithoutOffset(stream, signature);
		break;
	}
	case FILE_TYPE_GEN::HEADER:
	{
		auto libGenerator = dynamic_cast<LibGenerator*>(generator.get());
		auto twoFileGenerator = dynamic_cast<TwoFileGenerator*>(generator.get());
		std::string signature;
		if (libGenerator != nullptr || nameConvert(generator->mainFunctionName()) != algorithmName)
			signature = twoFileGenerator->genSignatureHeader(returnType, algorithmName, args.first);
		else
			signature = twoFileGenerator->genMainSignatureHeader(returnType, algorithmName, args.first);
		genWithoutOffset(stream, signature);
		break;
	}
	case FILE_TYPE_GEN::LIB_SOURCE:
		genWithoutOffset(stream, dynamic_cast<LibGenerator*>
			(generator.get())->genSignatureLib(returnType, algorithmName, args.first));
		break;
	default:
		log("Некорректный тип генерируемой функции");
		return false;
	}

	genWithOffset(stream, outputInfo);
	return true;
}

std::pair<OneFileGenerator::singature_build_args, bool> CodeGenerator::buildArgsForSignature(const AlgorithmInfo & algorithmData, const std::string & algorithmName) const
{
	if (algorithmData.inputParam.empty() && algorithmData.localParam.empty() && algorithmData.outerParam.size() <= 1)
		return std::make_pair<OneFileGenerator::singature_build_args, bool>({}, true);
	OneFileGenerator::singature_build_args result;
	for (auto& arg : algorithmData.inputParam)
	{
		auto type = getType(*arg, true);
		if (arg->argInfo == ABILITY_IN_ARGS::RETURNABLE)
			type = dictionary->makeRefArg(type);
		if (arg->name.empty())
		{
			this->log("Отсутствует имя входной переменной в алгоритме: " + algorithmName);
			return{ {}, false };
		}
		else if (type.empty())
		{
			this->log("Некорректный тип входной переменной в алгоритме: " + algorithmName);
			return{ {}, false };
		}
		else
			result.emplace_back(std::make_pair(std::move(type), arg->name));
	}
	if (checkIsOneReturnAlgorithm(algorithmData))
		return std::make_pair(result, true);
	for (auto& arg : algorithmData.outerParam)
	{
		if ((arg->argInfo != ABILITY_IN_ARGS::RETURNABLE && 
			arg->argInfo != ABILITY_IN_ARGS::PTRABLE_AND_RETURNABLE && arg->name.front() != _PLANTUML_OUTPUT_PAR_CH)
			|| arg->name.front() == PLANTUML_INPUT_PAR_CH)
			continue;
		auto type = getType(*arg, arg->name.front() == _PLANTUML_OUTPUT_PAR_CH);
		if (arg->argInfo == ABILITY_IN_ARGS::RETURNABLE || (arg->name.front() == _PLANTUML_OUTPUT_PAR_CH && arg->argInfo != ABILITY_IN_ARGS::PTRABLE))
			type = dictionary->makeRefArg(type);
		if (arg->name.empty())
		{
			this->log("Отсутствует имя выходной/локальной  переменной в алгоритме: " + algorithmName);
			return{ {}, false };
		}
		else if (type.empty())
		{
			this->log("Некорректный тип выходной/локальной переменной в алгоритме: " + algorithmName);
			return{ {}, false };
		}
		else
			result.emplace_back(std::make_pair(std::move(type), arg->name));

	}
	return std::make_pair(result, true);
}

bool CodeGenerator::genReturn(stream_type & stream, const AlgorithmInfo & algorithmData)
{
	if (algorithmData.outerParam.empty())
		return true;
	if (checkIsOneReturnAlgorithm(algorithmData))
	{
		auto iter = std::find_if(algorithmData.outerParam.begin(), algorithmData.outerParam.end(),
			[](const AlgorithmInfo::param_type& arg) {return arg->name.front() != PLANTUML_INPUT_PAR_CH && arg->isPtrType == false; });
		//TODO: Работоспособно, но может быть не всех случаях
		genWithoutOffset(stream, generator->genReturnBody(iter->get()->name));
	}
	return true;
}

bool CodeGenerator::genFunctionCall(stream_type & stream, const algorithm_data_type & algorithmData, const FunctionAlgorithmCommand & instruction, const std::string& algorithmName)
{
	auto calledAlgorithm = algorithmInfo->find(instruction.calledFunction);
	if (calledAlgorithm == algorithmInfo->end())
	{
		log("Алгоритм \"" + instruction.calledFunction + "\" не определен. Вызов в алгоритме \"" + algorithmName + "\".");
		return false;
	}
	std::vector<std::string> args;
	for (std::size_t i = 0; i < calledAlgorithm->second->inputParam.size(); ++i)
	{
		auto argInfo = calledAlgorithm->second->inputParam.at(i);
		auto signal = termsToCodeLine(algorithmData, instruction.inputFunctionArgs.at(i));
		auto signalData = signalInfo->getSignalInfo(signal, algorithmData);
		if ((argInfo->isPtrType || argInfo->argInfo == ABILITY_IN_ARGS::PTRABLE || argInfo->argInfo == ABILITY_IN_ARGS::RETURNABLE ||
			argInfo->argInfo == ABILITY_IN_ARGS::PTRABLE_AND_RETURNABLE) != signalData.isPtrType)
		{
			if (signalData.isPtrType)
				signal = dictionary->makeUnPtrable(signal);
			else
				signal = dictionary->makeRef(signal);
		}
		args.emplace_back(signal);
	}
	std::string returnData;
	if (checkIsOneReturnAlgorithm(*calledAlgorithm->second))
	{
		//TODO: Может работать некорректно во некоторых случаях - нужны примеры
		if (instruction.outputFunctionArgs.size())
			returnData = termsToCodeLine(algorithmData, instruction.outputFunctionArgs.front());
	}
	else
	{
		if (calledAlgorithm->second->outerParam.size())
		{
			for (std::size_t i = 0; i < calledAlgorithm->second->outerParam.size(); ++i)
			{
				auto argInfo = calledAlgorithm->second->outerParam.at(i);
				auto signal = termsToCodeLine(algorithmData, instruction.outputFunctionArgs.at(i));
				auto signalData = signalInfo->getSignalInfo(signal, algorithmData);
				if ((argInfo->isPtrType || argInfo->argInfo == ABILITY_IN_ARGS::PTRABLE || argInfo->argInfo == ABILITY_IN_ARGS::RETURNABLE ||
					argInfo->argInfo == ABILITY_IN_ARGS::PTRABLE_AND_RETURNABLE) != signalData.isPtrType)
				{
					if (signalData.isPtrType)
						signal = dictionary->makeUnPtrable(signal);
					else
						signal = dictionary->makeRef(signal);
				}
				args.emplace_back(signal);
			}
		}
	}
	genWithoutOffset(stream, generator->genFunctionCall(returnData,	nameConvert(instruction.calledFunction), args));
	return true;
}

bool CodeGenerator::genCommand(stream_type & stream, const std::string & line, const COMMAND_TYPE command)
{
	switch (command)
	{
	case COMMAND_TYPE::SWITCH_COMMENT:
	case COMMAND_TYPE::SWITCH_KEY:
		genWithoutOffset(stream, generator->genSwitchBody(line));
		genWithOffset(stream, generator->startSwitch());
		break;
	case COMMAND_TYPE::WHILE_KEY:
		genWithoutOffset(stream, generator->genWhileBody(line));
		genWithOffset(stream, generator->startWhile());
		break;
	case COMMAND_TYPE::CASE_KEY:
		genWithoutOffset(stream, generator->genCaseBody(line));
		genWithOffset(stream, generator->startCase());
		break;
	case COMMAND_TYPE::ELIF_KEY:
		genWithoutOffset(stream, generator->genIfElseBody(line));
		genWithOffset(stream, generator->startIf());
		break;
	case COMMAND_TYPE::IF_KEY:
		genWithoutOffset(stream, generator->genIfBody(line));
		genWithOffset(stream, generator->startIf());
		break;
	case COMMAND_TYPE::END_SWITCH_KEY:
		genWithOffset(stream, generator->endSwitch());
		break;
	case COMMAND_TYPE::END_WHILE_KEY:
		genWithOffset(stream, generator->endWhile());
		break;
	case COMMAND_TYPE::END_CASE_KEY:
		genWithOffset(stream, generator->endCase());
		break;
	case COMMAND_TYPE::ELSE_KEY:
	case COMMAND_TYPE::END_IF_KEY:
		genWithOffset(stream, generator->endIf());
		if (command == COMMAND_TYPE::ELSE_KEY)
		{
			genWithoutOffset(stream, generator->genElse());
			genWithOffset(stream, generator->startElse());
		}
		break;
	default:
		return false;
	}
	return true;
}

bool CodeGenerator::genFunctionInstruction(stream_type & stream, const AlgorithmInfo & algorithmData, const AlgorithmInfo::command_type & instruction, const std::string & algorithmName)
{
	switch (instruction->command)
	{
	case COMMAND_TYPE::SWITCH_COMMENT:
	case COMMAND_TYPE::COMMENT:
		genWithoutOffset(stream, dictionary->getCommentSymbol() + instruction->line);
		if (instruction->command == COMMAND_TYPE::COMMENT)
			return true;
	case COMMAND_TYPE::WHILE_KEY:
	case COMMAND_TYPE::CASE_KEY:
	case COMMAND_TYPE::IF_KEY:
	case COMMAND_TYPE::ELIF_KEY:
		for (auto& term : std::dynamic_pointer_cast<AlgorithmCommand>(instruction)->terms)
		{
			auto codeLine = termsToCodeLine(algorithmData, term);
			if (codeLine.empty() && term.size() || genCommand(stream, codeLine, instruction->command) == false)
			{
				log("Невозможно сгенерировать строку \"" + instruction->line + "\"в функции " + algorithmName);
				return false;
			}
		}
		return true;
	case COMMAND_TYPE::CALCULATION:
		for (auto& term : std::dynamic_pointer_cast<AlgorithmCommand>(instruction)->terms)
		{
			auto codeLine = termsToCodeLine(algorithmData, term);
			if (codeLine.empty() && term.size())
			{
				log("Невозможно сгенерировать строку \"" + instruction->line + "\" в функции " + algorithmName);
				return false;
			}
			if (codeLine.size())
				genWithoutOffset(stream, codeLine + dictionary->getEndLine());
		}
		return true;
	case COMMAND_TYPE::END_SWITCH_KEY:
	case COMMAND_TYPE::END_WHILE_KEY:
	case COMMAND_TYPE::END_IF_KEY:
	case COMMAND_TYPE::ELSE_KEY:
	case COMMAND_TYPE::END_CASE_KEY:
		return genCommand(stream, instruction->line, instruction->command);
	case COMMAND_TYPE::END_MARKER:
		return genReturn(stream, algorithmData);
	case COMMAND_TYPE::FUNCTION:
		return genFunctionCall(stream, algorithmData,
			*std::dynamic_pointer_cast<FunctionAlgorithmCommand>(instruction),
			algorithmName);
	default:
		return false;
	}
	return true;
}

std::string CodeGenerator::termsToCodeLine(const algorithm_data_type & algorithmData, const AlgorithmCommand::terms_type & terms) const
{
	if (terms.size() && terms.front().second == TERM::UNKNOWN || terms.empty())
		return std::string{};
	AlgorithmCommand::terms_type fixedTerms{ terms.begin(), terms.end() };
	for (AlgorithmCommand::terms_type::size_type i = 0; i < fixedTerms.size(); ++i)
	{
		if (fixedTerms.at(i).second == TERM::SIGNAL)
			fixSignalsInTerms(algorithmData, fixedTerms, i);
		else if (fixedTerms.at(i).second == TERM::SQRT || 
			fixedTerms.at(i).second == TERM::EXP || 
			fixedTerms.at(i).second == TERM::ABS_OPEN)
			fixInlineFunctionInTerms(algorithmData, fixedTerms, i);
		if (fixedTerms.empty())
			return std::string{};
	}
	fixPowInTerms(algorithmData, fixedTerms);
	std::string result;
	const AlgorithmCommand::terms_type::size_type PENULTIMATE_INDEX = fixedTerms.size() - 1;
	for (auto i = 0u; i < fixedTerms.size(); ++i)
	{
		auto &data = *std::next(fixedTerms.begin(), i);
		if (data.second == TERM::NONE)
			continue;

		if ((data.second == TERM::ARRAY_INDEX_CLOSE ||
			data.second == TERM::ABS_CLOSE ||
			data.second == TERM::BRACKET_CLOSE) && result.size())
			result.pop_back();

		result += data.first;

		if ((i != PENULTIMATE_INDEX) &&
			(data.second != TERM::ARRAY_INDEX_OPEN &&
				data.second != TERM::ABS_OPEN &&
				data.second != TERM::BRACKET_OPEN))
			result += ' ';
	}
	auto pos = result.find(INCORRENT_BRACKET_LINE);
	while (pos != std::string::npos)
	{
		result.erase(result.begin() + pos + 1);
		pos = result.find(INCORRENT_BRACKET_LINE);
	}
	pos = result.find_last_not_of(' ');
	if ((pos != result.size() - 1) && std::all_of(result.begin() + pos + 1, result.end(), StringFunction::_isspace))
		result.erase(pos + 1);
	return result;
}

void CodeGenerator::fixSignalsInTerms(const algorithm_data_type & algorithmData, AlgorithmCommand::terms_type & terms, AlgorithmCommand::terms_type::size_type& index) const
{
	auto& data = terms.at(index);
	auto signal = signalInfo->getSignalInfo(data.first, algorithmData);
	auto isStruct = signalInfo->structData.find(signal.pumlType);
	auto dot = data.first.find(STRUCT_FIELD_SYMBOL);
	if (isStruct == signalInfo->structData.end() && signal.isData && dot == std::string::npos)
	{
		auto termsArray = getScope(terms, index, TERM::ARRAY_INDEX_OPEN, TERM::ARRAY_INDEX_CLOSE);
		if (termsArray.size())
		{
			auto innerLine = termsToCodeLine(algorithmData, termsArray);
			if (termsArray.front().second == TERM::UNKNOWN ||
				(signal.isPtrType == false && signal.abitilyInArgs != ABILITY_IN_ARGS::PTRABLE 
					&& signal.abitilyInArgs != ABILITY_IN_ARGS::PTRABLE_AND_RETURNABLE) || innerLine.empty())
			{
				terms.clear();
				return;
			}
			data.first = dictionary->makeUnPtrable(data.first, innerLine);
		}
		else
		{
			if (signal.isPtrType || signal.abitilyInArgs == ABILITY_IN_ARGS::PTRABLE
				|| signal.abitilyInArgs == ABILITY_IN_ARGS::PTRABLE_AND_RETURNABLE ||
				signal.abitilyInArgs == ABILITY_IN_ARGS::RETURNABLE)
				data.first = dictionary->makeUnPtrable(data.first);
		}
	}
	else
	{
		
		signal = signalInfo->getSignalInfo(data.first.substr(0, dot), algorithmData);
		if (signal.isData == false)
			terms.clear();
		else
		{
			isStruct = signalInfo->structData.find(signal.pumlType);
			if (isStruct == signalInfo->structData.end())
				terms.clear();
			else
			{
				signal.isPtrType |= signal.abitilyInArgs == ABILITY_IN_ARGS::PTRABLE ||
					signal.abitilyInArgs == ABILITY_IN_ARGS::PTRABLE_AND_RETURNABLE ||
					signal.abitilyInArgs == ABILITY_IN_ARGS::RETURNABLE;
				auto structLine = structTermsToLine(algorithmData, terms, index, &isStruct->second, &signal);
				if (structLine.empty())
					terms.clear();
				else
				{
					data.first = structLine;
					data.second = TERM::SIGNAL;
				}
			}
		}
		
	}
}

void CodeGenerator::fixInlineFunctionInTerms(const algorithm_data_type & algorithmData, AlgorithmCommand::terms_type & terms, AlgorithmCommand::terms_type::size_type & index) const
{
	auto& data = terms.at(index);
	auto key = data.second;
	AlgorithmCommand::terms_type lineTerms;
	if (data.second != TERM::ABS_OPEN)
		lineTerms = getScope(terms, index, TERM::BRACKET_OPEN, TERM::BRACKET_CLOSE);
	else
	{
		lineTerms = getScope(terms, --index, TERM::ABS_OPEN, TERM::ABS_CLOSE);
		data.second = TERM::VALUE;
	}
	if (lineTerms.empty())
		terms.clear();
	else
	{
		auto innerLine = termsToCodeLine(algorithmData, lineTerms);
		std::size_t offset{};
		if (generator->shoudGuessInlineFunction() == OneFileGenerator::BOOL_ANSWER::YES)
			if ((lineTerms.size() == 1 && lineTerms.front().second == TERM::VALUE) || innerLine.find(GEN_DIV))
				offset = 1;
			else if ((lineTerms.size() == 1 && lineTerms.front().second != TERM::VALUE) || lineTerms.size() != 1)
				offset = 2;
		data.first = (*generator.*inlineFunctions.at(key).at(offset))(innerLine);
	}
}

void CodeGenerator::fixPowInTerms(const algorithm_data_type & algorithmData, AlgorithmCommand::terms_type & terms) const
{
	auto findIter = [](AlgorithmCommand::terms_type & terms, AlgorithmCommand::terms_type::iterator& start)
	{
		return std::find_if(start, terms.end(), [](AlgorithmCommand::terms_type::value_type& el)
		{
			return el.second == TERM::POW;
		});
	};
	auto iter = findIter(terms, terms.begin());
	while (iter != terms.end())
	{
		auto next = std::next(iter);
		std::string pow{ next->first };
		if (next->second == TERM::BRACKET_OPEN)
		{
			auto index = static_cast<std::size_t>(std::distance(terms.begin(), next) - 1);
			auto scope = getScope(terms, index, TERM::BRACKET_OPEN, TERM::BRACKET_CLOSE);
			pow = termsToCodeLine(algorithmData, scope);
		}
		next->second = TERM::NONE;
		auto prev = std::prev(iter);
		std::string value{ prev->first };
		if (prev->second == TERM::BRACKET_CLOSE)
		{
			std::size_t index = 0;
			AlgorithmCommand::terms_type reversed;
			std::reverse_copy(terms.begin(), std::next(iter), std::back_inserter(reversed));
			auto scope = getScope(reversed, index, TERM::BRACKET_CLOSE, TERM::BRACKET_OPEN);
			std::reverse(scope.begin(), scope.end());
			for (auto i = std::prev(iter, scope.size() + 2); i != iter; i = std::next(i))
				i->second = TERM::NONE;
			value = termsToCodeLine(algorithmData, scope);
		}
		prev->second = TERM::NONE;
		iter->first = generator->genPow(value, pow);
		iter = findIter(terms, next);
	}
}

bool CodeGenerator::genFunctionBody(stream_type & stream, const AlgorithmInfo & algorithmData, const std::string & algorithmName, const bool isAccessFunction)
{
	genWithOffset(stream, generator->startFunction());

	if (isAccessFunction)
	{
		
		genWithoutOffset(stream, generator->genReturnBody(
			std::dynamic_pointer_cast<AlgorithmCommand>(algorithmData.algorithmData.back())->
			terms.back().back().first));
	}
	else
	{
		for (auto& signal : algorithmData.localParam)
			if (genLocalSignal(stream, algorithmData,signal, algorithmName) == false)
				return false;
		for (auto& signal : algorithmData.outerParam)
			if (genLocalSignal(stream, algorithmData, signal, algorithmName) == false)
				return false;
		for (auto& instruction : algorithmData.algorithmData)
			if (genFunctionInstruction(stream, algorithmData, instruction, algorithmName) == false)
				return false;
	}

	genWithOffset(stream, generator->endFunction());
	return true;
}

bool CodeGenerator::genLocalSignal(stream_type & stream, const algorithm_data_type& algorithmData, const algorithm_data_type::param_type& signalData, const std::string & algorithmName)
{
	if (signalData->name.empty())
	{
		log("Не указан тип переменной в функции " + algorithmName);
		return false;
	}
	if (signalData->argInfo == ABILITY_IN_ARGS::RETURNABLE || signalData->name.front() == PLANTUML_INPUT_PAR_CH ||
		(checkIsOneReturnAlgorithm(algorithmData) == false && signalData->name.front() == _PLANTUML_OUTPUT_PAR_CH))
		return true;
	auto type = signalData->pumlType;
	if (type.empty())
	{
		log("Не указан тип переменной " + signalData->name + " функции " + algorithmName);
		return false;
	}
	type = dictionary->getType(type);
	auto defaultValue = dictionary->getDefaultValue(type,
		signalData->isPtrType || signalData->argInfo == ABILITY_IN_ARGS::RETURNABLE, signalData->elementCount);
	if (signalData->isPtrType || signalData->argInfo == ABILITY_IN_ARGS::RETURNABLE)
		type = dictionary->makePtrable(type, signalData->elementCount);
	if (type.empty())
	{
		log("Невозможно вывести тип " + signalData->pumlType + " переменной " + signalData->name + " функции " + algorithmName);
		return false;
	}
	auto line = generator->genLocalVariable(type, signalData->name, defaultValue);
	if (signalData->title.size())
	{
		StringFunction::appendString(dictionary->getCommentSymbol(), line, '\t');
		StringFunction::appendString(signalData->title, line);
	}
	genWithoutOffset(stream, line);
	return true;
}

AlgorithmCommand::terms_type CodeGenerator::getScope(std::vector<AlgorithmCommand::terms_type::value_type>& terms, AlgorithmCommand::terms_type::size_type & start, const TERM startTerm, const TERM endTerm) const
{
	if ((terms.size() <= start + 1) || (std::next(terms.begin(), start+1)->second != startTerm))
		return{};
	else
		std::next(terms.begin(), ++start)->second = TERM::NONE;
	AlgorithmCommand::terms_type scopeTerms;
	AlgorithmCommand::terms_type::size_type indexCount{ 1 };
	for (++start; start < terms.size(); ++start)
	{
		auto& term = *std::next(terms.begin(), start);
		if (term.second == startTerm)
			++indexCount;
		if (term.second == endTerm)
			if (--indexCount == 0)
			{
				term.second = TERM::NONE;
				return scopeTerms;
			}
		scopeTerms.emplace_back(term);
		term.second = TERM::NONE;
	}
	return AlgorithmCommand::terms_type{ std::make_pair(std::string{}, TERM::UNKNOWN) };
}

AlgorithmCommand::terms_type CodeGenerator::getArrayIndexTerms(const AlgorithmCommand::terms_type & terms,  const AlgorithmCommand::terms_type::size_type startIndex) const
{
	auto start = std::find_if(terms.begin() + startIndex, terms.end(), [](const AlgorithmCommand::terms_type::value_type& term)
	{
		return term.second == TERM::ARRAY_INDEX_OPEN;
	});
	if (start == terms.cend() || start == terms.cbegin())
		return{};
	auto pos = static_cast<AlgorithmCommand::terms_type::size_type>(std::distance(terms.begin(), start) + 1);
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

AlgorithmCommand::terms_type CodeGenerator::getStructTerms(const algorithm_data_type & algorithmData, std::vector<AlgorithmCommand::terms_type::value_type>& terms, AlgorithmCommand::terms_type::size_type & start) const
{
	AlgorithmCommand::terms_type result{ *std::next(terms.begin(), start) };
	auto i = start;
	for (; i < terms.size(); ++i)
	{
		auto value = std::next(terms.begin(), i + 1);
		if (value == terms.end() || 
			(value->second != TERM::ARRAY_INDEX_OPEN && value->second != TERM::PART_SIGNAL))
			break;
		if (value->second == TERM::ARRAY_INDEX_OPEN)
		{
			auto arrayTerms = getScope(terms, i, TERM::ARRAY_INDEX_OPEN, TERM::ARRAY_INDEX_CLOSE);
			if (arrayTerms.empty() || arrayTerms.front().second == TERM::UNKNOWN)
				return AlgorithmCommand::terms_type{ std::make_pair(std::string{}, TERM::UNKNOWN) };
			auto arrayLine = termsToCodeLine(algorithmData, arrayTerms);
			if (arrayLine.empty())
				return AlgorithmCommand::terms_type{ std::make_pair(std::string{}, TERM::UNKNOWN) };
			result.emplace_back(std::make_pair(arrayLine, TERM::NONE));
			--i;
		}
		else
			result.emplace_back(*value);
	}
	for (size_t j = start; j <= i; ++j)
		std::next(terms.begin(), j)->second = TERM::NONE;
	start = i;
	return result;
}

std::string CodeGenerator::structTermsToLine(const algorithm_data_type & algorithmData, AlgorithmCommand::terms_type & terms, AlgorithmCommand::terms_type::size_type & start, const StructInfo * structInfo, const ValueInfo * valueInfo) const
{
	auto structTerms = getStructTerms(algorithmData, terms, start);
	if (structTerms.empty() || structTerms.front().second == TERM::UNKNOWN)
		return std::string{};
	enum class STRUCT_PART_TYPE : bool
	{
		STRUCT_INDEX = false,
		STRUCT_PART
		
	};
	std::vector<std::pair<STRUCT_PART_TYPE, std::string>> splitStructArray;
	std::string result;
	for (auto& term : structTerms)
	{
		if (term.second == TERM::NONE)
			splitStructArray.emplace_back(STRUCT_PART_TYPE::STRUCT_INDEX, term.first);
		else
			for (auto& structPart : Readers::CSVLineReader(term.first, STRUCT_FIELD_SYMBOL))
				splitStructArray.emplace_back(STRUCT_PART_TYPE::STRUCT_PART, structPart);
	}
	const auto PENULTIMATE_INDEX = splitStructArray.size() - 1;
	for (decltype(splitStructArray)::size_type i{}; i < splitStructArray.size(); ++i)
	{
		if (i <= PENULTIMATE_INDEX && structInfo == nullptr)
		{
			result.clear();
			break;
		}
		std::string name{ splitStructArray.at(i).second };
		if (i != 0)
		{
			decltype(signalInfo->structData)::mapped_type::storage_type::size_type index{};
			try
			{
				auto tempIndex = std::stoi(name);
				if (tempIndex < 0 ||
					static_cast<decltype(index)>(tempIndex) >= structInfo->structFieldType.size())
					throw "IncorrectIndex";
				index = static_cast<decltype(index)>(tempIndex);
			}
			catch (...)
			{
				result.clear();
				break;
			}
			name = valueInfo->pumlType + GEN_STRUCT_FIELD_MARKER + name;
			valueInfo = structInfo->structFieldType.at(index).get();
			auto nextStructIter = signalInfo->structData.find(valueInfo->pumlType);
			if (nextStructIter == signalInfo->structData.end())
				structInfo = nullptr;
			else
				structInfo = &nextStructIter->second;
		}
		if (valueInfo == nullptr || name.empty())
		{
			result.clear();
			break;
		}
		result += (result.empty() ? "" : dictionary->getReferencingInStruct()) + name;
		if (valueInfo->isPtrType)
		{
			std::string index;
			if (i != PENULTIMATE_INDEX && splitStructArray.at(i + 1).first == STRUCT_PART_TYPE::STRUCT_INDEX)
				index = splitStructArray.at(++i).second;
			result = dictionary->makeUnPtrable(result, index);
		}
		
	}
	return result;
}

std::vector<AlgorithmInfo> CodeGenerator::getAccessData(const std::string & name, const GeneralSignalData & signalData) const
{
	std::vector<AlgorithmInfo> result;
	if (signalData.pumlType.size() && signalData.pumlType.front() == STRUCT_SIGNAL)
	{
		auto structInfo = signalInfo->structData.at(signalData.pumlType);
		for (StructInfo::storage_type::size_type i = 0; i < structInfo.structFieldType.size(); ++i)
		{
			std::string fieldName = name + STRUCT_FIELD_SYMBOL + std::to_string(i);
			auto signalData = signalInfo->getSignalInfo(fieldName, AlgorithmInfo{});
			auto tempResult = getAccessData(fieldName, signalData);
			std::copy(tempResult.begin(), tempResult.end(), std::back_inserter(result));
		}
	}
	else
	{
		result.emplace_back();
		result.back().outerParam.emplace_back(new ParamName);
		result.back().outerParam.back()->isPtrType = false;
		result.back().outerParam.back()->elementCount = signalData.elementCount;
		result.back().outerParam.back()->pumlType = signalData.pumlType;
		if (name.find(STRUCT_FIELD_SYMBOL) == std::string::npos)
			result.back().outerParam.back()->name = name;
		else
		{
			//TODO: Не работает, если промежуточные поля - массивы
			auto splitted = Readers::CSVLineReader(name, STRUCT_FIELD_SYMBOL);
			std::string typeName = splitted.front();
			std::string fixName = typeName;
			splitted.erase(splitted.begin());
			for (auto& el : splitted)
			{
				auto structInfo = signalInfo->getSignalInfo(typeName, AlgorithmInfo{});
				StringFunction::appendString(el, typeName, STRUCT_FIELD_SYMBOL);
				fixName += STRUCT_FIELD_SYMBOL + structInfo.pumlType + '_' + el;
			}
			result.back().outerParam.back()->name = fixName;
		}
		result.back().outerParam.back()->title = signalData.title;
		result.back().outerParam.back()->argInfo = ABILITY_IN_ARGS::RETURNABLE;
		result.back().outerParam.back()->title = signalData.title;
		result.back().algorithmData.emplace_back(new AlgorithmCommand);
		result.back().algorithmData.back()->command = COMMAND_TYPE::END_MARKER;
		auto returnType = result.back().outerParam.back()->name;
		if (signalData.isPtrType == false)
			returnType = dictionary->makeRef(returnType);
		std::dynamic_pointer_cast<AlgorithmCommand>(result.back().algorithmData.back())
			->terms.emplace_back(AlgorithmCommand::terms_type{ std::make_pair(returnType, TERM::SIGNAL)});
	}
	return result;
}

bool CodeGenerator::signalSortFunction(const std::string& l, const std::string& r)
{
	return std::stoul(l.substr(1)) < std::stoul(r.substr(1));
}

std::vector<std::string> CodeGenerator::getSortedFunctionsKeys() const
{
	auto& currentAlgorithm = algorithmInfo->find(generator->mainFunctionName());
	std::vector<std::string> ordered;
	std::set<std::string> usedAlgorithm;
	if (currentAlgorithm == algorithmInfo->end())
	{
		log("Отсутствует определение начальной функции \""+ generator->mainFunctionName() +"\"");
		ordered.clear();
	}
	else
	{
		ordered.reserve(algorithmInfo->size());
		std::function<void(const std::string&)> lambda;
		lambda = [&](const std::string& key) 
		{
			if (usedAlgorithm.find(key) == usedAlgorithm.end())
			{
				ordered.emplace_back(key);
				usedAlgorithm.insert(key);
				for (auto& el : algorithmInfo->at(key)->usedAlgorithm)
					lambda(el);
			}			
		};
		lambda(generator->mainFunctionName());
	}

	return ordered;
}

std::string CodeGenerator::nameConvert(const std::string & pumlName) const
{
	std::string newName;
	newName.reserve(pumlName.size());
	for (auto& ch : pumlName)
		newName += dictionary->getTranscription(ch);
	return newName;
}

bool CodeGenerator::checkIsOneReturnAlgorithm(const AlgorithmInfo & algorithmData)
{
	if (algorithmData.localParam.empty() && algorithmData.outerParam.empty())
		return false;
	auto count = std::count_if(algorithmData.outerParam.begin(), algorithmData.outerParam.end(),
		[](const AlgorithmInfo::param_type& arg) {return arg->name.front() != PLANTUML_INPUT_PAR_CH && arg->isPtrType == false; });
	return count == 1;
}

FileReading::unique_ofstream_type CodeGenerator::openFile(const std::string& path)
{
	FileReading::unique_ofstream_type stream{new FileReading::unique_ofstream_type::element_type{ path, std::ios::out | std::ios::trunc }, FileReading::closeFunction};
	offsetInFile.clear();
	if (!stream->is_open())
	{
		log("Невозможно открыть файл \"" + path + "\" для генерации кода.");
		return { nullptr , FileReading::closeFunction };
	}
	return stream;
}

bool CodeGenerator::genLibHeaderFile(stream_type& stream)
{
	auto libGenerator = dynamic_cast<LibGenerator*>(generator.get());
	genWithOffset(stream, libGenerator->beforeGenLibHeader());
	genIncludeFile(stream, HEADER_FILE + libGenerator->headerFileExtension());
	bool isOk{ genLibStruct(stream) };
	isOk &= genLibHeaderFunction(stream);

	genWithOffset(stream, libGenerator->afterGenLibHeader());
	return isOk;
}

bool CodeGenerator::genLibStruct(stream_type& stream)
{
	genDoxygenBriefComment(stream, " - Структура для переноса данных в C# код");
	genWithOffset(stream, generator->startGenStruct(STRUCT_FOR_LIB));

	auto type = dictionary->makePtrable(dictionary->getCharType());
	genDoxygenFieldComment(stream, generator->genField(type, STRUCT_FOR_LIB_NAME), "- Название сигнала");
	genDoxygenFieldComment(stream, generator->genField(type, STRUCT_FOR_LIB_TYPE), "- Тип сигнала");

	type = dictionary->makePtrable(dictionary->getVoidType());
	genDoxygenFieldComment(stream, generator->genField(type, STRUCT_FOR_LIB_PTR), "- Указатель на данные");

	type = dictionary->getBoolType();
	genDoxygenFieldComment(stream, generator->genField(type, STRUCT_FOR_LIB_IS_PTR), "- Является ли тип указателем");

	type = dictionary->getSizeType();
	genDoxygenFieldComment(stream, generator->genField(type, STRUCT_FOR_LIB_ELEMENT_COUNT), "- Кол-во элементов(для указателей от 1 и выше, для значений = 1)");
	genDoxygenFieldComment(stream, generator->genField(type, STRUCT_FOR_LIB_SIGNAL_GROUPE), "- Группа сигналов");
	genWithOffset(stream, generator->endGenStruct(STRUCT_FOR_LIB));
	genWithoutOffset(stream);
	return true;
}

bool CodeGenerator::genLibHeaderFunction(stream_type& stream)
{
	auto libGenerator = dynamic_cast<LibGenerator*>(generator.get());
	auto beforeHeaderData = libGenerator->beforeGenSignatureLib();
	auto afterHeaderData = libGenerator->afterGenSignatureLib();

	genDoxygenBriefComment(stream, "Функция получения указателей на существующие переменные");
	genDoxygenParamComment(stream, LIB_SIGNAL_GET_SIZE,
		"Указатель (uint32_t) на переменную для записи кол-ва элементов в массиве структур", DOXYGEN_PARAM_IN_OUT);
	genDoxygenParamComment(stream, {}, "Массив структур TransferredStruct, содержащих данные о сигналах", DOXYGEN_RETURN);
	genWithOffset(stream, beforeHeaderData);
	genWithoutOffset(stream, libGenerator->genHeaderSignatureLib(dictionary->makePtrable(STRUCT_FOR_LIB), 
		LIB_SIGNAL_GET, { std::make_pair(dictionary->makePtrable(dictionary->getSizeType(), 1), LIB_SIGNAL_GET_SIZE) }));
	genWithOffset(stream, afterHeaderData);
	genWithoutOffset(stream);

	genDoxygenBriefComment(stream, "Функция очистки выделенной памяти");
	genWithOffset(stream, beforeHeaderData);
	genWithoutOffset(stream, libGenerator->genHeaderSignatureLib(dictionary->getVoidType(),	LIB_SIGNAL_CLEAR, { }));
	genWithOffset(stream, afterHeaderData);
	genWithoutOffset(stream);
	
	genDoxygenBriefComment(stream, "Основная функция алгоритма");
	genWithOffset(stream, beforeHeaderData);
	genWithoutOffset(stream, libGenerator->genHeaderSignatureLib(dictionary->getVoidType(),	LIB_MAIN_FUNCTION, { }));
	genWithOffset(stream, afterHeaderData);
	genWithoutOffset(stream);
	return true;
}

bool CodeGenerator::genLibSourceFile(stream_type& stream)
{
	auto libGenerator = dynamic_cast<LibGenerator*>(generator.get());
	genIncludeFile(stream, LIB_FILE + libGenerator->headerFileExtension());
	genWithOffset(stream, libGenerator->beforeGenLib());
	

	genDoxygenFieldComment(stream, libGenerator->genGlobalVariable(dictionary->makePtrable(STRUCT_FOR_LIB, getSignalCount()), STRUCT_LIB, {}),
		" - Массив данных сигналов");
	genWithoutOffset(stream);
	bool isOk{ genLibSignalGetFunction(stream) };
	isOk &= genLibSignalClearFunction(stream);
	isOk &= genLibOneStepFunctionFunction(stream);
	isOk &= genLibSignalResetFunction(stream);

	genWithOffset(stream, libGenerator->afterGenLib());
	return isOk;
}

std::uint32_t CodeGenerator::getSignalCount() const
{
	auto signalCount = signalInfo->timers.size();
	std::unordered_map<std::string, std::uint32_t> elementInStruct;
	for (auto & key : getOrderForStructGen())
	{
		auto structData = signalInfo->structData.find(key);
		auto count = structData->second.structFieldType.size();
		for (auto& field : structData->second.structFieldType)
		{
			auto iter = signalInfo->structData.find(field->pumlType);
			if (iter != signalInfo->structData.end())
				count += static_cast<decltype(count)>(field->elementCount * elementInStruct.at(iter->first) - 1);
		}
		elementInStruct.emplace(std::make_pair(key, static_cast<std::uint32_t>(count)));
	}
	auto lambda = [&signalCount, &elementInStruct](const InfoTable::signals_type& store) -> void
	{
		for (auto& signal : store)
		{
			if (elementInStruct.find(signal.second.pumlType) != elementInStruct.end())
				signalCount += elementInStruct.at(signal.second.pumlType);
			else
				++signalCount;
		}
	};
	lambda(signalInfo->innerValues);
	lambda(signalInfo->inputSignals);
	lambda(signalInfo->outputSignals);
	return static_cast<std::uint32_t>(signalCount);
}

bool CodeGenerator::genLibSignalGetFunction(stream_type& stream)
{
	auto libGenerator = dynamic_cast<LibGenerator*>(generator.get());
	genDoxygenBriefComment(stream, "Функция получения указателей на существующие переменные");
	genDoxygenParamComment(stream, LIB_SIGNAL_GET_SIZE,
		"Указатель (uint32_t) на переменную для записи кол-ва элементов в массиве структур", DOXYGEN_PARAM_IN_OUT);
	genDoxygenParamComment(stream, {}, "Массив структур TransferredStruct, содержащих данные о сигналах", DOXYGEN_RETURN);
	genWithOffset(stream, libGenerator->beforeGenSignatureLib());
	genWithoutOffset(stream, libGenerator->genSignatureLib(dictionary->makePtrable(STRUCT_FOR_LIB),
		LIB_SIGNAL_GET, { std::make_pair(dictionary->makePtrable(dictionary->getSizeType(), 1), LIB_SIGNAL_GET_SIZE) }));
	genWithOffset(stream, libGenerator->afterGenSignatureLib());

	genWithOffset(stream, libGenerator->startFunction());

	genWithoutOffset(stream, libGenerator->makeArrayHeap(STRUCT_FOR_LIB, STRUCT_LIB, getSignalCount()));

	std::uint32_t counter{};
	bool isOk{ genLibTimerGetFunction(stream,counter)};

	isOk &= genLibGeneralSignalGetFunction(stream, signalInfo->inputSignals, SIGNAL_TYPE::INPUT_TYPE, counter);
	isOk &= genLibGeneralSignalGetFunction(stream, signalInfo->innerValues, SIGNAL_TYPE::INNER_TYPE, counter);
	isOk &= genLibGeneralSignalGetFunction(stream, signalInfo->outputSignals, SIGNAL_TYPE::OUTPUT_TYPE, counter);

	genWithoutOffset(stream, dictionary->makeUnPtrable(LIB_SIGNAL_GET_SIZE) + " = " + std::to_string(counter) + dictionary->getEndLine());
	genWithoutOffset(stream, libGenerator->genReturnBody(STRUCT_LIB));

	genWithOffset(stream, libGenerator->endFunction());
	genWithoutOffset(stream);
	return isOk;
}

bool CodeGenerator::genLibTimerGetFunction(stream_type& stream, std::uint32_t& counter)
{
	const auto DEAFULT_PTR = dictionary->getDefaultValue(dictionary->getVoidType(), true, 1);
	const auto REFERENCING_SIGN = dictionary->getReferencingInStruct();
	const auto ENDL = dictionary->getEndLine();
	const auto COMMENT_SIGN = dictionary->getCommentSymbol();
	auto libGenerator = dynamic_cast<LibGenerator*>(generator.get());
	for (auto& key : getSortedSignalsKeys(signalInfo->timers))
	{
		auto& timer = signalInfo->timers.at(key);
		genWithoutOffset(stream, COMMENT_SIGN + timer);

		const auto UNPTRABLE_STRUCT = dictionary->makeUnPtrable(STRUCT_LIB, std::to_string(counter++)) + REFERENCING_SIGN;

		if (libGenerator->needGenAccessFunction() == TwoFileGenerator::BOOL_ANSWER::NO)
			genWithoutOffset(stream, UNPTRABLE_STRUCT + STRUCT_FOR_LIB_PTR + " = " + dictionary->makeRef(key) + ENDL);
		else
			genWithoutOffset(stream, generator->genFunctionCall(UNPTRABLE_STRUCT + STRUCT_FOR_LIB_PTR, nameConvert(GEN_ACCESS_PREFIX + key), {}));
			//genWithoutOffset(stream, UNPTRABLE_STRUCT + STRUCT_FOR_LIB_PTR + " = "+ nameConvert(GEN_ACCESS_PREFIX + key) + "()" + ENDL);

		genWithoutOffset(stream, libGenerator->makeString2HeapPtr("\"" + key + "\"", UNPTRABLE_STRUCT + STRUCT_FOR_LIB_NAME) + ENDL);

		genWithoutOffset(stream, UNPTRABLE_STRUCT + STRUCT_FOR_LIB_TYPE + " = " + DEAFULT_PTR + ENDL);
		genWithoutOffset(stream, UNPTRABLE_STRUCT + STRUCT_FOR_LIB_IS_PTR + " = " + std::to_string(false) + ENDL);
		genWithoutOffset(stream, UNPTRABLE_STRUCT + STRUCT_FOR_LIB_ELEMENT_COUNT + " = " + std::to_string(1) + ENDL);
		genWithoutOffset(stream, UNPTRABLE_STRUCT + STRUCT_FOR_LIB_SIGNAL_GROUPE + " = " + std::to_string(static_cast<std::uint32_t>(SIGNAL_TYPE::TIMERS_TYPE)) + ENDL);
		genWithoutOffset(stream);
	}
	return true;
}


bool CodeGenerator::genLibGeneralSignalGetFunction(stream_type& stream, const InfoTable::signals_type& storage, const SIGNAL_TYPE type, std::uint32_t& counter)
{
	const auto REFERENCING_SIGN = dictionary->getReferencingInStruct();
	const auto ENDL = dictionary->getEndLine();
	const auto COMMENT_SIGN = dictionary->getCommentSymbol();
	auto libGenerator = dynamic_cast<LibGenerator*>(generator.get());
	for (auto& key : getSortedSignalsKeys(storage))
	{
		auto& signal = storage.at(key);
		genWithoutOffset(stream, COMMENT_SIGN + signal.title);

		const auto UNPTRABLE_STRUCT = dictionary->makeUnPtrable(STRUCT_LIB, std::to_string(counter)) + REFERENCING_SIGN;
		auto isStructInfo = signalInfo->structData.find(signal.pumlType);
		if (isStructInfo == signalInfo->structData.end())
		{
			if (libGenerator->needGenAccessFunction() == TwoFileGenerator::BOOL_ANSWER::NO)
				genWithoutOffset(stream, UNPTRABLE_STRUCT + STRUCT_FOR_LIB_PTR + " = " + (signal.isPtrType ?
					key : dictionary->makeRef(key)) + ENDL);
			else
				genWithoutOffset(stream, generator->genFunctionCall(UNPTRABLE_STRUCT + STRUCT_FOR_LIB_PTR, nameConvert(GEN_ACCESS_PREFIX + key), {}));

			genWithoutOffset(stream, libGenerator->makeString2HeapPtr("\"" + key + "\"", UNPTRABLE_STRUCT + STRUCT_FOR_LIB_NAME) + ENDL);
			genWithoutOffset(stream, libGenerator->makeString2HeapPtr("\"" + signal.pumlType + "\"", UNPTRABLE_STRUCT + STRUCT_FOR_LIB_TYPE) + ENDL);
			genWithoutOffset(stream, UNPTRABLE_STRUCT + STRUCT_FOR_LIB_IS_PTR + " = " + std::to_string(signal.isPtrType) + ENDL);
			genWithoutOffset(stream, UNPTRABLE_STRUCT + STRUCT_FOR_LIB_ELEMENT_COUNT + " = " + std::to_string(signal.elementCount) + ENDL);
			genWithoutOffset(stream, UNPTRABLE_STRUCT + STRUCT_FOR_LIB_SIGNAL_GROUPE + " = " + std::to_string(static_cast<std::uint32_t>(type)) + ENDL);
			genWithoutOffset(stream);
			++counter;
		}
		else
		{
			if (signal.isPtrType)
				for (decltype(signal.elementCount) i = 0; i < signal.elementCount; ++i)
					genLibStructGetFunction(stream, *isStructInfo, i, signal.isPtrType, counter, key,type);
			else
				genLibStructGetFunction(stream, *isStructInfo, 0, signal.isPtrType, counter, key, type);
		}
	}
	return true;
}

static std::string prefixToName(const std::string& prefix, const std::string& referencing)
{
	std::string name;
	for (auto& el : Readers::CSVLineReader(prefix, STRUCT_FIELD_SYMBOL))
	{
		for (std::string::size_type i = 0u; i < el.size(); ++i)
			if (el.at(i) == LEFT_SIMPLE_BRACKET || el.at(i) == RIGHT_SIMPLE_BRACKET ||
				el.at(i) == PTR_CHAR)
				el.erase(i--, 1);	
		std::string index;
		auto pos = el.find("+");
		if (pos != std::string::npos)
		{
			index = StringFunction::removeExtraSpacesFromCodeText(el.substr(pos + 1));
			el = StringFunction::removeExtraSpacesFromCodeText(el.substr(0, pos));
		}
		auto split = Readers::CSVLineReader(el, '_');
		if (split.size() == 1)
			name += split.front();
		else
			name += referencing + split.back();
		if (index.size())
			name += "[" + index + "]";
	}
	return name;
}

void CodeGenerator::genLibStructGetFunction(stream_type& stream, const InfoTable::struct_type::value_type& structData, const std::uint32_t index, const bool isPtr,
	std::uint32_t& counter, const std::string& prefix, const SIGNAL_TYPE type)
{
	const auto REFERENCING_SIGN = dictionary->getReferencingInStruct();
	const auto ENDL = dictionary->getEndLine();
	const auto COMMENT_SIGN = dictionary->getCommentSymbol();
	auto libGenerator = dynamic_cast<LibGenerator*>(generator.get());
	
	for (decltype(structData.second.structFieldType)::size_type currentIndex = 0u; 
		currentIndex < structData.second.structFieldType.size(); ++currentIndex)
	{
		auto& signal = structData.second.structFieldType.at(currentIndex);
		auto newPrefix = (isPtr ? dictionary->makeUnPtrable(prefix, std::to_string(index)) : prefix) + REFERENCING_SIGN + 
			structData.first + "_" + std::to_string(currentIndex);
		auto isStructInfo = signalInfo->structData.find(signal->pumlType);
		if (isStructInfo == signalInfo->structData.end())
		{
			genWithoutOffset(stream, COMMENT_SIGN + signal->title);
			const auto UNPTRABLE_STRUCT = dictionary->makeUnPtrable(STRUCT_LIB, std::to_string(counter)) + REFERENCING_SIGN;
			if (libGenerator->needGenAccessFunction() == TwoFileGenerator::BOOL_ANSWER::NO)
				genWithoutOffset(stream, UNPTRABLE_STRUCT + STRUCT_FOR_LIB_PTR + " = " + (signal->isPtrType ?
					newPrefix : dictionary->makeRef(newPrefix)) + ENDL);
			else
			{
				genWithoutOffset(stream, UNPTRABLE_STRUCT + STRUCT_FOR_LIB_PTR + " = " + nameConvert(GEN_ACCESS_PREFIX + newPrefix) + "()" + ENDL);
			}
			
			genWithoutOffset(stream, libGenerator->makeString2HeapPtr("\"" + prefixToName(newPrefix, REFERENCING_SIGN) + "\"", UNPTRABLE_STRUCT + STRUCT_FOR_LIB_NAME) + ENDL);
			genWithoutOffset(stream, libGenerator->makeString2HeapPtr("\"" + signal->pumlType + "\"", UNPTRABLE_STRUCT + STRUCT_FOR_LIB_TYPE) + ENDL);
			genWithoutOffset(stream, UNPTRABLE_STRUCT + STRUCT_FOR_LIB_IS_PTR + " = " + std::to_string(signal->isPtrType) + ENDL);
			genWithoutOffset(stream, UNPTRABLE_STRUCT + STRUCT_FOR_LIB_ELEMENT_COUNT + " = " + std::to_string(signal->elementCount) + ENDL);
			genWithoutOffset(stream, UNPTRABLE_STRUCT + STRUCT_FOR_LIB_SIGNAL_GROUPE + " = " + std::to_string(static_cast<std::uint32_t>(type)) + ENDL);
			++counter;
		}
		else
		{
			if (signal->isPtrType)
				for (ValueInfo::element_count_type i = 0; i < signal->elementCount; ++i)
					genLibStructGetFunction(stream, *isStructInfo, i, signal->isPtrType, counter, newPrefix, type);
			else
				genLibStructGetFunction(stream, *isStructInfo, 0, signal->isPtrType, counter, newPrefix, type);
		}
	}
}

bool CodeGenerator::genLibSignalClearFunction(stream_type& stream)
{

	genDoxygenBriefComment(stream, "Функция очистки выделенной памяти");
	auto libGenerator = dynamic_cast<LibGenerator*>(generator.get());
	genWithOffset(stream, libGenerator->beforeGenSignatureLib());
	genWithoutOffset(stream, libGenerator->genSignatureLib(dictionary->getVoidType(),
		LIB_SIGNAL_CLEAR, {  }));
	genWithOffset(stream, libGenerator->afterGenSignatureLib());

	genWithOffset(stream, libGenerator->startFunction());
	const auto REFERENCING_SIGN = dictionary->getReferencingInStruct();

	std::string tempVar = "i";

	genWithoutOffset(stream, libGenerator->genLocalVariable(dictionary->getSizeType(), tempVar,
		dictionary->getDefaultValue(dictionary->getSizeType(), false, 1)));
	genWithoutOffset(stream, libGenerator->genWhileBody(tempVar + " < " + std::to_string(getSignalCount())));
	genWithOffset(stream, libGenerator->startWhile());

	genWithoutOffset(stream, libGenerator->genIfBody(dictionary->makeUnPtrable(STRUCT_LIB, tempVar)
		+ REFERENCING_SIGN + STRUCT_FOR_LIB_NAME));
	genWithOffset(stream, libGenerator->startIf());
	genWithoutOffset(stream, libGenerator->deleteHeapPtr(dictionary->makeUnPtrable(STRUCT_LIB, tempVar)
		+ REFERENCING_SIGN + STRUCT_FOR_LIB_NAME));
	genWithOffset(stream, libGenerator->endIf());

	genWithoutOffset(stream, libGenerator->genIfBody(dictionary->makeUnPtrable(STRUCT_LIB, tempVar)
		+ REFERENCING_SIGN + STRUCT_FOR_LIB_TYPE));
	genWithOffset(stream, libGenerator->startIf());
	genWithoutOffset(stream, libGenerator->deleteHeapPtr(dictionary->makeUnPtrable(STRUCT_LIB, tempVar)
		+ REFERENCING_SIGN + STRUCT_FOR_LIB_TYPE));
	genWithOffset(stream, libGenerator->endIf());


	genWithoutOffset(stream, tempVar +" += 1" + dictionary->getEndLine());

	genWithOffset(stream, libGenerator->endWhile());

	genWithoutOffset(stream, libGenerator->deleteHeapPtr(STRUCT_LIB));
	genWithOffset(stream, libGenerator->endFunction());
	return true;
}

bool CodeGenerator::genLibOneStepFunctionFunction(stream_type& stream)
{
	auto libGenerator = dynamic_cast<LibGenerator*>(generator.get());
	genDoxygenBriefComment(stream, "Основная функция алгоритма");
	genWithOffset(stream, libGenerator->beforeGenSignatureLib());
	genWithoutOffset(stream, libGenerator->genSignatureLib(dictionary->getVoidType(),
		LIB_MAIN_FUNCTION, { }));
	genWithOffset(stream, libGenerator->afterGenSignatureLib());
	genWithOffset(stream, libGenerator->startFunction());

	genWithoutOffset(stream, libGenerator->genFunctionCall({},
		nameConvert(libGenerator->mainFunctionName()), {}));

	genWithOffset(stream, libGenerator->endFunction());
	return true;
}
bool CodeGenerator::genLibSignalResetFunctionRaw(stream_type & stream, const std::string & functionName, const std::vector<PUML_SIGNAL_TYPE>& signalsToReset)
{
	auto libGenerator = dynamic_cast<LibGenerator*>(generator.get());
	std::string signals = getSignals();
	genDoxygenBriefComment(stream, " Функция обнуления переменных" + functionName);
	genWithOffset(stream, libGenerator->beforeGenSignatureLib());

	std::string defaultStringType = dictionary->getDefaultValue(dictionary->getCharType(), true, 1);
	genWithOffset(stream, libGenerator->beforeGenSignatureLib());
	genWithoutOffset(stream, libGenerator->genSignatureLib(dictionary->getVoidType(),
		functionName, { std::make_pair(dictionary->makeRefArg(dictionary->getCharType()) , std::string{ "signalName" }) }));
	genWithOffset(stream, libGenerator->afterGenSignatureLib());
	genWithOffset(stream, libGenerator->startFunction());

	std::function<std::string(const std::string&, const GeneralSignalData&, const std::int32_t)> signalResetLambda;
	signalResetLambda = [&](const std::string& _signal, const GeneralSignalData& _currentSignalInfo, const std::int32_t index) -> std::string
	{
		std::string clearSignal = _signal;
		while (clearSignal.find("_") != std::string::npos)
		{
			auto posStart = clearSignal.find(STRUCT_SIGNAL);
			clearSignal = clearSignal.erase(posStart, clearSignal.find("_") - posStart + 1);
		}
		auto type = dictionary->getType(_currentSignalInfo.pumlType);
		if (_currentSignalInfo.pumlType.size() && _currentSignalInfo.pumlType.front() == STRUCT_SIGNAL)
		{
			auto currentSignalStruct = signalInfo->structData.find(_currentSignalInfo.pumlType);
			if (currentSignalStruct != signalInfo->structData.end())
			{
				std::string result;
				for (size_t i = 0; i < currentSignalStruct->second.structFieldType.size(); ++i)
				{
					GeneralSignalData tempData{};
					tempData.copy(*currentSignalStruct->second.structFieldType.at(i));
					auto sgn = _signal + dictionary->getReferencingInStruct() + _currentSignalInfo.pumlType + "_" + std::to_string(i);
					if (tempData.isPtrType)
					{
						for (size_t j = 0u; j < tempData.elementCount; ++j)
							StringFunction::appendString(signalResetLambda(sgn, tempData, static_cast<std::int32_t>(j)), result);
					}
					else
						StringFunction::appendString(signalResetLambda(sgn, tempData, -1), result);
				}
				return result;
			}
			else
			{
				log("Невозможно сгенерировать обнуление для " + clearSignal);
				return dictionary->getCommentSymbol() + "Невозможно сгенерировать обнуление для " + clearSignal;
			}
		}
		else
		{
			if (type.empty() || type == dictionary->getVoidType())
				type = dictionary->getTimerType();
			if (libGenerator->needGenAccessFunction() == OneFileGenerator::BOOL_ANSWER::YES)
			{
				clearSignal = "getSignal" + _signal + "()";
				std::replace(clearSignal.begin(), clearSignal.end(), '.', '_');
			}
			else
				clearSignal = _signal;
			return dictionary->makeUnPtrable(clearSignal, index == -1 ? std::string{} : std::to_string(index)) + " = " + dictionary->getDefaultValue(type, false, 1) + dictionary->getEndLine();
		}
	};

	for (auto& currentSignal : Readers::CSVLineReader(getSignals(), '\n'))
	{
		auto currentSignalInfo = signalInfo->getSignalInfo(currentSignal, {});
		if (signalsToReset.size() && std::find(signalsToReset.begin(), signalsToReset.end(), currentSignalInfo.type) == signalsToReset.end())
			continue;
		if (currentSignalInfo.isPtrType)
		{
			for (size_t i = 0u; i < currentSignalInfo.elementCount; ++i)
			{
				genWithoutOffset(stream, libGenerator->genIfBody("(signalName == " + defaultStringType + ") || " +
					libGenerator->stringCompare("signalName", "\"" + currentSignal + "\"") + " || " +
					libGenerator->stringCompare("signalName", "\"" + currentSignal + "[" + std::to_string(i) + "]\"")));
				genWithOffset(stream, libGenerator->startIf());

				genWithoutOffset(stream, signalResetLambda(currentSignal, currentSignalInfo, static_cast<std::int32_t>(i)));

				genWithOffset(stream, libGenerator->endIf());
			}
		}
		else
		{
			genWithoutOffset(stream, libGenerator->genIfBody("(signalName == " + defaultStringType + ") || " +
				libGenerator->stringCompare("signalName", "\"" + currentSignal + "\"")));
			genWithOffset(stream, libGenerator->startIf());

			genWithoutOffset(stream, signalResetLambda(currentSignal, currentSignalInfo, -1));

			genWithOffset(stream, libGenerator->endIf());
		}

	}

	genWithOffset(stream, libGenerator->endFunction());
	return true;
}
std::string CodeGenerator::getSignals()
{
	std::string result;
	for (auto& el : getSortedSignalsKeys(signalInfo->timers))
		StringFunction::appendString(el, result, '\n');
	for (auto& el : getSortedSignalsKeys(signalInfo->inputSignals))
		StringFunction::appendString(el, result, '\n');
	for (auto& el : getSortedSignalsKeys(signalInfo->innerValues))
		StringFunction::appendString(el, result, '\n');
	for (auto& el : getSortedSignalsKeys(signalInfo->outputSignals))
		StringFunction::appendString(el, result, '\n');
	return result;
}
bool CodeGenerator::genLibSignalResetFunction(stream_type & stream)
{
	auto result{ genLibSignalResetFunctionRaw(stream, LIB_SIGNAL_RESET,{ PUML_SIGNAL_TYPE::INNER,PUML_SIGNAL_TYPE::OUTPUT }) };
	result &= genLibSignalResetFunctionRaw(stream, LIB_INPUT_SIGNAL_RESET, { PUML_SIGNAL_TYPE::INPUT });
	return result;
}


void CodeGenerator::setSignals(const signal_type& newSignals)
{
	signalInfo = newSignals;
}

void CodeGenerator::setPuml(const algorithm_type& newAlgorithms)
{
	algorithmInfo = newAlgorithms;
}

void CodeGenerator::setOutputPath(const std::string& newPath)
{
	outputPath = newPath;
}

std::string CodeGenerator::getOutputPath() const
{
	return outputPath;
}

bool CodeGenerator::createSourceFile()
{
	auto stream = openFile(getSourceFile());
	if (stream == nullptr)
		return false;
	bool isOk{ true };
	genWithOffset(*stream, generator->beforeGen());
	auto twoFileGenerator = dynamic_cast<TwoFileGenerator*>(generator.get());
	if (twoFileGenerator != nullptr)
		genIncludeFile(*stream, HEADER_FILE + twoFileGenerator->headerFileExtension());
	else
		isOk &= genStructTypes(*stream);
	if (twoFileGenerator == nullptr || twoFileGenerator->needGenGlobalVarInHeader() == TwoFileGenerator::BOOL_ANSWER::NO)
		isOk &= genGlobalSignal(*stream);
	if(algorithmInfo->empty())
	{
		log("Нет алгоритмов для генерации.");
		isOk = false;
	}
	else
		isOk &= genFunctions(*stream, FILE_TYPE_GEN::SOURCE);
	genWithOffset(*stream, generator->afterGen());
	return true;
}

bool CodeGenerator::createHeaderFile()
{
	auto stream = openFile(getHeaderFile());
	if (stream == nullptr)
		return false;
	auto twoFileGenerator = dynamic_cast<TwoFileGenerator*>(generator.get());
	if (twoFileGenerator == nullptr)
	{
		log("Установлен некорректный генератор.");
		return false;
	}
	genWithOffset(*stream, twoFileGenerator->beforeGenHeader());
	bool isOk{ genStructTypes(*stream) };
	if (twoFileGenerator->needGenGlobalVarInHeader() == TwoFileGenerator::BOOL_ANSWER::YES)
		isOk &= genGlobalSignal(*stream);
	if (algorithmInfo->empty())
	{
		log("Нет алгоритмов для генерации.");
		isOk = false;
	}
	isOk &= genFunctions(*stream, FILE_TYPE_GEN::HEADER);
	genWithOffset(*stream, twoFileGenerator->afterGenHeader());
	return isOk;
}

bool CodeGenerator::createLibFile()
{
	auto libGenerator = dynamic_cast<LibGenerator*>(generator.get());
	if (libGenerator == nullptr)
	{
		log("Установлен некорректный генератор.");
		return false;
	}
	auto stream = openFile(getLibFile());
	if (stream == nullptr)
		return false;
	bool isOk{ genLibHeaderFile(*stream) };
	stream.swap(openFile(getLibFile(true, false)));
	if (stream == nullptr)
		return false;
	return isOk && genLibSourceFile(*stream);
}

std::string CodeGenerator::getSourceFile(bool isExtension) const
{
	return genPath(SOURCE_FILE, isExtension);
}

std::string CodeGenerator::getHeaderFile(bool isExtension) const
{
	return genPath(HEADER_FILE, isExtension, true);
}

std::string CodeGenerator::getLibFile(bool isExtension, bool isHeader) const
{
	return genPath(LIB_FILE, isExtension, isHeader);
}

std::string CodeGenerator::getOutput() const
{
	return (outputPath.empty() ? "" : outputPath + "\\");
}

void CodeGenerator::clear()
{
	signalInfo.reset();
	algorithmInfo.reset();
}
