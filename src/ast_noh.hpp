#pragma once

#include <cstdint>
#include <iostream>
#include <vector>
#include <boost/fusion/include/adapt_struct.hpp>

#include "isdbg_noh.hpp"

namespace Noh {
namespace ast {
enum AstID {
	BaseID,
	VoidID,
	FuncID,
	CallID,
	ModuleID,
	NumberID,
	StringID,
	IdentID,
	MonoExpID,
	BinaryExpID,
	BuiltinID,
	AssignID,
	ReAssignID,
	StmtID,
	IfStmtID,
	WhileStmtID,
	RangeID,
	ForStmtID,
	TupleID,

	NoneID,
};

class BaseAst {
	AstID id;
public:
	BaseAst(AstID x) : id(x) {}
	virtual ~BaseAst() {}
	AstID getID() const { return this->id; }
};

class VoidAst : public BaseAst {
	VoidAst(int x) : BaseAst(AstID::VoidID) {}
};

class FuncAst : public BaseAst {
public:
	std::string Name;
	std::vector<std::string> ParamNames;
	std::vector<BaseAst*> Inst;
	BaseAst* RetValue;

	FuncAst(const std::string& name) : BaseAst(AstID::FuncID), Name(name)
	{
		if constexpr (isDebug) { std::cerr << "FuncAst(" << this << ") " << name << std::endl; }
	}
	~FuncAst()
	{
		for(auto& ptr : this->Inst) { delete ptr; }
		delete RetValue;
	}
	static inline bool classOf(const BaseAst* base) { return base->getID() == AstID::FuncID; }
	std::string& getName() { return this->Name; }
	std::vector<std::string>& getParamNames() { return this->ParamNames; }
	std::vector<BaseAst*>& getInst() { return this->Inst; }
	BaseAst*& getRetValue() { return this->RetValue; }
};

class CallAst : public BaseAst {
public:
	std::string FuncName;
	std::vector<BaseAst*> Params;

	CallAst(const std::string& name) : BaseAst(AstID::CallID), FuncName(name)
	{
		if constexpr (isDebug) { std::cerr << "CallAst(" << this << ") " << FuncName << std::endl; }
	}
	static inline bool classOf(const BaseAst* base) { return base->getID() == AstID::CallID; }
	std::string& getFuncName() { return this->FuncName; }
	std::vector<BaseAst*>& getParams() { return this->Params; }
};

class ModuleAst : public BaseAst {
public:
	std::vector<FuncAst*> Funcs;

	ModuleAst() : BaseAst(AstID::ModuleID)
	{
		if constexpr (isDebug) { std::cerr << "ModuleAst(" << this << ")" << std::endl; }
	}
	~ModuleAst() { for(auto& p : this->Funcs) { delete p; } }
	static inline bool classOf(const BaseAst* base) { return base->getID() == AstID::ModuleID; }
	std::vector<FuncAst*>& getFuncs() { return this->Funcs; }
};

class NumberAst : public BaseAst {
	std::int_fast64_t Val;
public:
	NumberAst(const std::int_fast64_t& val) : BaseAst(AstID::NumberID), Val(val)
	{
		if constexpr (isDebug) { std::cerr << "NumberAst(" << this << ") " << Val << std::endl; }
	}
	static inline bool classOf(const BaseAst* base) { return base->getID() == AstID::NumberID; }
	std::int_fast64_t& getVal() { return this->Val; }
};

class StringAst : public BaseAst {
	std::string Val;
public:
	StringAst(const std::string& val) : BaseAst(AstID::StringID), Val()
	{
		if constexpr(isDebug) { std::cerr << "StringAst(" << this << ") " << val << std::endl; }
		const std::uint_fast32_t siz = val.size();
		for(std::uint_fast32_t i{}; i < siz; i++)
		{
			if(i == siz - 1) { Val += val[i]; continue;}
			
			if(val[i] == '\\' and val[i + 1] == '0') { Val += '\0'; }
			else if(val[i] == '\\' and val[i + 1] == 'a') { Val += '\a'; i++; }
			else if(val[i] == '\\' and val[i + 1] == 'b') { Val += '\b'; i++; }
			else if(val[i] == '\\' and val[i + 1] == 'f') { Val += '\f'; i++; }
			else if(val[i] == '\\' and val[i + 1] == 'n') { Val += '\n'; i++; }
			else if(val[i] == '\\' and val[i + 1] == 'r') { Val += '\r'; i++; }
			else if(val[i] == '\\' and val[i + 1] == 't') { Val += '\t'; i++; }
			else if(val[i] == '\\' and val[i + 1] == 'v') { Val += '\v'; i++; }
			else if(val[i] == '\\' and val[i + 1] == '\\') { Val += '\\'; i++; }
			else if(val[i] == '\\')continue;
			else Val += val[i];
		}
	}
	static inline bool classOf(const BaseAst* base) { return base->getID() == AstID::StringID; }
	std::string getVal() { return this->Val; }
};

class IdentAst : public BaseAst {
	std::string Ident;
public:
	IdentAst(const std::string& ident) : BaseAst(AstID::IdentID), Ident(ident)
	{
		if constexpr (isDebug) { std::cerr << "IdentAst(" << this << ") " << Ident << std::endl; }
	}
	static inline bool classOf(const BaseAst* base) { return base->getID() == AstID::IdentID; }
	std::string& getIdent() { return this->Ident; }
};

class MonoExpAst : public BaseAst {
	std::string Op;
	BaseAst* Lhs;
public:
	MonoExpAst(const std::string& op, BaseAst* lhs) : BaseAst(AstID::MonoExpID), Op(op), Lhs(lhs)
	{
		if constexpr (isDebug) { std::cerr << "MonoExpAst(" << this << ") " << op << ' ' << lhs << std::endl; }
	}
	~MonoExpAst() { delete this->Lhs; }
	static inline bool classOf(const BaseAst* base) { return base->getID() == AstID::MonoExpID; }
	std::string& getOp() { return this->Op; }
	BaseAst* getLhs() { return this->Lhs; }
};

class BinaryExpAst : public BaseAst {
public:
	std::string Op;
	BaseAst* Lhs, * Rhs;

