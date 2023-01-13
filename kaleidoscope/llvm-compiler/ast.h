#pragma once

#include <string>
#include <memory>
#include <vector>
#include <utility>

namespace ast {
  struct ExprAST {
    virtual ~ExprAST() {}
  };

  struct NumberAST: public ExprAST {
    private:
      double val_;
    public:
      NumberAST(double val): val_(val) {}
  };

  struct VariableAST: public ExprAST {
    private:
      std::string name_;
    public:
      VariableAST(const std::string& name): name_(name) {}
  };

  struct BinaryAST: public ExprAST {
    private:
      char op_;
      std::unique_ptr<ExprAST> LHS_, RHS_;
    public:
      BinaryAST(char op, std::unique_ptr<ExprAST> LHS, std::unique_ptr<ExprAST> RHS)
        : op(op_), LHS_(std::move(LHS)) , RHS_(std::move(RHS)) {}
  };

  struct CallExprAST: public ExprAST {
    private:
      std::string callee_;
      std::vector<std::unique_ptr<ExprAST>> args_;

    public:
      CallExprAST(const std::string& callee, std::vector<std::unique_ptr<ExprAST>> args)
        : callee_(callee), args_(std::move(args)) {}
  };

  struct ProtoTypeAST {
    private:
      std::string name_;
      std::vector<std::string> args_;

    public:
      ProtoTypeAST(const std::string& name, std::vector<std::string> args)
        : name_(name), args_(std::move(args)) {}

      const std::string& get_name() {return name_;}
  };

  struct FunctionAST {
    private:
      std::unique_ptr<ProtoTypeAST> proto_;
      std::unique_ptr<ExprAST> body_;
    public:
      FunctionAST(std::unique_ptr<ProtoTypeAST> proto, std::unique_ptr<ExprAST> body)
        :proto_(std::move(proto)), body_(std::move(body)) {}
  };

  //TODO: Parser Basics
  // https://llvm.org/docs/tutorial/MyFirstLanguageFrontend/LangImpl02.html

};
