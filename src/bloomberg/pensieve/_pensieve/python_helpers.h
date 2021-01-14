#pragma once

#include <functional>
#include <memory>
#include <string>
#include <unordered_map>

#include "Python.h"

namespace pensieve::python_helpers {
class PyUnicode_Cache
{
  public:
    PyObject* getUnicodeObject(const std::string& str);

  private:
    using py_capsule_t = std::unique_ptr<PyObject, std::function<void(PyObject*)>>;
    std::unordered_map<std::string, py_capsule_t> d_cache{};
};
}  // namespace pensieve::python_helpers
