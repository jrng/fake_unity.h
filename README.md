**fake_unity.h** is a single header library to simulate the minimum required functionality
of the unity game engine to load and run native plugins compiled against the unity native plugin api.
The main use case of this library is to run these plugins outside the engine. This can be used to
do unit testing without needing a full unity project and the whole engine.

## How to use

```c
#include "IUnityProfiler.h"
#include "IUnityGraphics.h"
#define VK_NO_PROTOTYPES
#include "IUnityGraphicsVulkan.h"

#define FAKE_UNITY_IMPLEMENTATION
#include "fake_unity.h"

typedef int32_t (*PFN_MyNativeFunction)(int32_t a, int32_t b);

PFN_MyNativeFunction MyNativeFunction;

int main(void)
{
    fake_unity_initialize(8);
#if FAKE_UNITY_PLATFORM_WINDOWS
    uint32_t plugin = fake_unity_load_native_plugin("libNativePlugin.dll");
#else
    uint32_t plugin = fake_unity_load_native_plugin("./libNativePlugin.so");
#endif
    fake_unity_create_vulkan_renderer(-1);

    MyNativeFunction = (PFN_MyNativeFunction) fake_unity_native_plugin_get_proc_address(plugin, "MyNativeFunction");

    int32_t result = MyNativeFunction(16, 7);

    return 0;
}
```
