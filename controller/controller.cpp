#include <dlfcn.h>
#include <pybind11/pybind11.h>

static void (*injector_set_watch)(bool) = nullptr;

void SetWatch(bool tag) {
  assert(injector_set_watch && "Injector not inited.");
  injector_set_watch(tag);
}

void Init() {
  auto handle = dlopen(nullptr, RTLD_LAZY);
  injector_set_watch =
      (decltype(injector_set_watch))dlsym(handle, "__injector_set_watch");
}

PYBIND11_MODULE(Controller, m) {
  m.doc() = "Injector Plugin.";

  m.def("init", &Init, "Init first.");

  m.def("set_watch", &SetWatch, "Control Injector working...");
}
