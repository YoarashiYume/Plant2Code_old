#pragma once
#include "../EditableData/Generators.hpp"
class LibGeneratorC : public LibGenerator
{
public:
	virtual BOOL_ANSWER shoudGuessInlineFunction() const override;
	virtual return_type genSignature(const std::string& returnType, const std::string& functionName, singature_build_args args) const override;
	virtual return_type genSignatureMain(const std::string& returnType, const std::string& functionName, singature_build_args args) const override;
	virtual return_type genField(const std::string& type, const std::string& fieldName, std::string defaultValue = {}) const override;
	virtual return_type genGlobalVariable(const std::string& type, const std::string& fieldName, std::string defaultValue) const override;
	virtual return_type genLocalVariable(const std::string& type, const std::string& fieldName, std::string defaultValue) const override;
	virtual return_type genConst(const std::string& name, const std::string& value) const override;
	virtual return_with_offset_type startGenStruct(const std::string& structName) const override;
	virtual return_with_offset_type endGenStruct(const std::string& structName) const override;

	virtual return_with_offset_type beforeGen() const override;
	virtual return_with_offset_type afterGen() const override;

	virtual return_with_offset_type startFunction() const override;
	virtual return_with_offset_type endFunction() const override;

	virtual return_type genIfElseBody(const std::string& body) const override;

	virtual return_type genIfBody(const std::string& body) const override;
	virtual return_with_offset_type startIf() const override;
	virtual return_with_offset_type endIf() const override;

	virtual return_type genSwitchBody(const std::string& body) const override;
	virtual return_with_offset_type startSwitch() const override;
	virtual return_with_offset_type endSwitch() const override;

	virtual return_type genCaseBody(const std::string& body) const override;
	virtual return_with_offset_type startCase() const override;
	virtual return_with_offset_type endCase() const override;

	virtual return_type genWhileBody(const std::string& body) const override;
	virtual return_with_offset_type startWhile() const override;
	virtual return_with_offset_type endWhile() const override;

	virtual return_type genElse() const override;
	virtual return_with_offset_type startElse() const override;
	virtual return_with_offset_type endElse() const override;

	virtual return_type genReturnBody(const std::string& body) const override;

	virtual return_type genPowf(const std::string& body, const std::string& pow) const override;
	virtual return_type genPowi(const std::string& body, const std::string& pow) const override;
	virtual return_type genPow(const std::string& body, const std::string& pow) const override;

	virtual return_type genSqrtf(const std::string& body) const override;
	virtual return_type genSqrti(const std::string& body) const override;
	virtual return_type genSqrt(const std::string& body) const override;

	virtual return_type genAbsf(const std::string& body) const override;
	virtual return_type genAbsi(const std::string& body) const override;
	virtual return_type genAbs(const std::string& body) const override;

	virtual return_type genExpf(const std::string& body) const override;
	virtual return_type genExpi(const std::string& body) const override;
	virtual return_type genExp(const std::string& body) const override;

	virtual return_type genFunctionCall(const std::string& output, const std::string& functionName, const std::vector<std::string>& args) const override;

	virtual return_with_offset_type beforeGenGlobalVar() const override;
	virtual return_with_offset_type afterGenGlobalVar() const override;

	virtual return_with_offset_type beforeGenSignature() const override;
	virtual return_with_offset_type afterGenSignature() const override;

	virtual return_type sourceFileExtension() const override;

	virtual BOOL_ANSWER needGenGlobalVarInHeader() const override;

	virtual return_with_offset_type beforeGenGlobalVarHeader() const override;
	virtual return_with_offset_type afterGenGlobalVarHeader() const override;

	virtual return_with_offset_type beforeGenSignatureHeader() const override;
	virtual return_with_offset_type afterGenSignatureHeader() const override;

	virtual return_type genSignatureHeader(const std::string& returnType, const std::string& functionName, singature_build_args args) const override;
	virtual return_type genMainSignatureHeader(const std::string& returnType, const std::string& functionName, singature_build_args args) const override;

	virtual return_with_offset_type beforeGenHeader() const override;
	virtual return_with_offset_type afterGenHeader() const override;

	virtual return_type genInclude(const std::string& file) const override;

	virtual return_type headerFileExtension() const override;

	virtual return_type genSignatureLib(const std::string& returnType, const std::string& functionName, singature_build_args args) const override;
	virtual return_type genHeaderSignatureLib(const std::string& returnType, const std::string& functionName, singature_build_args args) const override;
	virtual return_type mainFunctionName() const override;

	virtual return_with_offset_type beforeGenLibHeader() const override;
	virtual return_with_offset_type afterGenLibHeader() const override;

	virtual return_with_offset_type beforeGenLib() const override;
	virtual return_with_offset_type afterGenLib() const override;

	virtual return_type makeArrayHeap(const std::string& type, const std::string& varName, const std::uint32_t& elementCount) const override;

	virtual return_type makeString2HeapPtr(const std::string& string, const std::string& charPtr) const override;
	virtual return_type deleteHeapPtr(const std::string& ptr) const override;

	virtual return_with_offset_type beforeGenSignatureLib() const override;
	virtual return_with_offset_type afterGenSignatureLib() const override;

	virtual BOOL_ANSWER needGenAccessFunction() const override;

	virtual return_type stringCompare(const std::string& lString, const std::string& rString) const override;
};

