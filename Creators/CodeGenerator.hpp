#pragma once
#include <type_traits>
#include <memory>

#include "../Common/Loggable.hpp"
#include "../Common/FileReading.hpp"
#include "../EditableData/Dictionary.hpp"
#include "../EditableData/Generators.hpp"
#include "../Struct/Algorithm/AlgorithmStruct.hpp"
#include "../Struct/Signals/SignalStruct.hpp"

class CodeGenerator : public Loggable
{
public:
	using algorithm_data_type = AlgorithmInfo;
	using store_algorithm_data_type = std::shared_ptr<algorithm_data_type>;
	using algorithm_type = std::shared_ptr<std::unordered_map<std::string, store_algorithm_data_type>>;
	using signal_type = std::shared_ptr<const InfoTable>;
	
	using generator_simple_type = OneFileGenerator::return_type;
	using generator_offset_type = OneFileGenerator::return_with_offset_type;

	using stream_type = std::ofstream;
private:
	using inline_function_type = std::string(OneFileGenerator::*)(const std::string&) const;
	using inline_function_list_type = std::vector<inline_function_type>;
	using inline_function_store_type = std::unordered_map<TERM, inline_function_list_type>;
	
	inline_function_store_type inlineFunctions;		///< - Список встроенных функций языка
	std::unique_ptr<TypeDictionary> dictionary;		///< - Словарь языка для генерации
	std::unique_ptr<OneFileGenerator> generator;	///< - Генератор конструкций языка
			
	signal_type signalInfo;							///< - Список используемых сигналов
	algorithm_type algorithmInfo;					///< - Список исправляемых алгоритмов

	std::string outputPath;							///< - Путь до директории для генерации
	std::string offsetInFile;						///< - Отступ от начала строки при генерации


	CodeGenerator(TypeDictionary*&& dictionary, OneFileGenerator*&& generator);
	/*
	///\brief - Тип генерируемого файла
	*/
	enum FILE_TYPE_GEN
	{
		SOURCE,				///< - source-файл
		HEADER,				///< - header-файл
		LIB_HEADER,			///< - библиотечный header-файл
		LIB_SOURCE			///< - библиотечный source-файл
	};
	
