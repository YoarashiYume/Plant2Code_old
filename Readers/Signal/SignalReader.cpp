#include "SignalReader.hpp"

#include <algorithm>

#include "../../Common/StringFunction.hpp"
#include "../../Common/CSVReader.hpp"
#include "../../Common/FileReading.hpp"
#include "../../Common/Def.hpp"

static auto commentWord = CSV_SINGAL_COMMENT;


bool SignalReader::isStructTable(std::string path) const
{
	auto fileNamePos = path.find_last_of(PATH_SEPARATOR);
	if (fileNamePos != decltype(path)::npos)
		path.erase(path.begin(), path.begin() + fileNamePos+1);
	if (path.size() != (sizeof(STRUCT_DATA) / sizeof(*STRUCT_DATA) - 1))
		return false;
	return StringFunction::toLower(path) == STRUCT_DATA;
}

bool SignalReader::readStructTable(std::ifstream & stream, const std::string & path)
{
	std::string line;
	std::getline(stream, line);
	auto cellsArray = parseCSV(line);

	auto nameIndex{ findIndex(cellsArray, CSV_STRUCT_NAME) };
	if (nameIndex == -1)
	{
		log("В таблице \"" + path + "\" невозможно найти обозначение структур(" CSV_STRUCT_NAME ".");
		return false;
	}
	auto typeIndex{ findIndex(cellsArray, CSV_STRUCT_TYPE) };
	if (typeIndex == -1)
	{
		log("В таблице \"" + path + "\" невозможно найти тип структур(" CSV_STRUCT_TYPE ".");
		return false;
	}
	std::int32_t commentIndex;
	for (auto& word : commentWord)
	{
		commentIndex = findIndex(cellsArray, word);
		if (commentIndex != -1)
			break;		
	}
	bool isOk{ true };
	while (std::getline(stream, line))
	{
		cellsArray = parseCSV(line);
		auto structName = cellsArray.at(nameIndex);
		if (signalNameValidation(structName, STRUCT_SIGNAL) == false)
		{
			log("Некорректное название структуры(структура будет проигнорирована): " + structName + 
				". Также возможен некорректный порядок определения полей структуры. Таблица: " + path);
			continue;
		}
		auto fieldPos = structName.find(STRUCT_FIELD_SYMBOL);
		if (isReserveSignal(cellsArray))
			continue;
		if (fieldPos == std::string::npos)
		{
			if (signals->structData.find(structName) == signals->structData.end())
			{
				auto it = signals->structData.emplace(std::make_pair(std::move(structName), StructInfo{}));
				if (commentIndex != -1)
					it.first->second.title = std::move(cellsArray.at(static_cast<std::uint32_t>(commentIndex)));
			}
			else
				this->log("Повтор структуры(повтор будет проигнорирован): " + structName + ". Таблица: " + path);

		}
		else
		{
			StructInfo::storage_type::size_type fieldIndex;
			try
			{
				auto index = std::stoi(structName.substr(fieldPos + 1));
				if (index < 0)
					throw "No negative index";
				fieldIndex = static_cast<StructInfo::storage_type::size_type>(index);
			}
			catch (...)
			{
				this->log("Некорректное поле структуры: " + structName + ". Таблица: " + path);
				isOk = false;
				continue;
			}
			structName.erase(fieldPos);

			auto structIter = signals->structData.find(structName);
			if (structIter == signals->structData.end())
			{
				this->log("Info: Отсутствует объявление структуры: \"" + structName + "\" перед полем " + structName
					+ '.' + std::to_string(fieldIndex) + ". Таблица: " + path);
				structIter = signals->structData.emplace(std::make_pair(structName, StructInfo{})).first;
			}
			if (structIter->second.structFieldType.size() <= fieldIndex)
				structIter->second.structFieldType.resize(fieldIndex + 1);
			if (structIter->second.structFieldType.at(fieldIndex) != nullptr)
			{
				this->log("Повтор поля структуры(повтор будет проигнорирован): " + structName + '.' + std::to_string(fieldIndex) + ". Таблица: " + path);
				continue;
			}
			StructInfo::field_type newField{ new StructInfo::field_type::element_type{} };
			newField->elementCount = 1;
			newField->pumlType = std::move(cellsArray.at(typeIndex));
			if (commentIndex != -1)
				newField->title = std::move(cellsArray.at(static_cast<std::uint32_t>(commentIndex)));

			auto iterPtr = std::find(newField->pumlType.begin(), newField->pumlType.end(), PTR_CHAR);
			auto iterArr = std::find(newField->pumlType.begin(), newField->pumlType.end(), LEFT_SQARE_BRACKET);

			newField->isPtrType = iterPtr != newField->pumlType.end() || iterArr != newField->pumlType.end();

			if (newField->isPtrType)
			{
				auto countInType = getElementCountFromType(newField->pumlType.substr(
					std::distance(newField->pumlType.begin(), iterArr < iterPtr ? iterArr : iterPtr)));
				if (iterArr == newField->pumlType.end())
					newField->pumlType.erase(iterPtr);
				else
				{
					auto iterArrBack = std::find(newField->pumlType.begin(), newField->pumlType.end(), RIGHT_SQARE_BRACKET);
					if (iterArrBack == newField->pumlType.end() || std::next(iterArrBack) != newField->pumlType.end())
					{
						this->log("Некорректный тип поля структуры: " + structName + '.' + std::to_string(fieldIndex));
						isOk = false;
						continue;
					}
					std::string elementString{ std::next(iterArr), iterArrBack };
					try
					{
						auto value = std::stoi(elementString);
						if (value < 0)
							throw "Wrong array length";
						newField->elementCount = static_cast<std::uint32_t>(value);
					}
					catch (...)
					{

						this->log("Некорректный тип поля структуры: " + structName + '.' + std::to_string(fieldIndex) + ". Таблица: " + path);
						isOk = false;
						continue;
					}
					newField->pumlType.erase(iterArr, newField->pumlType.end());
				}
				if (countInType != newField->elementCount)
					this->log("Указана различная длина массива у поля структуры: " + structName + '.' + std::to_string(fieldIndex) + "Будет использовано большее значение. Таблица: " + path);
				
				if (countInType > static_cast<std::int32_t>(newField->elementCount))
					newField->elementCount = static_cast<std::uint32_t>(countInType);
			}
			

			structIter->second.structFieldType.at(fieldIndex) = std::move(newField);
		}
	}

	return isOk;
}

