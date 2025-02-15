#include <RendererVulkan/RendererVulkanPCH.h>


VKAPI_ATTR void VKAPI_CALL vkGetDeviceBufferMemoryRequirements(
  VkDevice device,
  const VkDeviceBufferMemoryRequirements* pInfo,
  VkMemoryRequirements2* pMemoryRequirements)
{
  EZ_REPORT_FAILURE("FIXME: Added to prevent the error: The procedure entry point vkGetDeviceBufferMemoryRequirements could not be located in the dynamic link library ezRendererVulkan.dll.");
}

#include <Foundation/Basics/Platform/Win/IncludeWindows.h>

#define VMA_VULKAN_VERSION 1001000
#define VMA_STATIC_VULKAN_FUNCTIONS 0
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 1
#define VMA_STATS_STRING_ENABLED 1


//
//#define VMA_DEBUG_LOG(format, ...)   \
//  do                                 \
//  {                                  \
//    ezStringBuilder tmp;             \
//    tmp.Printf(format, __VA_ARGS__); \
//    ezLog::Error("{}", tmp);         \
//  } while (false)

#include <RendererVulkan/MemoryAllocator/MemoryAllocatorVulkan.h>

#define VMA_IMPLEMENTATION

#ifndef VA_IGNORE_THIS_FILE
#  define VA_INCLUDE_HIDDEN <vma/vk_mem_alloc.h>
#else
#  define VA_INCLUDE_HIDDEN ""
#endif

#include VA_INCLUDE_HIDDEN

EZ_CHECK_AT_COMPILETIME(VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT == ezVulkanAllocationCreateFlags::DedicatedMemory);
EZ_CHECK_AT_COMPILETIME(VMA_ALLOCATION_CREATE_NEVER_ALLOCATE_BIT == ezVulkanAllocationCreateFlags::NeverAllocate);
EZ_CHECK_AT_COMPILETIME(VMA_ALLOCATION_CREATE_MAPPED_BIT == ezVulkanAllocationCreateFlags::Mapped);
EZ_CHECK_AT_COMPILETIME(VMA_ALLOCATION_CREATE_CAN_ALIAS_BIT == ezVulkanAllocationCreateFlags::CanAlias);
EZ_CHECK_AT_COMPILETIME(VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT == ezVulkanAllocationCreateFlags::HostAccessSequentialWrite);
EZ_CHECK_AT_COMPILETIME(VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT == ezVulkanAllocationCreateFlags::HostAccessRandom);
EZ_CHECK_AT_COMPILETIME(VMA_ALLOCATION_CREATE_STRATEGY_MIN_MEMORY_BIT == ezVulkanAllocationCreateFlags::StrategyMinMemory);
EZ_CHECK_AT_COMPILETIME(VMA_ALLOCATION_CREATE_STRATEGY_MIN_TIME_BIT == ezVulkanAllocationCreateFlags::StrategyMinTime);

EZ_CHECK_AT_COMPILETIME(VMA_MEMORY_USAGE_UNKNOWN == ezVulkanMemoryUsage::Unknown);
EZ_CHECK_AT_COMPILETIME(VMA_MEMORY_USAGE_GPU_LAZILY_ALLOCATED == ezVulkanMemoryUsage::GpuLazilyAllocated);
EZ_CHECK_AT_COMPILETIME(VMA_MEMORY_USAGE_AUTO == ezVulkanMemoryUsage::Auto);
EZ_CHECK_AT_COMPILETIME(VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE == ezVulkanMemoryUsage::AutoPreferDevice);
EZ_CHECK_AT_COMPILETIME(VMA_MEMORY_USAGE_AUTO_PREFER_HOST == ezVulkanMemoryUsage::AutoPreferHost);

EZ_CHECK_AT_COMPILETIME(sizeof(ezVulkanAllocation) == sizeof(VmaAllocation));

EZ_CHECK_AT_COMPILETIME(sizeof(ezVulkanAllocationInfo) == sizeof(VmaAllocationInfo));


struct ezMemoryAllocatorVulkan::Impl
{
  EZ_DECLARE_POD_TYPE();
  VmaAllocator m_allocator;
};

ezMemoryAllocatorVulkan::Impl* ezMemoryAllocatorVulkan::m_pImpl = nullptr;

vk::Result ezMemoryAllocatorVulkan::Initialize(vk::PhysicalDevice physicalDevice, vk::Device device, vk::Instance instance)
{
  EZ_ASSERT_DEV(m_pImpl == nullptr, "ezMemoryAllocatorVulkan::Initialize was already called");
  m_pImpl = EZ_DEFAULT_NEW(Impl);

  VmaVulkanFunctions vulkanFunctions = {};
  vulkanFunctions.vkGetInstanceProcAddr = &vkGetInstanceProcAddr;
  vulkanFunctions.vkGetDeviceProcAddr = &vkGetDeviceProcAddr;

  VmaAllocatorCreateInfo allocatorCreateInfo = {};
  allocatorCreateInfo.vulkanApiVersion = VK_API_VERSION_1_1;
  allocatorCreateInfo.physicalDevice = physicalDevice;
  allocatorCreateInfo.device = device;
  allocatorCreateInfo.instance = instance;
  allocatorCreateInfo.pVulkanFunctions = &vulkanFunctions;

  vk::Result res = (vk::Result)vmaCreateAllocator(&allocatorCreateInfo, &m_pImpl->m_allocator);
  if (res != vk::Result::eSuccess)
  {
    EZ_DEFAULT_DELETE(m_pImpl);
  }
  return res;
}