	BinaryExpAst(const std::string& op, BaseAst* lhs, BaseAst* rhs)
		: BaseAst(AstID::BinaryExpID), Op(op), Lhs(lhs), Rhs(rhs)
	{
		if constexpr (isDebug) { std::cerr << "BinaryExpAst(" << this << ") " << lhs << ' ' << op << ' ' << rhs << std::endl; }
	}
	BinaryExpAst(const std::string& op)
		: BaseAst(AstID::BinaryExpID)
	{
		if constexpr (isDebug) { std::cerr << "BinaryExpAst(" << this << ")" << std::endl; }
	}
	~BinaryExpAst() { delete this->Lhs; delete this->Rhs; }
	static inline bool classOf(const BaseAst* base) { return base->getID() == AstID::BinaryExpID; }
	std::string& getOp() { return this->Op; }
	BaseAst* getLhs() { return this->Lhs; }
	BaseAst* getRhs() { return this->Rhs; }
};

class BuiltinAst : public BaseAst {
public:
	std::string Name;
	std::vector<BaseAst*> Args;

	BuiltinAst(const std::string& name) : BaseAst(AstID::BuiltinID), Name(name)
	{
		if constexpr (isDebug) { std::cerr << "BuiltinAst(" << this << ") " << name << std::endl; }
	}
	~BuiltinAst() { for(auto& arg : this->Args) { delete arg; } }
	static inline bool classOf(const BaseAst* base) { return base->getID() == AstID::BuiltinID; }
	std::string& getName() { return this->Name; }
	std::vector<BaseAst*>& getArgs() { return this->Args; }
};

class AssignAst : public BaseAst {
public:
	std::string Name;
	BaseAst* Val;

	AssignAst(const std::string& name) : BaseAst(AstID::AssignID), Name(name)
	{
		if constexpr (isDebug) { std::cerr << "AssignAst(" << this << ") " << name << std::endl; }
	}
	~AssignAst() { delete this->Val; }
	static inline bool classOf(const BaseAst* base) { return base->getID() == AstID::AssignID; }
	std::string& getName(void) { return this->Name; }
	BaseAst*& getVal() { return this->Val; }
};

class ReAssignAst : public BaseAst {
public:
	std::string Name;
	BaseAst* Val;

	ReAssignAst(const std::string& name) : BaseAst(AstID::ReAssignID), Name(name)
	{
		if constexpr (isDebug) { std::cerr << "ReAssignAst(" << this << ") " << name << std::endl; }
	}
	~ReAssignAst() { delete this->Val; }
	static inline bool classOf(const BaseAst* base) { return base->getID() == AstID::ReAssignID; }
	std::string& getName(void) { return this->Name; }
	BaseAst*& getVal() { return this->Val; }	
};

class IfStmtAst : public BaseAst {
public:
	BaseAst* Cond;
	std::vector<BaseAst*> ThenStmt;
	std::vector<BaseAst*> ElseStmt;

	IfStmtAst() : BaseAst(AstID::IfStmtID), Cond(), ThenStmt(), ElseStmt()
	{
		if constexpr (isDebug) { std::cerr << "IfStmtAst(" << this << ")" << std::endl; }
	}
	~IfStmtAst()
	{
		delete this->Cond;
		for(auto& s : this->ThenStmt) { delete s; }
		for(auto& s : this->ElseStmt) { delete s; }
	}
	static inline bool classOf(const BaseAst* base) { return base->getID() == AstID::IfStmtID; }
	BaseAst* getCond() { return this->Cond; }
	std::vector<BaseAst*>& getThenStmt() { return this->ThenStmt; }
	std::vector<BaseAst*>& getElseStmt() { return this->ElseStmt; }
};

class WhileStmtAst : public BaseAst {
public:
	BaseAst* Cond;
	std::vector<BaseAst*> LoopStmt;
	
