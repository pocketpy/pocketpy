#include "pocketpy/csv.h"
#include "pocketpy/config.h"

namespace pkpy{

void add_module_csv(VM *vm){
    PyObject* mod = vm->new_module("csv");

    vm->bind(mod, "reader(csvfile: list[str]) -> list", [](VM* vm, ArgsView args){
        const List& csvfile = CAST(List&, args[0]);
        List ret;
        for(int i=0; i<csvfile.size(); i++){
            std::string_view line = CAST(Str&, csvfile[i]).sv();
            if(i == 0){
                // Skip utf8 BOM if there is any.
                if (strncmp(line.data(), "\xEF\xBB\xBF", 3) == 0) line = line.substr(3);
            }
            List row;
            int j = 0;
            bool in_quote = false;
            std::string buffer;
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
                vm->ValueError("unterminated quote");
            }
            row.push_back(VAR(buffer));
            ret.push_back(VAR(std::move(row)));
        }
        return VAR(std::move(ret));
    });
}

}   // namespace pkpy