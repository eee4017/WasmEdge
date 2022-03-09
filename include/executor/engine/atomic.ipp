#include "executor/executor.h"
#include "runtime/instance/memory.h"

#include <cstdint>

namespace WasmEdge {
namespace Executor {

template <typename T>
TypeT<T> Executor::runAtomicWaitOp(Runtime::StackManager &StackMgr,
                                   Runtime::Instance::MemoryInstance &MemInst,
                                   const AST::Instruction &Instr) {

  const uint32_t BitWidth = sizeof(T) * 8;
  ValVariant Address = StackMgr.pop();
  ValVariant Val = StackMgr.pop();
  [[maybe_unused]] ValVariant Timeout = StackMgr.pop();

  StackMgr.push(Address);
  runLoadOp<T>(StackMgr, MemInst, Instr, BitWidth);
  ValVariant &Loaded = StackMgr.getTop();

  if (Loaded.get<T>() == Val.get<T>()) {
    StackMgr.push(ValVariant(0));
  } else {
    StackMgr.push(ValVariant(1));
  }
  // TODO: Implement Timeout
  return {};
}

template <typename T, typename I>
TypeT<T> Executor::runAtomicLoadOp(Runtime::StackManager &StackMgr,
                                   Runtime::Instance::MemoryInstance &MemInst,
                                   const AST::Instruction &Instr) {

  const uint32_t BitWidth = sizeof(I) * 8;
  ValVariant &Address = StackMgr.getTop();
  if ((Address.get<uint32_t>() & ((BitWidth >> 3U) - 1)) != 0) {
    spdlog::error(ErrCode::UnalignedAtomicAccess);
    spdlog::error(
        ErrInfo::InfoInstruction(Instr.getOpCode(), Instr.getOffset()));
    return Unexpect(ErrCode::UnalignedAtomicAccess);
  }
  runLoadOp<T>(StackMgr, MemInst, Instr, BitWidth);
  return {};
}

template <typename T, typename I>
TypeT<T> Executor::runAtomicStoreOp(Runtime::StackManager &StackMgr,
                                    Runtime::Instance::MemoryInstance &MemInst,
                                    const AST::Instruction &Instr) {

  const uint32_t BitWidth = sizeof(I) * 8;
  ValVariant &Address = StackMgr.getBottomN(2);
  if ((Address.get<uint32_t>() & ((BitWidth >> 3U) - 1)) != 0) {
    spdlog::error(ErrCode::UnalignedAtomicAccess);
    spdlog::error(
        ErrInfo::InfoInstruction(Instr.getOpCode(), Instr.getOffset()));
    return Unexpect(ErrCode::UnalignedAtomicAccess);
  }
  typedef typename std::make_unsigned<T>::type UT;
  runStoreOp<UT>(StackMgr, MemInst, Instr, BitWidth);
  return {};
}

template <typename T, typename I>
TypeT<T> Executor::runAtomicAddOp(Runtime::StackManager &StackMgr,
                                  Runtime::Instance::MemoryInstance &MemInst,
                                  const AST::Instruction &Instr) {
  const uint32_t BitWidth = sizeof(I) * 8;
  ValVariant RawValue = StackMgr.pop();
  ValVariant RawAddress = StackMgr.pop();
  int32_t Address = RawAddress.get<int32_t>();
  if ((Address & ((BitWidth >> 3U) - 1)) != 0) {
    spdlog::error(ErrCode::UnalignedAtomicAccess);
    spdlog::error(
        ErrInfo::InfoInstruction(Instr.getOpCode(), Instr.getOffset()));
    return Unexpect(ErrCode::UnalignedAtomicAccess);
  }

  // make sure the address no OOB with size I
  I *RawPointer = MemInst.getPointer<I *>(Address);
  if (!RawPointer) {
    spdlog::error(ErrCode::MemoryOutOfBounds);
    spdlog::error(
        ErrInfo::InfoInstruction(Instr.getOpCode(), Instr.getOffset()));
    return Unexpect(ErrCode::MemoryOutOfBounds);
  }
  auto *AtomicObj =
      static_cast<std::atomic<I> *>(reinterpret_cast<void *>(RawPointer));
  I Value = static_cast<I>(RawValue.get<T>());

  I Return = AtomicObj->fetch_add(Value, std::memory_order_acquire);
  StackMgr.push(static_cast<T>(Return));
  return {};
}

template <typename T, typename I>
TypeT<T> Executor::runAtomicSubOp(Runtime::StackManager &StackMgr,
                                  Runtime::Instance::MemoryInstance &MemInst,
                                  const AST::Instruction &Instr) {
  const uint32_t BitWidth = sizeof(I) * 8;
  ValVariant RawValue = StackMgr.pop();
  ValVariant RawAddress = StackMgr.pop();
  int32_t Address = RawAddress.get<int32_t>();
  if ((Address & ((BitWidth >> 3U) - 1)) != 0) {
    spdlog::error(ErrCode::UnalignedAtomicAccess);
    spdlog::error(
        ErrInfo::InfoInstruction(Instr.getOpCode(), Instr.getOffset()));
    return Unexpect(ErrCode::UnalignedAtomicAccess);
  }

  // make sure the address no OOB with size I
  I *RawPointer = MemInst.getPointer<I *>(Address);
  if (!RawPointer) {
    spdlog::error(ErrCode::MemoryOutOfBounds);
    spdlog::error(
        ErrInfo::InfoInstruction(Instr.getOpCode(), Instr.getOffset()));
    return Unexpect(ErrCode::MemoryOutOfBounds);
  }
  auto *AtomicObj =
      static_cast<std::atomic<I> *>(reinterpret_cast<void *>(RawPointer));
  I Value = static_cast<I>(RawValue.get<T>());

  I Return = AtomicObj->fetch_sub(Value, std::memory_order_acquire);
  StackMgr.push(static_cast<T>(Return));
  return {};
}

template <typename T, typename I>
TypeT<T> Executor::runAtomicOrOp(Runtime::StackManager &StackMgr,
                                 Runtime::Instance::MemoryInstance &MemInst,
                                 const AST::Instruction &Instr) {
  const uint32_t BitWidth = sizeof(I) * 8;
  ValVariant RawValue = StackMgr.pop();
  ValVariant RawAddress = StackMgr.pop();
  int32_t Address = RawAddress.get<int32_t>();
  if ((Address & ((BitWidth >> 3U) - 1)) != 0) {
    spdlog::error(ErrCode::UnalignedAtomicAccess);
    spdlog::error(
        ErrInfo::InfoInstruction(Instr.getOpCode(), Instr.getOffset()));
    return Unexpect(ErrCode::UnalignedAtomicAccess);
  }

  // make sure the address no OOB with size I
  I *RawPointer = MemInst.getPointer<I *>(Address);
  if (!RawPointer) {
    spdlog::error(ErrCode::MemoryOutOfBounds);
    spdlog::error(
        ErrInfo::InfoInstruction(Instr.getOpCode(), Instr.getOffset()));
    return Unexpect(ErrCode::MemoryOutOfBounds);
  }
  auto *AtomicObj =
      static_cast<std::atomic<I> *>(reinterpret_cast<void *>(RawPointer));
  I Value = static_cast<I>(RawValue.get<T>());

  I Return = AtomicObj->fetch_or(Value, std::memory_order_acquire);
  StackMgr.push(static_cast<T>(Return));
  return {};
}

template <typename T, typename I>
TypeT<T> Executor::runAtomicAndOp(Runtime::StackManager &StackMgr,
                                  Runtime::Instance::MemoryInstance &MemInst,
                                  const AST::Instruction &Instr) {
  const uint32_t BitWidth = sizeof(I) * 8;
  ValVariant RawValue = StackMgr.pop();
  ValVariant RawAddress = StackMgr.pop();
  int32_t Address = RawAddress.get<int32_t>();
  if ((Address & ((BitWidth >> 3U) - 1)) != 0) {
    spdlog::error(ErrCode::UnalignedAtomicAccess);
    spdlog::error(
        ErrInfo::InfoInstruction(Instr.getOpCode(), Instr.getOffset()));
    return Unexpect(ErrCode::UnalignedAtomicAccess);
  }

  // make sure the address no OOB with size I
  I *RawPointer = MemInst.getPointer<I *>(Address);
  if (!RawPointer) {
    spdlog::error(ErrCode::MemoryOutOfBounds);
    spdlog::error(
        ErrInfo::InfoInstruction(Instr.getOpCode(), Instr.getOffset()));
    return Unexpect(ErrCode::MemoryOutOfBounds);
  }
  auto *AtomicObj =
      static_cast<std::atomic<I> *>(reinterpret_cast<void *>(RawPointer));
  I Value = static_cast<I>(RawValue.get<T>());

  I Return = AtomicObj->fetch_and(Value, std::memory_order_acquire);
  StackMgr.push(static_cast<T>(Return));
  return {};
}

template <typename T, typename I>
TypeT<T> Executor::runAtomicXorOp(Runtime::StackManager &StackMgr,
                                  Runtime::Instance::MemoryInstance &MemInst,
                                  const AST::Instruction &Instr) {
  const uint32_t BitWidth = sizeof(I) * 8;
  ValVariant RawValue = StackMgr.pop();
  ValVariant RawAddress = StackMgr.pop();
  int32_t Address = RawAddress.get<int32_t>();
  if ((Address & ((BitWidth >> 3U) - 1)) != 0) {
    spdlog::error(ErrCode::UnalignedAtomicAccess);
    spdlog::error(
        ErrInfo::InfoInstruction(Instr.getOpCode(), Instr.getOffset()));
    return Unexpect(ErrCode::UnalignedAtomicAccess);
  }

  // make sure the address no OOB with size I
  I *RawPointer = MemInst.getPointer<I *>(Address);
  if (!RawPointer) {
    spdlog::error(ErrCode::MemoryOutOfBounds);
    spdlog::error(
        ErrInfo::InfoInstruction(Instr.getOpCode(), Instr.getOffset()));
    return Unexpect(ErrCode::MemoryOutOfBounds);
  }
  auto *AtomicObj =
      static_cast<std::atomic<I> *>(reinterpret_cast<void *>(RawPointer));
  I Value = static_cast<I>(RawValue.get<T>());

  I Return = AtomicObj->fetch_xor(Value, std::memory_order_acquire);
  StackMgr.push(static_cast<T>(Return));
  return {};
}

// ------------------------

template <typename T, typename I>
TypeT<T>
Executor::runAtomicExchangeOp(Runtime::StackManager &StackMgr,
                              Runtime::Instance::MemoryInstance &MemInst,
                              const AST::Instruction &Instr) {

  const uint32_t BitWidth = sizeof(I) * 8;
  ValVariant RHS = StackMgr.pop();
  ValVariant Address = StackMgr.getTop();
  if ((Address.get<uint32_t>() & ((BitWidth >> 3U) - 1)) != 0) {
    spdlog::error(ErrCode::UnalignedAtomicAccess);
    spdlog::error(
        ErrInfo::InfoInstruction(Instr.getOpCode(), Instr.getOffset()));
    return Unexpect(ErrCode::UnalignedAtomicAccess);
  }
  runLoadOp<T>(StackMgr, MemInst, Instr, BitWidth);

  typedef typename std::make_unsigned<T>::type UT;
  ValVariant Loaded = StackMgr.pop();
  StackMgr.push(Address);
  StackMgr.push(RHS);
  runStoreOp<UT>(StackMgr, MemInst, Instr, BitWidth);

  StackMgr.push(Loaded);
  return {};
}

template <typename T, typename I>
TypeT<T>
Executor::runAtomicCompareExchangeOp(Runtime::StackManager &StackMgr,
                                     Runtime::Instance::MemoryInstance &MemInst,
                                     const AST::Instruction &Instr) {

  const uint32_t BitWidth = sizeof(I) * 8;
  ValVariant Val = StackMgr.pop();
  ValVariant Cmp = StackMgr.pop();
  ValVariant Address = StackMgr.getTop();
  if ((Address.get<uint32_t>() & ((BitWidth >> 3U) - 1)) != 0) {
    spdlog::error(ErrCode::UnalignedAtomicAccess);
    spdlog::error(
        ErrInfo::InfoInstruction(Instr.getOpCode(), Instr.getOffset()));
    return Unexpect(ErrCode::UnalignedAtomicAccess);
  }
  runLoadOp<T>(StackMgr, MemInst, Instr, BitWidth);
  ValVariant Loaded = StackMgr.pop();

  typedef typename std::make_unsigned<T>::type UT;
  if (Loaded.get<T>() == Cmp.get<T>()) {
    StackMgr.push(Address);
    StackMgr.push(Val);
    runStoreOp<UT>(StackMgr, MemInst, Instr, BitWidth);
  }

  StackMgr.push(Loaded);
  return {};
}

} // namespace Executor
} // namespace WasmEdge