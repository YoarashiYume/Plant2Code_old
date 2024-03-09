#pragma once
#include "../EditableData/Dictionary.hpp"
#include <unordered_map>
#include <set>
class SimpleDictionaryC : public TypeDictionary
{
protected:
	std::vector<char> ru{ 'а','б','в','г','д','е','ё','ж','з','и','й','к','л','м','н','о','п','р','с','т','у','ф','х','ц','ч','ш','щ','ъ','ы','ь','э','ю','я',
		'А','Б','В','Г','Д','Е','Ё','Ж','З','И','Й','К','Л','М','Н','О','П','Р','С','Т','У','Ф','Х','Ц','Ч','Ш','Щ','Ъ','Ы','Ь','Э','Ю','Я' };
	std::vector < std::string > eng
	{
		"a","b","v","g","d","e","yo","zh","z","i","y","k","l","m","n","o","p","r","s","t","u","f","kh","ts","ch","sh","shch","","y","","e","yu","ya",
		"A","B","V","G","D","E","Yo","Zh","Z","I","Y","K","L","M","N","O","P","R","S","T","U","F","Kh","Ts","Ch","Sh","Shch","","Y","","E","Yu","Ya"
	};
	std::set<char>engCh{ 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
		'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z' };
	std::unordered_map<std::string, std::string> typeDictionary
	{
		{ "bool", "bool" },
		{ "Int8", "int8_t" },
		{ "UInt8", "uint8_t" },
		{ "Int16", "int16_t" },
		{ "UInt16", "uint16_t" },
		{ "Int32", "int32_t" },
		{ "UInt32", "uint32_t" },
		{ "Int64", "int64_t" },
		{ "UInt64", "uint64_t" },
		{ "float", "float" },
		{ "", "void" }
	};
	std::unordered_map<std::string, std::string> typeDefaultValueDictionary
	{
		{ "bool", "0" },
		{ "int8_t", "0" },
		{ "uint8_t", "0" },
		{ "int16_t", "0" },
		{ "uint16_t", "0" },
		{ "int32_t", "0" },
		{ "uint32_t", "0" },
		{ "int64_t", "0" },
		{ "uint64_t", "0" },
		{ "float", "0.0f" }
	};
public:
	virtual return_type getPumlTimerType() const override;
	virtual return_type getType(const std::string & pumlType) const override;
	virtual return_type getTimerType() const override;
	virtual return_type getCharType() const override;
	virtual return_type getVoidType() const override;
	virtual return_type getSizeType() const override;
	virtual return_type getBoolType() const override;

	virtual return_type getType(const std::string & pumlType, const std::uint32_t & elementCount) const override;
	virtual return_type getDefaultValue(const std::string & type, const bool isPtr, const std::uint32_t & elementCount) const override;

	virtual return_type getCommentSymbol() const override;
	virtual return_type getDoxygenCommentSymbol() const override;
	virtual return_type getDoxygenCommentFieldSymbol() const override;
	virtual return_type makePtrable(const std::string & type, const std::uint32_t & elementCount = 1) const override;
	virtual return_type makeUnPtrable(const std::string& value, const std::string& index = std::string{}) const override;

	virtual return_type makeRefArg(const std::string & type) const override;
	virtual return_type makeRef(const std::string& value) const override;

	virtual return_type getTranscription(const char ch) const override;
	virtual return_type getEndLine() const override;

	virtual return_type getReferencingInStruct() const override;


	virtual ~SimpleDictionaryC() = default;
};

