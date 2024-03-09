#include "DictionaryC.hpp"


SimpleDictionaryC::return_type SimpleDictionaryC::getPumlTimerType() const
{
	return "UInt32";
}

SimpleDictionaryC::return_type SimpleDictionaryC::getType(const std::string & pumlType) const
{
	auto iter = typeDictionary.find(pumlType);
	if (iter == typeDictionary.end())
		return{};
	return iter->second;
}
SimpleDictionaryC::return_type SimpleDictionaryC::getTimerType() const 
{
	return "uint32_t";
}
SimpleDictionaryC::return_type SimpleDictionaryC::makePtrable(const std::string & type, const std::uint32_t & elementCount) const 
{
	return type + "*";
}
SimpleDictionaryC::return_type SimpleDictionaryC::getType(const std::string & pumlType, const std::uint32_t & elementCount) const 
{
	auto str = getType(pumlType);
	if (str.size())
	{
		if (elementCount == 0)
			str.clear();
		else
			str = makePtrable(str, elementCount);
	}
	return str;
}
SimpleDictionaryC::return_type SimpleDictionaryC::makeRefArg(const std::string & type) const 
{
	auto typeCopy = type;
	if (typeCopy.find('[') != std::string::npos)
	{
		auto count = std::count(typeCopy.begin(), typeCopy.end(), '[');
		typeCopy = typeCopy.substr(0, typeCopy.find('['));
		typeCopy.append(std::string(count, '*'));
	}
	if (typeCopy.find('*') != std::string::npos)
		return typeCopy;
	return makePtrable(type, 1);
}
SimpleDictionaryC::return_type SimpleDictionaryC::makeRef(const std::string& value) const
{
	return "&" + value;
}
SimpleDictionaryC::return_type SimpleDictionaryC::getDefaultValue(const std::string & type, const bool isPtr, const std::uint32_t & elementCount) const 
{
	if (isPtr)
	{
		if (elementCount == 1)
			return "NULL";
	}
	auto iter = typeDefaultValueDictionary.find(type);
	if (iter == typeDefaultValueDictionary.end())
		if (isPtr)
			return "NULL";
		else
			return{};
	if (elementCount != 1)
	{
		std::string value{ "{" };
		for (auto i = 0u; i < elementCount; ++i)
			value += iter->second + ", ";
		value.pop_back();
		value.back() = '}';
		return value;
	}
	return iter->second;
}
SimpleDictionaryC::return_type SimpleDictionaryC::getCommentSymbol() const 
{
	return "//";
}
SimpleDictionaryC::return_type SimpleDictionaryC::getDoxygenCommentSymbol() const 
{
	return "///";
}
SimpleDictionaryC::return_type SimpleDictionaryC::getDoxygenCommentFieldSymbol() const 
{
	return "///<";
}
SimpleDictionaryC::return_type SimpleDictionaryC::getTranscription(const char ch) const 
{
	if(engCh.find(ch) != engCh.end() || isdigit(static_cast<std::uint8_t>(ch)))
		return std::string(1, ch);
	if (isspace(static_cast<std::uint8_t>(ch)))
		return "_";
	auto iter = std::find(ru.begin(), ru.end(), ch);
	if (iter == ru.end())
		return "_";
	return eng.at(std::distance(ru.begin(), iter));
}
SimpleDictionaryC::return_type SimpleDictionaryC::getEndLine() const 
{
	return ";";
}
SimpleDictionaryC::return_type SimpleDictionaryC::makeUnPtrable(const std::string& value, const std::string& index) const 
{
	auto result = "(*(" + value;
	if (!index.empty())
		result += " + " + index;
	result += "))";
	return result;
}
SimpleDictionaryC::return_type SimpleDictionaryC::getReferencingInStruct() const 
{
	return ".";
}
SimpleDictionaryC::return_type SimpleDictionaryC::getCharType() const 
{
	return "char";
}
SimpleDictionaryC::return_type SimpleDictionaryC::getVoidType() const 
{
	return typeDictionary.at({});
}

SimpleDictionaryC::return_type SimpleDictionaryC::getSizeType() const
{
	return "uint32_t";
}

SimpleDictionaryC::return_type SimpleDictionaryC::getBoolType() const
{
	return "bool";
}
