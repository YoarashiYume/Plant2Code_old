/**
\file CSVReader.hpp
\brief Содержит функцию разбора строки CSV-формата
 */
#pragma once
#include <vector>
#include <sstream>
#include <string>

namespace Readers
{
	/**
	\brief Функиця преобразующая строку CSV-файла в массив данных
	\param text исходная строка
	\param splitter разделитель, используемый в CSV
	\param saveEmpty флаг необходимости сохранения пустых полей
	\return список полей
	*/
	template<typename Tchar>
	std::vector<std::basic_string<Tchar>> CSVLineReader(const std::basic_string<Tchar>& text, const Tchar splitter, bool saveEmpty = false)
	{
		std::basic_stringstream<Tchar> ss{ text.data() };
		std::basic_string<Tchar> string;
		std::vector<std::basic_string<Tchar>> vec;
		while (std::getline(ss, string, splitter))
			if (saveEmpty)
				vec.emplace_back(string);
			else if (string.empty() == false)
				vec.emplace_back(string);
		return vec;
	}
}