bool SignalReader::readSignalTable(std::ifstream & stream, const std::string & path)
{
	std::string line;
	std::getline(stream, line);
	auto cellsArray = parseCSV(line);

	auto signalNameIndex{ getNameIndex(cellsArray) };
	if (signalNameIndex == -1)
	{
		this->log("Отсутствует столбец с названием сигнала("  CSV_SINGALS_NAME_VAR_1"," CSV_SINGALS_NAME_VAR_2 ", " CSV_SINGALS_NAME_VAR_3 ") таблицы: " + path);
		return false;
	}
	bool shouldBeConstTable{ false };
	auto typeIndex{ findIndex(cellsArray, CSV_STRUCT_TYPE) };
	if (typeIndex == -1)
	{
		shouldBeConstTable = true;
		typeIndex = findIndex(cellsArray, CSV_CONST_VALUE);
	}

	std::int32_t commentIndex{ -1 };
	for (auto& word : commentWord)
	{
		commentIndex = findIndex(cellsArray, word);
		if (commentIndex != -1)
			break;
	}
	auto currentPos = stream.tellg();
	std::string signalType;

	while (std::getline(stream, line))
	{
		cellsArray = parseCSV(line);
		signalType = cellsArray.at(signalNameIndex);
		try
		{
			signalType.erase(std::find_if_not(signalType.begin(),
				signalType.end(), isalpha), signalType.end());
			if (signalType.size() != 1 || islower(signalType.front()))
				throw "Incorrect signal type";
		}
		catch (...)
		{
			this->log("Некорректный тип сигнала \"" + cellsArray.at(signalNameIndex) + "\". Файл: " + path);
			currentPos = stream.peek();
			signalType.clear();
			continue;
		}
		break;
	}
	if (signalType.empty())
	{
		this->log("Невозможно определить тип сигнала в файле: " + path);
		return false;
	}
	if (stream.eof())
	{
		stream.close();
		stream.open(path, std::ios::binary | std::ios::in);
	}
	stream.seekg(currentPos);

	InfoTable::signals_type* usedStorage{nullptr};
	switch (signalType.front())
	{
	case INNER_SIGNAL:
		usedStorage = &this->signals->innerValues;
	case INPUT_SIGNAL:
		if (usedStorage == nullptr)
			usedStorage = &this->signals->inputSignals;
	case OUTPUT_SIGNAL:
		if (usedStorage == nullptr)
			usedStorage = &this->signals->outputSignals;
		if (typeIndex == -1 || shouldBeConstTable)
		{
			this->log("Отсутствует тип сигнала (" CSV_STRUCT_TYPE ") \"" + cellsArray.at(signalNameIndex) + "\". Файл: " + path);
			return false;
		}
		return readSignalTable(*usedStorage, signalType.front(),
			stream, signalNameIndex, typeIndex, commentIndex, path);
	case TIMER_SIGNAL:
		return readTimerTable(stream, signalNameIndex, commentIndex, path);
	case CONST_SIGNAL:
		if (shouldBeConstTable == false)
			typeIndex = findIndex(cellsArray, CSV_CONST_VALUE);
		if (typeIndex == -1)
		{
			this->log("Отсутствует значение (" CSV_CONST_VALUE ") константы  \"" + cellsArray.at(signalNameIndex) + "\". Файл: " + path);
			return false;
		}
		return readConstTable(stream, signalNameIndex, typeIndex, commentIndex, path);
	default:
		this->log("Непредусмотренный тип сигнала в файле: " + path);
		return false;
	}

	
}

