#include "LibGeneratorC.hpp"


LibGeneratorC::BOOL_ANSWER LibGeneratorC::shoudGuessInlineFunction() const
{
	return BOOL_ANSWER::NO;
}

LibGeneratorC::return_type LibGeneratorC::genSignature(const std::string& returnType, const std::string& functionName, singature_build_args args) const
{
	std::string resultLine = returnType + ' ' + functionName + '(';
	for (auto& argData : args)
	{
		if (resultLine.back() == ',')
			resultLine += ' ';
		resultLine += argData.first + ' ' + argData.second + ',';
	}
	if (resultLine.back() == ',')
		resultLine.back() = ')';
	else
		resultLine += ')';
	return resultLine;
}
LibGeneratorC::return_type LibGeneratorC::genSignatureMain(const std::string& returnType, const std::string& functionName, singature_build_args args) const 
{
	return genSignature(returnType, functionName, args);
}
LibGeneratorC::return_type LibGeneratorC::genConst(const std::string& name, const std::string& value) const 
{
	return "#define " + name + " " + value;
}
LibGeneratorC::return_with_offset_type LibGeneratorC::startGenStruct(const std::string& structName) const 
{
	return LibGeneratorC::return_with_offset_type{ "typedef struct " + structName + "{", 1 , false };
}
LibGeneratorC::return_with_offset_type LibGeneratorC::endGenStruct(const std::string& structName) const
{
	return LibGeneratorC::return_with_offset_type{ "} " + structName + ";", -1, true};
}
LibGeneratorC::return_type LibGeneratorC::genField(const std::string& type, const std::string& fieldName, std::string defaultValue) const 
{
	if (type.back() == '*' && type != "void*" && type != "char*")
		return genLocalVariable(type.substr(0, type.size()-1), fieldName + "["
			+ std::to_string(std::count(defaultValue.begin(), defaultValue.end(), ',') +1)
			+"]", {});
	return genLocalVariable(type, fieldName, {});
}
LibGeneratorC::return_type LibGeneratorC::genGlobalVariable(const std::string& type, const std::string& fieldName, std::string defaultValue) const 
{
	auto string = genLocalVariable(type, fieldName, defaultValue);
	if (string.size())
		string = "static " + string;
	return string;
}
LibGeneratorC::return_type LibGeneratorC::genLocalVariable(const std::string& type, const std::string& fieldName, std::string defaultValue) const 
{
	std::string string;
	if (!defaultValue.empty() && defaultValue.find('{') != std::string::npos)
	{
		auto elCount = std::count(defaultValue.begin(), defaultValue.end(), ',') + 1;
		string = type.substr(0, type.size() - 1) + " " + fieldName + "[" + std::to_string(elCount) + "]";
	}
	else
		string = type + " " + fieldName;
	if (!defaultValue.empty())
		string += " = " + defaultValue;
	string += +";";
	return string;
}
LibGeneratorC::return_with_offset_type LibGeneratorC::beforeGen() const 
{
	return LibGeneratorC::return_with_offset_type{ std::string{}, 0 , false };
}
LibGeneratorC::return_with_offset_type LibGeneratorC::afterGen() const 
{
	return LibGeneratorC::return_with_offset_type{ std::string{}, 0 , false };
}
LibGeneratorC::return_with_offset_type LibGeneratorC::startFunction() const 
{
	return LibGeneratorC::return_with_offset_type{ "{", 1 , false };
}
LibGeneratorC::return_with_offset_type LibGeneratorC::endFunction() const 
{
	return LibGeneratorC::return_with_offset_type{ "}", -1 ,true};
}

