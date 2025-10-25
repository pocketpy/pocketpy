class Json:

    def __init__(self, filename:str = None, dict_obj:dict = None, array:list = None, simple:(int|str|float) = None) -> None:
        self.file = filename
        self.dict_obj = dict_obj
        self.array = array
        self.simple = simple
        

    def get_type(self):
        if self.simple != None:
            return "simple"
        elif self.array != None :   
            return "array"
        else :
            return "associative array"

    def print_json(self):
        if self.simple != None:
            print(self.simple)
        elif self.array != None :   
            print(self.array)
        else :
            print(self.dict_obj)
    
    def get_json(self) -> (dict | list | float | int | str):
        if self.simple != None:
            return self.simple
        elif self.array != None:   
            return self.array
        else :
            return self.dict_obj
    

    def serialize(self,file_output:str):
        with open(file_output,'w') as file:
            if self.dict_obj != None:    
                self.serialize_dict(file = file, diz = self.dict_obj)
            elif self.array != None:
                self.serialize_array(file = file, array = self.array )
    
    def serialize_array(self, file, array:list):        

        file.write("[") 
        
        size = len(array)

        line = 0
        for i in array:
            if line != size-1 :
                if type(i) == dict:
                    self.serialize_dict(file,i)
                    file.write(",")
                elif type(i) == list:
                    self.serialize_array(file,i)
                    file.write(",")
                else:
                    file.write(str(self.serialize_simple(i)) + ",")
            else :
                if type(i) == dict:
                    self.serialize_dict(file,i)
                elif type(i) == list:
                    self.serialize_array(file ,i)
                else:
                    file.write(str(self.serialize_simple(i)))
            line += 1

        
        file.write("]")   


    def serialize_dict(self,file,diz):

        file.write("{")    
        
        size = len(diz)
        line = 0
        for i in diz:
            if line != size-1 :
                if type(diz[i]) == dict:
                    file.write(f"\"{i}\":")
                    self.serialize_dict(file,diz[i])
                    file.write(",")
                elif type(diz[i]) == list:
                    file.write(f"\"{i}\":")
                    self.serialize_array(file,diz[i])
                    file.write(",")
                else:
                    file.write(f"\"{i}\":{str(self.serialize_simple(diz[i]))},")
            else :
                if type(diz[i]) == dict:
                    file.write(f"\"{i}\":")
                    self.serialize_dict(file,diz[i])
                elif type(diz[i]) == list:
                    file.write(f"\"{i}\":")
                    self.serialize_array(file,diz[i])
                else:
                    file.write(f"\"{i}\":{str(self.serialize_simple(diz[i]))}")
            line += 1
            
         
        file.write("}")


    def serialize_simple(self,value):
        if value == True:
            return "true"
        elif value == False:
            return "false"
        elif value == None:
            return "null"
        elif type(value) == list:
            return value
        elif type(value) == int or type(value) == float:
            return value
        elif type(value) == str:
            return f"\"{value}\""


    def parse_dict(self,file):
        diz = {}
        chunk = '{'
        key  = ""   
             
        while True: 
            chunk = file.read(1)
            if chunk == '{':
                diz[key] = self.parse_dict(file)
            elif chunk == '"' and len(key) == 0:
                value = self.parse_simple(file,chunk)
                key = value[0]
            elif chunk == ',':
                key = ""
            elif chunk.isspace():
                pass
            elif chunk == '}':
                return diz
            elif chunk == ':':
                pass
            elif chunk == "[":
                diz[key] = self.parse_array(file)
            else:
                value = self.parse_simple(file, chunk)                
                if value[1] == '}':
                    diz[key] = (value[0])
                    return diz
                
                diz[key] = (value[0])
                

    def parse_array(self,file):
        arr = []
        chunk = '['
        while True: 
            chunk = file.read(1)
            if chunk == '[':
                arr.append(self.parse_array(file))
            elif chunk == ',' or chunk.isspace():
                pass
            elif chunk == ']':
                return arr
            elif chunk == '{':
                arr.append(self.parse_dict(file))
            else:
                value = self.parse_simple(file, chunk)
                if value[1] == ']':
                    arr.append(value[0])
                    return arr
                arr.append(value[0])
    
    def parse_simple(self,file,ch:str):
        token = ch
        chunk = ch
        while True :  
            if(chunk.isspace()):
                pass
            elif(chunk.isdigit()):
                dot = False
                chunk = file.read(1)
                while chunk.isdigit():
                    token +=chunk 
                    chunk = file.read(1)
                    if chunk == '.':
                        dot = True
                        token +=chunk 
                        chunk = file.read(1)
                        continue
                if dot == True:
                    return float(token),chunk
                else :
                    return int(token),chunk
            elif chunk == 't' or chunk == 'f': 

                while chunk != 'e' :
                    token +=chunk 
                    chunk = file.read(1)
                
                if token[1:] + 'e' == "true":
                    return True, chunk
                else:
                    return False, chunk
            elif chunk == 'n':
                for i in range(4):
                    token +=chunk 
                    chunk = file.read(1)
            
                if "null" == token[1:]:
                    return None, chunk

            elif chunk == '"':
                chunk = file.read(1)
                
                while chunk != '"' :
                    token += chunk 
                    chunk = file.read(1)
                return token[1:], chunk

            chunk = file.read(1)



    def parse(self):
        
        with open(self.file,mode = "r") as file:
            chunk = file.read(1) 
            if(chunk.isspace()):
                pass
            elif chunk == '{':
                self.dict_obj = self.parse_dict(file)
            elif chunk == '[':
                self.array = self.parse_array(file)
            else:
                value = self.parse_simple(file,chunk)
                self.simple = value[0]
