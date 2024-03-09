#pragma once
#include <string>
#include <vector>
/*
///\brief - Структура, хранящая информацию о генерации с отступом
*/
struct OffsetInfo
{
	std::string stringReturn;		///< - Сгенерированные данные
	std::int8_t offset;				///< - Отступ
	bool changeOffsetBeforeGen;		///< - Необходимо менять отступ до генерации
};
/*
///\brief - Генератор конструкций языков опирирующих source-файлом
*/
class OneFileGenerator
{
public:
	using return_type = std::string;
	using return_with_offset_type = OffsetInfo;
	using singature_build_args = std::vector<std::pair<std::string, std::string>>;
public:
	/*
	///\brief - Вспомогательный класс ответов на логические вопросы
	*/
	enum BOOL_ANSWER : bool
	{
		NO = false,	///< - Нет
		YES = true	///< - да
	};
public:
	OneFileGenerator() = default;
	virtual ~OneFileGenerator() = default;
	/*
	///\brief - Метод возвращающий ответ на вопрос "Должен ли угадываться тип данных для встроенных ф-й(sqrt/exp/pow/abs)?"
	///\return Если да - YES, иначе - NO
	*/
	virtual BOOL_ANSWER shoudGuessInlineFunction() const = 0;
	/*
	///\brief - Метод возвращающий нвзвание ф-ии, принимаемой за основную
	///\return Название основной функции
	*/
	virtual return_type mainFunctionName() const = 0;
	/*
	///\brief - Метод генерирующий сигнатуру в source-файле
	///\param [in] returnType - Возвращаемый тип ф-ии
	///\param [in] functionName - Название ф-ии
	///\param [in] args - Аргументы ф-ии
	///\return Сгенерированная сигнатура
	*/
	virtual return_type genSignature(const std::string& returnType, const std::string& functionName, singature_build_args args) const = 0;
	/*
	///\brief - Метод генерирующий сигнатуру основной функции в source-файле
	///\param [in] returnType - Возвращаемый тип ф-ии
	///\param [in] functionName - Название ф-ии
	///\param [in] args - Аргументы ф-ии
	///\return Сгенерированная сигнатура основной функции
	*/
	virtual return_type genSignatureMain(const std::string& returnType, const std::string& functionName, singature_build_args args) const = 0;
	/*
	///\brief - Метод генерирующий поле структуры
	///\param [in] type - Тип поля
	///\param [in] fieldName - Название поля
	///\param [in] defaultValue - Значение поля by default
	///\return Сгенерированное поле структуры
	*/
	virtual return_type genField(const std::string& type, const std::string& fieldName, std::string defaultValue = {}) const = 0;
	/*
	///\brief - Метод генерирующий глобальную переменную
	///\param [in] type - Тип переменной
	///\param [in] fieldName - Название переменной
	///\param [in] defaultValue - Значение переменной by default
	///\return Сгенерированное объявление глобальной структуры
	*/
	virtual return_type genGlobalVariable(const std::string& type, const std::string& fieldName, std::string defaultValue) const = 0;
	/*
	///\brief - Метод генерирующий локальную переменную
	///\param [in] type - Тип переменной
	///\param [in] fieldName - Название переменной
	///\param [in] defaultValue - Значение переменной by default
	///\return Сгенерированное объявление локальной структуры
	*/
	virtual return_type genLocalVariable(const std::string& type, const std::string& fieldName, std::string defaultValue) const = 0;
	/*
	///\brief - Метод генерирующий константы
	///\param [in] fieldName - Название константы
	///\param [in] value - Значение константы
	///\return Сгенерированное объявление константы
	*/
	virtual return_type genConst(const std::string& name, const std::string& value) const = 0;
	/*
	///\brief - Метод объявляющий структуру
	///\param [in] structName - Название структуры
	///\return Сгенерированное начало объявления структуры
	*/
	virtual return_with_offset_type startGenStruct(const std::string& structName) const = 0;
	/*
	///\brief - Метод оканчивающий объявление структуры
	///\param [in] structName - Название структуры
	///\return Сгенерированное окончание объявления структуры
	*/
	virtual return_with_offset_type endGenStruct(const std::string& structName) const = 0;
	/*
	///\brief - Метод получения добавочной информации в начало source-файла
	///\return Добавочная информация
	*/
	virtual return_with_offset_type beforeGen() const = 0;
	/*
	///\brief - Метод получения добавочной информации в конец source-файла
	///\return Добавочная информация
	*/
	virtual return_with_offset_type afterGen() const = 0;
	/*
	///\brief - Метод возвращающий начало блока функции
	///\return Начало блока функции
	*/
	virtual return_with_offset_type startFunction() const = 0;
	/*
	///\brief - Метод возвращающий конец блока функции
	///\return Конец блока функции
	*/
	virtual return_with_offset_type endFunction() const = 0;
	/*
	///\brief - Метод генерирующий блок else if
	///\param [in] body - Логическое тело блока
	///\return Блок else if
	*/
	virtual return_type genIfElseBody(const std::string& body) const = 0;
	/*
	///\brief - Метод генерирующий блок if
	///\param [in] body - Логическое тело блока
	///\return Блок if
	*/
	virtual return_type genIfBody(const std::string& body) const = 0;
	/*
	///\brief - Метод возвращающий начало if-блока
	///\return Начало if-блока
	*/
	virtual return_with_offset_type startIf() const = 0;
	/*
	///\brief - Метод возвращающий конец if-блока
	///\return Конец if-блока
	*/
	virtual return_with_offset_type endIf() const = 0;
	/*
	///\brief - Метод генерирующий блок switch
	///\param [in] body - Логическое тело блока
	///\return Блок switch
	*/
	virtual return_type genSwitchBody(const std::string& body) const = 0;
	/*
	///\brief - Метод возвращающий начало switch-блока
	///\return Начало switch-блока
	*/
	virtual return_with_offset_type startSwitch() const = 0;
	/*
	///\brief - Метод возвращающий конец switch-блока
	///\return Конец switch-блока
	*/
	virtual return_with_offset_type endSwitch() const = 0;
	/*
	///\brief - Метод генерирующий блок case
	///\param [in] body - Тело блока
	///\return Блок case
	*/
	virtual return_type genCaseBody(const std::string& body) const = 0;
	/*
	///\brief - Метод возвращающий начало case-блока
	///\return Начало case-блока
	*/
	virtual return_with_offset_type startCase() const = 0;
	/*
	///\brief - Метод возвращающий конец case-блока
	///\return Конец case-блока
	*/
	virtual return_with_offset_type endCase() const = 0;
	/*
	///\brief - Метод генерирующий блок while
	///\param [in] body - Логическое тело блока
	///\return Блок while
	*/
	virtual return_type genWhileBody(const std::string& body) const = 0;
	/*
	///\brief - Метод возвращающий начало while-блока
	///\return Начало while-блока
	*/
	virtual return_with_offset_type startWhile() const = 0;
	/*
	///\brief - Метод возвращающий конец while-блока
	///\return Конец while-блока
	*/
	virtual return_with_offset_type endWhile() const = 0;
	/*
	///\brief - Метод генерирующий блок else
	///\return Блок else
	*/
	virtual return_type genElse() const = 0;
	/*
	///\brief - Метод возвращающий начало else-блока
	///\return Начало else-блока
	*/
	virtual return_with_offset_type startElse() const = 0;
	/*
	///\brief - Метод возвращающий конец else-блока
	///\return Конец else-блока
	*/
	virtual return_with_offset_type endElse() const = 0;
	/*
	///\brief - Метод генерирующий конструкцию return
	///\param [in] body - Возвращаемые данные
	///\return Конструкция return
	*/
	virtual return_type genReturnBody(const std::string& body) const = 0;
	/*
	///\brief - Метод генерирующий вызов функции pow для float-значений
	///\param [in] body - Данные, возводимые в степень
	///\param [in] pow - Степень
	///\return Вызов ф-ии pow для float-значений 
	*/
	virtual return_type genPowf(const std::string& body, const std::string& pow) const = 0;
	/*
	///\brief - Метод генерирующий вызов функции pow для целочисленных-значений
	///\param [in] body - Данные, возводимые в степень
	///\param [in] pow - Степень
	///\return Вызов ф-ии pow для целочисленных-значений
	*/
	virtual return_type genPowi(const std::string& body, const std::string& pow) const = 0;
	/*
	///\brief - Метод генерирующий вызов универсальной функции pow
	///\param [in] body - Данные, возводимые в степень
	///\param [in] pow - Степень
	///\return Вызов универсальной ф-ии pow
	*/
	virtual return_type genPow(const std::string& body, const std::string& pow) const = 0;
	/*
	///\brief - Метод генерирующий вызов функции sqrt для float-значений
	///\param [in] body - Данные для извлечения корня
	///\return Вызов ф-ии sqrt для float-значений
	*/
	virtual return_type genSqrtf(const std::string& body) const = 0;
	/*
	///\brief - Метод генерирующий вызов функции sqrt для целочисленных-значений
	///\param [in] body - Данные для извлечения корня
	///\return Вызов ф-ии sqrt для целочисленных-значений
	*/
	virtual return_type genSqrti(const std::string& body) const = 0;
	/*
	///\brief - Метод генерирующий вызов универсальной функции sqrt
	///\param [in] body - Данные для извлечения корня
	///\return Вызов универсальной ф-ии sqrt
	*/
	virtual return_type genSqrt(const std::string& body) const = 0;
	/*
	///\brief - Метод генерирующий вызов функции abs для float-значений
	///\param [in] body - Данные для взятия модуля
	///\return Вызов ф-ии abs для float-значений
	*/
	virtual return_type genAbsf(const std::string& body) const = 0;
	/*
	///\brief - Метод генерирующий вызов функции abs для целочисленных-значений
	///\param [in] body - Данные для взятия модуля
	///\return Вызов ф-ии abs для целочисленных-значений
	*/
	virtual return_type genAbsi(const std::string& body) const = 0;
	/*
	///\brief - Метод генерирующий вызов универсальной функции abs
	///\param [in] body - Данные для взятия модуля
	///\return Вызов универсальной ф-ии abs
	*/
	virtual return_type genAbs(const std::string& body) const = 0;
	/*
	///\brief - Метод генерирующий вызов функции exp для float-значений
	///\param [in] body - Степень экспоненты
	///\return Вызов ф-ии exp для float-значений
	*/
	virtual return_type genExpf(const std::string& body) const = 0;
	/*
	///\brief - Метод генерирующий вызов функции exp для целочисленных-значений
	///\param [in] body - Степень экспоненты
	///\return Вызов ф-ии exp для целочисленных-значений
	*/
	virtual return_type genExpi(const std::string& body) const = 0;
	/*
	///\brief - Метод генерирующий вызов универсальной функции exp
	///\param [in] body - Степень экспоненты
	///\return Вызов универсальной ф-ии exp
	*/
	virtual return_type genExp(const std::string& body) const = 0;
	/*
	///\brief - Метод генерирующий вызов функции
	///\param [in] output - Переменная для записи результата функции
	///\param [in] functionName - Название вызываемой функции
	///\param [in] args - Аргументы
	///\return Сгенерированный вызов функции
	*/
	virtual return_type genFunctionCall(const std::string& output, const std::string& functionName, const std::vector<std::string>& args) const = 0;
	/*
	///\brief - Метод возвращающий дополнительные данные встраиваемые перед генерацией глобальных переменных source-файла
	///\return Дополнительные данные
	*/
	virtual return_with_offset_type beforeGenGlobalVar() const = 0;
	/*
	///\brief - Метод возвращающий дополнительные данные встраиваемые после генерации глобальных переменных source-файла
	///\return Дополнительные данные
	*/
	virtual return_with_offset_type afterGenGlobalVar() const = 0;
	/*
	///\brief - Метод возвращающий дополнительные данные встраиваемые перед генерацией сигнатуры в source-файле
	///\return Дополнительные данные
	*/
	virtual return_with_offset_type beforeGenSignature() const = 0;
	/*
	///\brief - Метод возвращающий дополнительные данныем встраиваемые после генерацией сигнатуры в source-файле
	///\return Дополнительные данные
	*/
	virtual return_with_offset_type afterGenSignature() const = 0;
	/*
	///\brief - Метод возвращающий расширение source-файла
	///\return расширение source-файла
	*/
	virtual return_type sourceFileExtension() const = 0;
};
/*
///\brief - Генератор конструкций языков оперирующих source и header файлами
*/
class TwoFileGenerator : public OneFileGenerator
{
public:
	TwoFileGenerator() = default;
	virtual ~TwoFileGenerator() = default;

