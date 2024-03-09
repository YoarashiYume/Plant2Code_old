#pragma once
#include <string>
/*
///\brief - Базовый допускающий логирование
*/
class Loggable
{
public:
	using log_type = void(*)(const char*);
private:
	static log_type logFunction;			///< - Указатель на функцию для вывода сообщений
protected:
	/*
	///\brief - Метод логирования
	///\param[in] message - Сообщение передающееся в лог
	*/
	void log(const std::string& message) const
	{
		if (logFunction)
			logFunction(message.data());
	}
public:
	/*
	///\brief - Метод для установки функции-логирования
	///\param[in] newLog - Новая ф-я логирования
	*/
	static void setLog(log_type newLog)
	{
		logFunction = newLog;
	}
};