	/*
	///\brief - Метод для получения расположения файлов
	///\param [in] isExtension - Необходимо ли расширение файла
	///\param [in] isHeader - true для заголовка, false - для source-файла
	///\return Расположение файла
	*/
	std::string genPath(const std::string& fileName, bool isExtension, bool isHeader = false) const;
	/*
	///\brief - Метод для изменения отступа в файле
	///\param [in] offsetChange - Изменения отступа
	*/
	void changeOffset(const std::int32_t offsetChange);
	/*
	///\brief - Метод для генерации пользовательской информации с отступом
	///\param [in, out] stream - Поток для записи данных
	///\param [in] dataForGen - Данные для генерации
	///\param [in] changeBefore - Отступ изменяется до написания
	*/
	void genWithOffset(stream_type& stream, const generator_offset_type& dataForGen = {});
	/*
	///\brief - Метод для генерации пользовательской информации без отступа
	///\param [in, out] stream - Поток для записи данных
	///\param [in] dataForGen - Данные для генерации
	*/
	void genWithoutOffset(stream_type& stream, const generator_simple_type& dataForGen = {}) const;
	/*
	///\brief - Метод для генерации include`ов
	///\param [in, out] stream - Поток для записи данных
	///\param [in] fileForInclude - Подключаемый файл
	*/
	void genIncludeFile(stream_type& stream, const std::string& fileForInclude);
	/*
	///\brief - Метод для генерации глобальных сигналов
	///\param [in, out] stream - Поток для записи данных
	*/
	bool genGlobalSignal(stream_type& stream);
	/*
	///\brief - Метод для генерации структур
	///\param [in, out] stream - Поток для записи данных
	*/
	bool genStructTypes(stream_type& stream);
	/*
	///\brief - Метод для определения порядка генерации структур
	///\return Порядок генерации структур
	*/
	std::vector<std::string> getOrderForStructGen() const;
	/*
	///\brief - Метод для генерации brief-комментария
	///\param [in, out] stream - Поток для записи данных
	///\param [in] brief - Описание
	*/
	void genDoxygenBriefComment(stream_type& stream, const std::string& brief);
	/*
	///\brief - Метод для определения необходимого типа переменной
	///\param [in] pumlType - Puml-тип
	///\param [in] isPtr - Является ли тип указателем
	///\param [in] elementCount - Кол-во элементов в типе
	///\param [in] canBeTimer - Может ли переменная являться сигналом
	///\return Тип переменной
	*/
	std::string getType(const std::string& pumlType, const bool isPtr, const std::uint32_t elementCount, const bool canBeTimer) const;
	/*
	///\brief - Метод для определения необходимого типа переменной
	///\param [in] valueInfo - Информация о переменной
	///\param [in] canBeTimer - Может ли переменная являться сигналом
	///\return Тип переменной
	*/
	std::string getType(const ValueInfo& valueInfo, const bool canBeTimer) const;
	/*
	///\brief - Метод для генерации field-комментария
	///\param [in, out] stream - Поток для записи данных
	///\param [in] field - Описание
	*/
	void genDoxygenFieldComment(stream_type& stream, const std::string& field, const std::string& fieldComment);
	/*
	///\brief - Метод для генерации констант
	///\param [in, out] stream - Поток для записи данных
	///\return При успешной генерации - true
	*/
	bool genConst(stream_type& stream);
	/*
	///\brief - Метод генерации для таймеров
	///\param [in, out] stream - Поток для записи данных
	///\return При успешной генерации - true
	*/
	bool genTimers(stream_type& stream);
	/*
	///\brief - Метод генерации для сигналов
	///\param [in, out] stream - Поток для записи данных
	///\param [in] signal - Данные о генерируемых сигналах
	///\param [in] signalType - Тип генерируемых сигналов
	///\return При успешной генерации - true
	*/
	bool genGeneralSignals(stream_type& stream, const InfoTable::signals_type& signal, const std::string& signalType);
	/*
	///\brief - Метод генерации кода алгоритмов
	///\param [in, out] stream - Поток для записи данных
	///\param [in] type - Тип генерируемого файла
	///\return При успешной генерации - true
	*/
	bool genFunctions(stream_type& stream, const FILE_TYPE_GEN type);
	/*
	///\brief - Метод генерации кода алгоритма
	///\param [in, out] stream - Поток для записи данных
	///\param [in] algorithmData - Данные алгоритма
	///\param [in] algorithmName - Название алгоритма
	///\param [in] type - Тип генерируемого файла
	///\param [in] isAccessFunction - Генерируется ли функция доступа
	///\return При успешной генерации - true
	*/
	bool genFunctions(stream_type& stream, const AlgorithmInfo& algorithmData, const std::string& algorithmName, const FILE_TYPE_GEN type, const bool isAccessFunction = false);
	/*
	///\brief - Метод генерации комментария алгоритма
	///\param [in, out] stream - Поток для записи данных
	///\param [in] algorithmData - Данные алгоритма
	///\param [in] algorithmName - Название алгоритма
	///\param [in] isAccessFunction - Генерируется ли функция доступа
	///\return При успешной генерации - true
	*/
	bool genFunctionComment(stream_type& stream, const AlgorithmInfo& algorithmData, const std::string& algorithmName, const bool isAccessFunction = false);
	/*
	///\brief - Метод для генерации Param-комментария
	///\param [in, out] stream - Поток для записи данных
	///\param [in] param - Название параметра
	///\param [in] paramCommand - Описание параметра
	///\param [in] argsType - Является ли параметром только входным
	*/
	void genDoxygenParamComment(stream_type& stream, const std::string& param, const std::string& paramCommand, const std::string& argsType) const;
	/*
	///\brief - Метод генерации сигнатуры функции
	///\param [in, out] stream - Поток для записи данных
	///\param [in] algorithmData - Данные алгоритма
	///\param [in] algorithmName - Название алгоритма
	///\param [in] type - Тип генерируемого файла
	///\param [in] isAccessFunction - Генерируется ли функция доступа
	///\return При успешной генерации - true
	*/
	bool genFunctionSignature(stream_type& stream, const AlgorithmInfo& algorithmData, const std::string& algorithmName, const FILE_TYPE_GEN type, const bool isAccessFunction = false);
	/*
	///\brief - Метод собирающий аргументы для сигнатуры
	///\param [in] algorithmData - Данные алгоритма
	///\param [in] algorithmName - Название алгоритма
	///\return При успешном сборе - true и массив аргументов, иначе false
	*/
	std::pair<OneFileGenerator::singature_build_args, bool> buildArgsForSignature(const AlgorithmInfo& algorithmData, const std::string& algorithmName) const;
	/*
	///\brief - Метод генерирующий конец алгоритма
	///\param [in, out] stream - Поток для записи данных
	///\param [in] algorithmData - Данные алгоритма
	///\return При успешной генерации - true
	*/
	bool genReturn(stream_type& stream, const AlgorithmInfo& algorithmData);
	/*
	///\brief - Метод генерирующий вызов функции
	///\param [in, out] stream - Поток для записи данных
	///\param [in] algorithmData - Данные алгоритма
	///\param [in] instruction - Данные вызова
	///\param [in] algorithmName - Название алгоритма
	///\return При успешной генерации - true
	*/
	bool genFunctionCall(stream_type& stream, const algorithm_data_type& algorithmData, const FunctionAlgorithmCommand& instruction, const std::string& algorithmName);
	/*
	///\brief - Метод генерирующий конструкцию языка
	///\param [in] line - Данные для генерации
	///\param [in] command - Тип конструкции
	///\return При успешной генерации - true
	*/
	bool genCommand(stream_type & stream, const std::string& line, const COMMAND_TYPE command);
	/*
	///\brief - Метод генерирующий инструкцию алгоритма
	///\param [in, out] stream - Поток для записи данных
	///\param [in] algorithmData - Данные алгоритма
	///\param [in] instruction - Генерируемая инструкция
	///\param [in] algorithmName - Название алгоритма
	///\return При успешной генерации - true
	*/
	bool genFunctionInstruction(stream_type& stream, const AlgorithmInfo& algorithmData, const AlgorithmInfo::command_type & instruction, const std::string& algorithmName);
	/*
	///\brief - Метод для преобразования термов в строку с кодом
	///\param [in] algorithmData - Данные алгоритма
	///\param [in] terms - Данные строки
	///\return Кодовая строка
	*/
	std::string termsToCodeLine(const algorithm_data_type& algorithmData, const AlgorithmCommand::terms_type& terms) const;
	/*
	///\brief - Метод для исправления обращений к сигналам
	///\param [in] algorithmData - Данные алгоритма
	///\param [in, out] terms - Данные строки
	///\param [in, out] index - текущий индекс
	*/
	void fixSignalsInTerms(const algorithm_data_type& algorithmData, AlgorithmCommand::terms_type& terms, AlgorithmCommand::terms_type::size_type& index) const;
	/*
	///\brief - Метод для исправления обращений ко встроенным функциям
	///\param [in] algorithmData - Данные алгоритма
	///\param [in, out] terms - Данные строки
	///\param [in, out] index - текущий индекс
	*/
	void fixInlineFunctionInTerms(const algorithm_data_type& algorithmData, AlgorithmCommand::terms_type& terms, AlgorithmCommand::terms_type::size_type& index) const;
	/*
	///\brief - Метод для исправления использования степени
	///\param [in] algorithmData - Данные алгоритма
	///\param [in, out] terms - Данные строки
	///\param [in, out] index - текущий индекс
	*/
	void fixPowInTerms(const algorithm_data_type& algorithmData, AlgorithmCommand::terms_type& terms) const;
	/*
	///\brief - Метод генерации кода алгоритма
	///\param [in, out] stream - Поток для записи данных
	///\param [in] algorithmData - Данные алгоритма
	///\param [in] algorithmName - Название алгоритма
	///\param [in] isAccessFunction - Генерируется ли функция доступа
	///\return При успешной генерации - true
	*/
	bool genFunctionBody(stream_type& stream, const AlgorithmInfo& algorithmData, const std::string& algorithmName, const bool isAccessFunction = false);
	/*
	///\brief - Метод генерации кода алгоритма
	///\param [in, out] stream - Поток для записи данных
	///\param [in] signalData - Данные сигнала
	///\param [in] algorithmName - Название алгоритма
	///\return При успешной генерации - true
	*/
	bool genLocalSignal(stream_type& stream, const algorithm_data_type& algorithmData, const algorithm_data_type::param_type& signalData, const std::string& algorithmName);
	/*
	///\brief - Метод для выделения блока термов
	///\param [in, out] terms - Данные строки
	///\param [in, out] start - Начальный индекс
	///\param [in] startTerm - Начальный терм
	///\param [in] endTerm - Конечный терм
	///\return Искомый массив термов
	*/
	AlgorithmCommand::terms_type getScope(std::vector<AlgorithmCommand::terms_type::value_type>& terms,
		AlgorithmCommand::terms_type::size_type& start, const TERM startTerm, const TERM endTerm) const;
	/*
	///\brief - Метод находящий вложенный алгоритм
	///\param [in] terms - Проверяемая строка
	///\param [in] start - Начало поиска
	///\return При успехе - массив термов массива
	*/
	AlgorithmCommand::terms_type getArrayIndexTerms(const AlgorithmCommand::terms_type & terms, const AlgorithmCommand::terms_type::size_type startIndex = 0) const;
	/*
	///\brief - Метод находящий структуру
	///\param [in] algorithmData - Данные алгоритма
	///\param [in] terms - Проверяемая строка
	///\param [in] start - Индекс начала структуры
	///\return При успехе - массив термов структуры
	*/
	AlgorithmCommand::terms_type getStructTerms(const algorithm_data_type& algorithmData, std::vector<AlgorithmCommand::terms_type::value_type>& terms, AlgorithmCommand::terms_type::size_type& start) const;
	/*
	///\brief - Метод преобразования термов структуры в строку
	///\param [in] algorithmData - Данные алгоритма
	///\param [in] terms - Строка со структурой
	///\param [in] start - Индекс начала структуры
	///\param [in] structInfo - Данные структуры
	///\param [in] valueInfo - Данные сигнала
	///\param [in] algorithmName - Название алгоритма
	///\return При успехе - массив термов структуры
	*/
	std::string structTermsToLine(const algorithm_data_type& algorithmData, AlgorithmCommand::terms_type & terms, AlgorithmCommand::terms_type::size_type& start, const StructInfo* structInfo, const ValueInfo* valueInfo) const;
	/*
	///\brief - Метод создающий алгоритм для функций доступа
	///\param [in] name - Название сигнала
	///\param [in] signalData - Данные сигнала
	///\return При успехе - алгоритмы для генерации
	*/
	std::vector<AlgorithmInfo> getAccessData(const std::string& name, const GeneralSignalData& signalData) const;
	/*
	///\brief - Метод для генерации функций доступа
	///\param [in, out] stream - Поток для записи данных
	///\param [in] signals - данные о сигналах
	///\param [in] type - Тип генерируемого файла
	///\return При успешной генерации - true
	*/
	template <typename T>
	bool genAccessFunction(stream_type& stream, const std::unordered_map < std::string, T >& signals, const FILE_TYPE_GEN type);
	/*
	///\brief - Метод для сортировки сигналов по имени
	///\param [in] l - Первый сигнал для сравнения
	///\param [in] r - Второй сигнал для сравнения
	///\return При успешной генерации - true
	*/
	static bool signalSortFunction(const std::string& l, const std::string& r);
	/*
	///\brief - Метод сортирующий сигналы по имени
	///\param [in] signals - Данные о сигналах
	///\return Отсортированный список сигналов
	*/
	template <typename T>
	static std::vector<std::string> getSortedSignalsKeys(const std::unordered_map < std::string, T >& signals );
	/*
	///\brief - Метод проверяющий есть ли у алгоритма инструкция return
	///\param [in] algorithmData - Данные алгоритма
	///\return Есть ли у алгоритма инструкция return
	*/
	static bool checkIsOneReturnAlgorithm(const AlgorithmInfo& algorithmData);
	/*
	///\brief - Метод создающий поток для записи данных
	///\param [in] path - Путь до файла
	///\return Поток для записи данных
	*/
	FileReading::unique_ofstream_type openFile(const std::string& path);
	/*
	///\brief - Метод для генерации lib-header-файла
	///\param [in, out] stream - Поток для записи данных
	///\return Если генерация выполнена успешно - true
	*/
	bool genLibHeaderFile(stream_type& stream);
	/*
	///\brief - Метод для генерации библиотечных структур
	///\param [in, out] stream - Поток для записи данных
	///\return Если генерация выполнена успешно - true
	*/
	bool genLibStruct(stream_type& stream);
	/*
	///\brief - Метод для генерации библиотечных сигнатур
	///\param [in, out] stream - Поток для записи данных
	///\return Если генерация выполнена успешно - true
	*/
	bool genLibHeaderFunction(stream_type& stream);
	/*
	///\brief - Метод для генерации lib-source-файла
	///\param [in, out] stream - Поток для записи данных
	///\return Если генерация выполнена успешно - true
	*/
	bool genLibSourceFile(stream_type& stream);
	/*
	///\brief - Метод Подсчета кол-ва сигналов для передачи в C#
	///\return Кол-во передаваемых сигналов
	*/
	std::uint32_t getSignalCount() const;
	/*
	///\brief - Метод для генерации библиотечной функции доступа к сигналам
	///\param [in, out] stream - Поток для записи данных
	///\return Если генерация выполнена успешно - true
	*/
	bool genLibSignalGetFunction(stream_type& stream);
	/*
	///\brief - Метод для генерации библиотечной функции доступа к таймерам
	///\param [in, out] stream - Поток для записи данных
	///\param [in, out] counter - Счетчик сигналов
	///\return Если генерация выполнена успешно - true
	*/
	bool genLibTimerGetFunction(stream_type& stream, std::uint32_t& counter);
	/*
	///\brief - Метод для генерации библиотечной функции доступа к сигналам из CSV-таблиц
	///\param [in, out] stream - Поток для записи данных
	///\param [in] storage - Список сигналов
	///\param [in] type - Тип сигналов
	///\param [in, out] counter - Счетчик сигналов
	///\return Если генерация выполнена успешно - true
	*/
	bool genLibGeneralSignalGetFunction(stream_type& stream, const InfoTable::signals_type& storage, const SIGNAL_TYPE type, std::uint32_t& counter);
	/*
	///\brief - Метод для генерации библиотечной функции доступа к полям структуры
	///\param [in, out] stream - Поток для записи данных
	///\param [in] structData - Данные структуры
	///\param [in] index - Индекс
	///\param [in] isPtr - Является ли поле указателем
	///\param [in, out] counter - Счетчик сигналов
	///\param [in] prefix - Префикс обращения к структуре
	///\param [in, out] type - Тип сигналов
	///\return Если генерация выполнена успешно - true
	*/
	void genLibStructGetFunction(stream_type& stream, const InfoTable::struct_type::value_type& structData, const std::uint32_t index, const bool isPtr, std::uint32_t& counter, const std::string& prefix, const SIGNAL_TYPE type);
	/*
	///\brief - Метод для генерации библиотечной функции очистки памяти
	///\param [in, out] stream - Поток для записи данных
	///\return Если генерация выполнена успешно - true
	*/
	bool genLibSignalClearFunction(stream_type& stream);
	/*
	///\brief - Метод для генерации библиотечной функции шага алгоритма
	///\param [in, out] stream - Поток для записи данных
	///\return Если генерация выполнена успешно - true
	*/
	bool genLibOneStepFunctionFunction(stream_type& stream);
	/*
	///\brief - Метод для генерации функции сброса сигналов
	///\param [in, out] stream - Поток для записи данных
	///\param [in] functionName - Название ф-ии
	///\param [in] signalsToReset - Типы сбрасываемых сигналов
	///\return Если генерация выполнена успешно - true
	*/
	bool genLibSignalResetFunctionRaw(stream_type& stream, const std::string& functionName, const std::vector<PUML_SIGNAL_TYPE>& signalsToReset = {});