bool SignalReader::isReserveSignal(const std::vector<std::string>& cells) const
{
	auto reserveSign = StringFunction::toLower(CSV_UNUSED_SIGNAL);
	for (auto& cell : cells)
	{
		if (cell.empty())
			continue;
		if (StringFunction::toLower(cell) == reserveSign)
			return true;
	}
	return false;
}

void SignalReader::removeExtraSymbolsFromCells(std::vector<std::string>& cells) const
{
	for (auto& cell : cells)
	{
		using str_size = std::string::size_type;
		using str_char = std::string::value_type;
		
		if (cell.empty())
			continue;
		if (cell.size() != 1)
			cell = cell.substr(cell.front() == CSV_BORDER,
				cell.size() - static_cast<str_size>(cell.front() == CSV_BORDER)
				- static_cast<str_size>(cell.back() == CSV_BORDER));
		std::transform(cell.begin(), cell.end(), cell.begin(), [](const str_char ch) -> char
		{
			if (StringFunction::_iscntrl(ch))
				return 32;
			return ch;
		});
		cell.erase(std::unique(cell.begin(), cell.end(),
			[](const str_char& a, const str_char& b)
		{
			return StringFunction::_isspace(a) && StringFunction::_isspace(b);
		}),
			cell.end());
		while (!cell.empty() && StringFunction::_isspace(cell.front()))
			cell.erase(cell.begin());
		while (!cell.empty() && StringFunction:: _isspace(cell.back()))
			cell.pop_back();
	}
}

std::int32_t SignalReader::findIndex(const std::vector<std::string>& cells, const std::string &indexName) const
{
	auto lowIndexName = StringFunction::toLower(indexName);
	auto iter = std::find_if(cells.begin(), cells.end(),
		[&lowIndexName](std::string cell)
	{
		return lowIndexName == StringFunction::toLower(cell);
	});
	return iter == cells.end() ? 
		std::int32_t{-1} : 
		static_cast<std::int32_t>(std::distance(cells.begin(), iter));
}