	/*
	///\brief - Метод возвращающий ответ на вопрос "Нужно ли генерировать глобальные переменные в header`е?"
	///\return Если да - YES, иначе - NO
	*/
	virtual BOOL_ANSWER needGenGlobalVarInHeader() const = 0;
	/*
	///\brief - Метод возвращающий дополнительные данные встраиваемые перед генерацией глобальных переменных header-файла
	///\return Дополнительные данные
	*/
	virtual return_with_offset_type beforeGenGlobalVarHeader() const = 0;
	/*
	///\brief - Метод возвращающий дополнительные данные встраиваемые после генерации глобальных переменных header-файла
	///\return Дополнительные данные
	*/
	virtual return_with_offset_type afterGenGlobalVarHeader() const = 0;
	/*
	///\brief - Метод возвращающий дополнительные данные встраиваемые перед генерацией сигнатуры в header-файле
	///\return Дополнительные данные
	*/
	virtual return_with_offset_type beforeGenSignatureHeader() const = 0;
	/*
	///\brief - Метод возвращающий дополнительные данные встраиваемые после генерацией сигнатуры в header-файле
	///\return Дополнительные данные
	*/
	virtual return_with_offset_type afterGenSignatureHeader() const = 0;
	/*
	///\brief - Метод генерирующий сигнатуру в header-файле
	///\param [in] returnType - Возвращаемый тип ф-ии
	///\param [in] functionName - Название ф-ии
	///\param [in] args - Аргументы ф-ии
	///\return Сгенерированная сигнатура
	*/
	virtual return_type genSignatureHeader(const std::string& returnType, const std::string& functionName, singature_build_args args) const = 0;
	/*
	///\brief - Метод генерирующий сигнатуру основной функции в header-файле
	///\param [in] returnType - Возвращаемый тип ф-ии
	///\param [in] functionName - Название ф-ии
	///\param [in] args - Аргументы ф-ии
	///\return Сгенерированная сигнатура основной функции
	*/
	virtual return_type genMainSignatureHeader(const std::string& returnType, const std::string& functionName, singature_build_args args) const = 0;
	/*
	///\brief - Метод получения добавочной информации в начало header-файла
	///\return Добавочная информация
	*/
	virtual return_with_offset_type beforeGenHeader() const = 0;
	/*
	///\brief - Метод получения добавочной информации в конец header-файла
	///\return Добавочная информация
	*/
	virtual return_with_offset_type afterGenHeader() const = 0;
	/*
	///\brief - Метод генерирующий подключение заголовочного файла
	///\param [in] file - Имя файла
	///\return Сгенерированное подключение
	*/
	virtual return_type genInclude(const std::string& file) const = 0;
	/*
	///\brief - Метод возвращающий расширение header-файла
	///\return расширение header-файла
	*/
	virtual return_type headerFileExtension() const = 0;
	
};
/*
///\brief - Генератор конструкций языков допускающих взаимодействие с C#
*/
class LibGenerator : public TwoFileGenerator
{
public:
	LibGenerator() = default;
	virtual ~LibGenerator() = default;