	std::string getSignals();

	/*
	///\brief - Метод для генерации функции сброса сигналов
	///\param [in, out] stream - Поток для записи данных
	///\return Если генерация выполнена успешно - true
	*/
	bool genLibSignalResetFunction(stream_type& stream);

public:
	/*
	///\brief - Метод сортирует ф-ии в порядке их использования
	///\param [in] functions - Данные о функциях
	///\return Отсортированный список функций
	*/
	std::vector<std::string> getSortedFunctionsKeys() const;
	~CodeGenerator() = default;
	CodeGenerator() = delete;
	/*
	///\brief - Метод создания генератора
	///\tparam - T Словарь языка для генерации
	///\tparam - U Генератор конструкций языка
	///\return - Экземпляр генератора
	*/
	template <typename T, typename U,
		typename X = typename  std::enable_if<std::is_base_of<TypeDictionary, T>::value>,
		typename Y = typename  std::enable_if<std::is_base_of<OneFileGenerator, U>::value>>
		static CodeGenerator* create();
	/*
	///\brief - Метод установки используемых сигналов
	///\param [in] newSignals - Новый список используемых сигналов
	*/
	void setSignals(const signal_type& newSignals);
	/*
	///\brief - Метод установки алгоритмов для генерации
	///\param [in] newAlgorithms - Новый список алгоритмов
	*/
	void setPuml(const algorithm_type& newAlgorithms);
	/*
	///\brief - Метод установки директории генерации
	///\param [in] newPath - Директория для генерации
	*/
	void setOutputPath(const std::string& newPath);
	/*
	///\brief - Метод установки директории генерации
	///\return Директория для генерации
	*/
	std::string getOutputPath() const;