bool SignalReader::signalNameValidation(std::string currentName, const char expectedNameSign) const
{
	auto iter{ std::find_if_not(currentName.begin(), currentName.end(), StringFunction::_isalpha) };
	if (iter != currentName.end())
		currentName.erase(iter, currentName.end());
	return (currentName.size() == 1 && 
		currentName.front() == expectedNameSign);
}

std::int32_t SignalReader::getElementCountFromType(std::string type) const
{
	auto frontPos = std::find(type.begin(), type.end(), LEFT_SQARE_BRACKET);
	auto backPos = std::find(type.rbegin(), type.rend(), RIGHT_SQARE_BRACKET);
	if (frontPos == type.end() || backPos == type.rend())
		return std::int32_t{ -1 };
	type = type.substr(std::distance(type.begin(), std::next(frontPos)),
		std::distance(std::next(frontPos), std::next(backPos).base()));
	auto frontValueIter = std::find_if_not(type.begin(), type.end(), StringFunction::_isdigit);
	auto backValueIter = std::find_if_not(type.rbegin(), type.rend(), StringFunction::_isdigit);
	std::int32_t frontValue{},
		backValue{};
	try
	{
		if (frontValueIter == type.end())
			return std::stoi(type);
		else
			frontValue = std::stoi(type.substr(0, std::distance(type.begin(), frontValueIter)));
		backValue = std::stoi(type.substr(std::distance(type.begin(), std::next(backValueIter).base() + 1)));
	}
	catch (...)
	{
		return std::int32_t{ -1 };
	}
	if (frontValue < 0 || backValue < 0 || backValue < frontValue)
		return std::int32_t{ -1 };
	return std::int32_t{ ((backValue - frontValue) + 1) };
}

std::int32_t SignalReader::getNameIndex(const std::vector<std::string>& cells) const
{
	auto signalNameIndex{ findIndex(cells, CSV_SINGALS_NAME_VAR_1) };
	if (signalNameIndex == -1)
	{
		signalNameIndex = findIndex(cells, CSV_SINGALS_NAME_VAR_2);
		if (signalNameIndex == -1)
			signalNameIndex = findIndex(cells, CSV_SINGALS_NAME_VAR_3);
	}
	return signalNameIndex;
}

