#include "Coverage/CoverageMapping.h"
#include "llvm/ADT/iterator_range.h"
#include "llvm/Support/VirtualFileSystem.h"
#include <iostream>
#include <llvm/ADT/SmallVector.h>
#include <llvm/ProfileData/Coverage/CoverageMapping.h>
#include <ranges>
#include <stdexcept>

namespace Coverage {

CoverageMapping
CoverageMapping::LoadFromFile(const std::vector<std::string> &objFilenames,
                              const std::string &profileFilename) {
  auto fsPtr = llvm::vfs::getRealFileSystem();
  assert(fsPtr);

  auto mapping = LCovMapping::load(
      llvm::SmallVector<llvm::StringRef>{objFilenames.cbegin(),
                                         objFilenames.cend()},
      profileFilename, *fsPtr);

  if (auto Err = mapping.takeError()) {
    throw std::runtime_error(llvm::toString(std::move(Err)));
  }

  return CoverageMapping(std::move(*mapping));
}

static void DumpCountedRegions(const auto &regions) {
  for (const llvm::coverage::CountedRegion &region : regions) {
    std::cout << "        - " << region.LineStart << ":" << region.ColumnStart
              << " - " << region.LineEnd << ":" << region.ColumnEnd << " - "
              << region.ExecutionCount << "\n";
  }
}

static void DumpMCDCRecords(const auto &records) {
  for (const llvm::coverage::MCDCRecord &record : records) {
    auto numConditions = record.getNumConditions();
    std::cout << "        - count: " << numConditions << "\n";
    std::cout << "          " << record.getTestVectorHeaderString() << "\n";
    // for (auto idx : std::views::iota(0u, numConditions)) {
    // }
  }
}

void CoverageMapping::dump() const noexcept {
  std::cout << "Dump For CoverageMapping" << "\n";

  std::cout << ">> functions: " << "\n";
  for (const llvm::coverage::FunctionRecord &func_record :
       _mapping->getCoveredFunctions()) {
    // FunctionRecord has:
    // std::string Name;
    // std::vector<CountedRegion> CountedRegions;
    // std::vector<CountedRegion> CountedBranchRegions;
    // std::vector<MCDCRecord> MCDCRecords;
    // dump them.
    std::cout << "  - " << func_record.Name << "\n";
    std::cout << "    - CountedRegions: " << "\n";
    std::cout << "      - count: " << func_record.CountedRegions.size() << "\n";
    DumpCountedRegions(func_record.CountedRegions);

    std::cout << "    - CountedBranchRegions: " << "\n";
    std::cout << "      - count: " << func_record.CountedBranchRegions.size()
              << "\n";
    DumpCountedRegions(func_record.CountedBranchRegions);

    std::cout << "    - MCDCRecord: " << "\n";
    std::cout << "      - count: " << func_record.MCDCRecords.size() << "\n";
    DumpMCDCRecords(func_record.MCDCRecords);
  }
}

std::ranges::subrange<CoverageMapping::FunctionRecordIterator>
CoverageMapping::getCoveredFunctions() const noexcept {
  auto lrange = _mapping->getCoveredFunctions();

  std::ranges::subrange<CoverageMapping::FunctionRecordIterator> r =
      std::ranges::subrange<CoverageMapping::FunctionRecordIterator>(
          FunctionRecordIterator{lrange.begin()},
          FunctionRecordIterator{lrange.end()});
  return r;
}

} // namespace Coverage