	/*
	///\brief - Метод генерирующий сигнатуру функций в source-файлах библиотеки
	///\param [in] returnType - Возвращаемый тип ф-ии
	///\param [in] functionName - Название ф-ии
	///\param [in] args - Аргументы ф-ии
	///\return Сгенерированная сигнатура функции
	*/
	virtual return_type genSignatureLib(const std::string& returnType, const std::string& functionName, singature_build_args args) const = 0;
	/*
	///\brief - Метод генерирующий сигнатуру функций в header-файлах библиотеки
	///\param [in] returnType - Возвращаемый тип ф-ии
	///\param [in] functionName - Название ф-ии
	///\param [in] args - Аргументы ф-ии
	///\return Сгенерированная сигнатура функции
	*/
	virtual return_type genHeaderSignatureLib(const std::string& returnType, const std::string& functionName, singature_build_args args) const = 0;
	/*
	///\brief - Метод получения добавочной информации в начало header-файла библиотеки
	///\return Добавочная информация
	*/
	virtual return_with_offset_type beforeGenLibHeader() const = 0;
	/*
	///\brief - Метод получения добавочной информации в конец header-файла библиотеки
	///\return Добавочная информация
	*/
	virtual return_with_offset_type afterGenLibHeader() const = 0;
	/*
	///\brief - Метод получения добавочной информации в начало header-файла библиотеки
	///\return Добавочная информация
	*/
	virtual return_with_offset_type beforeGenLib() const = 0;
	/*
	///\brief - Метод получения добавочной информации в конец header-файла библиотеки
	///\return Добавочная информация
	*/
	virtual return_with_offset_type afterGenLib() const = 0;
	/*
	///\brief - Метод генерирующий выделение место в Heap для массива
	///\param [in] type - Тип элементов массива
	///\param [in] varName - Название переменной для выделения
	///\param [in] elementCount - Кол-во элементов массива
	///\return Сгенерированное выделение памяти
	*/
	virtual return_type makeArrayHeap(const std::string& type, const std::string& varName, const std::uint32_t& elementCount) const = 0;
	/*
	///\brief - Метод генерирующий копирующий данные строки в Heap
	///\param [in] string - Строка, содержащая данные
	///\param [in] charPtr - Название указателя для копирования
	///\return Сгенерированное копирование строки
	*/
	virtual return_type makeString2HeapPtr(const std::string& string, const std::string& charPtr) const = 0;
	/*
	///\brief - Метод генерирующий очистку Heap
	///\param [in] ptr - Название указателя для очистки
	///\return Сгенерированная очистка памяти
	*/
	virtual return_type deleteHeapPtr(const std::string& ptr) const = 0;
	/*
	///\brief - Метод возвращающий дополнительные данные встраиваемые перед генерацией сигнатуры в библиотечных-файлах
	///\return Дополнительные данные
	*/
	virtual return_with_offset_type beforeGenSignatureLib() const = 0;
	/*
	///\brief - Метод возвращающий дополнительные данныем встраиваемые после генерацией сигнатуры в библиотечных-файлах
	///\return Дополнительные данные
	*/
	virtual return_with_offset_type afterGenSignatureLib() const = 0;
	/*
	///\brief - Метод возвращающий ответ на вопрос "Необходимо ли генерировать функции доступа?"
	///\return Если да - YES, иначе - NO
	*/
	virtual BOOL_ANSWER needGenAccessFunction() const = 0;

	virtual return_type stringCompare(const std::string& lString, const std::string& rString) const = 0;
};
/*
///\brief - Тип генератора
*/
enum GENERATOR_TYPE : std::uint8_t
{
	UNKNOWN,		///< - Неизвестный генератор
	ONE_FILE,		///< - Source генератор
	TWO_FILE,		///< - Header генератор
	LIBRARY			///< - Lib генератор
};