bool SignalReader::readSignalTable(InfoTable::signals_type & storage, const char type, std::ifstream & stream, const std::uint32_t nameIndex, const std::uint32_t typeIndex, const std::int32_t commenIndex, const std::string & path)
{
	std::string line;
	bool isOk{ true };
	while (std::getline(stream, line))
	{
		auto cellsArray = parseCSV(line);
		if (isReserveSignal(cellsArray))
			continue;
		auto& signalName = cellsArray.at(nameIndex);
		if (signalNameValidation(signalName, type) == false)
		{
			this->log("Некорректный тип сигнала \"" + signalName + "\". Сигнал будет проигнорирован. Файл: " + path);
			isOk = false;
			continue;
		}
		InfoTable::signals_type::mapped_type newSignal;
		newSignal.pumlType = cellsArray.at(typeIndex);
		if (commenIndex != -1)
			newSignal.title = cellsArray.at(commenIndex);

		auto nameBorder = std::find_if_not(signalName.begin(), signalName.end(), isalnum);
		auto typeBorder = std::find_if_not(newSignal.pumlType.begin(), newSignal.pumlType.end(), isalnum);
		bool isArraySizeSeparator{ false };
		if (nameBorder != signalName.end() || typeBorder != newSignal.pumlType.end())
		{
			auto countInName = getElementCountFromType(signalName.substr(std::distance(signalName.begin(), nameBorder)));
			isArraySizeSeparator = signalName.find(ARRAY_SEPAROTOR, std::distance(signalName.begin(), nameBorder)) != std::string::npos;
			signalName.erase(nameBorder, signalName.end());
			auto ptrPos = newSignal.pumlType.find('*');
			if (ptrPos != std::string::npos)
			{
				newSignal.pumlType.erase(ptrPos);
				newSignal.elementCount = countInName;
			}
			else
			{
				newSignal.elementCount = getElementCountFromType(newSignal.pumlType.substr(std::distance(newSignal.pumlType.begin(), typeBorder)));
				newSignal.pumlType.erase(typeBorder, newSignal.pumlType.end());
			}

			if (signalName.size() < 2)
			{
				log("Некорректное название сигнала " + signalName+". Файл: " + path);
				isOk = false;
				continue;
			}

			if (newSignal.elementCount == countInName && countInName <= -1)
			{
				this->log("Отрицательная размерность сигнала " + signalName + ". Используется используется значение MAX(std::uint32_t). Файл: " + path);
				newSignal.elementCount = std::numeric_limits<decltype(InfoTable::signals_type::mapped_type::elementCount)>::max();
			}
			else if (newSignal.elementCount != countInName)
			{
				if (static_cast<std::int32_t>(newSignal.elementCount) < 0 && countInName < 0)
				{
					this->log("Отрицательная размерность сигнала " + signalName + ". Используется используется значение MAX(std::uint32_t). Файл: " + path);
					newSignal.elementCount = std::numeric_limits<decltype(InfoTable::signals_type::mapped_type::elementCount)>::max();
				}
				else
					if (isArraySizeSeparator)
						this->log("Различная размерность сигнала " + signalName + ". Размеры " + std::to_string(newSignal.elementCount) + " и "
							+ std::to_string(countInName) + ". Будет взято наибольшее значение. Файл: " + path);
					else
					{
						
						if (countInName < 0)
						{
							this->log("Отрицательная размерность сигнала " + signalName + ". Используется используется значение MAX(std::uint32_t). Файл: " + path);
							newSignal.elementCount = std::numeric_limits<decltype(InfoTable::signals_type::mapped_type::elementCount)>::max();
						}
						else
							newSignal.elementCount = static_cast<std::uint32_t>(countInName);
					}
			}
			newSignal.isPtrType = true;
			if (countInName > static_cast<std::int32_t>(newSignal.elementCount))
				newSignal.elementCount = static_cast<std::uint32_t>(countInName);
		}
		if (newSignal.pumlType.empty())
		{
			log("Info: Отсутствует тип сигнала(Сигнал будет проигнорирован): " + signalName + ". Файл: " + path);
			isOk = false;
			continue;
		}
		if (isArraySizeSeparator)
		{
			if (storage.emplace(signalName, std::move(newSignal)).second == false)
				this->log("Повтор сигнала: " + signalName + ". Повтор будет проигнорирован. Файл: " + path);
		}
		else
		{
			auto emplaceResult = storage.emplace(signalName, newSignal);
			if (emplaceResult.second == false)
			{
				auto& oldSignal = emplaceResult.first->second;
				if (oldSignal.isPtrType != newSignal.isPtrType)
				{
					this->log("Сигнал \"" + signalName + "\" имеет различные типы. Тип и размер сигнала используется по последней встречи без коллизии. Файл: " + path);
					continue;
				}
				if (oldSignal.elementCount <= newSignal.elementCount)
				{
					if (newSignal.title.size())
						oldSignal.title += ". " + newSignal.title;
					oldSignal.elementCount = newSignal.elementCount + 1;
				}
			}
		}

	}
	return isOk;
}

bool SignalReader::readTimerTable(std::ifstream & stream, const std::uint32_t nameIndex, const std::int32_t commenIndex, const std::string & path)
{
	std::string line;
	bool isOk{ true };
	while (std::getline(stream, line))
	{
		auto cellsArray = parseCSV(line);
		if (isReserveSignal(cellsArray))
			continue;
		auto& signalName = cellsArray.at(nameIndex);
		if (signalNameValidation(signalName, TIMER_SIGNAL) == false)
		{
			this->log("Сигнал \"" + signalName + "\" не ожидался в таблице с таймерами (сигнал будет проигнорирован). Файл: " + path);
			isOk = false;
			continue;
		}
		auto result = this->signals->timers.emplace(signalName, std::string{});
		if (!result.second)
			this->log("Повтор сигнала: " + signalName + ". Повтор будет проигнорирован. Файл: " + path);
		else if (commenIndex != -1)
			result.first->second = cellsArray.at(commenIndex);
	}
	return isOk;
}

