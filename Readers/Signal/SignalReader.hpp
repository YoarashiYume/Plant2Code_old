#pragma once

#include <vector>

#include "../../Struct/Signals/SignalStruct.hpp"
#include "../../Common/Loggable.hpp"
/*
///\brief - Класс для считывания и хранения сигналов из CSV-таблиц
*/
class SignalReader : public Loggable
{
public:
	using signal_type = std::shared_ptr<InfoTable>;
	using const_signal_type = std::shared_ptr<const InfoTable>;
private:
	signal_type signals;			///< - Считанные сигналы из CSV
	/*
	///\brief - Метод проверяющий является ли таблица - таблицей со структурами
	///\param[in] path - Путь до CSV-таблицы
	///\return Возвращает true, если таблица должна содержать данные о структурах
	*/
	bool isStructTable(std::string path) const;
	/*
	///\brief - Метод считывающий данные структур из CSV-таблиц
	///\param[in, out] path - Stream I до таблицы path
	///\param[in] path - Путь до CSV-таблицы
	///\return Возвращает true, если считывание произошло успешно
	*/
	bool readStructTable(std::ifstream& stream, const std::string& path);
	/*
	///\brief - Метод считывающий данные сигналов из CSV-таблиц
	///\param[in, out] path - Stream I до таблицы path
	///\param[in] path - Путь до CSV-таблицы
	///\return Возвращает true, если считывание произошло успешно
	*/
	bool readSignalTable(std::ifstream& stream, const std::string& path);
	/*
	///\brief - Метод проверяющий является ли сигнал резервным
	///\param[in] cells - Поля CSV-таблицы
	///\return Возвращает true, если сигнал является резервным
	*/
	bool isReserveSignal(const std::vector<std::string>& cells) const;
	/*
	///\brief - Метод удаляющий из записей лишние символы
	///\param[in] cells - Поля CSV-таблицы
	*/
	void removeExtraSymbolsFromCells(std::vector<std::string>& cells)const;
	/*
	///\brief - Метод поиска индекса необходимого поля CSV-таблицы
	///\param[in] cells - Поля CSV-таблицы
	///\param[in] indexName - Искомое содержание ячейки
	///\return Индекс ячейки или -1, если ячейка найдена не была
	*/
	std::int32_t findIndex(const std::vector<std::string>& cells, const std::string &indexName) const;
	/*
	///\brief - Метод проверки названия сигнала
	///\param[in] currentName - Проверяемое название
	///\param[in] expectedNameSign -Ожидаемая буква сигнала
	///\return true - если название сигнала валидное
	*/
	bool signalNameValidation(std::string currentName, const char expectedNameSign) const;
	/*
	///\brief - Метод поиска размерности массива в типе
	///\param[in] type - тип
	///\return Размер массива или -1, если данные не были найдены
	*/
	std::int32_t getElementCountFromType(std::string type) const;
	/*
	///\brief - Метод поиска индекса необходимого названия сигнала
	///\param[in] cells - Поля CSV-таблицы
	///\return Размер массива или -1, если данные не были найдены
	*/
	std::int32_t getNameIndex(const std::vector<std::string>& cells) const;
	/*
	///\brief - Метод считывающий данные сигналов из CSV-таблиц
	///\param[in, out] storage - Контейнер для хранения сигналов
	///\param[in] type - Тип считываемых сигналов
	///\param[in, out] path - Stream I до таблицы path
	///\param[in] nameIndex - Индекс поля с названием сигнала
	///\param[in] typeIndex - Индекс поля с типом сигнала
	///\param[in] commenIndex - Индекс поля с комментарием к сигналу
	///\param[in] path - Путь до CSV-таблицы
	///\return Возвращает true, если считывание произошло успешно
	*/
	bool readSignalTable(InfoTable::signals_type& storage, const char type, std::ifstream& stream, const std::uint32_t nameIndex, const std::uint32_t typeIndex, const std::int32_t commenIndex, const std::string& path);
	/*
	///\brief - Метод считывающий данные таймеров из CSV-таблиц
	///\param[in, out] path - Stream I до таблицы path
	///\param[in] nameIndex - Индекс поля с названием сигнала
	///\param[in] commenIndex - Индекс поля с комментарием к сигналу
	///\param[in] path - Путь до CSV-таблицы
	///\return Возвращает true, если считывание произошло успешно
	*/
	bool readTimerTable(std::ifstream& stream, const std::uint32_t nameIndex, const std::int32_t commenIndex, const std::string& path);
	/*
	///\brief - Метод считывающий данные таймеров из CSV-таблиц
	///\param[in, out] path - Stream I до таблицы path
	///\param[in] nameIndex - Индекс поля с названием сигнала
	///\param[in] valueIndex - Индекс поля со значением
	///\param[in] commenIndex - Индекс поля с комментарием к сигналу
	///\param[in] path - Путь до CSV-таблицы
	///\return Возвращает true, если считывание произошло успешно
	*/
	bool readConstTable(std::ifstream& stream, const std::uint32_t nameIndex, const std::uint32_t valueIndex, const std::int32_t commenIndex, const std::string& path);
	
	/*
	///\brief - Метод парсинга строки CSV-таблиц в массив
	///\param[in] path - Строка CSV-таблицы
	///\return Возвращает массив из полей таблицы
	*/

	std::vector<std::string> parseCSV(const std::string& line) const;
	/*
	///\brief - Метод проверяющий соответствие сигналов из CSV-таблиц и puml-файлов
	///\param[in] pumlSignals - Сигналы из puml-файлов
	///\param[in] csvSignals - Сигналы из CSV-таблиц
	///\return Возвращает true, если сигналы совпадают
	*/
	bool compareSignals(const InfoTableLight::signal_type& pumlSignals, const InfoTable::signals_type& csvSignals) const;
	/*
	///\brief - Метод проверяющий соответствие сигналов из CSV-таблиц и puml-файлов
	///\param[in] pumlSignals - Сигналы из puml-файлов
	///\param[in] csvSignals - Сигналы из CSV-таблиц
	///\return Возвращает true, если сигналы совпадают
	*/
	bool compareSignals(const InfoTableLight::simple_signal_type& pumlSignals, const std::set<InfoTable::key_type> &csvSignals) const;
	/*
	///\brief - Метод проверяющий соответствие сигналов из CSV-таблиц и puml-файлов
	///\param[in] pumlSignals - Табличные сигналы из puml-файлов
	///\return Возвращает true, если сигналы совпадают
	*/
	bool compareStructSignals(const InfoTableLight::simple_signal_type& pumlSignals) const;
public:
	SignalReader();
	/*
	///\brief - Метод считывающий сигналы из CSV-таблиц
	///\param[in] signalsPaths - Список путей до CSV-таблиц
	///\return Возвращает true, если сигналы были считаны успешно
	*/
	bool readSignals(const std::vector<std::string>& signalsPaths);
	/*
	///\brief - Метод выдает считанные сигналы
	///\return Считанные сигналы
	*/
	const_signal_type getSignals() const;
	/*
	///\brief - Метод очистки данных
	*/
	void clear();

	/*
	///\brief - Метод проверяющая соответствие сигналов из CSV-таблиц и puml-файлов
	///\param[in] pumlSignal - Сигналы из puml-файлов
	///\return Возвращает true, если сигналы совпадают
	*/
	bool compareSignalsWithPumlSignals(const InfoTableLight& pumlSignal) const;

	~SignalReader();
};

