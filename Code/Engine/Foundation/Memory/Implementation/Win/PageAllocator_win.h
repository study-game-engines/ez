#include <Foundation/FoundationInternal.h>
EZ_FOUNDATION_INTERNAL_HEADER

#include <Foundation/Basics/Platform/Win/Platform_win.h>
#include <Foundation/Time/Time.h>

// static
void* ezPageAllocator::AllocatePage(size_t uiSize)
{
  ezTime fAllocationTime = ezTime::Now();

  void* ptr = ::VirtualAlloc(nullptr, uiSize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
  EZ_ASSERT_DEV(ptr != nullptr, "Could not allocate memory pages. Error Code '{0}'", ezArgErrorCode(::GetLastError()));

  size_t uiAlign = ezSystemInformation::Get().GetMemoryPageSize();
  EZ_CHECK_ALIGNMENT(ptr, uiAlign);

  if ((ezMemoryTrackingFlags::Default & ezMemoryTrackingFlags::EnableAllocationTracking) != 0)
  {
    ezMemoryTracker::AddAllocation(GetPageAllocatorId(), ezMemoryTrackingFlags::Default, ptr, uiSize, uiAlign, ezTime::Now() - fAllocationTime);
  }

  return ptr;
}

// static
void ezPageAllocator::DeallocatePage(void* pPtr)
{
  if ((ezMemoryTrackingFlags::Default & ezMemoryTrackingFlags::EnableAllocationTracking) != 0)
  {
    ezMemoryTracker::RemoveAllocation(GetPageAllocatorId(), pPtr);
  }

  EZ_VERIFY(::VirtualFree(pPtr, 0, MEM_RELEASE), "Could not free memory pages. Error Code '{0}'", ezArgErrorCode(::GetLastError()));
}