LibGeneratorC::return_type LibGeneratorC::genIfElseBody(const std::string& body) const 
{
	return "else " + genIfBody(body);
}
LibGeneratorC::return_type LibGeneratorC::genIfBody(const std::string& body) const 
{
	return "if (" + body + ")";
}
LibGeneratorC::return_with_offset_type LibGeneratorC::startIf() const 
{
	return LibGeneratorC::return_with_offset_type{ "{", 1 , false };
}
LibGeneratorC::return_with_offset_type LibGeneratorC::endIf() const 
{
	return LibGeneratorC::return_with_offset_type{ "}", -1, true };
}
LibGeneratorC::return_type LibGeneratorC::genSwitchBody(const std::string& body) const 
{
	return "switch (" + body + ")";
}
LibGeneratorC::return_with_offset_type LibGeneratorC::startSwitch() const 
{
	return startIf();
}
LibGeneratorC::return_with_offset_type LibGeneratorC::endSwitch() const 
{
	return endIf();
}
LibGeneratorC::return_type LibGeneratorC::genElse() const 
{
	return "else";
}
LibGeneratorC::return_with_offset_type LibGeneratorC::startElse() const 
{
	return startIf();
}
LibGeneratorC::return_with_offset_type LibGeneratorC::endElse() const 
{
	return endIf();
}
LibGeneratorC::return_type LibGeneratorC::genReturnBody(const std::string& body) const 
{
	return "return " + body + ";";
}
LibGeneratorC::return_type LibGeneratorC::genWhileBody(const std::string& body) const 
{
	return "while (" + body + ")";
}
LibGeneratorC::return_with_offset_type LibGeneratorC::startWhile() const 
{
	return startIf();
}
LibGeneratorC::return_with_offset_type LibGeneratorC::endWhile() const 
{
	return endIf();
}
LibGeneratorC::return_type LibGeneratorC::genCaseBody(const std::string& body) const 
{
	return "case " + body + ":";
}
LibGeneratorC::return_with_offset_type LibGeneratorC::startCase() const 
{
	return startIf();
}
LibGeneratorC::return_with_offset_type LibGeneratorC::endCase() const 
{
	return endIf();
}
LibGeneratorC::return_type LibGeneratorC::genSqrtf(const std::string& body) const 
{
	return genSqrt(body);
}
LibGeneratorC::return_type LibGeneratorC::genSqrti(const std::string& body) const 
{
	return genSqrt(body);
}
LibGeneratorC::return_type LibGeneratorC::genSqrt(const std::string& body) const 
{
	return "_sqrt(" + body + ")";
}
LibGeneratorC::return_type LibGeneratorC::genPowf(const std::string& body, const std::string& pow) const 
{
	return genPow(body, pow);
}
LibGeneratorC::return_type LibGeneratorC::genPowi(const std::string& body, const std::string& pow) const 
{
	return genPow(body, pow);
}
LibGeneratorC::return_type LibGeneratorC::genPow(const std::string& body, const std::string& pow) const 
{
	return "_pow(" + body + ", " + pow + ")";
}
LibGeneratorC::return_type LibGeneratorC::genAbsf(const std::string& body) const 
{
	return genAbs(body);
}
LibGeneratorC::return_type LibGeneratorC::genAbsi(const std::string& body) const 
{
	return genAbs(body);
}
LibGeneratorC::return_type LibGeneratorC::genAbs(const std::string& body) const 
{
	return "_abs(" + body + ")";
}
LibGeneratorC::return_type LibGeneratorC::genExpf(const std::string& body) const 
{
	return genExp(body);
}
LibGeneratorC::return_type LibGeneratorC::genExpi(const std::string& body) const 
{
	return genExp(body);
}
LibGeneratorC::return_type LibGeneratorC::genExp(const std::string& body) const 
{
	return "_exp(" + body + ")";
}
LibGeneratorC::return_type LibGeneratorC::genFunctionCall(const std::string& output, const std::string& functionName, const std::vector<std::string>& args)const 
{
	std::string result;
	if (output.size())
		result += output + " = ";
	result += functionName + "(";
	for (auto& arg : args)
		result += arg + ", ";
	if (result.back() == 32)
	{
		result.pop_back();
		result.back() = ')';
	}
	else
		result += ')';
	result += ';';
	return result;
}

LibGeneratorC::BOOL_ANSWER LibGeneratorC::needGenGlobalVarInHeader() const
{
	return TwoFileGenerator::BOOL_ANSWER::YES;
}


LibGeneratorC::return_with_offset_type LibGeneratorC::beforeGenHeader() const 
{
	return LibGeneratorC::return_with_offset_type{ "#ifndef HEADER_H\n#define HEADER_H\n\n\t#include <stdbool.h>\n\t#include <stdint.h>\n\t#include <math.h>\n\t#include <stdlib.h>"
		"\n\n\t#define _sqrt(X) _Generic((X), \\\n\t\tlong double: sqrtl, \\\n\t\tfloat: sqrtf, \\\n\t\tdefault: sqrt \\\n\t\t)(X)"
		"\n\n\t#define _abs(X) _Generic((X), \\\n\t\tfloat: fabs, \\\n\t\tdefault: abs \\\n\t\t)(X)"
		"\n\n\t#define _exp(X) _Generic((X), \\\n\t\tlong double: expl, \\\n\t\tfloat: expf, \\\n\t\tdefault: exp \\\n\t\t)(X)"
		"\n\n\t#define _pow(X, Y) _Generic((X), \\\n\t\tlong double: powl, \\\n\t\tfloat: powf, \\\n\t\tdefault: pow \\\n\t\t)(X, Y)\n", 1 , false };
}
LibGeneratorC::return_type LibGeneratorC::genSignatureHeader(const std::string& returnType, const std::string& functionName, singature_build_args args) const 
{
	return genSignature(returnType, functionName, args) + ';';
}
LibGeneratorC::return_type LibGeneratorC::genMainSignatureHeader(const std::string& returnType, const std::string& functionName, singature_build_args args) const 
{
	return genSignature(returnType, functionName, args) + ';';
}
LibGeneratorC::return_with_offset_type LibGeneratorC::afterGenHeader() const 
{
	return LibGeneratorC::return_with_offset_type{ "#endif ", -1, true };
}
LibGeneratorC::return_with_offset_type LibGeneratorC::beforeGenGlobalVarHeader() const 
{
	return LibGeneratorC::return_with_offset_type{ std::string{}, 0 , false };
}
LibGeneratorC::return_with_offset_type LibGeneratorC::afterGenGlobalVarHeader() const 
{
	return LibGeneratorC::return_with_offset_type{ std::string{}, 0 , false };
}
LibGeneratorC::return_with_offset_type LibGeneratorC::beforeGenSignatureHeader() const 
{
	return LibGeneratorC::return_with_offset_type{ std::string{}, 0 , false };
}
LibGeneratorC::return_with_offset_type LibGeneratorC::afterGenSignatureHeader() const 
{
	return LibGeneratorC::return_with_offset_type{ std::string{}, 0 ,false };
}



