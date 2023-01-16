#pragma once

#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "lexer.h"

namespace ast {
using lexer::get_tok;
using lexer::identifier_str;
using lexer::num_val;
using lexer::token;

struct ExprAST {
  virtual ~ExprAST() {}
};

struct NumberAST : public ExprAST {
private:
  double val_;

public:
  NumberAST(double val) : val_(val) {}
};

struct VariableAST : public ExprAST {
private:
  std::string name_;

public:
  VariableAST(const std::string &name) : name_(name) {}
};

struct BinaryAST : public ExprAST {
private:
  char op_;
  std::unique_ptr<ExprAST> LHS_, RHS_;

public:
  BinaryAST(char op, std::unique_ptr<ExprAST> LHS, std::unique_ptr<ExprAST> RHS)
      : op_(op), LHS_(std::move(LHS)), RHS_(std::move(RHS)) {}
};

struct CallExprAST : public ExprAST {
private:
  std::string callee_;
  std::vector<std::unique_ptr<ExprAST>> args_;

public:
  CallExprAST(const std::string &callee,
              std::vector<std::unique_ptr<ExprAST>> args)
      : callee_(callee), args_(std::move(args)) {}
};

struct ProtoTypeAST {
private:
  std::string name_;
  std::vector<std::string> args_;

public:
  ProtoTypeAST(const std::string &name, std::vector<std::string> args)
      : name_(name), args_(std::move(args)) {}

  const std::string &get_name() { return name_; }
};

struct FunctionAST {
private:
  std::unique_ptr<ProtoTypeAST> proto_;
  std::unique_ptr<ExprAST> body_;

public:
  FunctionAST(std::unique_ptr<ProtoTypeAST> proto,
              std::unique_ptr<ExprAST> body)
      : proto_(std::move(proto)), body_(std::move(body)) {}
};

static int cur_tok;
inline static int get_next_token() { return cur_tok = get_tok(); }

std::unique_ptr<ExprAST> log_err(const char *str) {
  fprintf(stderr, "logerr: %s \n", str);
  return nullptr;
}

std::unique_ptr<ProtoTypeAST> log_err_p(const char *str) {
  log_err(str);
  return nullptr;
}

static std::unique_ptr<ExprAST> parse_num_expr() {
  auto res = std::make_unique<NumberAST>(lexer::num_val);
  get_next_token();
  return std::move(res);
}

static std::unique_ptr<ExprAST> parse_expr();

static std::unique_ptr<ExprAST> parse_parent_expr() {
  get_next_token();
  auto v = parse_expr();
  if (!v)
    return nullptr;
  if (cur_tok != ')')
    return log_err("expected ')' ");
  get_next_token();
  return v;
}

static std::unique_ptr<ExprAST> parse_identifier_expr() {
  std::string id_name = identifier_str;
  get_next_token();

  if (cur_tok != '(')
    return std::make_unique<VariableAST>(id_name);

  get_next_token();
  std::vector<std::unique_ptr<ExprAST>> args;
  if (cur_tok != ')') {
    while (1) {
      if (auto arg = parse_expr()) {
        args.push_back(std::move(arg));
      } else {
        return nullptr;
      }

      if (cur_tok == ')') {
        break;
      }

      if (cur_tok != ',') {
        return log_err("expected '(' or ',' in the arg list");
      }

      get_next_token();
    }
  }

  get_next_token();
  return std::make_unique<CallExprAST>(id_name, std::move(args));
}

static std::unique_ptr<ExprAST> parse_primary() {
  switch (cur_tok) {
  default:
    return log_err("unkown token when expecting an expression");
  case token::tok_identifier:
    return parse_identifier_expr();
  case token::tok_number:
    return parse_num_expr();
  case '(':
    return parse_parent_expr();
  }
}

static std::map<char, int> binary_oper;

static int get_tok_precedence() {
  if (!isascii(cur_tok))
    return -1;
  int tok_prec = binary_oper[cur_tok];
  if (tok_prec <= 0)
    return -1;
  return tok_prec;
}

static std::unique_ptr<ExprAST> parse_bin_op_rhs(int expr_prec,
                                                 std::unique_ptr<ExprAST> LHS);

static std::unique_ptr<ExprAST> parse_expr() {
  auto LHS = parse_primary();
  if (!LHS)
    return nullptr;
  return parse_bin_op_rhs(0, std::move(LHS));
}

static std::unique_ptr<ExprAST> parse_bin_op_rhs(int expr_prec,
                                                 std::unique_ptr<ExprAST> LHS) {
  while (1) {
    int tok_prec = get_tok_precedence();
    if (tok_prec < expr_prec)
      return LHS;
    int bin_op = cur_tok;
    get_next_token();

    auto RHS = parse_primary();
    if (!RHS)
      return nullptr;

    int next_prec = get_tok_precedence();

    if (tok_prec < next_prec) {
      LHS = std::make_unique<BinaryAST>(bin_op, std::move(LHS), std::move(RHS));
    }

    next_prec = get_tok_precedence();
    if (tok_prec < next_prec) {
      RHS = parse_bin_op_rhs(tok_prec + 1, std::move(RHS));
      if (!RHS)
        return nullptr;
    }

    LHS = std::make_unique<BinaryAST>(bin_op, std::move(LHS), std::move(RHS));
  }
}

static std::unique_ptr<ProtoTypeAST> parse_proto_type() {
  if (cur_tok != token::tok_identifier)
    return log_err_p("expection function in prototype");

  std::string fn_name = identifier_str;
  get_next_token();
  if (cur_tok != '(')
    return log_err_p("expected '(' in prototype ");

  std::vector<std::string> args_names;
  while (get_next_token() == token::tok_identifier)
    args_names.push_back(identifier_str);
  if (cur_tok != ')')
    return log_err_p("expected ')' in prototype");
  get_next_token();
  return std::make_unique<ProtoTypeAST>(fn_name, std::move(args_names));
}

static std::unique_ptr<FunctionAST> parse_definition() {
  get_next_token();
  auto proto = parse_proto_type();
  if (!proto)
    return nullptr;
  if (auto E = parse_expr())
    return std::make_unique<FunctionAST>(std::move(proto), std::move(E));
  return nullptr;
}

static std::unique_ptr<ProtoTypeAST> parse_extern() {
  get_next_token();
  return parse_proto_type();
}

static std::unique_ptr<FunctionAST> parse_top_level_expr() {
  if (auto E = parse_expr()) {
    auto proto = std::make_unique<ProtoTypeAST>("", std::vector<std::string>());
    return std::make_unique<FunctionAST>(std::move(proto), std::move(E));
  }
  return nullptr;
}

static void handle_def() {
  if (parse_expr()) {
    fprintf(stderr, "parse function def. \n");
  } else {
    get_next_token();
  }
}

static void handle_exter() {
  if (parse_expr()) {
    fprintf(stderr, "parse an extern \n");
  } else {
    get_next_token();
  }
}

static void handle_top_level_expr() {
  if (parse_top_level_expr()) {
    fprintf(stderr, "parsed a top level expr \n");
  } else {
    get_next_token();
  }
}

static void main_loop() {
  while (1) {
    fprintf(stderr, "ready> ");
    switch (cur_tok) {
    case token::tok_eof:
      return;
    case ';':
      get_next_token();
      break;
    case token::tok_def:
      handle_def();
      break;
    case token::tok_exter:
      handle_exter();
      break;
    default:
      handle_top_level_expr();
      break;
    }
  }
}

}; // namespace ast
