**fake_unity.h** is a single header library to simulate the minimum required functionality
of the unity game engine to load and run native plugins compiled against the unity native plugin api.
The main use case of this library is to run these plugins outside the engine. This can be used to
do unit testing without needing a full unity project and the whole engine.

## How to use

```cpp
#include <stddef.h>

#include "IUnityLog.h" // includes IUnityInterface.h
#include "IUnityProfiler.h"
#include "IUnityGraphics.h"
#if defined(__APPLE__) && defined(__MACH__)
#  include "IUnityGraphicsMetal.h"
#  define FAKE_UNITY_GRAPHICS_METAL
#else
#  define VK_NO_PROTOTYPES
#  include "IUnityGraphicsVulkan.h" // includes vulkan/vulkan.h
#  define FAKE_UNITY_GRAPHICS_VULKAN
#endif

#define FAKE_UNITY_IMPLEMENTATION
#include "fake_unity.h"

typedef int32_t (*PFN_MyNativeFunction)(int32_t a, int32_t b);

PFN_MyNativeFunction MyNativeFunction;

int main()
{
    fake_unity_initialize(8, 4);
#if FAKE_UNITY_PLATFORM_WINDOWS
    uint32_t plugin = fake_unity_load_native_plugin("libNativePlugin.dll");
#else
    uint32_t plugin = fake_unity_load_native_plugin("./libNativePlugin.so");
#endif
#if FAKE_UNITY_PLATFORM_MACOS
    fake_unity_create_metal_renderer(-1);
#else
    fake_unity_create_vulkan_renderer(-1);
#endif

    MyNativeFunction = (PFN_MyNativeFunction) fake_unity_native_plugin_get_proc_address(plugin, "MyNativeFunction");

    int32_t result = MyNativeFunction(16, 7);

    return 0;
}
```

### Notes on using C

The Unity Plugin API headers are not really meant to be compiled in a C program.
Most of the enums and structs defined in the header files have no typedef, but
are used without the enum or struct specifier. To come around this issue you have
to predeclare these typedefs before including the Unity headers.
There is also headers that can't be compiled at all in C mode, like `IUnityProfiler.h`.
So for this to work you have to disable the profiler interface, by defining
`FAKE_UNITY_NO_PROFILER` and not include `IUnityProfiler.h`. So the example from above
for C would look like this:

```c
#include <stddef.h>
#include <stdbool.h>

typedef enum UnityLogType UnityLogType;
typedef enum UnityVulkanResourceAccessMode UnityVulkanResourceAccessMode;
typedef enum UnityVulkanEventRenderPassPreCondition UnityVulkanEventRenderPassPreCondition;
typedef enum UnityVulkanGraphicsQueueAccess UnityVulkanGraphicsQueueAccess;
typedef enum UnityVulkanEventConfigFlagBits UnityVulkanEventConfigFlagBits;
typedef enum UnityVulkanSwapchainMode UnityVulkanSwapchainMode;
typedef struct UnityVulkanInstance UnityVulkanInstance;
typedef struct UnityVulkanMemory UnityVulkanMemory;
typedef struct UnityVulkanImage UnityVulkanImage;
typedef struct UnityVulkanBuffer UnityVulkanBuffer;
typedef struct UnityVulkanRecordingState UnityVulkanRecordingState;
typedef struct UnityVulkanPluginEventConfig UnityVulkanPluginEventConfig;
typedef struct UnityVulkanSwapchainConfiguration UnityVulkanSwapchainConfiguration;

#include "IUnityLog.h" // includes IUnityInterface.h
#include "IUnityGraphics.h"
#if defined(__APPLE__) && defined(__MACH__)
#  include "IUnityGraphicsMetal.h"
#  define FAKE_UNITY_GRAPHICS_METAL
#else
#  define VK_NO_PROTOTYPES
#  include "IUnityGraphicsVulkan.h" // includes vulkan/vulkan.h
#  define FAKE_UNITY_GRAPHICS_VULKAN
#endif

#define FAKE_UNITY_IMPLEMENTATION
#define FAKE_UNITY_NO_PROFILER
#include "fake_unity.h"

typedef int32_t (*PFN_MyNativeFunction)(int32_t a, int32_t b);

PFN_MyNativeFunction MyNativeFunction;

int main(void)
{
    fake_unity_initialize(8, 4);
#if FAKE_UNITY_PLATFORM_WINDOWS
    uint32_t plugin = fake_unity_load_native_plugin("libNativePlugin.dll");
#else
    uint32_t plugin = fake_unity_load_native_plugin("./libNativePlugin.so");
#endif
#if FAKE_UNITY_PLATFORM_MACOS
    fake_unity_create_metal_renderer(-1);
#else
    fake_unity_create_vulkan_renderer(-1);
#endif

    MyNativeFunction = (PFN_MyNativeFunction) fake_unity_native_plugin_get_proc_address(plugin, "MyNativeFunction");

    int32_t result = MyNativeFunction(16, 7);

    return 0;
}
```
