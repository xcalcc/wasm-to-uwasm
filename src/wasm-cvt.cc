/*
 * Copyright 2017 WebAssembly Community Group participants
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <cassert>
#include <cinttypes>
#include <cstdio>
#include <cstdlib>

#include "src/binary-reader.h"
#include "src/binary-reader-ir.h"
// #include "src/error-formatter.h"
#include "src/ir.h"
#include "src/option-parser.h"
#include "src/stream.h"
#include "basics.h" // basics utilities
#include "convertor.h"

using namespace wabt;

static int s_verbose;
static std::string s_infile;
static Features s_features;
static bool s_read_debug_names = true;
static bool s_fail_on_custom_section_error = true;
static bool s_dis = false;
static std::unique_ptr<FileStream> s_log_stream;

static const char s_description[] =
R"(  Read a file in the WebAssembly binary format, and read into an IR file,
     then do transcode to uWASM, generate a uWASM file if needed.

examples:
  # transcode binary file test.wasm
  $ wasm_cvt 0xffff test.wasm test.uwasm
)";

static void ParseOptions(int argc, char** argv) {
  OptionParser parser("wasm_cvt", s_description);

  parser.AddOption('v', "verbose", "Use multiple times for more info", []() {
    s_verbose++;
    s_log_stream = FileStream::CreateStdout();

  });
  s_features.AddOptions(&parser);
  parser.AddOption("no-debug-names", "Ignore debug names in the binary file",
                   []() { s_read_debug_names = false; });
  parser.AddOption("ignore-custom-section-errors",
                   "Ignore errors in custom sections",
                   []() { s_fail_on_custom_section_error = false; });
  parser.AddOption('d', "disassemble", "Print out the disassemble messages",
                   []() { s_dis = true; });


  parser.AddArgument("trace", OptionParser::ArgumentCount::One,
                     [](const char* argument) {
                       UINT64 s_trace_opt = strtol(argument, NULL, 16);
                       for (UINT32 i = 0; i < 64; i++) {
                         UINT64 tk = 1 << i;
                         UINT32 masked = (s_trace_opt & tk) ? 1000 : 0;
                         Set_tracing_opt((TRACE_KIND) i,
                                         masked);
                       }
                     });

  parser.AddArgument("filename", OptionParser::ArgumentCount::One,
                     [](const char* argument) {
                       s_infile = argument;
                       ConvertBackslashToSlash(&s_infile);
                     });

  parser.AddArgument("output", OptionParser::ArgumentCount::One,
                     [](const char *arg) {
                       Is_True(arg != nullptr, ("Argument should not be null"));
                       Get_conv_opt().Set_output_name(arg);
                     });

  parser.Parse(argc, argv);
}

int ProgramMain(int argc, char** argv) {
  Result result;

  InitStdio();
  ParseOptions(argc, argv);

  std::vector<uint8_t> file_data;
  result = ReadFile(s_infile.c_str(), &file_data);
  if (Succeeded(result)) {
    Errors errors;
    Module module;
    const bool kStopOnFirstError = true;
    ReadBinaryOptions options(s_features, s_log_stream.get(),
                              s_read_debug_names, kStopOnFirstError,
                              s_fail_on_custom_section_error);
    FILE_CONTEXT ctx;
    ctx.Set_kind(FILE_WASM);
    Get_conv_opt().Set_conv_in_read(false);
    result = ReadBinaryIr(s_infile.c_str(), file_data.data(), file_data.size(),
                          options, &errors, &module);
    ctx.Set_module(module);
    if (Succeeded(result)) {
      // Process all functions.
      ctx.Set_func_ctx(0);
      wabt::Func *func = ctx.Get_func();
      while (func) {
        Is_Trace(Tracing(TP_PARSE),
                 (TFile, "Converting func : %s, bin = 0x%016llx, size = %lu\n",
                     func->name.c_str(), (UINT64) func->wasm_bin, func->wasm_bin_size));
        if (func->wasm_bin != NULL) {
          Read_insts(func->wasm_bin, func->wasm_bin_size, ctx);
        }
        func = ctx.Next_func();
      }
      return 0;
    }
    // FormatErrorsToFile(errors, Location::Type::Binary);
    Is_True(false, ("Error occurred."));
    exit(1);
  }
  return result != Result::Ok;
}

int main(int argc, char** argv) {
  WABT_TRY
  return ProgramMain(argc, argv);
  WABT_CATCH_BAD_ALLOC_AND_EXIT
}
