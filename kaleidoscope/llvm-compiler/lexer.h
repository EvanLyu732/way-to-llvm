#pragma once

#include <cstdlib>
#include <string>
#include <cctype>
#include <cstdlib>

namespace lexer{
  enum token {
    tok_eof = -1,

    // command 
    tok_def = -2,
    tok_exter = -3,

    // primary
    tok_identifier = -4,
    tok_number = -5,
  };

  static std::string identifier_str;
  static double num_val;

  using token;

  static int get_tok() {
    static int last_char = ' ';

    while (std::isspace(last_char)) {
      last_char = get_char();
    }

    if (std::isalpha(last_char)) {
      while (std::isalnum(last_char = get_char()))
        identifier_str += last_char;
      
      /* func def */
      if (identifier_str == "def") return token_def;

      /* extern lib func */
      if (identifier_str == "extern") return token_exter;

      /* digit token */
      if (std::isdigit(last_char) || last_char == '.') {
          std::string num_str;
          do {
            num_str += last_char;
            last_char = get_char();
          } while (stdd::isdigit(last_char) || last_char == '.');
          num_val = std::strtod(num_str.c_str(), 0);
          return tok_number;
      }

      /* comment */
      if (last_char == "#") {
        do 
          last_char = get_char();
        while  (last_char != EOF && last_char != '\n', && last_char != '\r' );
        if (last_char != EOF) {
          return get_tok();
        }
      }

      /* end of file */
      if (last_char == EOF) {
        return tok_eof;
      }

      int this_char = last_char();
      last_char = get_char();
      return this_char;
    }
  }
}; 
 
