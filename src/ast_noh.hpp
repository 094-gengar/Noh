#pragma once

#include <cstdint>
#include <iostream>
#include <vector>
#include <boost/fusion/include/adapt_struct.hpp>

#include "isDebug_noh.hpp"

namespace Noh {
namespace ast {
enum AstID {
	BaseID,
	FuncID,
	ModuleID,
	NumberID,
	IdentID,
	MonoExpID,
	BinaryExpID,
	BuiltinID,
	AssignID,
	StmtID,
	IfStmtID,
	WhileStmtID,
};

class BaseAst {
	AstID id;
public:
	BaseAst(AstID x) : id(x) {}
	virtual ~BaseAst() {}
	AstID getID() const { return this->id; }
};

class FuncAst : public BaseAst {
public:
	std::string Name;
	std::vector<BaseAst*> Inst;

	FuncAst(const std::string& name) : BaseAst(AstID::FuncID), Name(name)
	{
		if constexpr (isDebug) { std::cerr << "FuncAst(" << this << ") " << name << std::endl; }
	}
	~FuncAst() { for(auto ptr : this->Inst) { delete ptr; } }
	static inline bool classOf(const BaseAst* base) { return base->getID() == AstID::FuncID; }
	std::string& getName() { return this->Name; }
	std::vector<BaseAst*>& getInst() { return this->Inst; }
};

class ModuleAst : public BaseAst {
public:
	std::vector<std::string> Vars;
	std::vector<FuncAst*> Funcs;

	ModuleAst() : BaseAst(AstID::ModuleID)
	{
		if constexpr (isDebug) { std::cerr << "ModuleAst(" << this << ") " << std::endl; }
	}
	~ModuleAst() { for(auto p : this->Funcs) { delete p; } }
	static inline bool classOf(const BaseAst* base) { return base->getID() == AstID::ModuleID; }
	std::vector<std::string>& getVars() { return this->Vars; }
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
	std::int_fast64_t getVal() { return this->Val; }
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
	~BuiltinAst() { for(auto arg : this->Args) { delete arg; } }
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
	BaseAst* getVal() { return this->Val; }
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
		for(auto s : this->ThenStmt) { delete s; }
		for(auto s : this->ElseStmt) { delete s; }
	}
	static inline bool classOf(const BaseAst* base) { return base->getID() == AstID::IfStmtID; }
	BaseAst* getCond() { return this->Cond; }
	std::vector<BaseAst*> getThenStmt() { return this->ThenStmt; }
	std::vector<BaseAst*> getElseStmt() { return this->ElseStmt; }
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
		for(auto s : this->LoopStmt) { delete s; }
	}
	static inline bool classOf(const BaseAst* base) { return base->getID() == AstID::WhileStmtID; }
	BaseAst* getCond() { return this->Cond; }
	std::vector<BaseAst*>& getLoopStmt() { return this->LoopStmt; }
};
} // namespace ast
} // namespace Noh



BOOST_FUSION_ADAPT_STRUCT(
	Noh::ast::ModuleAst,
	(std::vector<std::string>, Vars)
	(std::vector<Noh::ast::FuncAst*>, Funcs)
)
BOOST_FUSION_ADAPT_STRUCT(
	Noh::ast::FuncAst,
	(std::string, Name)
	(std::vector<Noh::ast::BaseAst*>, Inst)
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