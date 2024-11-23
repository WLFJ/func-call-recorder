// #include "AddrToLine.h"
#include <algorithm>
#include <bits/ranges_algo.h>
#include <cassert>
#include <cstdio>
#include <dlfcn.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <list>
#include <memory>
#include <stack>
#include <string>
#include <string_view>
#include <vector>

namespace Injector {

using std::string;

class Filter {
public:
  Filter(std::vector<std::string> srcList) : srcList{srcList} {};

  bool acceptSrc(std::string_view srcFilePath) const {
    if (std::ranges::any_of(
            srcList, [&srcFilePath](const std::string &acceptStrPart) {
              return srcFilePath.find(acceptStrPart) != std::string::npos;
            }))
      return true;
    return false;
  }

private:
  std::vector<std::string> srcList{};
};

struct StackTrace {
  const std::string_view libName;
  const uintptr_t childPtr, parentPtr;
  const uintptr_t baseAddr;

  size_t indent = 0;

  std::string_view filePath{"INVALID_PATH"};
  size_t loc{};
  std::string funcName{"INVALID"};
};

struct Context {

  using SymbolRecord = std::tuple<uintptr_t, uintptr_t, std::string>;
  using SymbolTable = std::vector<SymbolRecord>;

  explicit Context() {}

  const auto getFilter() const { return *filter; }

  bool isWatch = false;
  std::stack<StackTrace *> stack{};
  std::list<std::unique_ptr<StackTrace *>> traceList{};

private:
  // std::unordered_map<std::string_view, std::unique_ptr<ElfTools::ElfDumper
  // *>>
  //     dumperMap{};

  // TODO: 更好的初始化
  std::unique_ptr<Filter *> filter =
      std::make_unique<Filter *>(new Filter{{"/home/yvesw"}});
};

} // namespace Injector

/**
 * 下面如此算是最小编程接口了.其实仔细想想,我们注入的目的,就是为了减少类似gdb带来的overhead(因为调试器会来回切换上下文)
 * 因此这种注入的办法相当于一个受限的gdb
 * 我们下一步希望在"断下来"的时候同时追踪到代码信息,这样我们就撕开了一个比较大的口子了.
 */
extern "C" {

Injector::Context *ctx;

void __attribute__((constructor)) inject_ctor(void) {
  ctx = new Injector::Context();
}

void __attribute__((destructor)) inject_dtor(void) { delete ctx; }

void __cyg_profile_func_enter(void *child, void *parent) {
  if (not ctx->isWatch)
    return;

  uintptr_t childAddr = (uintptr_t)child;
  uintptr_t parentAddr = (uintptr_t)parent;

  size_t indent = 0;

  if (not ctx->stack.empty()) [[likely]] {
    Injector::StackTrace *parentTrackNode = ctx->stack.top();
    indent = parentTrackNode->indent;
    if (parentTrackNode->childPtr == parentAddr) [[likely]] {
      indent++;
    } else {
      indent += 2;
    }
  }

  // check if chain is correct.

  Dl_info info;
  dladdr(child, &info);
  const std::string_view name = info.dli_fname;
  const auto baseAddr = (uintptr_t)info.dli_fbase;

  auto traceNode = new Injector::StackTrace{.libName = name,
                                            .childPtr = childAddr,
                                            .parentPtr = parentAddr,
                                            .baseAddr = baseAddr,
                                            .indent = indent};

  ctx->traceList.emplace_back(
      std::make_unique<Injector::StackTrace *>(traceNode));
  ctx->stack.push(traceNode);
}

void __cyg_profile_func_exit(void *child, void *parent) {
  if (not ctx->isWatch)
    return;

  assert(not ctx->stack.empty());

  auto traceNode = ctx->stack.top();

  ctx->stack.pop();
}

// export to python.
void __injector_set_watch(bool tag) {
  // 在停止的时候将结果导出
  if (ctx->isWatch && not tag) {
    // std::cerr << "track finished.\n";
    assert(ctx->stack.empty());

    const auto traceList = &ctx->traceList;

    auto file = std::filesystem::path("./trace.txt");
    auto fos = std::ofstream(file, std::ios::out);

    const Injector::StackTrace *lastTrace{};

    std::for_each(traceList->cbegin(), traceList->cend(),
                  [&fos, &lastTrace](const auto &traceNode) {
                    auto node = *traceNode;

                    fos << node->childPtr - node->baseAddr << "\n";
                  });

    fos.flush();
    fos.close();

    traceList->clear();
  }
  ctx->isWatch = tag;
}
}
