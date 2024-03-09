#include "SignalStruct.hpp"
#include "../../Common/CSVReader.hpp"
#include "../../Common/StringFunction.hpp"
void InfoTable::getStructData(GeneralSignalData & signalData, const std::string & structLine) const
{
	auto structInfo = structData.find(signalData.pumlType);
	if (structInfo != structData.end())
	{
		auto sepPart = Readers::CSVLineReader(structLine, STRUCT_FIELD_SYMBOL, true);
		for (decltype(sepPart)::size_type i = 0, j = 0; i < sepPart.size();)
		{
			if (sepPart.at(i).empty() ||
				StringFunction::checkStringBalance(sepPart.at(i), LEFT_SQARE_BRACKET, RIGHT_SQARE_BRACKET) == 0)
			{
				++i;
				continue;
			}
			if (i <= j)
				j = i + 1;
			if (j >= sepPart.size())
			{
				signalData.isData = false;
				return;
			}
			sepPart.at(i) += STRUCT_FIELD_SYMBOL + sepPart.at(j);
			sepPart.at(j++).clear();
		}
		sepPart.erase(std::remove(sepPart.begin(), sepPart.end(), std::string{}), sepPart.end());
		auto prevData = structInfo;
		for (auto i = 0u; i <= sepPart.size() && signalData.isData; ++i)
		{
			if (i == sepPart.size())
				return;
			auto &el = sepPart.at(i);
			if (el.empty())
				signalData.isData = false;
			else
			{
				auto arrPos = el.find(LEFT_SQARE_BRACKET);
				bool isPtrInArg{ arrPos != std::string::npos };
				if (isPtrInArg)
					el.erase(el.begin() + arrPos, el.end());
				try
				{
					auto value = std::stoi(el);
					if (value < 0 || static_cast<decltype(structInfo->second.structFieldType)::size_type>
						(value) >= structInfo->second.structFieldType.size())
						signalData.isData = false;
					else
					{
						auto nextType = structInfo->second.structFieldType.at(value)->pumlType;
						if (nextType.empty())
							signalData.isData = false;
						else
						{
							arrPos = el.find(PTR_SIGN);
							if (arrPos != std::string::npos)
								nextType.erase(nextType.begin() + arrPos, nextType.end());

							prevData = structInfo;
							structInfo = structData.find(nextType);
							if (structInfo == structData.end())
								if (i == (sepPart.size() - 1))
									structInfo = prevData;
								else
									signalData.isData = false;
							if (signalData.isData)
							{
								signalData.copy(*prevData->second.structFieldType.at(value).get());
								if (isPtrInArg != prevData->second.structFieldType.at(value)->isPtrType && i
									!= (sepPart.size() - 1))
									signalData.isData = true;
							}
						}
					}
				}
				catch (...)
				{
					signalData.isData = false;
				}
			}
		}
		return;

	}
	signalData.isData = false;
}

GeneralSignalData InfoTable::getSignalInfo(const std::string & signal, const AlgorithmInfo & currentAlgorithm) const
{
	if (signal.empty())
		return{};
	auto workLine = signal;
	GeneralSignalData signalInfo;
	signalInfo.type = static_cast<PUML_SIGNAL_TYPE>(workLine.front());
	std::unique_ptr<std::uint32_t> elementCountInArray;

	std::string structPart, arrayPart;
	auto posVar = signal.find(STRUCT_FIELD_SYMBOL);
	if (posVar != std::string::npos)
	{
		structPart = workLine.substr(posVar + 1);
		workLine.erase(workLine.begin() + posVar, workLine.end());
	}
	posVar = workLine.find(LEFT_SQARE_BRACKET);
	if (posVar != std::string::npos)
	{
		arrayPart = workLine.substr(posVar);
		std::size_t index{};
		StringFunction::checkStringBalance(arrayPart, LEFT_SQARE_BRACKET, RIGHT_SQARE_BRACKET, &index);
		arrayPart = arrayPart.substr(1, index - 1);
		if (arrayPart != TERM_ARRAY_STR)
		{
			auto isData = getSignalInfo(arrayPart, currentAlgorithm).isData;
			if (isData == false)
			{
				if (!std::all_of(arrayPart.begin(), arrayPart.end(), StringFunction::_isdigit))
					return{};
				else
					elementCountInArray.reset(new std::uint32_t{ static_cast<std::uint32_t>(std::stoi(arrayPart)) });
			}
		}
		workLine.erase(workLine.begin() + workLine.find(LEFT_SQARE_BRACKET), workLine.end());
	}
	const InfoTable::signals_type* signalContainer = nullptr;
	const std::vector<AlgorithmInfo::param_type>* argsContainer = nullptr;
	switch (signalInfo.type)
	{
	case PUML_SIGNAL_TYPE::INPUT:
		signalContainer = &inputSignals;
	case PUML_SIGNAL_TYPE::INNER:
		if (signalContainer == nullptr)
			signalContainer = &innerValues;
	case PUML_SIGNAL_TYPE::OUTPUT:
	{
		if (signalContainer == nullptr)
			signalContainer = &outputSignals;
		auto signalData = signalContainer->find(workLine);
		if (signalData == signalContainer->end())
			return{};
		signalInfo.copy(signalData->second);
		break;
	}
	case PUML_SIGNAL_TYPE::CONST_SIGN:
	{
		auto iter = constValues.find(signal);
		if (iter == constValues.end())
			break;
		signalInfo.title = iter->second.title;
		signalInfo.isData = true;
	}
	case PUML_SIGNAL_TYPE::TIMER:
		if (signalInfo.isData == false)
		{
			auto iter = timers.find(signal);
			if (iter == timers.end())
				break;
			signalInfo.isData = true;
			signalInfo.title = iter->second;
		}
		signalInfo.abitilyInArgs = ABILITY_IN_ARGS::SIMPLE;
		break;
	case PUML_SIGNAL_TYPE::INPUT_ARG:
		argsContainer = &currentAlgorithm.inputParam;
	case PUML_SIGNAL_TYPE::LOCAL_ARS:
		if (argsContainer == nullptr)
			argsContainer = &currentAlgorithm.localParam;
	case PUML_SIGNAL_TYPE::OUTPUT_ARG:
	{
		if (argsContainer == nullptr)
			argsContainer = &currentAlgorithm.outerParam;
		auto argsData = std::find_if(argsContainer->begin(), argsContainer->end(), [&workLine](const AlgorithmInfo::param_type& data)
		{
			return workLine == data->name;
		});
		if (argsData == argsContainer->end())
			return{};
		signalInfo.copy(*argsData->get());
		break;
	}
	default:
		return{};
	}
	if (arrayPart.size() && signalInfo.isPtrType == false)
		return{};
	if (structPart.size())
	{
		if (signalInfo.pumlType.empty() ||
			signalInfo.pumlType.front() != STRUCT_SIGNAL)
			return{};
		getStructData(signalInfo, structPart);
	}
	if (elementCountInArray)
		if (signalInfo.isPtrType == false || signalInfo.elementCount <= *elementCountInArray)
			return{};
	return signalInfo;
}
