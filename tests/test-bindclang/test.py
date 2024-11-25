from pyinjector.bindclang import bindclang

CoverageMapping = bindclang.CoverageMapping

cm = CoverageMapping.LoadFromFile(["/home/yvesw/Software-Construction/thesis/plg/pytorch/torch/lib/libtorch_cpu.so"], "./addmm.profdata")
# cm.dump()

func_records = list(cm.getCoveredFunctions())

func = func_records[0]
print(func.Name, func.Filenames)

crs = func.CountedRegions

for cr in crs:
    print({
        "ExecutionCount": cr.ExecutionCount,
        "FalseExecutionCount": cr.FalseExecutionCount,
        "Folded": cr.Folded
    })
