
#ifndef _LIBUBH_H
#define _LIBUBH_H


#include "ubh.h"   // all basic typedef of uWasm binary header

class U_MODULE {
private:
  UBH_MHDR      _module_hdr;     // module header
  UBH_SHDR      _sec_hdr;        // section header

  UBH_FUNCTYPE  _ty_sec;         // type section(function type)
  UBH_IMPORT    _import_sec;     // import section
  // TODO: Function section
  // TODO: Table section
  // TODO: Memory section
  // TODO: Global section
  UBH_EXPORT     _export_sec;     // export section
  // Element section?
  UBH_CODE       _code_sec;       // code section
  UBH_DATA       _data_sec;       // data section?
  vector<string> _module_str;     // module string
  vector<string> _name_str;       // module string
  vector<string> _non_null_str;   // module string
public:
  U_MODULE() {};

  const UBH_MHDR       &Get_module_hdr()   const            { return _module_hdr; }
  const UBH_SHDR       &Get_sec_hdr()      const            { return _sec_hdr; }
  const UBH_FUNCTYPE   &Get_ty_sec()       const            { return _ty_sec; }
  const UBH_IMPORT     &Get_import_sec()   const            { return _import_sec; }
  const UBH_EXPORT     &Get_export_sec()   const            { return _export_sec; }
  const UBH_CODE       &Get_code_sec()     const            { return _code_sec; }
  const UBH_DATA       &Get_data_sec()     const            { return _data_sec; }
  const vector<string> &Get_module_str()   const            { return _module_str; }
  const vector<string> &Get_name_str()     const            { return _name_str; }
  const vector<string> &Get_non_null_str() const            { return _non_null_str; }

  void Set_module_hdr(const UBH_MHDR &module_hdr)           { _module_hdr = module_hdr; }
  void Set_sec_hdr(const UBH_SHDR &sec_hdr)                 { _sec_hdr = sec_hdr; }
  void Set_ty_sec(const UBH_FUNCTYPE &ty_sec)               { _ty_sec = ty_sec; }
  void Set_import_sec(const UBH_IMPORT &import_sec)         { _import_sec = import_sec; }
  void Set_export_sec(const UBH_EXPORT &export_sec)         { _export_sec = export_sec; }
  void Set_code_sec(const UBH_CODE &code_sec)               { _code_sec = code_sec; }
  void Set_data_sec(const UBH_DATA &data_sec)               { _data_sec = data_sec; }
  void Set_module_str(const vector<string> &module_str)     { _module_str = module_str; }
  void Set_name_str(const vector<string> &name_str)         { _name_str = name_str; }
  void Set_non_null_str(const vector<string> &non_null_str) { _non_null_str = non_null_str; }
};


#endif  // _LIBUBH_H
