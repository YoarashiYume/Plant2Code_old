#pragma once
#include "../Wrappers/Wrapper.hpp"
#include <vector>
#include "../filesForC/DictionaryC.hpp"
#include "../filesForC/LibGeneratorC.hpp"

using wrapper_type = Wrapper;
#define EXPORT extern __declspec(dllexport)
#define EXPORT_BOOL EXPORT bool __stdcall
#define EXPORT_VOID EXPORT void __stdcall
#define EXPORT_CHAR_PTR EXPORT char* __stdcall


/*
Порядок выполнения функций для генерации (без учета создания/удаления/ошибок при работе и использования force-вариаций функций)
1. congigureLog
2. readSignalFolder
3. readPUMLFolder
4. compareSignals
5. сheckPumlData (При возврате false повторное использование и дальнейшая генерация - ub (Изменяет данные). 
	Используется ф-я clear и переход ко 2`му шагу.)
6. buildCode
7. buildLibrary

//setOutputLocation - должна быть выполнена до шага 6
*/

#ifdef __cplusplus
extern "C" {
#endif
	///\brief Функция возвращающая объект Wrapper, занимающийся генерацией кода
	///\return Указатель на объект Wrapper
	EXPORT wrapper_type* __stdcall createWrapper();
	///\brief Функция удаляющая объект Wrapper
	///\param[in,out] ptr - Указатель на объект Wrapper
	EXPORT_VOID destroyWrapper(wrapper_type* ptr);
	///\brief Функция устанавливающая вывод сообщений от Wrapper`а
	///\param[in,out] ptr - Указатель на объект Wrapper
	///\param[in,out] logFunction - Функция-логгер
	EXPORT_VOID configureLog(wrapper_type* ptr, Loggable::log_type logFunction);
	///\brief Функция устанавливающая директорию для генерации файлов
	///\param[in,out] ptr - Указатель на объект Wrapper
	///\param[in] path - Путь до директории
	EXPORT_VOID setOutputLocation(wrapper_type* ptr, char* path);
	///\brief Функция считывающая сигналы CSV-таблиц
	///\param[in,out] ptr - Указатель на объект Wrapper
	///\param[in] path - Файлы, содержащие сигналы
	///\param[in] elementCount - Кол-во файлов
	///\return Возвращает true, если в считывании puml-алгоритмов не было ошибки
	EXPORT_BOOL readSignalFiles(wrapper_type* ptr, char** paths, std::uint32_t elementCount);
	///\brief Функция считывающая Puml-файлы
	///\param[in,out] ptr - Указатель на объект Wrapper
	///\param[in] path - Файлы, содержащие puml-алгоритмы
	///\param[in] elementCount - Кол-во файлов
	///\return Возвращает true, если в считывании сигналов не было ошибки
	EXPORT_BOOL readPumlFiles(wrapper_type* ptr, char** paths, std::uint32_t elementCount);
	///\brief Функция сверяющая считанные сигналы из CSV-таблиц с сигналами, собранными из puml-файлов (проверка происходит, если предыдущие этапы выполнены успешно)
	///\param[in,out] ptr - Указатель на объект Wrapper
	///\return Возвращает true, если в считывании сигналов не было ошибки
	EXPORT_BOOL compareSignals(wrapper_type* ptr);
	///\brief Функция сверяющая считанные сигналы из CSV-таблиц с сигналами, собранными из puml-файлов
	///\param[in,out] ptr - Указатель на объект Wrapper
	///\return Возвращает true, если в считывании сигналов не было ошибки
	EXPORT_BOOL forceToCompareSignals(wrapper_type* ptr);
	///\brief Функция очищающая данные
	///\param[in,out] ptr - Указатель на объект Wrapper
	EXPORT_VOID clear(wrapper_type* ptr);
	///\brief Функция парсинга и проверки puml-алгоритмов (происходит, если предыдущие этапы выполнены успешно)
	///\param[in,out] ptr - Указатель на объект Wrapper
	///\return Возвращает true, если при проверке/парсинге puml-алгоритмов
	EXPORT_BOOL checkPumlData(wrapper_type* ptr);
	///\brief Функция парсинга и проверки puml-алгоритмов
	///\param[in,out] ptr - Указатель на объект Wrapper
	///\return Возвращает true, если при проверке/парсинге puml-алгоритмов
	EXPORT_BOOL forceToCheckPumlData(wrapper_type* ptr);
	///\brief Функция генерации кода библиотеки (происходит, если предыдущие этапы выполнены успешно). Произведет попытку генерации файлов SourceInfo.c (при использовании класса однофайловой генерации и выше), LibInfo.h (при использовании класса двухфайловой генерации и выше), LibInfo.c (при использовании класса двухфайловой генерации и выше) и HeaderInfo.h (при использовании класса библиотечной генерации)
	///\param[in,out] ptr - Указатель на объект Wrapper 
	///\return Возвращает true, если генерация кода выполнена успешно
	EXPORT_BOOL buildCode(wrapper_type* ptr);
	///\brief Функция генерации кода библиотеки (происходит, если предыдущие этапы выполнены успешно). Произведет попытку генерации файлов SourceInfo.c (при использовании класса однофайловой генерации и выше), LibInfo.h (при использовании класса двухфайловой генерации и выше), LibInfo.c (при использовании класса двухфайловой генерации и выше) и HeaderInfo.h (при использовании класса библиотечной генерации)
	///\param[in,out] ptr - Указатель на объект Wrapper
	///\return Возвращает true, если генерация кода выполнена успешно
	EXPORT_BOOL forceToBuildCode(wrapper_type* ptr);
	///\brief Функция генерации dll-библиотеки (происходит, если предыдущие этапы выполнены успешно). Произведет попытку генерации файла outputLib.dll
	///\param[in,out] ptr - Указатель на объект Wrapper
	///\return Возвращает true, если генерация библиотеки
	EXPORT_BOOL buildLibrary(wrapper_type* ptr);
	///\brief Функция генерации dll-библиотеки. Произведет попытку генерации файла outputLib.dll
	///\param[in,out] ptr - Указатель на объект Wrapper
	///\return Возвращает true, если генерация библиотеки
	EXPORT_BOOL forceToBuildLibrary(wrapper_type* ptr);

#ifdef EXTENDED
	struct LatexPersonDataC
	{
		char* name;
		char* position;

	};
	struct LatexPersonDataExpandedC 
	{
		char* name;
		char* position;
		char* workPlace;
	};


	struct LatexTitleDataC
	{
		char* fname;
		char* fposition;
		std::uint32_t firstLineCount;

		char* sname;
		char* sposition;
		std::uint32_t secondLineCount;
		LatexPersonDataExpandedC military;
		LatexPersonDataExpandedC approve;
	};
	struct LatexDataMainC
	{
		char* documentName;
		char* fullProgramName;
		char* documentNumber;

		std::uint16_t documentVersion;
		std::uint16_t objectCode;

		char* programShort;

		bool needMilitary;

		LatexTitleDataC title;
	};
	struct LatexTableDataC
	{
		char* name;
		char* path;
		char* ref;
		bool isLandscape;
		char* sizes;	//split with \n
	};
	struct UsedAlgorithmC
	{
		char* name;
		char* changedName;
		char* extraData;
	};

	struct LatexAlgorithmDataC
	{
		char* name;
		char* changedName;
		char* extraData;

		char* sizeKoef;
		std::uint8_t* isLandscape;
		char* pdfSize;
		std::uint32_t elementCount;
		UsedAlgorithmC* used;
		std::uint32_t usedCount;
	};
	struct LatexSectoinDataC
	{
		char* name;
		char* changedName;
		char* extraData;

		LatexAlgorithmDataC* sections;
		std::uint32_t sectionsCount;
	};
	EXPORT_CHAR_PTR getLib(wrapper_type* ptr);
	EXPORT_VOID clearString(char* string);
	EXPORT_VOID clearUint8_t(std::uint8_t* arr);
	EXPORT_CHAR_PTR genUE(wrapper_type* ptr, LatexDataMainC& data);
	EXPORT_CHAR_PTR genLatexTables(wrapper_type* ptr, LatexTableDataC* data, std::uint32_t dataCount);
	EXPORT_CHAR_PTR genLatexSections(wrapper_type* ptr, LatexSectoinDataC* sections, std::uint32_t sectionCount);
	EXPORT_CHAR_PTR getTablesOutputPath(wrapper_type* ptr);
	EXPORT LatexAlgorithmDataC* __stdcall getLatexAlgorithmData(wrapper_type* ptr, std::uint32_t& count);
#endif // EXTENDED


#ifdef __cplusplus
}
#endif 