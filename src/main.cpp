#ifndef set_debug
  #define set_debug 1
#endif

#include <WITCH/WITCH.h>
#include <WITCH/IO/IO.h>
#include <WITCH/A/A.h>
#include <WITCH/PR/PR.h>

#include "dme.h"

#include <print>
#include <vector>
#include <string>

#ifdef assert
  #undef assert
#endif
#define assert(v) if(!(v)){ __abort(); }

struct pile_t{
  struct FileDatas_t{
    struct FileData_t{
      uint8_t *ptr;
      uintptr_t size;
    };
    std::vector<FileData_t> FileDatas;

    const FileData_t& operator[](uintptr_t i){
      return FileDatas[i];
    }

    uintptr_t push(std::string full_path){
      IO_fd_t fd;
      assert(IO_open(full_path.c_str(), O_RDONLY, &fd) == 0);
    
      IO_stat_t s;
      assert(IO_fstat(&fd, &s) == 0);
    
      auto size = IO_stat_GetSizeInBytes(&s);

      FileData_t FileData = {
        .ptr = A_resize(NULL, size + 1),
        .size = (uintptr_t)size,
      };

      assert(IO_read(&fd, FileData.ptr, size) == size);

      FileData.ptr[size] = 0;

      FileDatas.push_back(FileData);

      return FileDatas.size() - 1;
    }

    ~FileDatas_t(){
      for(uintptr_t i = FileDatas.size() - 1; i--;){
        A_resize(FileDatas[i].ptr, 0);
      }
    }
  }FileDatas;

  struct Expands_t{
    struct Expand_t{
      const uint8_t *i;
      uintptr_t FileDataID;
    };
    std::vector<Expand_t> Expands;

    void _pop(){
      Expands.pop_back();
    }
    void push(uintptr_t FileDataID){
      auto pile = OFFSETLESS(this, pile_t, Expands);

      Expand_t Expand = {
        .i = pile->FileDatas[FileDataID].ptr,
        .FileDataID = FileDataID,
      };
      Expands.push_back(Expand);
    }

    void _if_possible_seek_to_nonzero(){
      #if set_debug
        assert(Expands.size() > 0);
      #endif

      while(*Expands.back().i == 0){
        if(Expands.size() == 1){
          break;
        }

        _pop();
      }
    }

    uint8_t gc(){
      #if set_debug
        assert(Expands.size() > 0);
      #endif

      return *Expands.back().i;
    }

    uint8_t i(){
      #if set_debug
        assert(Expands.size() > 0);
      #endif

      auto ret = gc();

      Expands.back().i += 1;
      
      while(*Expands.back().i == 0){
        if(Expands.size() == 1){
          break;
        }

        _pop();
      }

      return ret;
    }

    void SkipTillSomething(){
      while(1){
        auto r = gc();
        if(
          r != ' '
          && r != '\r'
          && r != '\n'
          && r != '\t'
        ){
          break;
        }

        i();
      }
    }
  }Expands;

  _dme(keywords, __empty_struct
    ,_struct
    ,_include
    ,_let
    ,_fn
  );

  uintptr_t run(){
    auto& e = Expands;
    e._if_possible_seek_to_nonzero();

    struct token_t{
      enum class t_t{
        keyword,
        number,
        dot,
        plus_or_minus,
        string,
        character,
        cbracket_open,
        cbracket_close,
        sbracket_open,
        sbracket_close,
        parenthese_open,
        parenthese_close,
        semicolon,
      };
      t_t t;
      std::string data;
    };
    auto parse_token = [&](){
      auto t = (token_t::t_t)-1;
      std::string data;

      if(STR_ischar_char(e.gc()) || e.gc() == '_'){
        t = token_t::t_t::keyword;
      }
      else if(STR_ischar_digit(e.gc())){
        t = token_t::t_t::number;
      }
      else if(e.gc() == '.'){
        t = token_t::t_t::dot;
        e.i();
      }
      else if(e.gc() == '-' || e.gc() == '+'){
        t = token_t::t_t::plus_or_minus;
        data.push_back(e.gc());
        e.i();
      }
      else if(e.gc() == '"'){
        t = token_t::t_t::string;
        e.i();
        while(1){
          auto c = e.gc();
          if(c == '"'){
            e.i();
            break;
          }
          if(c == 0){
            __abort();
          }
          data.push_back(c);
          e.i();
        }
      }
      else if(e.gc() == '\''){
        t = token_t::t_t::character;
        e.i();
        while(1){
          auto c = e.gc();
          if(c == '\''){
            e.i();
            break;
          }
          if(c == 0){
            __abort();
          }
          data.push_back(c);
          e.i();
        }
      }
      else if(e.gc() == '{'){
        t = token_t::t_t::cbracket_open;
        e.i();
      }
      else if(e.gc() == '}'){
        t = token_t::t_t::cbracket_close;
        e.i();
      }
      else if(e.gc() == '['){
        t = token_t::t_t::sbracket_open;
        e.i();
      }
      else if(e.gc() == ']'){
        t = token_t::t_t::sbracket_close;
        e.i();
      }
      else if(e.gc() == '('){
        t = token_t::t_t::parenthese_open;
        e.i();
      }
      else if(e.gc() == ')'){
        t = token_t::t_t::parenthese_close;
        e.i();
      }
      else if(e.gc() == ';'){
        t = token_t::t_t::semicolon;
        e.i();
      }
      #if set_debug
        else{
          std::println("first character was {}", (char)e.gc());
        }
      #endif

      assert(t != (token_t::t_t)-1);

      if(t == token_t::t_t::keyword || t == token_t::t_t::number){
        while(1){
          auto c = e.gc();
          if(STR_ischar_digit(c) || STR_ischar_char(c) || c == '_'){}
          else{
            break;
          }
          data.push_back(c);
          e.i();
        }
      }

      return (token_t){
        .t = t,
        .data = data,
      };
    };

    std::vector<token_t> tokens;

    struct struct_data_t{

    };
    std::vector<struct_data_t> structs;
    structs.push_back({});

    struct scope_t{
      uintptr_t StructID;
      std::vector<token_t> tokens;
    };
    std::vector<scope_t> scopes;
    scopes.push_back({
      .StructID = 0
    });

    while(1){
      e.SkipTillSomething();
      if(e.gc() == 0){
        break;
      }

      auto token = parse_token();
      scopes.back().tokens.push_back(token);
      std::println("{}", token.data);
      //if(token.t == token_t::t_t::cbracket_close)
    }
    return 0;
  }
}pile;

int main(int argc, const char **argv){
  if(argc != 2){
    exit(1);
  }

  pile.Expands.push(pile.FileDatas.push(argv[1]));

  pile.Expands.push(pile.FileDatas.push("lib/std/std.vbsl"));

  assert(pile.run() == 0);

  return 0;
}
