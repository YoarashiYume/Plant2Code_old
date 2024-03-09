#pragma once
#include <string>
#include <memory>
#include <vector>
#include "../../Common/Def.hpp"
/*
///\brief - Базовый тип сигналов
*/
struct NoTypeValue
{
	using string_type = std::string;

	string_type title;				///< - Описание сигнала
	NoTypeValue() = default;
	virtual ~NoTypeValue() = default;
};
/*
///\brief - Тип для хранения информации о константных сигналов
*/
struct ConstInfo : NoTypeValue
{
	string_type value;					///< - Значение константы

	bool operator==(const ConstInfo& other) const
	{
		return other.value == value;
	}
	ConstInfo() = default;
	virtual ~ConstInfo() = default;
};
/*
///\brief - Тип для хранения информации о входных, выходных и внутренних сигналах
*/
struct ValueInfo : public NoTypeValue
{
	using element_count_type = std::uint32_t;

	string_type pumlType;				///< - Puml-тип сигнала
	bool isPtrType{false};				///< - Является ли тип указателем/массивом
	element_count_type elementCount{1};	///< - Кол-во элементов

	bool operator==(const ValueInfo& other) const
	{
		return ((pumlType == other.pumlType) &&
			(isPtrType == other.isPtrType) &&
			(elementCount == other.elementCount));
	}
	ValueInfo() = default;
	virtual ~ValueInfo() = default;
};
/*
///\brief - Возможные состояния сигнала в качестве переменных
*/
enum class ABILITY_IN_ARGS : std::uint8_t
{
	UNKNOWN = 0,							///< - Неизвестно
	SIMPLE,									///< - Передаваться по значению
	PTRABLE,								///< - Передаваться как указатель
	RETURNABLE,								///< - Передаваться по ссылке
	PTRABLE_AND_RETURNABLE					///< - Передача по указателю с возможностью изменения
};
/*
///\brief - Тип для хранения информации о сигналах, используемых в качестве параметров
*/
struct ParamName : ValueInfo
{
	string_type name;					///< - Название аргумента
	
	ABILITY_IN_ARGS argInfo{ ABILITY_IN_ARGS::UNKNOWN };

	bool operator==(const ParamName& other) const
	{
		return ((*dynamic_cast<const ValueInfo*>(this) == *dynamic_cast<const ValueInfo*>(&other))
			&& (name == other.name));
	}
	bool operator!=(const ParamName& other) const
	{
		return ((*this == other) == false);
	}
	ParamName() = default;
	virtual ~ParamName() = default;
};

/*
///\brief - Тип для хранения информации о структурах
*/
struct StructInfo : NoTypeValue
{
	using field_type = std::shared_ptr<ValueInfo>;
	using storage_type = std::vector<field_type>;

	storage_type structFieldType;			///< - Поля структуры (в названии нет необходимости	т.к. обращение по индексам)
	StructInfo() = default;
	virtual ~StructInfo() = default;
};


/*
///\brief - Тип Сигнала из Puml
*/
enum class PUML_SIGNAL_TYPE : char
{
	UNKNOWN = 0,							///< - Неизвестный сигнал
	INPUT = INPUT_SIGNAL,					///< - Входной сигнал
	OUTPUT = OUTPUT_SIGNAL,					///< - Выходной сигнал
	INNER = INNER_SIGNAL,					///< - Внутренний сигнал
	CONST_SIGN = CONST_SIGNAL,				///< - Константа
	TIMER = TIMER_SIGNAL,					///< - Таймер
	LOCAL_ARS = PLANTUML_LOCAL_PAR_CH,		///< - Локальный сигнал алгоритма
	INPUT_ARG = PLANTUML_INPUT_PAR_CH,		///< - Входной сигнал алгоритма
	OUTPUT_ARG = _PLANTUML_OUTPUT_PAR_CH	///< - Выходной сигнал алгоритма
};
/*
///\brief - Тип для получения базовой информации о сигнале
*/
struct GeneralSignalData : ValueInfo
{
	bool isData{false};											///< - Был ли найден сигнал
	ABILITY_IN_ARGS abitilyInArgs{ ABILITY_IN_ARGS ::UNKNOWN};	///< - Сигнал как аргумент
	PUML_SIGNAL_TYPE type{ PUML_SIGNAL_TYPE ::UNKNOWN};			///< - Тип сигнала

	void copy(const ValueInfo& other)
	{
		pumlType = other.pumlType;
		isPtrType = other.isPtrType;
		title = other.title;
		elementCount = other.elementCount;
		abitilyInArgs = other.isPtrType ? ABILITY_IN_ARGS::PTRABLE
			: ABILITY_IN_ARGS::SIMPLE;
		isData = true;
	}
	void copy(const ParamName& other)
	{
		copy(static_cast<ValueInfo>(other));
		abitilyInArgs = other.argInfo;
	}

	GeneralSignalData() = default;
	virtual ~GeneralSignalData() = default;
};
/*
///\brief - Тип сигнала передаваемого в C#
*/
enum class SIGNAL_TYPE : std::uint32_t
{
	INPUT_TYPE = 0,		///< - Входной сигнал
	INNER_TYPE,			///< - Внутренний сигнал
	OUTPUT_TYPE,		///< - Выходной сигнал
	TIMERS_TYPE			///< - Таймер
};