	/*
	///\brief - Метод для генерации source-файла
	///\return Если генерация выполнена успешно - true
	*/
	bool createSourceFile();
	/*
	///\brief - Метод для генерации header-файла
	///\return Если генерация выполнена успешно - true
	*/
	bool createHeaderFile();
	/*
	///\brief - Метод для генерации lib-файлов
	///\return Если генерация выполнена успешно - true
	*/
	bool createLibFile();
	/*
	///\brief - Метод для получения расположения source-файла
	///\param [in] isExtension - Необходимо ли расширение файла
	///\return Расположение source-файла
	*/
	std::string getSourceFile(bool isExtension = true) const;
	/*
	///\brief - Метод для получения расположения header-файла
	///\param [in] isExtension - Необходимо ли расширение файла
	///\return Расположение header-файла
	*/
	std::string getHeaderFile(bool isExtension = true) const;
	/*
	///\brief - Метод для получения расположения библиотечного-файла
	///\param [in] isExtension - Необходимо ли расширение файла
	///\param [in] isHeader - true для заголовка, false - для source-файла
	///\return Расположение библиотечного-файла
	*/
	std::string getLibFile(bool isExtension = true, bool isHeader = true) const;
	/*
	///\brief - Метод для получения расположения генерируемых файлов
	///\return Расположение генерируемых файлов
	*/
	std::string getOutput() const;
	/*
	///\brief - Метод очистки данных
	*/
	void clear();
	/*
	///\brief - Метод для транскрипции названия алгоритма
	///\param [in] pumlName - Puml-название алгоритма
	///\return Имя алгоритма, используемое для генерации
	*/
	std::string nameConvert(const std::string &pumlName) const;

};

template<typename T>
inline bool CodeGenerator::genAccessFunction(stream_type& stream, const std::unordered_map<std::string, T>& signals, const FILE_TYPE_GEN type)
{
	bool isOk{ true };
	for (auto& signal : getSortedSignalsKeys(signals))
	{
		auto signalData = signalInfo->getSignalInfo(signal, AlgorithmInfo{});
		for (auto& algorithm : getAccessData(signal, signalData))
			isOk &= genFunctions(stream, algorithm,
				GEN_ACCESS_PREFIX + algorithm.outerParam.front()->name, type, true);
	}
	return isOk;
}

template<typename T>
inline std::vector<std::string> CodeGenerator::getSortedSignalsKeys(const std::unordered_map<std::string, T>& signals)
{
	std::vector<std::string> sortedData;
	sortedData.reserve(signals.size());
	for (auto& data : signals)
		sortedData.emplace_back(data.first);
	std::sort(sortedData.begin(), sortedData.end(), signalSortFunction);
	return sortedData;
}


template<typename T, typename U, typename X, typename Y>
inline CodeGenerator* CodeGenerator::create()
{
	return new CodeGenerator(new T, new U);
}
