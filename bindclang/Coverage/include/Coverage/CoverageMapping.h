#pragma once

#include <llvm/ProfileData/Coverage/CoverageMapping.h>
#include <vector>

namespace Coverage {

using LCovMapping = llvm::coverage::CoverageMapping;

class CoverageMapping {
private:
  CoverageMapping() {};

public:
  static CoverageMapping
  LoadFromFile(const std::vector<std::string> &objFilenames,
               const std::string &profileFilename);

  CoverageMapping(std::unique_ptr<LCovMapping> &&mapping)
      : _mapping{std::move(mapping)} {}

  void dump() const noexcept;

  class FunctionRecordIterator {
  public:
    using iterator_category = std::input_iterator_tag;
    using value_type = const llvm::coverage::FunctionRecord;
    using difference_type = std::ptrdiff_t;
    using pointer = const llvm::coverage::FunctionRecord *;
    using reference = const llvm::coverage::FunctionRecord &;

    FunctionRecordIterator() = default;

    FunctionRecordIterator(llvm::coverage::FunctionRecordIterator it)
        : _it{it} {}

    reference operator*() const noexcept { return *_it; }

    FunctionRecordIterator &operator++() noexcept {
      ++_it;
      return *this;
    }

    FunctionRecordIterator operator++(int) noexcept {
      auto tmp = *this;
      ++_it;
      return tmp;
    }

    bool operator==(const FunctionRecordIterator &other) const noexcept {
      return _it == other._it;
    }

    bool operator!=(const FunctionRecordIterator &other) const noexcept {
      return !(*this == other);
    }

  private:
    llvm::coverage::FunctionRecordIterator _it;
  };

  std::ranges::subrange<CoverageMapping::FunctionRecordIterator>
  getCoveredFunctions() const noexcept;

public:
  CoverageMapping(const CoverageMapping &) = delete;
  CoverageMapping &operator=(const CoverageMapping &) = delete;
  CoverageMapping(CoverageMapping &&) = default;
  CoverageMapping &operator=(CoverageMapping &&) = default;
  ~CoverageMapping() = default;

private:
  std::unique_ptr<LCovMapping> _mapping;
};

} // namespace Coverage
