#ifndef CONFIG_ONFIG_PARSER_HPP_
#define CONFIG_ONFIG_PARSER_HPP_

#include <list>
#include <string>

#include "config/config.hpp"
#include "config/location_conf.hpp"
#include "config/virtual_server_conf.hpp"

namespace config {

class Parser {
 public:
  Parser(const char *filename);
  Parser(const Parser &rhs);
  Parser &operator=(const Parser &rhs);
  ~Parser();

  // config: server (NEWLINE server)*;
  Config ParseConfig();

  class ParserException : public std::exception {
   public:
    ParserException(const char *errmsg = "Parser error.");
    const char *what() const throw();

   private:
    const char *errmsg_;
  };

 private:
  // 文法の詳細は docks/configuration.g4 に書いてある｡

  // server block
  // server: 'server' '{' directive+ '}';
  void ParseServerBlock(Config &config);

  // listen_directive: 'listen' WHITESPACE NUMBER END_DIRECTIVE;
  void ParseListenDirective(VirtualServerConf &vserver);

  // servername_directive: 'server_name' WHITESPACE DOMAIN_NAME+ END_DIRECTIVE;
  void ParseServerNameDirective(VirtualServerConf &vserver);

  // location block
  // location_directive: 'location' PATH '{' directive_in_location+ '}';
  void ParseLocationBlock(VirtualServerConf &vserver);

  void ParseAllowMethodDirective(LocationConf &location);

  void ParseClientMaxBodySizeDirective(LocationConf &location);

  void ParseRootDirective(LocationConf &location);

  void ParseIndexDirective(LocationConf &location);

  void ParseAutoindexDirective(LocationConf &location);

  void ParseIscgiDirective(LocationConf &location);

  void ParseReturnDirective(LocationConf &location);

  // Parser utils

  // 1文字content_buffer_[buf_idx_]を返して buf_idx_ を1進める
  char GetC();

  // buf_idx_ を1戻してbuffer_[buf_idx_]返す
  char UngetC();

  // 空白文字以外が見つかるまで buf_idx_ を進める
  void SkipSpaces();

  // 次のスペース(改行等含む)までの文字列を取得し返す｡
  // そしてその文字数分 buf_idx_ を進める｡
  // 現在の buf_idx_ の位置が 'server {' の1文字目だった場合､ "server" を返す｡
  std::string GetWord();

  // 符号なし整数かどうか
  bool IsUnsignedNumber(const std::string &str);

  // ドメイン名の条件に合っているか
  // DOMAIN_NAME: (ALPHABET | NUMBER)+
  //   | (ALPHABET | NUMBER)+ (ALPHABET | NUMBER | HYPHEN)* (ALPHABET| NUMBER)+;
  bool IsDomainName(const std::string &domain_name);

  // webservで利用可能なメソッドか
  // METHOD: 'GET' | 'POST' | 'DELETE';
  bool IsHttpMethod(const std::string &method);

  // "on" なら true, "off" なら false を返す｡
  bool ParseOnOff(const std::string &on_or_off);

  // file_content_ をすべて読み込んだか
  bool IsReachedEOF();

  // filename の内容を file_content_ に載せる
  void LoadFileData(const char *filename);

  std::string file_content_;
  size_t buf_idx_;
};

};  // namespace config

#endif
