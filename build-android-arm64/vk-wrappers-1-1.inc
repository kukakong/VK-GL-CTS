
if (!(ptrs_.vkGetPhysicalDeviceProperties2 = reinterpret_cast<PFN_vkGetPhysicalDeviceProperties2>(getInstanceProcAddr(instance_, "vkGetPhysicalDeviceProperties2")))) {
  return Result("Vulkan: Unable to load vkGetPhysicalDeviceProperties2 pointer");
}

ptrs_.vkCreateRayTracingPipelinesKHR = reinterpret_cast<PFN_vkCreateRayTracingPipelinesKHR>(getInstanceProcAddr(instance_, "vkCreateRayTracingPipelinesKHR"));

ptrs_.vkCreateAccelerationStructureKHR = reinterpret_cast<PFN_vkCreateAccelerationStructureKHR>(getInstanceProcAddr(instance_, "vkCreateAccelerationStructureKHR"));

ptrs_.vkDestroyAccelerationStructureKHR = reinterpret_cast<PFN_vkDestroyAccelerationStructureKHR>(getInstanceProcAddr(instance_, "vkDestroyAccelerationStructureKHR"));

ptrs_.vkGetAccelerationStructureBuildSizesKHR = reinterpret_cast<PFN_vkGetAccelerationStructureBuildSizesKHR>(getInstanceProcAddr(instance_, "vkGetAccelerationStructureBuildSizesKHR"));

ptrs_.vkBuildAccelerationStructuresKHR = reinterpret_cast<PFN_vkBuildAccelerationStructuresKHR>(getInstanceProcAddr(instance_, "vkBuildAccelerationStructuresKHR"));

ptrs_.vkCmdBuildAccelerationStructuresKHR = reinterpret_cast<PFN_vkCmdBuildAccelerationStructuresKHR>(getInstanceProcAddr(instance_, "vkCmdBuildAccelerationStructuresKHR"));

ptrs_.vkGetAccelerationStructureDeviceAddressKHR = reinterpret_cast<PFN_vkGetAccelerationStructureDeviceAddressKHR>(getInstanceProcAddr(instance_, "vkGetAccelerationStructureDeviceAddressKHR"));

ptrs_.vkCmdTraceRaysKHR = reinterpret_cast<PFN_vkCmdTraceRaysKHR>(getInstanceProcAddr(instance_, "vkCmdTraceRaysKHR"));

ptrs_.vkGetRayTracingShaderGroupHandlesKHR = reinterpret_cast<PFN_vkGetRayTracingShaderGroupHandlesKHR>(getInstanceProcAddr(instance_, "vkGetRayTracingShaderGroupHandlesKHR"));