LibGeneratorC::return_with_offset_type LibGeneratorC::beforeGenGlobalVar() const 
{
	return LibGeneratorC::return_with_offset_type{ std::string{}, 0 , false };
}
LibGeneratorC::return_with_offset_type LibGeneratorC::afterGenGlobalVar() const 
{
	return LibGeneratorC::return_with_offset_type{ std::string{}, 0 , false };
}
LibGeneratorC::return_with_offset_type LibGeneratorC::beforeGenSignature() const 
{
	return LibGeneratorC::return_with_offset_type{ std::string{}, 0 , false };
}
LibGeneratorC::return_with_offset_type LibGeneratorC::afterGenSignature() const 
{
	return LibGeneratorC::return_with_offset_type{ std::string{}, 0 ,true};
}
LibGeneratorC::return_type LibGeneratorC::genInclude(const std::string& file) const 
{
	return "#include \"" + file + "\"\n";
}
LibGeneratorC::return_type LibGeneratorC::sourceFileExtension() const 
{
	return "c";
}
LibGeneratorC::return_type LibGeneratorC::headerFileExtension() const 
{
	return "h";
}
LibGeneratorC::return_type LibGeneratorC::genSignatureLib(const std::string& returnType, const std::string& functionName, singature_build_args args) const 
{
	//return "extern __declspec(dllexport) " + genSignature(returnType, functionName, args);
	return "__declspec(dllexport) " + genSignature(returnType, functionName, args);
}
LibGeneratorC::return_type LibGeneratorC::genHeaderSignatureLib(const std::string& returnType, const std::string& functionName, singature_build_args args) const 
{
	return genSignatureLib(returnType, functionName, args) + ";";
	//return genSignature(returnType, functionName, args);
}
LibGeneratorC::return_type LibGeneratorC::mainFunctionName() const 
{
	return "Основной алгоритм";
}
LibGeneratorC::return_with_offset_type LibGeneratorC::beforeGenLibHeader() const 
{
	return LibGeneratorC::return_with_offset_type{ "#ifndef LIB_H\n#define LIB_H\n\n#include <string.h>\n#include <stdlib.h>\n\n ", 1, false };
}
LibGeneratorC::return_with_offset_type LibGeneratorC::beforeGenLib() const 
{
	return LibGeneratorC::return_with_offset_type{ "char* string2char(const char* str, const int size)\n{\n\tchar* result = (char*)malloc((size+1) * sizeof(char));\n\tstrcpy(result, str);\n\treturn result;\n}",
		0 , true };
};
LibGeneratorC::return_with_offset_type LibGeneratorC::afterGenLib() const 
{
	return LibGeneratorC::return_with_offset_type{ std::string{}, 0 , false };
};
LibGeneratorC::return_with_offset_type LibGeneratorC::afterGenLibHeader() const 
{
	return LibGeneratorC::return_with_offset_type{ "#endif ", -1, true };
}
LibGeneratorC::return_type LibGeneratorC::makeArrayHeap(const std::string& type, const std::string& varName, const std::uint32_t& elementCount) const 
{
	return varName + " = (" + type + "*)malloc(" + std::to_string(elementCount) + " * sizeof(" + type + "));";
	//return varName + " = new " +type + "[" + std::to_string(elementCount) + "];";
	//return type + " " + varName + "[" + std::to_string(elementCount) + "];\n";
}
LibGeneratorC::return_type LibGeneratorC::makeString2HeapPtr(const std::string& string, const std::string& charPtr) const 
{
	return charPtr + " = string2char(" + string + ", " + std::to_string(string.size()-2) + ")";
}
LibGeneratorC::return_type LibGeneratorC::deleteHeapPtr(const std::string& ptr) const 
{
	return "free(" + ptr + ");";
}
LibGeneratorC::return_with_offset_type LibGeneratorC::beforeGenSignatureLib() const 
{
	return LibGeneratorC::return_with_offset_type{ std::string{}, 0 , false };
}
LibGeneratorC::return_with_offset_type LibGeneratorC::afterGenSignatureLib() const 
{
	return LibGeneratorC::return_with_offset_type{ std::string{}, 0 , true};
}

LibGeneratorC::BOOL_ANSWER LibGeneratorC::needGenAccessFunction() const
{
	return BOOL_ANSWER::YES;
}

LibGeneratorC::return_type LibGeneratorC::stringCompare(const std::string & lString, const std::string & rString) const
{
	return "(strcmp(" + lString +", "+ rString +") == 0)";
}