	WhileStmtAst() : BaseAst(AstID::WhileStmtID), Cond()
	{
		if constexpr (isDebug) { std::cerr << "WhileStmtAst(" << this << ")" << std::endl; }
	}
	~WhileStmtAst()
	{
		delete this->Cond;
		for(auto& s : this->LoopStmt) { delete s; }
	}
	static inline bool classOf(const BaseAst* base) { return base->getID() == AstID::WhileStmtID; }
	BaseAst* getCond() { return this->Cond; }
	std::vector<BaseAst*>& getLoopStmt() { return this->LoopStmt; }
};

class RangeAst : public BaseAst {
public:
	BaseAst* From;
	BaseAst* To;

	RangeAst() : BaseAst(AstID::RangeID), From(), To()
	{
		if constexpr (isDebug) { std::cerr << "RangeAst(" << this << ")" << std::endl; }
	}
	~RangeAst()
	{
		delete this->From;
		delete this->To;
	}
	static inline bool classOf(const BaseAst* base) { return base->getID() == AstID::RangeID; }
	BaseAst* getFrom() { return this->From; }
	BaseAst* getTo() { return this->To; }
};

class ForStmtAst : public BaseAst {
public:
	std::string Ident;
	RangeAst* Range;
	std::vector<BaseAst*> Stmts;

	ForStmtAst() : BaseAst(AstID::ForStmtID), Ident(), Range(), Stmts()
	{
		if constexpr (isDebug) { std::cerr << "ForStmtAst(" << this << ")" << std::endl; }
	}
	~ForStmtAst()
	{
		delete this->Range;
		for(auto& s : this->Stmts) { delete s; }
	}
	static inline bool classOf(const BaseAst* base) { return base->getID() == AstID::ForStmtID; }
	std::string& getIdent() { return this->Ident; }
	RangeAst* getRange() { return this->Range; }
	std::vector<BaseAst*>& getStmts() { return this->Stmts; }
};

class TupleAst : public BaseAst {
public:
	std::vector<BaseAst*> Ary;

	TupleAst(std::vector<BaseAst*> ary) : BaseAst(AstID::TupleID), Ary(ary)
	{
		if constexpr (isDebug) { std::cerr << "TupleAst(" << this << ")" << std::endl; }
	}
	~TupleAst()
	{
		for(auto& e : this->Ary) { delete e; }
	}
	static inline bool classOf(const BaseAst* base) { return base->getID() == AstID::ForStmtID; }
	std::vector<BaseAst*> getAry() { return this->Ary; }
};

} // namespace ast
} // namespace Noh


BOOST_FUSION_ADAPT_STRUCT(
	Noh::ast::ModuleAst,
	(std::vector<Noh::ast::FuncAst*>, Funcs)
)
BOOST_FUSION_ADAPT_STRUCT(
	Noh::ast::FuncAst,
	(std::string, Name)
	(std::vector<std::string>, ParamNames)
	(std::vector<Noh::ast::BaseAst*>, Inst)
)
BOOST_FUSION_ADAPT_STRUCT(
	Noh::ast::CallAst,
	(std::string, FuncName)
	(std::vector<Noh::ast::BaseAst*>, Params)
)
BOOST_FUSION_ADAPT_STRUCT(
	Noh::ast::BuiltinAst,
	(std::string, Name)
	(std::vector<Noh::ast::BaseAst*>, Args)
)
BOOST_FUSION_ADAPT_STRUCT(
	Noh::ast::AssignAst,
	(std::string, Name)
	(Noh::ast::BaseAst*, Val)
)
BOOST_FUSION_ADAPT_STRUCT(
	Noh::ast::ReAssignAst,
	(std::string, Name)
	(Noh::ast::BaseAst*, Val)
)
BOOST_FUSION_ADAPT_STRUCT(
	Noh::ast::IfStmtAst,
	(Noh::ast::BaseAst*, Cond)
	(std::vector<Noh::ast::BaseAst*>, ThenStmt)
	(std::vector<Noh::ast::BaseAst*>, ElseStmt)
)
BOOST_FUSION_ADAPT_STRUCT(
	Noh::ast::WhileStmtAst,
	(Noh::ast::BaseAst*, Cond)
	(std::vector<Noh::ast::BaseAst*>, LoopStmt)
)
BOOST_FUSION_ADAPT_STRUCT(
	Noh::ast::RangeAst,
	(Noh::ast::BaseAst*, From)
	(Noh::ast::BaseAst*, To)
)
BOOST_FUSION_ADAPT_STRUCT(
	Noh::ast::ForStmtAst,
	(std::string, Ident)
	(Noh::ast::RangeAst*, Range)
	(std::vector<Noh::ast::BaseAst*>, Stmts)
)
BOOST_FUSION_ADAPT_STRUCT(
	Noh::ast::BinaryExpAst,
	(std::string, Op)
	(Noh::ast::BaseAst*, Lhs)
	(Noh::ast::BaseAst*, Rhs)
)
BOOST_FUSION_ADAPT_STRUCT(
	Noh::ast::TupleAst,
	(std::vector<Noh::ast::BaseAst*>, Ary)
)