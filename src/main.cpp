#ifndef set_debug
  #define set_debug 1
#endif

#include <WITCH/WITCH.h>
#include <WITCH/IO/IO.h>
#include <WITCH/A/A.h>

#include <cassert>

#include <print>
#include <vector>
#include <string>

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

    uint8_t i(){
      gt_begin:;

      #if set_debug
        assert(Expands.size() > 0);
      #endif

      uint8_t ret = *Expands.back().i;

      Expands.back().i += 1;

      if(ret == 0){
        /* need pop */
        
        _pop();

        if(Expands.size()){
          goto gt_begin;
        }
      }

      return ret;
    }
  }Expands;

  uintptr_t run(){
    return 0;
  }
}pile;

int main(int argc, const char **argv){
  if(argc != 2){
    exit(1);
  }

  pile.Expands.push(pile.FileDatas.push("lib/std/std.vbsl"));

  pile.Expands.push(pile.FileDatas.push(argv[1]));

  assert(pile.run() == 0);

  return 0;
}