bool SignalReader::readConstTable(std::ifstream & stream, const std::uint32_t nameIndex, const std::uint32_t valueIndex, const std::int32_t commenIndex, const std::string & path)
{
	std::string line;
	bool isOk{ true };
	while (std::getline(stream, line))
	{
		auto cellsArray = parseCSV(line);
		if (isReserveSignal(cellsArray))
			continue;
		auto& signalName = cellsArray.at(nameIndex);
		if (signalNameValidation(signalName, CONST_SIGNAL) == false)
		{
			this->log("Сигнал \"" + signalName + "\" не ожидался в таблице с константами (константа будет проигнорирована). Файл: " + path);
			isOk = false;
			continue;
		}
		auto& signalValue = cellsArray.at(valueIndex);
		if (signalValue.empty())
		{
			this->log("Константа \"" + signalName + "\" имеет пустое значение (константа будет проигнорирована). Файл: " + path);
			isOk = false;
			continue;
		}
		else
			if (signalValue.front() != STRING_TOPIC)
				std::transform(signalValue.begin(), signalValue.end(), signalValue.begin(),
					[](char ch) { return ch == INCORRECT_FLOAT_CHAR ? CORRECT_FLOAT_CHAR : ch;});
		auto result = this->signals->constValues.emplace(signalName,
			InfoTable::const_type::mapped_type{  });
		if (!result.second)
			this->log("Повтор сигнала: " + signalName + ". Таблица с константами. Повтор будет проигнорирован.");
		{
			result.first->second.value = std::move(signalValue);
			if (commenIndex != -1)
				result.first->second.title = std::move(cellsArray.at(commenIndex));
		}
	}
	return isOk;
}

std::vector<std::string> SignalReader::parseCSV(const std::string & line) const
{
	auto cellsArray = Readers::CSVLineReader(line, CSV_SEPARATOR, true);
	removeExtraSymbolsFromCells(cellsArray);
	return cellsArray;
}

bool SignalReader::compareSignals(const InfoTableLight::signal_type & pumlSignals, const InfoTable::signals_type & csvSignals) const
{
	bool isOk{ true };
	for (auto& el : pumlSignals)
	{
		if (csvSignals.find(el.first) == csvSignals.end())
		{
			log("В CSV не определен сигнал \"" + el.first + "\".");
			isOk = false;
		}
		else if (csvSignals.at(el.first).isPtrType != el.second.isPtr)
		{
			if (csvSignals.at(el.first).isPtrType)
				log("Возможно сигнал \"" + el.first + "\" не является массивом в алгоритмах (Не является критической ошибкой).");
			else
			{
				log("Сигнал \"" + el.first + "\" является массивом в алгоритмах, но не в CSV.");
				isOk = false;
			}
		}
	}
	for (auto& el : csvSignals)
		if (pumlSignals.find(el.first) == pumlSignals.end())
		{
			log("В алгоритмах не задействован сигнал \"" + el.first + "\" (Не является критической ошибкой).");
		}
	return isOk;
}

bool SignalReader::compareSignals(const InfoTableLight::simple_signal_type & pumlSignals, const std::set<InfoTable::key_type>& csvSignals) const
{
	bool isOk{ true };
	for (auto& el : pumlSignals)
		if (csvSignals.find(el) == csvSignals.end())
		{
			log("В CSV не определен сигнал \"" + el + "\".");
			isOk = false;
		}

	for (auto& el : csvSignals)
		if (pumlSignals.find(el) == pumlSignals.end())
		{
			log("В алгоритмах не задействован сигнал \"" + el + "\" (Не является критической ошибкой).");
		}
	return isOk;
}

