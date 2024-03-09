#pragma once
#include <unordered_map>
#include <set>
#include "SignalInfo.hpp"
#include "../Algorithm/AlgorithmStruct.hpp"
/*
///\brief - Структура для хранения информации о сигналах из CSV-таблиц
*/
struct InfoTable
{
	using key_type = std::string;
	using struct_type = std::unordered_map<key_type, StructInfo>;
	using const_type = std::unordered_map<key_type, ConstInfo>;
	using signals_type = std::unordered_map<key_type, ValueInfo>;
	using timer_type = std::unordered_map<key_type, std::string>;

	signals_type inputSignals;		///< - Данные о входных сигналах
	signals_type outputSignals;		///< - Данные о выходных сигналах
	signals_type innerValues;		///< - Данные о внутренних сигналах
	const_type constValues;			///< - Данные о константах
	timer_type timers;				///< - Данные о таймерах
	struct_type structData;			///< - Данные о структурах

	/*
	///\brief - Метод определения информации о поле таблицы
	///\param [in, out] signalData - Данные о сигнале. Используется при работе функции. Перезаписывается данными полей
	///\param [in] structLine - Информация о структуре
	*/
	void getStructData(GeneralSignalData& signalData, const std::string& structLine) const;
	/*
	///\brief - Метод получения информации о сигнале
	///\param [in] terms - Сигнал
	///\param [in] currentAlgorithm - Текущий алгоритм
	///\return Если усключение было успешно - true
	*/
	GeneralSignalData getSignalInfo(const std::string& signal, const AlgorithmInfo& currentAlgorithm) const;
};
/*
///\brief - Структура для хранения информации об обращении к полям сигналов
*/
struct FieldInfo
{
	using key_type = std::uint32_t;

	bool isPtr;						///< - Является ли сигнал указателем/массивом
	std::unordered_map<key_type, FieldInfo> usedField;///< -Список используемых полей

	FieldInfo& operator+=(const FieldInfo& rhs)
	{
		if (isPtr != rhs.isPtr)
			throw "Different type";
		for (auto& el : rhs.usedField)
		{
			auto iter = usedField.find(el.first);
			if (iter == usedField.end())
				usedField.emplace(el);
			else
				iter->second += el.second;
		}
		return *this;
	}

};
/*
///\brief - Структура для хранения информации о сигналах из CSV-таблиц
*/
struct InfoTableLight
{
	using key_type = std::string;
	using simple_signal_type = std::set<key_type>;
	using signal_type = std::unordered_map<key_type, FieldInfo>;

	signal_type inputSignals;			///< - Данные о входных сигналах
	signal_type outputSignals;			///< - Данные о выходных сигналах
	signal_type innerValues;			///< - Данные о внутренних сигналах
	simple_signal_type constValues;		///< - Данные о константах
	simple_signal_type structValues;	///< - Данные о структурах
	simple_signal_type timers;			///< - Данные о таймерах
	simple_signal_type unknownSignals;	///< - Данные о неизвестных сигналов
};