#pragma once
#include <string>
/*
///\brief  - Словарь для генерации кода из puml
*/
class TypeDictionary
{
public:
	using return_type = std::string;
	/*
	///\brief Функция преобразующая puml-тип в тип использующийся в коде
	///\param[in] pumlType - puml-тип
	///\return тип использующийся в коде
	*/
	virtual return_type getType(const std::string & pumlType) const = 0;
	/*
	///\brief Функция возвращающая тип для puml-таймера
	 ///\return тип для puml-таймера
	*/
	virtual return_type getTimerType() const = 0;
	/*
	///\brief Функция возвращающая puml-тип для puml-таймера
	///\return тип для puml-таймера
	*/
	virtual return_type getPumlTimerType() const = 0;
	/*
	///\brief Функция возвращающая тип для char (Не заменяет типы в getType())
	 ///\return тип char
	*/
	virtual return_type getCharType() const = 0;
	/*
	///\brief Функция возвращающая тип для void (Не заменяет типы в getType())
	 ///\return тип void
	*/
	virtual return_type getVoidType() const = 0;
	/*
	///\brief Функция возвращающая тип для bool (Не заменяет типы в getType())
	///\return тип bool
	*/
	virtual return_type getBoolType() const = 0;
	/*
	///\brief Функция возвращающая тип для размеров контейнеров (Не заменяет типы в getType())
	///\return тип тип для размеров контейнеров
	*/
	virtual return_type getSizeType() const = 0;
	/*
	///\brief Функция преобразующая puml-тип в тип использующийся в коде
	///\param[in] pumlType - puml-тип
	///\param[in] elementCount - количество элементов в массиве (Если тип не является массивов elementCount = 1)
	///\return тип использующийся в коде
	*/
	virtual return_type getType(const std::string & pumlType, const std::uint32_t & elementCount) const = 0;
	/*
	///\brief Функция возвращающая начальное значения для типа
	///\param[in] type - тип
	///\param[in] isPtr - является ли тип указателем
	///\param[in] elementCount - количество элементов в массиве (Если тип не является массивов elementCount = 1)
	///\return начальное значения для типа
	*/
	virtual return_type getDefaultValue(const std::string & type, const bool isPtr, const std::uint32_t & elementCount) const = 0;
	/*
	///\brief Функция возвращающая символы для однострочного комментария
	 ///\return символы для однострочного комментария
	*/
	virtual return_type getCommentSymbol() const = 0;
	/*
	///\brief Функция возвращающая символы для однострочного Doxygen-комментария
	 ///\return символы для однострочного Doxygen-комментария
	*/
	virtual return_type getDoxygenCommentSymbol() const = 0;
	/*
	///\brief Функция возвращающая символы для Doxygen-комментария поля структуры/переменной
	 ///\return символы для Doxygen-комментария поля структуры/переменной
	*/
	virtual return_type getDoxygenCommentFieldSymbol() const = 0;
	/*
	///\brief Функция преобразующая тип в указатель на тип
	///\param[in] type - puml-тип
	///\param[in] elementCount - количество элементов в массиве (Если тип не является массивов elementCount = 1)
	///\return указатель на тип
	*/
	virtual return_type makePtrable(const std::string & type, const std::uint32_t & elementCount = 1) const = 0;
	/*
	///\brief Функция разыменовывающая указатель
	///\param[in] value - значение для разыменования
	///\param[in] index - индекс массива, к которому происходит обращение (Если тип не является массивов index - пустая строка)
	///\return разыменовыванный указатель
	*/
	virtual return_type makeUnPtrable(const std::string& value, const std::string& index = std::string{}) const = 0;
	/*
	///\brief Функция преобразующая тип в ссылочный тип
	///\param[in] type - тип
	///\return ссылочный тип
	*/
	virtual return_type makeRefArg(const std::string & type) const = 0;
	/*
	///\brief Функция возвращающая ссылку на значение
	///\param[in] value - значение
	///\return ссылка на значение
	*/
	virtual return_type makeRef(const std::string& value) const = 0;
	/*
	///\brief Функция возвращающая puml-символ в англ. символ
	///\param[in] ch - символ для транскрипции
	///\return эквивалентный символ
	*/
	virtual return_type getTranscription(const char ch) const = 0;
	/*
	///\brief Функция символ конца строки
	///\return символ конца строки
	*/
	virtual return_type getEndLine() const = 0;
	/*
	///\brief Функция символ обращения к полю структуры
	///\return символ обращения к полю структуры
	*/
	virtual return_type getReferencingInStruct() const = 0;


	virtual ~TypeDictionary() = default;
};
