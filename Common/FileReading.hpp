#pragma once
#include <fstream>
#include <memory>


namespace FileReading
{
	struct OffsetInfo
	{
		std::string stringReturn;		///< - Сгенерированные данные
		std::int8_t offset;				///< - Отступ
		bool changeOffsetBeforeGen;		///< - Необходимо менять отступ до генерации
	};
	using unique_ifstream_type = std::unique_ptr<std::ifstream, void(*)(std::ifstream* ptr)>;
	/*
	///\brief - Deleter для ifstream
	///\param[in, out] ptr - Указатель на ifstream
	*/
	inline void closeFunction(std::ifstream* ptr)
	{
		if (ptr)
		{
			ptr->close();
			delete ptr;
		}
	}

	using unique_ofstream_type = std::unique_ptr<std::ofstream, void(*)(std::ofstream* ptr)>;
	/*
	///\brief - Deleter для ofstream
	///\param[in, out] ptr - Указатель на ofstream
	*/
	inline void closeFunction(std::ofstream* ptr)
	{
		if (ptr)
		{
			ptr->close();
			delete ptr;
		}
	}
	class FileWriter
	{
	private:
		using stream_type = FileReading::unique_ofstream_type;
		using writer_type = stream_type::element_type;
		stream_type stream;
		std::string offsetInFile;						///< - Отступ от начала строки при генерации
		std::string currentPath;
		static void deleter(std::ofstream* ptr)
		{
			closeFunction(ptr);
		}
	public:
		FileWriter(const std::string& path)
			: currentPath(path), stream(new writer_type{path}, deleter)
		{

		}
		std::string path() const
		{
			return currentPath;
		}
		/*
		///\brief - Метод для изменения отступа в файле
		///\param [in] offsetChange - Изменения отступа
		*/
		bool changeOffset(const std::int32_t offsetChange)
		{
			if (offsetChange == 0)
				return true;
			if (offsetInFile.size() == 1 && offsetChange == -1)
				offsetInFile.clear();
			else
			{
				if (offsetChange < 0 && offsetInFile.size() < static_cast<decltype(offsetInFile)::size_type>(abs(offsetChange)))
				{
					return false;
					offsetInFile.clear();
				}
				if (offsetChange > 0)
					offsetInFile.resize(offsetInFile.size() + offsetChange, '\t');
				else if (offsetChange < 0)
					offsetInFile.erase(offsetInFile.end() + offsetChange, offsetInFile.end());
			}
			return true;
		}
		/*
		///\brief - Метод для генерации пользовательской информации с отступом
		///\param [in, out] stream - Поток для записи данных
		///\param [in] dataForGen - Данные для генерации
		///\param [in] changeBefore - Отступ изменяется до написания
		*/
		void genWithOffset(const OffsetInfo& dataForGen = {})
		{
			if (dataForGen.changeOffsetBeforeGen)
				changeOffset(dataForGen.offset);
			if (dataForGen.stringReturn.size())
				genWithoutOffset(dataForGen.stringReturn);
			if (dataForGen.changeOffsetBeforeGen == false)
				changeOffset(dataForGen.offset);
		}
		/*
		///\brief - Метод для генерации пользовательской информации без отступа
		///\param [in, out] stream - Поток для записи данных
		///\param [in] dataForGen - Данные для генерации
		*/
		void genWithoutOffset(const std::string& dataForGen = {})
		{
			*stream << offsetInFile.c_str() << dataForGen.c_str() << std::endl;
		}
		bool isOpen() const
		{
			return stream && stream->is_open();
		}
		~FileWriter() = default;

	

	};

	
}