bool SignalReader::compareStructSignals(const InfoTableLight::simple_signal_type& pumlSignals) const
{
	std::set<std::string> nonDirectUsage;
	for (auto& structData : signals->structData)
	{
		auto iter = pumlSignals.find(structData.first);
		if (iter != pumlSignals.end())
			continue;
		else
			nonDirectUsage.emplace(structData.first);
	}
	bool isOk{ true };
	for (auto& signal : pumlSignals)
	{
		if (signal.front() == STRUCT_SIGNAL)
		{
			if (signals->structData.find(signal) == signals->structData.end())
			{
				log("Алгоритмы используют неопределенную структуру \""+ signal + "\".");
				isOk &= false;
			}
			continue;
		}
		auto splitted = Readers::CSVLineReader(signal, STRUCT_FIELD_SYMBOL);
		std::string fullSignal;
		for (auto& part : splitted)
		{
			StringFunction::appendString(part, fullSignal, STRUCT_FIELD_SYMBOL);
			//TODO: Возможно не работает определение структур, в обращении которых есть массив
			auto structInfo = signals->getSignalInfo(fullSignal, AlgorithmInfo{});
			if (structInfo.isData)
			{
				if (structInfo.pumlType.size() && structInfo.pumlType.front() == STRUCT_SIGNAL)
				{
					auto nonDirectIter = nonDirectUsage.find(structInfo.pumlType);
					if (nonDirectIter != nonDirectUsage.end())
						nonDirectUsage.erase(nonDirectIter);
					continue;
				}
				if (fullSignal != signal)
				{
					log("Алгоритмы используют сигнал \"" + signal + "\" с неопределённым типом.");
					isOk = false;
				}
			}
			else
			{
				log("Алгоритмы используют сигнал \"" + signal + "\" с неопределённым типом.");
				isOk = false;
			}
		}
	}
	for (auto& el : nonDirectUsage)
		log("В алгоритме, возможно, не используется \"" + el + "\"структура (Анализ является не полным т.к. не проверяет обращения локальных переменных).");
	return isOk && nonDirectUsage.empty();
}

SignalReader::SignalReader()
	:signals(new signal_type::element_type)
{
}

bool SignalReader::readSignals(const std::vector<std::string>& signalsPaths)
{
	clear();
	if (signalsPaths.empty())
		return false;
	bool isOk{ true };
	
	for (auto& currentPath : signalsPaths)
	{
		FileReading::unique_ifstream_type stream
		{ new FileReading::unique_ifstream_type::element_type{ currentPath, std::ios::binary | std::ios::in }, FileReading::closeFunction };
		if (!stream->is_open())
		{
			this->log("Невозможно открыть файл: " + currentPath);
			isOk = false;
			continue;
		}
		if (isStructTable(currentPath))
			isOk &= readStructTable(*stream, currentPath);
		else
			isOk &= readSignalTable(*stream, currentPath);
	}
	return isOk;
}

SignalReader::const_signal_type SignalReader::getSignals() const
{
	return signals;
}


void SignalReader::clear()
{
	signals.reset(new signal_type::element_type);
}

template<typename T>
static std::set<InfoTable::key_type> map2Set(std::unordered_map<InfoTable::key_type, T> map)
{
	std::set<InfoTable::key_type> transformedData;
	for (auto& el : map)
		transformedData.insert(el.first);
	return transformedData;
}

bool SignalReader::compareSignalsWithPumlSignals(const InfoTableLight & pumlSignal) const
{
	auto compareResult = compareSignals(pumlSignal.constValues, map2Set(signals->constValues));
	compareResult &= compareStructSignals(pumlSignal.structValues);
	compareResult &= compareSignals(pumlSignal.timers, map2Set(signals->timers));

	compareResult &= compareSignals(pumlSignal.inputSignals, signals->inputSignals);
	compareResult &= compareSignals(pumlSignal.innerValues, signals->innerValues);
	compareResult &= compareSignals(pumlSignal.outputSignals, signals->outputSignals);

	compareResult &= pumlSignal.unknownSignals.empty();
	for (auto& signal : pumlSignal.unknownSignals)
		log("В алгоритмах используется неизвестный сигнал \"" + signal + "\" (Возможны ошибки в определении сигналов. Возможно не является ошибкой).");
	return compareResult;
}

SignalReader::~SignalReader()
{
	clear();
}
