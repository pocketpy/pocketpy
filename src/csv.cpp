#include "pocketpy/csv.h"

namespace pkpy{

void add_module_csv(VM *vm){
    PyObject* mod = vm->new_module("csv");

    vm->bind(mod, "reader(csvfile: list[str]) -> list[list]", [](VM* vm, ArgsView args){
        const List& csvfile = CAST(List&, args[0]);
        List ret;
        for(int i=0; i<csvfile.size(); i++){
            std::string_view line = CAST(Str&, csvfile[i]).sv();
            if(i == 0){
                // Skip utf8 BOM if there is any.
                if (strncmp(line.data(), "\xEF\xBB\xBF", 3) == 0) line = line.substr(3);
            }
            List row;
            int j;
            bool in_quote = false;
            std::string buffer;
__NEXT_LINE:
            j = 0;
            while(j < line.size()){
                switch(line[j]){
                    case '"':
                        if(in_quote){
                            if(j+1 < line.size() && line[j+1] == '"'){
                                buffer += '"';
                                j++;
                            }else{
                                in_quote = false;
                            }
                        }else{
                            in_quote = true;
                        }
                        break;
                    case ',':
                        if(in_quote){
                            buffer += line[j];
                        }else{
                            row.push_back(VAR(buffer));
                            buffer.clear();
                        }
                        break;
                    case '\r':
                        break;  // ignore
                    default:
                        buffer += line[j];
                        break;
                }
                j++;
            }
            if(in_quote){
                if(i == csvfile.size()-1){
                    vm->ValueError("unterminated quote");
                }else{
                    buffer += '\n';
                    i++;
                    line = CAST(Str&, csvfile[i]).sv();
                    goto __NEXT_LINE;
                }
            }
            row.push_back(VAR(buffer));
            ret.push_back(VAR(std::move(row)));
        }
        return VAR(std::move(ret));
    });

    vm->bind(mod, "DictReader(csvfile: list[str]) -> list[dict]", [](VM* vm, ArgsView args){
        PyObject* csv_reader = vm->_modules["csv"]->attr("reader");
        PyObject* ret_obj = vm->call(csv_reader, args[0]);
        const List& ret = CAST(List&, ret_obj);
        if(ret.size() == 0){
            vm->ValueError("empty csvfile");
        }
        List header = CAST(List&, ret[0]);
        List new_ret;
        for(int i=1; i<ret.size(); i++){
            const List& row = CAST(List&, ret[i]);
            if(row.size() != header.size()){
                vm->ValueError("row.size() != header.size()");
            }
            Dict row_dict(vm);
            for(int j=0; j<header.size(); j++){
                row_dict.set(header[j], row[j]);
            }
            new_ret.push_back(VAR(std::move(row_dict)));
        }
        return VAR(std::move(new_ret));
    });
}

}   // namespace pkpy