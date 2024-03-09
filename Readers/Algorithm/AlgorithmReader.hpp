#pragma once

#include <unordered_map>
#include <regex>

#include "../../Common/Loggable.hpp"
#include "../../Struct/Algorithm/AlgorithmStruct.hpp"
#include "../../Struct/Signals/SignalStruct.hpp"

/*
///\brief - Класс обрабатывающий puml-данные алгоритма
*/
class AlgorithmReader : public Loggable
{
public:
	using algorithm_data_type = AlgorithmInfo;
	using store_algorithm_data_type = std::shared_ptr<algorithm_data_type>;
	using storage_type = std::shared_ptr<std::unordered_map<std::string, store_algorithm_data_type>>;
private:
	storage_type algorithmList;						///< - Считанные puml-алгоритмы
	store_algorithm_data_type currentAlgorithm;		///< - Данные считываемого алгоритма
	std::string algorithmName;						///< - Название считываемого алгоритма
	std::string currentRef;							///< - Название текущей ссылки алгоритма

	/*
	///\brief - Метод для проверки аргументов алгоритма
	///\param [in, out] signalStorage - Переменная для записи сигналов
	///\param [in] args - Аргументы
	*/
	void checkStructInArgs(InfoTableLight& signalStorage, const AlgorithmInfo::param_storage_type& args) const;
	/*
	///\brief - Метод для считывания сигналов из строки
	///\param [in, out] signalStorage - Переменная для записи сигналов
	///\param [in] line - Считываемая линия
	*/
	void checkSignalsInLine(InfoTableLight& signalStorage, const BaseAlgorithmCommand::string_type& line) const;
	/*
	///\brief - Метод для считывания строки с ключевым словом
	///\param [in] data - Строка содержащая данные
	///\param [in] command - Тип слова
	///\param [in] path - Путь до считываемого файла
	///\return Если считывание было успешно - true
	*/
	bool parseKeyWordLine(const std::string& data, const COMMAND_TYPE command, const std::string& path);
	/*
	///\brief - Метод для считывания буферов
	///\param [in, out] buffer - Буфер не относящийся к ключевым словам
	///\param [in, out] keywordBuffer - Буфер относящийся к ключевым словам
	///\param [in, out] line - Последняя считанная линия
	///\param [in, out] command - Последняя команда
	///\param [in] currentKeyWord - Текущее ключевое слово
	///\param [in] path - Путь до считываемого файла
	///\return Если считывание было успешно - true
	*/
	bool parseBuffers(std::string& buffer, std::string& keywordBuffer, std::string& line, COMMAND_TYPE& command,const std::size_t currentKeyWordIndex, const std::string& path);
	/*
	///\brief - Метод для считывания блока кода
	///\param [in] data - Строка содержащая данные блока
	///\param [in] path - Путь до считываемого файла
	///\return Если считывание было успешно - true
	*/
	bool parseKeyWords(const std::string& data, const std::string& path);
	/*
	///\brief - Метод проверяющий корректность аргументов вызываемой функции
	///\param [in] args - Строка содержащая аргументы
	///\param [in] path - Путь до считываемого файла
	///\return Если проверка была успешна - true
	*/
	bool checkFunctionArgs(const std::string& args) const;
	/*
	///\brief - Метод проверяющий корректность сигнатуры
	///\param [in] signature - Строка содержащая сигнатуру функции
	///\param [in] path - Путь до считываемого файла
	///\return Если проверка была успешна - true
	*/
	bool checkFunctionSignature(const std::string& signature, const std::string& path) const;
	/*
	///\brief - Метод для считывания вызова функций
	///\param [in, out] functionData - Строка содержащая данные вызова функции
	///\param [in] path - Путь до считываемого файла
	///\return Если считывание было успешно - true
	*/
	bool parseFunction(const std::string& functionData, const std::string& path);
	/*
	///\brief - Метод вычисляющий позицию следующего присваивания
	///\param [in] line - Строка содержащая вычисления
	///\param [in] offset - Отступ для начала поиска
	///\return Позиция следующего присваивания или std::string::npos
	*/
	std::string::size_type getAssignPos(const std::string& line, const std::string::size_type offset = 0) const;
	/*
	///\brief - Метод для считывания вычислений, содержащих внутренние комментарии
	///\param [in, out] calculationData - Строка содержащая данные для вычислений
	///\param [in] path - Путь до считываемого файла
	///\return Если считывание было успешно - true
	*/
	bool parseCalculationWithComment(const std::string& calculationData, const std::string& path);
	/*
	///\brief - Метод для считывания вычислений
	///\param [in, out] calculationData - Строка содержащая данные для вычислений
	///\param [in] path - Путь до считываемого файла
	///\return Если считывание было успешно - true
	*/
	bool parseCalculation(const std::string& calculationData, const std::string& path);
	/*
	///\brief - Метод проверяющий выходные аргументы алгоритма
	///\param [in] path - Путь до считываемого файла
	///\return Если проверка была успешно - true
	*/
	bool configureOutputArgs(const std::string& path);
	/*
	///\brief - Метод определяющий типа считываемых аргументов
	///\param [in] line - Строка с типом аргументов
	///\param [in, out] signalType - Тип считываемых сигналов
	///\param [in, out] possibleNames -  Допускаемые названия сигналов
	///\return Информация об аргументах
	*/
	algorithm_data_type::param_storage_type* defineArgsType(const std::string& line, std::string& signalType, std::vector<char>& possibleNames);
	/*
	///\brief - Метод считывающий regex-результат переменных
	///\param [in, out] matches - regex-результат
	///\return Информация об аргументах
	*/
	AlgorithmInfo::param_type parseRegexArg(std::smatch& matches) const;
	/*
	///\brief - Метод считывающий и проверяющий аргументы алгоритма
	///\param [in, out] stream - Stream I файла
	///\param [in, out] storage - Переменная для записи сигналов
	///\param [in] possibleNames - Допускаемые названия сигналов
	///\return Если считывание м проверка были успешно - true
	*/
	bool parseAlgorithmArgs(std::istream& stream, algorithm_data_type::param_storage_type& storage, const std::vector<char>& possibleNames);
	/*
	///\brief - Метод проверяющий является ли слово ключевым
	///\param [in] word - Проверяемое слово
	///\param [in, out] keyWord - Переменная для записи ключевого слова
	///\return Если слово является ключевым словом - true
	*/
	bool isWordKeyWord(const std::string& word, std::string* keyWord = nullptr) const;
	/*
	///\brief - Метод проверяющий является ли слово ключевым
	///\param [in] word - Проверяемое слово
	///\param [in, out] keyWordIndex - Переменная для записи индекса ключевого слова
	///\return Если слово является ключевым словом - true
	*/
	bool isWordKeyWord(const std::string& word, std::size_t* keyWordIndex = nullptr) const;
	/*
	///\brief - Метод для считывания комментария к ссылке
	///\param [in, out] stream - Stream I файла
	///\param [in] path - Путь до считываемого файла
	///\param [in] isInitNote - Является ли комментарий инициализирующим
	///\return Если считывание было успешно - true
	*/
	bool parseNote(std::istream& stream, const std::string& path, const bool isInitNote);
	/*
	///\brief - Метод для считывания ссылок
	///\param [in, out] stream - Stream I файла
	///\param [in, out] referenceData - Строка содержащая данные ссылки
	///\param [in] path - Путь до считываемого файла
	///\return Если считывание было успешно - true
	*/
	bool parseReference(std::istream& stream, std::string& referenceData, const std::string& path);
	/*
	///\brief - Метод вычисляющий применение дельты к элементам
	///\param [in, out] array - Массив элементов
	///\return true и дельта, при отсутствии ошибок
	*/
	std::pair<bool, std::int32_t> useDelta(std::vector<std::int32_t>& array) const;
	/*
	///\brief - Метод находящий разницу между begin и end
	///\param [in] begin - Начало блока
	///\param [in] end - Конец блока
	///\return Дельты между элементами begin и end, при успешном нахождении, иначе - пустой массив.
	*/
	std::vector<std::int32_t> getBlockExpandDelta(const std::vector<std::string>& begin, const std::vector<std::string>& end) const;
	/*
	///\brief - Метод разделяющий раскрываемые строки на элементы
	///\param [in] line - Разделяемая строка
	///\return Элементы строки line или пустой массив, если line - пустая
	*/
	std::vector<std::string> splitBlockData(std::string line) const;
	/*
	///\brief - Метод раскрывающий присваивания, между begin и end
	///\param [in] begin - Начало блока
	///\param [in] end - Конец блока
	///\param [in] line - Раскрываемая строка
	///\param [in] path - Путь до считываемого файла
	///\return Содержит true и раскрытый блок, если раскрытие было успешно. Иначе false и пустую строку
	*/
	std::pair<bool, std::string> textBlockExpand(const std::string& begin, const std::string& end, const std::string& line, const std::string& path) const;
	/*
	///\brief - Метод раскрывающий присваивания, использующие "..."
	///\param [in, out] line - Раскрываемая строка
	///\param [in] path - Путь до считываемого файла
	///\return Если раскрытие было успешно - true
	*/
	bool blockExpansion(std::string& line, const std::string& path) const;
	/*
	///\brief - Метод считывания блоков с ключевыми словами puml-алгоритма
	///\param [in, out] stream - Stream I файла
	///\param [in, out] line - Переменная для записи блока
	///\param [in] keyWord - Ключевое слово блока
	///\param [in] path - Путь до считываемого файла
	///\return Если считывание было успешно - true
	*/
	bool readKeyWordBlock(std::istream & stream, std::string & line, const std::string & keyWord, const std::string & path) const;
	/*
	///\brief - Метод проверяющий является ли строка - комментарием
	///\param [in, out] stream - Stream I файла
	///\param [in, out] line - Проверяемая строка
	///\return Если строка является комментарием - true
	*/
	bool isCommentLine(std::istream& stream, std::string& line) const;
	/*
	///\brief - Метод для первичного считывания блоков puml-алгоритма
	///\param [in, out] stream - Stream I файла
	///\param [in, out] line - Переменная для записи блока
	///\param[in] path - Путь до считываемого файла
	///\return Если считывание было успешно - true
	*/
	bool readPumlBlock(std::istream& stream, std::string& line, const std::string& path) const;
	/*
	///\brief - Метод для первичного считывания содержимого puml-алгоритма
	///\param [in, out] stream - Stream I файла
	///\param[in] path - Путь до считываемого файла
	///\return Если считывание было успешно - true
	*/
	bool parse(std::istream& stream, const std::string& path);
	
	
public:
	AlgorithmReader();
	~AlgorithmReader();
	/*
	///\brief - Метод очистки данных
	*/
	void clear();
	/*
	///\brief - Метод для первичного считывания puml-алгоритмов
	///\param[in] path - Путь до считываемого файла
	///\return Если считывание было успешно - true
	*/
	bool read(const std::string& path);

	/*
	///\brief - Метод для получения сигналов из puml-файлов
	///\return Если считывание было успешно - считанные сигналы, иначе nullptr
	*/
	std::shared_ptr<InfoTableLight> getSignals() const;
	/*
	///\brief - Метод для получения считанных алгоритмов
	///\return Считанные алгоритмы
	*/
	storage_type getAlgorithms();
};