void ezMemoryAllocatorVulkan::DeInitialize()
{
  EZ_ASSERT_DEV(m_pImpl != nullptr, "ezMemoryAllocatorVulkan is not initialized.");

  vmaDestroyAllocator(m_pImpl->m_allocator);
  EZ_DEFAULT_DELETE(m_pImpl);
}

vk::Result ezMemoryAllocatorVulkan::CreateImage(const vk::ImageCreateInfo& imageCreateInfo, const ezVulkanAllocationCreateInfo& allocationCreateInfo, vk::Image& out_image, ezVulkanAllocation& out_alloc, ezVulkanAllocationInfo* pAllocInfo)
{
  VmaAllocationCreateInfo allocCreateInfo = {};
  allocCreateInfo.usage = (VmaMemoryUsage)allocationCreateInfo.m_usage.GetValue();
  allocCreateInfo.flags = allocationCreateInfo.m_flags.GetValue() | VMA_ALLOCATION_CREATE_USER_DATA_COPY_STRING_BIT;
  allocCreateInfo.pUserData = (void*)allocationCreateInfo.m_pUserData;

  return (vk::Result)vmaCreateImage(m_pImpl->m_allocator, reinterpret_cast<const VkImageCreateInfo*>(&imageCreateInfo), &allocCreateInfo, reinterpret_cast<VkImage*>(&out_image), reinterpret_cast<VmaAllocation*>(&out_alloc), reinterpret_cast<VmaAllocationInfo*>(pAllocInfo));
}

void ezMemoryAllocatorVulkan::DestroyImage(vk::Image& image, ezVulkanAllocation& alloc)
{
  vmaSetAllocationUserData(m_pImpl->m_allocator, reinterpret_cast<VmaAllocation&>(alloc), nullptr);
  vmaDestroyImage(m_pImpl->m_allocator, reinterpret_cast<VkImage&>(image), reinterpret_cast<VmaAllocation&>(alloc));
  image = nullptr;
  alloc = nullptr;
}

vk::Result ezMemoryAllocatorVulkan::CreateBuffer(const vk::BufferCreateInfo& bufferCreateInfo, const ezVulkanAllocationCreateInfo& allocationCreateInfo, vk::Buffer& out_buffer, ezVulkanAllocation& out_alloc, ezVulkanAllocationInfo* pAllocInfo)
{
  VmaAllocationCreateInfo allocCreateInfo = {};
  allocCreateInfo.usage = (VmaMemoryUsage)allocationCreateInfo.m_usage.GetValue();
  allocCreateInfo.flags = allocationCreateInfo.m_flags.GetValue() | VMA_ALLOCATION_CREATE_USER_DATA_COPY_STRING_BIT;
  allocCreateInfo.pUserData = (void*)allocationCreateInfo.m_pUserData;

  return (vk::Result)vmaCreateBuffer(m_pImpl->m_allocator, reinterpret_cast<const VkBufferCreateInfo*>(&bufferCreateInfo), &allocCreateInfo, reinterpret_cast<VkBuffer*>(&out_buffer), reinterpret_cast<VmaAllocation*>(&out_alloc), reinterpret_cast<VmaAllocationInfo*>(pAllocInfo));
}

void ezMemoryAllocatorVulkan::DestroyBuffer(vk::Buffer& buffer, ezVulkanAllocation& alloc)
{
  vmaSetAllocationUserData(m_pImpl->m_allocator, reinterpret_cast<VmaAllocation&>(alloc), nullptr);
  vmaDestroyBuffer(m_pImpl->m_allocator, reinterpret_cast<VkBuffer&>(buffer), reinterpret_cast<VmaAllocation&>(alloc));
  buffer = nullptr;
  alloc = nullptr;
}

ezVulkanAllocationInfo ezMemoryAllocatorVulkan::GetAllocationInfo(ezVulkanAllocation alloc)
{
  VmaAllocationInfo info;
  vmaGetAllocationInfo(m_pImpl->m_allocator, reinterpret_cast<VmaAllocation&>(alloc), &info);

  return reinterpret_cast<ezVulkanAllocationInfo&>(info);
}

void ezMemoryAllocatorVulkan::SetAllocationUserData(ezVulkanAllocation alloc, const char* pUserData)
{
  vmaSetAllocationUserData(m_pImpl->m_allocator, reinterpret_cast<VmaAllocation&>(alloc), (void*)pUserData);
}

vk::Result ezMemoryAllocatorVulkan::MapMemory(ezVulkanAllocation alloc, void** pData)
{
  return (vk::Result)vmaMapMemory(m_pImpl->m_allocator, reinterpret_cast<VmaAllocation&>(alloc), pData);
}

void ezMemoryAllocatorVulkan::UnmapMemory(ezVulkanAllocation alloc)
{
  vmaUnmapMemory(m_pImpl->m_allocator, reinterpret_cast<VmaAllocation&>(alloc));
}

vk::Result ezMemoryAllocatorVulkan::FlushAllocation(ezVulkanAllocation alloc, vk::DeviceSize offset, vk::DeviceSize size)
{
  return (vk::Result)vmaFlushAllocation(m_pImpl->m_allocator, reinterpret_cast<VmaAllocation&>(alloc), offset, size);
}

vk::Result ezMemoryAllocatorVulkan::InvalidateAllocation(ezVulkanAllocation alloc, vk::DeviceSize offset, vk::DeviceSize size)
{
  return (vk::Result)vmaInvalidateAllocation(m_pImpl->m_allocator, reinterpret_cast<VmaAllocation&>(alloc), offset, size);
}
