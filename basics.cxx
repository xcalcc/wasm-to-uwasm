//
// Created by xc5 on 2020/7/11.
//
#include "basics.h"
#include "stdarg.h"
#include "stdio.h"
#include "unistd.h"
#include "stdlib.h"
#include <map>

using std::map;
class TRACE_OPT_INFO {
public:
  UINT32 global_lvl;
  map<TRACE_KIND, UINT32> kind_to_level;
  UINT32 Get(TRACE_KIND tk) {
    if(kind_to_level.find(tk) == kind_to_level.end()) {
      return 0;
    }
    return kind_to_level[tk];
  }
  void Set(TRACE_KIND tk, UINT32 level) {
    kind_to_level.insert(std::make_pair(tk, level));
  }
  UINT32 Level() {
    return global_lvl;
  }
  TRACE_OPT_INFO() : kind_to_level() {
    // Initialize defaults.
    global_lvl = 100;
    Set(TP_BLK_WINS, 100);
    Set(TP_BLK, 100);
    Set(TP_STACK, 100);
  }
};

TRACE_OPT_INFO trace_info;

void Assertion_Failure_Print ( const char *fmt, ... )
{
  va_list vp;

  char msg[512];
  INT32 pos;

  /* Prepare header line: */
  va_start ( vp, fmt );
  pos = sprintf ( &msg[0], "### Assertion Failure : \n");
  pos += vsprintf ( &msg[pos], fmt, vp );
  sprintf ( &msg[pos], "\n" );
  va_end ( vp );
  fprintf(stderr, "%s\n", msg);
  exit(1);
}


void Comp_Failure_Print ( const char *fmt, ... )
{
  va_list vp;

  char msg[512];
  INT32 pos;

  /* Prepare header line: */
  va_start ( vp, fmt );
  pos = sprintf ( &msg[0], "Compilation failure: \n");
  pos += vsprintf ( &msg[pos], fmt, vp );
  sprintf ( &msg[pos], "\n" );
  va_end ( vp );
  fprintf(stderr, "%s\n", msg);
  exit(2);
}

/***
 *  Quit the processing with trace
 **/
void Quit_with_tracing(const char * file, UINT32 line, const char *func_name) {
  fprintf(stderr, "\n\n\n### In file %s:%d, func: %s\n", file, line, func_name);
}

bool Tracing(TRACE_KIND tc) {
  return trace_info.Get(tc) > trace_info.Level();
}

UINT32 Set_tracing_opt(TRACE_KIND kind, UINT32 level) {
  trace_info.Set(kind, level);
  return level;
}