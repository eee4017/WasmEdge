#include <chrono>
#include <iostream>
#include <wasmedge/wasmedge.h>
using namespace std;

template <class result_t = std::chrono::nanoseconds,
          class clock_t = std::chrono::steady_clock,
          class duration_t = std::chrono::nanoseconds>
auto since(std::chrono::time_point<clock_t, duration_t> const &start) {
  return std::chrono::duration_cast<result_t>(clock_t::now() - start);
}

int main() {
  /* Create the VM context. */
  WasmEdge_VMContext *VMCxt = WasmEdge_VMCreate(NULL, NULL);
  WasmEdge_Result Res;

  WasmEdge_String ExportName = WasmEdge_StringCreateByCString("env");
  WasmEdge_ModuleInstanceContext *HostModCxt =
      WasmEdge_ModuleInstanceCreate(ExportName);
  WasmEdge_Limit MemLimit = {.HasMax = true, .Min = 60, .Max = 60};
  WasmEdge_MemoryTypeContext *MemTypeCxt = WasmEdge_MemoryTypeCreate(MemLimit);
  WasmEdge_MemoryInstanceContext *HostMemory =
      WasmEdge_MemoryInstanceCreate(MemTypeCxt);
  WasmEdge_String MemoryName = WasmEdge_StringCreateByCString("memory");
  WasmEdge_ModuleInstanceAddMemory(HostModCxt, MemoryName, HostMemory);
  WasmEdge_StringDelete(MemoryName);
  WasmEdge_VMRegisterModuleFromImport(VMCxt, HostModCxt);

  double x = -0.743644786;
  double y = 0.1318252536;
  double d = 0.00029336;
  int maxIterations = 100;
  WasmEdge_String ModName = WasmEdge_StringCreateByCString("mandelbrot");
  Res = WasmEdge_VMRegisterModuleFromFile(VMCxt, ModName, "./mandelbrot.so");
  if (!WasmEdge_ResultOK(Res)) {
    printf("WASM registration failed: %s\n", WasmEdge_ResultGetMessage(Res));
    return 1;
  }

  /* The parameters and returns arrays. */
  WasmEdge_Value Params[4] = {
      WasmEdge_ValueGenI32(maxIterations),
      WasmEdge_ValueGenF64(x),
      WasmEdge_ValueGenF64(y),
      WasmEdge_ValueGenF64(d),
  };
  WasmEdge_Value Returns[1];
  WasmEdge_String FuncName = WasmEdge_StringCreateByCString("mandelbrot");
  auto start = std::chrono::steady_clock::now();
  Res = WasmEdge_VMExecuteRegistered(VMCxt, ModName, FuncName, Params, 4, NULL,
                                     0);
  std::cout << "Elapsed Time: " << since(start).count()/1e6 << std::endl;

  if (WasmEdge_ResultOK(Res)) {
    cout << "Got the result: ok";
  } else {
    cout << "Error message: " << WasmEdge_ResultGetMessage(Res) << "\n";
  }

  /* Resources deallocations. */
  WasmEdge_VMDelete(VMCxt);
  WasmEdge_StringDelete(FuncName);
  WasmEdge_MemoryTypeDelete(MemTypeCxt);
  return 0;
}