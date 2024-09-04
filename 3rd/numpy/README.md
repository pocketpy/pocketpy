# numpy

### How to run **numpy** module programs with **gsoc-2024-dev** [pybind11](https://github.com/pocketpy/gsoc-2024-dev/tree/main/pybind11)

1. Prepare the python code file with the numpy operations you want to run. \
\
 For example : let's try out numpy [arange](https://numpy.org/doc/stable/reference/generated/numpy.arange.html) function in `test_numpy.py`
```py
  import numpy_bindings as np
  
  def test_arange(n):
      a = np.arange(n)
      print(a.sum())
  
  test_arange(100)
  ```

2. Read the script and execute it in `test_numpy.cpp`. 
```cpp
  #include <pybind11/embed.h>
  #include <fstream>
  #include <sstream>
  #include <string>
  
  namespace py = pybind11;
  using namespace pybind11;
  
  int main() {
      py::scoped_interpreter guard{};
      std::ifstream file("test_numpy.py");
      std::stringstream buffer;
      buffer << file.rdbuf();
      std::string script = buffer.str();
      py::exec(script);
  
      return 0;
  }
```

3. Build the project at root to generate the executable at `build/gsoc2024`.
```sh
  cmake -B build
  cmake --build build
```
4. Now run the executable to get the output. 
```sh
  |base| gsoc-2024-dev ±|main ✗|→ build/gsoc2024 
  4950
```
