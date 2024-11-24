#include <algorithm>
#include <bits/ranges_algo.h>
#include <cassert>
#include <cstdint>
#include <cstdio>
#include <dlfcn.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <list>
#include <memory>
#include <nlohmann/json_fwd.hpp>
#include <stack>
#include <string>
#include <string_view>
#include <vector>

#include <nlohmann/json.hpp>

using json = nlohmann::json;

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

  auto getSymbolAddr() const -> const uintptr_t {
    assert(childPtr >= baseAddr);
    return childPtr - baseAddr;
  }
};

// 设计一个结构体用于表示StackTrace的入栈出栈动作
struct StackTraceAction {
  StackTrace *traceNode;
  enum class Action { Enter, Exit } action;
};

struct Context {

  using SymbolRecord = std::tuple<uintptr_t, uintptr_t, std::string>;
  using SymbolTable = std::vector<SymbolRecord>;

  explicit Context() {}

  const auto getFilter() const { return *filter; }

  bool isWatch = false;
  std::stack<StackTrace *> stack{};
  std::vector<StackTraceAction> stackActionHistory{};

  // 这里做的是统一的内存管理
  std::list<std::unique_ptr<StackTrace>> traceNodes{};

  void clear() noexcept {
    assert(stack.empty());
    stackActionHistory.clear();
    traceNodes.clear();
  }

private:
  // std::unordered_map<std::string_view, std::unique_ptr<ElfTools::ElfDumper
  // *>>
  //     dumperMap{};

  // TODO: 更好的初始化
  std::unique_ptr<Filter *> filter =
      std::make_unique<Filter *>(new Filter{{"/home/yvesw"}});
};

void to_json(json &j, const StackTrace &n) {
  j = json{{"libName", n.libName},
           {"childPtr", n.childPtr},
           {"parentPtr", n.parentPtr},
           {"baseAddr", n.baseAddr},
           {"indent", n.indent}};
}

void to_json(json &j, const std::unique_ptr<StackTrace> &st_ptr) {
  j = json{{"id", reinterpret_cast<uintptr_t>(st_ptr.get())},
           {"value", *st_ptr}};
}

void to_json(json &j, const StackTraceAction &a) {
  j = json{{"traceNodeId", reinterpret_cast<uintptr_t>(a.traceNode)},
           {"action",
            a.action == StackTraceAction::Action::Enter ? "Enter" : "Exit"}};
}

void from_json(const json &j, StackTrace &n) {
  throw std::runtime_error("Not implemented");
}

void from_json(const json &j, StackTraceAction &a) {
  throw std::runtime_error("Not implemented");
}

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
      // TODO: 这样做是不准的
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

  // 这个列表并不能说明函数调用的顺序，他们顶多只能算是"节点"
  // 并且还有内存问题
  ctx->traceNodes.emplace_back(
      std::unique_ptr<Injector::StackTrace>(traceNode));
  ctx->stack.push(traceNode);

  // 我们在这里记录函数的变化路径，将进入和离开都记录一下吧！
  ctx->stackActionHistory.emplace_back(Injector::StackTraceAction{
      traceNode, Injector::StackTraceAction::Action::Enter});
}

void __cyg_profile_func_exit(void *child, void *parent) {
  if (not ctx->isWatch)
    return;

  uintptr_t childAddr = (uintptr_t)child;
  uintptr_t parentAddr = (uintptr_t)parent;

  assert(not ctx->stack.empty());

  auto traceNode = ctx->stack.top();

  // 这里没写完啊，我们需要在这里考虑mismatch的情况，目前我们先插入assertion
  assert(traceNode->parentPtr == parentAddr);

  ctx->stack.pop();
  ctx->stackActionHistory.emplace_back(Injector::StackTraceAction{
      traceNode, Injector::StackTraceAction::Action::Exit});
}

// export to python.
void __injector_set_watch(bool tag) {
  if (ctx->isWatch && not tag) {
    // std::cerr << "track finished.\n";
    assert(ctx->stack.empty());

    auto &history = ctx->stackActionHistory;

    json hist_j(history);
    json nodes_j(ctx->traceNodes);
    json db_j{{"nodes", nodes_j}, {"history", hist_j}};

    auto file = std::filesystem::path("trace.txt");
    auto fos = std::ofstream(file, std::ios::out);

    fos << db_j.dump(2) << "\n";

    fos.flush();
    fos.close();

    ctx->clear();
  }
  ctx->isWatch = tag;
}
}
