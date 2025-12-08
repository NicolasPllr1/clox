#include "scanner.h"
#include <ctype.h>
#include <stdbool.h>
#include <string.h>

typedef struct {
  const char *start;
  const char *current;
  int line;
} Scanner;

Scanner scanner;

void initScanner(const char *source) {
  scanner.start = source;
  scanner.current = source;
  scanner.line = 1;
}

static bool isAtEnd() { return *scanner.current == '\0'; }

static Token errorToken(const char *msg) {
  Token token;
  token.type = TOKEN_ERROR;
  token.start = msg;
  token.length = (int)strlen(msg);
  token.line = scanner.line;
  return token;
}

static Token makeToken(TokenType type) {
  Token token;
  token.type = type;
  token.start = scanner.start;
  token.length = (int)(scanner.current - scanner.start);
  token.line = scanner.line;
  return token;
}

static bool match(char expected) {
  if (isAtEnd())
    return false;
  if (*scanner.current != expected)
    return false;
  scanner.current++;
  return true;
}

static char advance() {
  scanner.current++;
  return scanner.current[-1];
}

static char peek() { return *scanner.current; }
static char peekNext() {
  if (isAtEnd())
    return '\0';
  else
    return *(scanner.current + 1);
}

static void skipWhitespace() {
  for (;;) {
    char c = peek();
    switch (c) {
    case ' ':
    case '\r':
    case '\t':
      advance();
      break;
    case '\n':
      scanner.line++;
      advance();
      break;
    case '/':
      if (peekNext() == '/') {
        while (peek() != '\n' && !isAtEnd()) {
          advance();
        }
      } else {
        return;
      }
      break;
    default:
      return;
    }
  }
}

static Token string() {
  while (peek() != '"' && !isAtEnd()) {
    if (peek() == '\n')
      scanner.line++;
    advance();
  }
  if (isAtEnd())
    return errorToken("Unterminated string");

  // Closing the quote
  advance();
  return makeToken(TOKEN_STRING);
}

static bool isDigit(char c) { return '0' <= c && c <= '9'; }

static Token number() {
  while (isDigit(peek()))
    advance();

  // Check for '.' (fraction)
  if (peek() == '.' && isDigit(peekNext())) {
    // consume the '.'
    advance();

    // Consume the decimal part
    while (isDigit(peek()))
      advance();
  }
  return makeToken(TOKEN_NUMBER);
}

static bool isAlpha(char c) {
  return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || (c == '_');
}

static TokenType checkKeyword(int prefix_len, int suffix_len,
                              const char *suffix, TokenType type) {
  // Prefix has already been checked. Now checking if suffix matches.
  if (scanner.current - scanner.start == prefix_len + suffix_len &&
      memcmp(scanner.start + prefix_len, suffix, suffix_len) == 0) {
    // (strcmp(suffix, scanner.start + offset) == 0)
    return type;
  }
  return TOKEN_IDENTIFIER;
}

static TokenType identifierType() {
  switch (scanner.start[0]) {
  case ('a'):
    return checkKeyword(1, 2, "nd", TOKEN_AND);
  case ('c'):
    return checkKeyword(1, 4, "lass", TOKEN_CLASS);
  case ('e'):
    return checkKeyword(1, 3, "lse", TOKEN_ELSE);
  case ('f'):
    if (scanner.current - scanner.start > 1) {
      switch (scanner.start[1]) {
      case 'a':
        return checkKeyword(2, 3, "lse", TOKEN_FALSE);
      case 'o':
        return checkKeyword(2, 1, "r", TOKEN_FOR);
      case 'u':
        return checkKeyword(2, 1, "n", TOKEN_FUN);
      }
    }
    break;
  case ('i'):
    return checkKeyword(1, 1, "f", TOKEN_IF);
  case ('n'):
    return checkKeyword(1, 2, "il", TOKEN_NIL);
  case ('o'):
    return checkKeyword(1, 1, "r", TOKEN_OR);
  case ('p'):
    return checkKeyword(1, 4, "rint", TOKEN_PRINT);
  case ('r'):
    return checkKeyword(1, 5, "eturn", TOKEN_RETURN);
  case ('s'):
    return checkKeyword(1, 4, "uper", TOKEN_SUPER);
  case ('t'):
    if (scanner.current - scanner.start > 1) {
      switch (scanner.start[1]) {
      case 'h':
        return checkKeyword(2, 2, "is", TOKEN_THIS);
      case 'r':
        return checkKeyword(2, 2, "ue", TOKEN_TRUE);
      }
    }
    break;
  case ('v'):
    return checkKeyword(1, 2, "ar", TOKEN_VAR);
  case ('w'):
    return checkKeyword(1, 4, "hile", TOKEN_WHILE);
  }
  return TOKEN_IDENTIFIER;
}

static Token identifier() {
  while (isalpha(peek()) || isDigit(peek()))
    advance();
  return makeToken(identifierType());
}

Token scanToken() {
  skipWhitespace();
  scanner.start = scanner.current;
  if (isAtEnd()) {
    return makeToken(TOKEN_EOF);
  }
  char c = advance();

  if (isDigit(c))
    return number();
  if (isAlpha(c))
    return identifier();

  switch (c) {
  case '(':
    return makeToken(TOKEN_LEFT_PAREN);
  case ')':
    return makeToken(TOKEN_RIGHT_PAREN);
  case '{':
    return makeToken(TOKEN_LEFT_BRACE);
  case '}':
    return makeToken(TOKEN_RIGHT_BRACE);
  case ';':
    return makeToken(TOKEN_SEMICOLON);
  case ',':
    return makeToken(TOKEN_COMMA);
  case '.':
    return makeToken(TOKEN_DOT);
  case '-':
    return makeToken(TOKEN_MINUS);
  case '+':
    return makeToken(TOKEN_PLUS);
  case '/':
    return makeToken(TOKEN_SLASH);
  case '*':
    return makeToken(TOKEN_STAR);
  case '!':
    return makeToken(match('=') ? TOKEN_BANG_EQUAL : TOKEN_BANG);
  case '=':
    return makeToken(match('=') ? TOKEN_EQUAL_EQUAL : TOKEN_EQUAL);
  case '<':
    return makeToken(match('=') ? TOKEN_LESS_EQUAL : TOKEN_LESS);
  case '>':
    return makeToken(match('=') ? TOKEN_GREATER_EQUAL : TOKEN_GREATER);
  case '"':
    return string();
  }

  return errorToken("Unexpected character.");
}
