// fake_unity.h - MIT License
// See end of file for full license

// fake_unity.h is a single header library to simulate the minimum required
// functionality of the unity game engine to load and run native plugins
// compiled against the unity native plugin api. The main use case of this
// library is to run these plugins outside the engine. This can be used to
// do unit testing without needing a full unity project and the whole engine.
//
// BATTERIES NOT INCLUDED
//
// You need to include the unity native plugin api headers like
// IUnityGraphics.h, IUnityGraphicsVulkan.h, etc. yourself.
// You can find them in the unity editor folder under 'Data/PluginAPI'.
//
// EXAMPLE
//
//   #include "IUnityProfiler.h"
//   #include "IUnityGraphics.h"
//   #define VK_NO_PROTOTYPES
//   #include "IUnityGraphicsVulkan.h"
//
//   #define FAKE_UNITY_IMPLEMENTATION
//   #include "fake_unity.h"
//
//   typedef int32_t (*PFN_MyNativeFunction)(int32_t a, int32_t b);
//
//   PFN_MyNativeFunction MyNativeFunction;
//
//   int main(void)
//   {
//       fake_unity_initialize(8);
//   #if FAKE_UNITY_PLATFORM_WINDOWS
//       uint32_t plugin = fake_unity_load_native_plugin("libNativePlugin.dll");
//   #else
//       uint32_t plugin = fake_unity_load_native_plugin("./libNativePlugin.so");
//   #endif
//       fake_unity_create_vulkan_renderer(-1);
//
//       MyNativeFunction = (PFN_MyNativeFunction) fake_unity_native_plugin_get_proc_address(plugin, "MyNativeFunction");
//
//       int32_t result = MyNativeFunction(16, 7);
//
//       return 0;
//   }

#ifndef __FAKE_UNITY_INCLUDE__
#define __FAKE_UNITY_INCLUDE__

#define FAKE_UNITY_PLATFORM_ANDROID 0
#define FAKE_UNITY_PLATFORM_WINDOWS 0
#define FAKE_UNITY_PLATFORM_LINUX   0
#define FAKE_UNITY_PLATFORM_MACOS   0

#if defined(__ANDROID__)
#  undef FAKE_UNITY_PLATFORM_ANDROID
#  define FAKE_UNITY_PLATFORM_ANDROID 1
#elif defined(_WIN32)
#  undef FAKE_UNITY_PLATFORM_WINDOWS
#  define FAKE_UNITY_PLATFORM_WINDOWS 1
#elif defined(__linux__)
#  undef FAKE_UNITY_PLATFORM_LINUX
#  define FAKE_UNITY_PLATFORM_LINUX 1
#elif defined(__APPLE__) && defined(__MACH__)
#  undef FAKE_UNITY_PLATFORM_MACOS
#  define FAKE_UNITY_PLATFORM_MACOS 1
#endif

#if defined(FAKE_UNITY_STATIC)
#  define FAKE_UNITY_DEF static
#else
#  define FAKE_UNITY_DEF extern
#endif

#include <stdint.h>

#ifndef __cplusplus
#  include <stdbool.h>
#endif

#if FAKE_UNITY_PLATFORM_WINDOWS
#  define WIN32_LEAN_AND_MEAN
#  include <windows.h>
#endif

typedef void (*PFN_UnityPluginLoad)(IUnityInterfaces *);
typedef void (*PFN_UnityPluginUnload)();

typedef struct FakeUnityInterface
{
    unsigned long long guid_high;
    unsigned long long guid_low;
    IUnityInterface *ptr;
} FakeUnityInterface;

typedef struct FakeUnityInterfaces
{
    int32_t count;
    int32_t allocated;
    FakeUnityInterface *items;
} FakeUnityInterfaces;

typedef struct FakeUnityNativePlugin
{
    PFN_UnityPluginLoad UnityPluginLoad;
    PFN_UnityPluginUnload UnityPluginUnload;

#if FAKE_UNITY_PLATFORM_WINDOWS
    HMODULE handle;
#elif FAKE_UNITY_PLATFORM_ANDROID || FAKE_UNITY_PLATFORM_LINUX || FAKE_UNITY_PLATFORM_MACOS
    void *handle;
#endif
} FakeUnityNativePlugin;

typedef struct FakeUnityGraphicsDeviceEventCallbacks
{
    int32_t count;
    int32_t allocated;
    IUnityGraphicsDeviceEventCallback *items;
} FakeUnityGraphicsDeviceEventCallbacks;

#define __FAKE_UNITY_VULKAN_GLOBAL_FUNCTIONS(__name__) \
    __name__(vkEnumerateInstanceVersion); \
    __name__(vkCreateInstance)

#define __FAKE_UNITY_VULKAN_INSTANCE_FUNCTIONS(__name__) \
    __name__(vkGetDeviceProcAddr); \
    __name__(vkEnumeratePhysicalDevices); \
    __name__(vkGetPhysicalDeviceProperties); \
    __name__(vkCreateDevice)

#define __FAKE_UNITY_VULKAN_DEVICE_FUNCTIONS(__name__) \
    __name__(vkGetDeviceQueue)

#define declare_function(name) PFN_##name name

typedef struct FakeUnityVulkanRenderer
{
#if FAKE_UNITY_PLATFORM_WINDOWS
    HMODULE loader_handle;
#elif FAKE_UNITY_PLATFORM_ANDROID || FAKE_UNITY_PLATFORM_LINUX || FAKE_UNITY_PLATFORM_MACOS
    void *loader_handle;
#endif

    VkInstance instance;
    VkPhysicalDevice physical_device;
    VkDevice device;
    VkQueue graphics_queue;
    uint32_t graphics_queue_index;

    PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr;
    PFN_vkGetInstanceProcAddr loader_vkGetInstanceProcAddr;

    __FAKE_UNITY_VULKAN_GLOBAL_FUNCTIONS(declare_function);

    __FAKE_UNITY_VULKAN_INSTANCE_FUNCTIONS(declare_function);

    __FAKE_UNITY_VULKAN_DEVICE_FUNCTIONS(declare_function);
} FakeUnityVulkanRenderer;

#undef declare_function

typedef struct FakeUnityState
{
    UnityGfxRenderer renderer_type;

    FakeUnityInterfaces interfaces;
    FakeUnityGraphicsDeviceEventCallbacks graphics_device_event_callbacks;

    FakeUnityNativePlugin *plugins;
    uint16_t *free_plugin_indices;
    uint16_t *plugin_generations;
    int32_t free_plugin_count;
    int32_t max_plugin_count;

    UnityVulkanInitCallback unity_vulkan_init_callback;
    void *unity_vulkan_init_userdata;

    IUnityInterfaces unity_interfaces;
    IUnityProfiler unity_profiler;
    IUnityGraphics unity_graphics;
    IUnityGraphicsVulkan unity_graphics_vulkan;

    union FakeUnityRenderer
    {
        FakeUnityVulkanRenderer vulkan;
    } renderer;
} FakeUnityState;

// This function initializes the fake_unity library and preallocates space
// for the native plugins. max_plugin_count determines how many plugins can
// be loaded at the same time, so this is best set to the upper bound of the
// expected number of plugins. Returns true on success.
FAKE_UNITY_DEF bool fake_unity_initialize(int32_t max_plugin_count);

// Loads a native plugin from a given filename and calls UnityPluginLoad if
// available. Returns a non zero plugin handle on success and zero on error.
FAKE_UNITY_DEF uint32_t fake_unity_load_native_plugin(const char *filename);

// Retrieves the pointer to a function from the native plugin.
FAKE_UNITY_DEF void *fake_unity_native_plugin_get_proc_address(uint32_t plugin_handle, const char *proc_name);

// Initializes the rendering subsystem with vulkan. device_index selects the
// physical vulkan device to use. If device_index is negative a default
// device is used. Returns true on success.
FAKE_UNITY_DEF bool fake_unity_create_vulkan_renderer(int32_t device_index);

// Returns the address of a vulkan instance procedure. Is only expected to be
// called after a successful call to fake_unit_create_vulkan_renderer.
// Returns non-NULL on success.
FAKE_UNITY_DEF void (*fake_unity_vulkan_get_instance_proc_address(const char *proc_name))(void);

// Returns the address of a vulkan device procedure. Is only expected to be
// called after a successful call to fake_unit_create_vulkan_renderer.
// Returns non-NULL on success.
FAKE_UNITY_DEF void (*fake_unity_vulkan_get_device_proc_address(const char *proc_name))(void);

#endif // __FAKE_UNITY_INCLUDE__

#if defined(FAKE_UNITY_IMPLEMENTATION)

static FakeUnityState __fake_unity_state;

#include <stdio.h>
#include <stdlib.h>

#if FAKE_UNITY_PLATFORM_ANDROID || FAKE_UNITY_PLATFORM_LINUX || FAKE_UNITY_PLATFORM_MACOS
#  include <dlfcn.h>
#endif

static inline const char *
__fake_unity_vk_physical_device_type_to_string(VkPhysicalDeviceType type)
{
    const char *str = "<unknown-vk-physical-device-type>";

#  define NAME(name) case name: str = #name; break

    switch (type)
    {
        NAME(VK_PHYSICAL_DEVICE_TYPE_OTHER);
        NAME(VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU);
        NAME(VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU);
        NAME(VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU);
        NAME(VK_PHYSICAL_DEVICE_TYPE_CPU);
        default: break;
    }

#  undef NAME

    return str;
}

#define ARRAY_ENSURE_SPACE(array, item_type)                                                      \
    do {                                                                                          \
        if ((array)->allocated == 0)                                                              \
        {                                                                                         \
            (array)->count = 0;                                                                   \
            (array)->allocated = 16;                                                              \
            (array)->items = (item_type *) malloc(sizeof(*(array)->items) * (array)->allocated);  \
        }                                                                                         \
        if ((array)->count == (array)->allocated)                                                 \
        {                                                                                         \
            (array)->allocated *= 2;                                                              \
            (array)->items = (item_type *) realloc((array)->items,                                \
                                                   sizeof(*(array)->items) * (array)->allocated); \
        }                                                                                         \
    } while (0)

static IUnityInterface *
IUnityInterfaces_GetInterfaceSplit(unsigned long long guid_high, unsigned long long guid_low)
{
    for (int32_t i = 0; i < __fake_unity_state.interfaces.count; i += 1)
    {
        FakeUnityInterface *item = __fake_unity_state.interfaces.items + i;

        if ((item->guid_high == guid_high) && (item->guid_low == guid_low))
        {
            return item->ptr;
        }
    }

    return 0;
}

static void
IUnityInterfaces_RegisterInterfaceSplit(unsigned long long guid_high, unsigned long long guid_low, IUnityInterface *ptr)
{
    ARRAY_ENSURE_SPACE(&__fake_unity_state.interfaces, FakeUnityInterface);

    FakeUnityInterface *item = __fake_unity_state.interfaces.items + __fake_unity_state.interfaces.count;
    __fake_unity_state.interfaces.count += 1;

    item->guid_high = guid_high;
    item->guid_low  = guid_low;
    item->ptr       = ptr;
}

static IUnityInterface *
IUnityInterfaces_GetInterface(UnityInterfaceGUID guid)
{
#ifndef __cplusplus
    fprintf(stderr, "[fake_unity] warning: your program is not compiled as a c++ program,\n"
                    "             but it looks like the native unity plugin you are loading is.\n"
                    "             Due to calling conventions being incompatible, calling GetInterface\n"
                    "             may not work. Please compile your program as a c++ program or\n"
                    "             make the native unity plugin call GetInterfaceSplit.\n");
#endif

    return IUnityInterfaces_GetInterfaceSplit(guid.m_GUIDHigh, guid.m_GUIDLow);
}

static void
IUnityInterfaces_RegisterInterface(UnityInterfaceGUID guid, IUnityInterface *ptr)
{
#ifndef __cplusplus
    fprintf(stderr, "[fake_unity] warning: your program is not compiled as a c++ program,\n"
                    "             but it looks like the native unity plugin you are loading is.\n"
                    "             Due to calling conventions being incompatible, calling RegisterInterface\n"
                    "             may not work. Please compile your program as a c++ program or\n"
                    "             make the native unity plugin call RegisterInterfaceSplit.\n");
#endif

    return IUnityInterfaces_RegisterInterfaceSplit(guid.m_GUIDHigh, guid.m_GUIDLow, ptr);
}

static void
IUnityProfiler_EmitEvent(const UnityProfilerMarkerDesc* markerDesc, UnityProfilerMarkerEventType eventType, uint16_t eventDataCount, const UnityProfilerMarkerData* eventData)
{
    fprintf(stderr, "[fake_unity] TODO: EmitEvent\n");
}

static int
IUnityProfiler_IsEnabled()
{
    fprintf(stderr, "[fake_unity] TODO: IsEnabled\n");
    return 0;
}

static int
IUnityProfiler_IsAvailable()
{
    fprintf(stderr, "[fake_unity] TODO: IsAvailable\n");
    return 0;
}

static int
IUnityProfiler_CreateMarker(const UnityProfilerMarkerDesc** desc, const char* name, UnityProfilerCategoryId category, UnityProfilerMarkerFlags flags, int eventDataCount)
{
    fprintf(stderr, "[fake_unity] TODO: CreateMarker\n");
    return 0;
}

static int
IUnityProfiler_SetMarkerMetadataName(const UnityProfilerMarkerDesc* desc, int index, const char* metadataName, UnityProfilerMarkerDataType metadataType, UnityProfilerMarkerDataUnit metadataUnit)
{
    fprintf(stderr, "[fake_unity] TODO: SetMarkerMetadataName\n");
    return 0;
}

static int
IUnityProfiler_RegisterThread(UnityProfilerThreadId* threadId, const char* groupName, const char* name)
{
    fprintf(stderr, "[fake_unity] TODO: RegisterThread\n");
    return 0;
}

static int
IUnityProfiler_UnregisterThread(UnityProfilerThreadId threadId)
{
    fprintf(stderr, "[fake_unity] TODO: UnregisterThread\n");
    return 0;
}

static UnityGfxRenderer
IUnityGraphics_GetRenderer()
{
    return __fake_unity_state.renderer_type;
}

static void
IUnityGraphics_RegisterDeviceEventCallback(IUnityGraphicsDeviceEventCallback callback)
{
    ARRAY_ENSURE_SPACE(&__fake_unity_state.graphics_device_event_callbacks, IUnityGraphicsDeviceEventCallback);

    __fake_unity_state.graphics_device_event_callbacks.items[__fake_unity_state.graphics_device_event_callbacks.count] = callback;
    __fake_unity_state.graphics_device_event_callbacks.count += 1;
}

static void
IUnityGraphics_UnregisterDeviceEventCallback(IUnityGraphicsDeviceEventCallback callback)
{
    for (int32_t i = 0; i < __fake_unity_state.graphics_device_event_callbacks.count; i += 1)
    {
        if (__fake_unity_state.graphics_device_event_callbacks.items[i] == callback)
        {
            __fake_unity_state.graphics_device_event_callbacks.count -= 1;
            __fake_unity_state.graphics_device_event_callbacks.items[i] =
                __fake_unity_state.graphics_device_event_callbacks.items[__fake_unity_state.graphics_device_event_callbacks.count];
        }
    }
}

static int
IUnityGraphics_ReserveEventIDRange(int count)
{
    fprintf(stderr, "[fake_unity] TODO: ReserveEventIDRange\n");
    // TODO:
    return 0;
}

static bool
UnityGraphicsVulkan_InterceptInitialization(UnityVulkanInitCallback func, void *userdata)
{
    __fake_unity_state.unity_vulkan_init_callback = func;
    __fake_unity_state.unity_vulkan_init_userdata = userdata;
    return true;
}

static PFN_vkVoidFunction
UnityGraphicsVulkan_InterceptVulkanAPI(const char *name, PFN_vkVoidFunction func)
{
    fprintf(stderr, "[fake_unity] TODO: InterceptVulkanAPI\n");
    return 0;
}

static void
UnityGraphicsVulkan_ConfigureEvent(int event_id, const UnityVulkanPluginEventConfig *plugin_event_config)
{
    fprintf(stderr, "[fake_unity] TODO: ConfigureEvent\n");
}

static UnityVulkanInstance
UnityGraphicsVulkan_Instance()
{
    UnityVulkanInstance vulkan_instance;

    vulkan_instance.pipelineCache = VK_NULL_HANDLE;
    vulkan_instance.instance = __fake_unity_state.renderer.vulkan.instance;
    vulkan_instance.physicalDevice = __fake_unity_state.renderer.vulkan.physical_device;
    vulkan_instance.device = __fake_unity_state.renderer.vulkan.device;
    vulkan_instance.graphicsQueue = __fake_unity_state.renderer.vulkan.graphics_queue;
    vulkan_instance.getInstanceProcAddr = __fake_unity_state.renderer.vulkan.loader_vkGetInstanceProcAddr;
    vulkan_instance.queueFamilyIndex = __fake_unity_state.renderer.vulkan.graphics_queue_index;

    return vulkan_instance;
}

static bool
UnityGraphicsVulkan_CommandRecordingState(UnityVulkanRecordingState *command_recording_state, UnityVulkanGraphicsQueueAccess queue_access)
{
    fprintf(stderr, "[fake_unity] TODO: CommandRecordingState\n");
    return false;
}

static bool
UnityGraphicsVulkan_AccessTexture(void* native_texture, const VkImageSubresource *sub_resource, VkImageLayout layout,
                                  VkPipelineStageFlags pipeline_stage_flags, VkAccessFlags access_flags,
                                  UnityVulkanResourceAccessMode access_mode, UnityVulkanImage *image)
{
    fprintf(stderr, "[fake_unity] TODO: AccessTexture\n");
    return false;
}

static bool
UnityGraphicsVulkan_AccessRenderBufferTexture(UnityRenderBuffer native_render_buffer, const VkImageSubresource *sub_resource, VkImageLayout layout,
                                              VkPipelineStageFlags pipeline_stage_flags, VkAccessFlags access_flags,
                                              UnityVulkanResourceAccessMode access_mode, UnityVulkanImage *image)
{
    fprintf(stderr, "[fake_unity] TODO: AccessRenderBufferTexture\n");
    return false;
}

static bool
UnityGraphicsVulkan_AccessRenderBufferResolveTexture(UnityRenderBuffer native_render_buffer, const VkImageSubresource *sub_resource, VkImageLayout layout,
                                                     VkPipelineStageFlags pipeline_stage_flags, VkAccessFlags access_flags,
                                                     UnityVulkanResourceAccessMode access_mode, UnityVulkanImage *image)
{
    fprintf(stderr, "[fake_unity] TODO: AccessRenderBufferResolveTexture\n");
    return false;
}

static bool
UnityGraphicsVulkan_AccessBuffer(void* native_buffer, VkPipelineStageFlags pipeline_stage_flags, VkAccessFlags access_flags,
                                 UnityVulkanResourceAccessMode access_mode, UnityVulkanBuffer *buffer)
{
    fprintf(stderr, "[fake_unity] TODO: AccessBuffer\n");
    return false;
}

static void
UnityGraphicsVulkan_EnsureOutsideRenderPass()
{
    fprintf(stderr, "[fake_unity] TODO: EnsureOutsideRenderPass\n");
}

static void
UnityGraphicsVulkan_EnsureInsideRenderPass()
{
    fprintf(stderr, "[fake_unity] TODO: EnsureInsideRenderPass\n");
}

static void
UnityGraphicsVulkan_AccessQueue(UnityRenderingEventAndData func, int event_id, void* userdata, bool flush)
{
    fprintf(stderr, "[fake_unity] TODO: AccessQueue\n");
}

static bool
UnityGraphicsVulkan_ConfigureSwapchain(const UnityVulkanSwapchainConfiguration *swapchain_config)
{
    fprintf(stderr, "[fake_unity] TODO: ConfigureSwapchain\n");
    return false;
}

static bool
UnityGraphicsVulkan_AccessTextureByID(UnityTextureID texture_id, const VkImageSubresource *sub_resource, VkImageLayout layout,
                                      VkPipelineStageFlags pipeline_stage_flags, VkAccessFlags access_flags,
                                      UnityVulkanResourceAccessMode access_mode, UnityVulkanImage *image)
{
    fprintf(stderr, "[fake_unity] TODO: AccessTextureByID\n");
    return false;
}

FAKE_UNITY_DEF bool
fake_unity_initialize(int32_t max_plugin_count)
{
    __fake_unity_state.renderer_type = kUnityGfxRendererNull;

    __fake_unity_state.unity_interfaces.GetInterface           = IUnityInterfaces_GetInterface;
    __fake_unity_state.unity_interfaces.RegisterInterface      = IUnityInterfaces_RegisterInterface;
    __fake_unity_state.unity_interfaces.GetInterfaceSplit      = IUnityInterfaces_GetInterfaceSplit;
    __fake_unity_state.unity_interfaces.RegisterInterfaceSplit = IUnityInterfaces_RegisterInterfaceSplit;

    __fake_unity_state.unity_profiler.EmitEvent             = IUnityProfiler_EmitEvent;
    __fake_unity_state.unity_profiler.IsEnabled             = IUnityProfiler_IsEnabled;
    __fake_unity_state.unity_profiler.IsAvailable           = IUnityProfiler_IsAvailable;
    __fake_unity_state.unity_profiler.CreateMarker          = IUnityProfiler_CreateMarker;
    __fake_unity_state.unity_profiler.SetMarkerMetadataName = IUnityProfiler_SetMarkerMetadataName;
    __fake_unity_state.unity_profiler.RegisterThread        = IUnityProfiler_RegisterThread;
    __fake_unity_state.unity_profiler.UnregisterThread      = IUnityProfiler_UnregisterThread;

    __fake_unity_state.unity_graphics.GetRenderer                   = IUnityGraphics_GetRenderer;
    __fake_unity_state.unity_graphics.RegisterDeviceEventCallback   = IUnityGraphics_RegisterDeviceEventCallback;
    __fake_unity_state.unity_graphics.UnregisterDeviceEventCallback = IUnityGraphics_UnregisterDeviceEventCallback;
    __fake_unity_state.unity_graphics.ReserveEventIDRange           = IUnityGraphics_ReserveEventIDRange;

    __fake_unity_state.unity_graphics_vulkan.InterceptInitialization          = UnityGraphicsVulkan_InterceptInitialization;
    __fake_unity_state.unity_graphics_vulkan.InterceptVulkanAPI               = UnityGraphicsVulkan_InterceptVulkanAPI;
    __fake_unity_state.unity_graphics_vulkan.ConfigureEvent                   = UnityGraphicsVulkan_ConfigureEvent;
    __fake_unity_state.unity_graphics_vulkan.Instance                         = UnityGraphicsVulkan_Instance;
    __fake_unity_state.unity_graphics_vulkan.CommandRecordingState            = UnityGraphicsVulkan_CommandRecordingState;
    __fake_unity_state.unity_graphics_vulkan.AccessTexture                    = UnityGraphicsVulkan_AccessTexture;
    __fake_unity_state.unity_graphics_vulkan.AccessRenderBufferTexture        = UnityGraphicsVulkan_AccessRenderBufferTexture;
    __fake_unity_state.unity_graphics_vulkan.AccessRenderBufferResolveTexture = UnityGraphicsVulkan_AccessRenderBufferResolveTexture;
    __fake_unity_state.unity_graphics_vulkan.AccessBuffer                     = UnityGraphicsVulkan_AccessBuffer;
    __fake_unity_state.unity_graphics_vulkan.EnsureOutsideRenderPass          = UnityGraphicsVulkan_EnsureOutsideRenderPass;
    __fake_unity_state.unity_graphics_vulkan.EnsureInsideRenderPass           = UnityGraphicsVulkan_EnsureInsideRenderPass;
    __fake_unity_state.unity_graphics_vulkan.AccessQueue                      = UnityGraphicsVulkan_AccessQueue;
    __fake_unity_state.unity_graphics_vulkan.ConfigureSwapchain               = UnityGraphicsVulkan_ConfigureSwapchain;
    __fake_unity_state.unity_graphics_vulkan.AccessTextureByID                = UnityGraphicsVulkan_AccessTextureByID;

    IUnityInterfaces_RegisterInterfaceSplit(0x2CE79ED8316A4833ULL, 0x87076B2013E1571FULL, &__fake_unity_state.unity_profiler);
    IUnityInterfaces_RegisterInterfaceSplit(0x7CBA0A9CA4DDB544ULL, 0x8C5AD4926EB17B11ULL, &__fake_unity_state.unity_graphics);
    IUnityInterfaces_RegisterInterfaceSplit(0x95355348d4ef4e11ULL, 0x9789313dfcffcc87ULL, &__fake_unity_state.unity_graphics_vulkan);

    if (max_plugin_count <= 0)
    {
        max_plugin_count = 8;
    }

    __fake_unity_state.plugins = (FakeUnityNativePlugin *) malloc(max_plugin_count * sizeof(FakeUnityNativePlugin));
    __fake_unity_state.free_plugin_indices = (uint16_t *) malloc(max_plugin_count * sizeof(uint16_t));
    __fake_unity_state.plugin_generations = (uint16_t *) malloc(max_plugin_count * sizeof(uint16_t));

    __fake_unity_state.max_plugin_count = max_plugin_count;
    __fake_unity_state.free_plugin_count = max_plugin_count;

    int32_t index = max_plugin_count;

    for (int32_t i = 0; i < max_plugin_count; i += 1)
    {
        __fake_unity_state.plugin_generations[i] = 1;
        __fake_unity_state.free_plugin_indices[i] = --index;
    }

    return true;
}

FAKE_UNITY_DEF uint32_t
fake_unity_load_native_plugin(const char *filename)
{
    uint32_t result = 0;

    if (__fake_unity_state.free_plugin_count > 0)
    {
#if FAKE_UNITY_PLATFORM_WINDOWS
        // TODO: use the unicode variant which requires converting utf8 to utf16
        HMODULE handle = LoadLibraryA(filename);

        if (!handle)
        {
            fprintf(stderr, "[fake_unity] error: could not load native plugin '%s'\n", filename);
            return 0;
        }
#elif FAKE_UNITY_PLATFORM_ANDROID || FAKE_UNITY_PLATFORM_LINUX || FAKE_UNITY_PLATFORM_MACOS
        void *handle = dlopen(filename, RTLD_NOW);

        if (!handle)
        {
            fprintf(stderr, "[fake_unity] error: could not load native plugin '%s' -> %s\n", filename, dlerror());
            return 0;
        }
#endif

        uint16_t index = __fake_unity_state.free_plugin_indices[--__fake_unity_state.free_plugin_count];
        uint16_t generation = __fake_unity_state.plugin_generations[index];

        result = ((uint32_t) generation << 16) | (uint32_t) index;

        FakeUnityNativePlugin *plugin = __fake_unity_state.plugins + index;

        plugin->handle = handle;

#if FAKE_UNITY_PLATFORM_WINDOWS
        plugin->UnityPluginLoad   = (PFN_UnityPluginLoad)   GetProcAddress(plugin->handle, "UnityPluginLoad");
        plugin->UnityPluginUnload = (PFN_UnityPluginUnload) GetProcAddress(plugin->handle, "UnityPluginUnload");
#elif FAKE_UNITY_PLATFORM_ANDROID || FAKE_UNITY_PLATFORM_LINUX || FAKE_UNITY_PLATFORM_MACOS
        plugin->UnityPluginLoad   = (PFN_UnityPluginLoad)   dlsym(plugin->handle, "UnityPluginLoad");
        plugin->UnityPluginUnload = (PFN_UnityPluginUnload) dlsym(plugin->handle, "UnityPluginUnload");
#endif

        if (plugin->UnityPluginLoad)
        {
            plugin->UnityPluginLoad(&__fake_unity_state.unity_interfaces);
        }
    }

    return result;
}

FAKE_UNITY_DEF void *
fake_unity_native_plugin_get_proc_address(uint32_t plugin_handle, const char *proc_name)
{
    void *result = 0;

    uint16_t index = (uint16_t) (plugin_handle & 0xFFFF);
    uint16_t generation = (uint16_t) ((plugin_handle >> 16) & 0xFFFF);

    if (__fake_unity_state.plugin_generations[index] == generation)
    {
        FakeUnityNativePlugin *plugin = __fake_unity_state.plugins + index;

#if FAKE_UNITY_PLATFORM_WINDOWS
        result = GetProcAddress(plugin->handle, proc_name);
#elif FAKE_UNITY_PLATFORM_ANDROID || FAKE_UNITY_PLATFORM_LINUX || FAKE_UNITY_PLATFORM_MACOS
        result = dlsym(plugin->handle, proc_name);
#endif
    }

    return result;
}

FAKE_UNITY_DEF bool
fake_unity_create_vulkan_renderer(int32_t device_index)
{
    if (__fake_unity_state.renderer_type != kUnityGfxRendererNull)
    {
        return false;
    }

    FakeUnityVulkanRenderer *renderer = &__fake_unity_state.renderer.vulkan;

    if (renderer->loader_handle)
    {
        return false;
    }

#if FAKE_UNITY_PLATFORM_WINDOWS
    renderer->loader_handle = LoadLibraryA("vulkan-1.dll");

    if (!renderer->loader_handle)
    {
        fprintf(stderr, "[fake_unity] error: could not load vulkan loader\n");
        return false;
    }

    renderer->loader_vkGetInstanceProcAddr = (PFN_vkGetInstanceProcAddr) GetProcAddress(renderer->loader_handle, "vkGetInstanceProcAddr");

#  define CLOSE_VULKAN_LOADER(handle) \
    FreeLibrary(handle); \
    handle = 0
#elif FAKE_UNITY_PLATFORM_ANDROID || FAKE_UNITY_PLATFORM_LINUX || FAKE_UNITY_PLATFORM_MACOS
    renderer->loader_handle = dlopen("libvulkan.so.1", RTLD_NOW);

    if (!renderer->loader_handle)
    {
        fprintf(stderr, "[fake_unity] error: could not load vulkan loader -> %s\n", dlerror());
        return false;
    }

    renderer->loader_vkGetInstanceProcAddr = (PFN_vkGetInstanceProcAddr) dlsym(renderer->loader_handle, "vkGetInstanceProcAddr");

#  define CLOSE_VULKAN_LOADER(handle) \
    dlclose(handle); \
    handle = 0
#endif

    if (!renderer->loader_vkGetInstanceProcAddr)
    {
        fprintf(stderr, "[fake_unity] error: could not load vulkan function 'vkGetInstanceProcAddr'.\n");
        CLOSE_VULKAN_LOADER(renderer->loader_handle);
        return false;
    }

    renderer->vkGetInstanceProcAddr = renderer->loader_vkGetInstanceProcAddr;

    PFN_vkGetInstanceProcAddr plugin_vkGetInstanceProcAddr = 0;

    if (__fake_unity_state.unity_vulkan_init_callback)
    {
        plugin_vkGetInstanceProcAddr = __fake_unity_state.unity_vulkan_init_callback(renderer->loader_vkGetInstanceProcAddr,
                                                                                     __fake_unity_state.unity_vulkan_init_userdata);

        if (plugin_vkGetInstanceProcAddr)
        {
            renderer->vkGetInstanceProcAddr = plugin_vkGetInstanceProcAddr;
        }
    }

#define load_function(name)                                                                       \
    do                                                                                            \
    {                                                                                             \
        renderer->name = (PFN_##name) renderer->vkGetInstanceProcAddr(VK_NULL_HANDLE, #name);     \
        if (!renderer->name)                                                                      \
        {                                                                                         \
            fprintf(stderr, "[fake_unity] error: could not load vulkan function '" #name "'.\n"); \
            CLOSE_VULKAN_LOADER(renderer->loader_handle);                                         \
            return false;                                                                         \
        }                                                                                         \
    } while (0)

    __FAKE_UNITY_VULKAN_GLOBAL_FUNCTIONS(load_function);

#undef load_function

    uint32_t vulkan_instance_version = VK_VERSION_1_0;
    renderer->vkEnumerateInstanceVersion(&vulkan_instance_version);

    fprintf(stderr, "[fake_unity] loaded vulkan library with instance version %u.%u.%u\n",
                    VK_VERSION_MAJOR(vulkan_instance_version),
                    VK_VERSION_MINOR(vulkan_instance_version),
                    VK_VERSION_PATCH(vulkan_instance_version));

    VkApplicationInfo application_info;
    application_info.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    application_info.pNext              = 0;
    application_info.pApplicationName   = "application";
    application_info.applicationVersion = 1;
    application_info.pEngineName        = "Unity";
    application_info.engineVersion      = 1;
    application_info.apiVersion         = VK_API_VERSION_1_0;

    VkInstanceCreateInfo instance_create_info;
    instance_create_info.sType                   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instance_create_info.pNext                   = 0;
    instance_create_info.flags                   = 0;
    instance_create_info.pApplicationInfo        = &application_info;
    instance_create_info.enabledLayerCount       = 0;
    instance_create_info.ppEnabledLayerNames     = 0;
    instance_create_info.enabledExtensionCount   = 0;
    instance_create_info.ppEnabledExtensionNames = 0;

    VkInstance instance;

    if (renderer->vkCreateInstance(&instance_create_info, 0, &instance) != VK_SUCCESS)
    {
        fprintf(stderr, "[fake_unity] error: vkCreateInstance failed.\n");
        CLOSE_VULKAN_LOADER(renderer->loader_handle);
        return false;
    }

    renderer->instance = instance;

#define load_function(name)                                                                       \
    do                                                                                            \
    {                                                                                             \
        renderer->name = (PFN_##name) renderer->vkGetInstanceProcAddr(instance, #name);           \
        if (!renderer->name)                                                                      \
        {                                                                                         \
            fprintf(stderr, "[fake_unity] error: could not load vulkan function '" #name "'.\n"); \
            CLOSE_VULKAN_LOADER(renderer->loader_handle);                                         \
            return false;                                                                         \
        }                                                                                         \
    } while (0)

    __FAKE_UNITY_VULKAN_INSTANCE_FUNCTIONS(load_function);

#undef load_function

    uint32_t physical_device_count = 0;

    if (renderer->vkEnumeratePhysicalDevices(instance, &physical_device_count, 0) != VK_SUCCESS)
    {
        CLOSE_VULKAN_LOADER(renderer->loader_handle);
        return false;
    }

    VkPhysicalDevice *physical_devices = (VkPhysicalDevice *) malloc(sizeof(*physical_devices) * physical_device_count);

    if (renderer->vkEnumeratePhysicalDevices(instance, &physical_device_count, physical_devices) != VK_SUCCESS)
    {
        free(physical_devices);
        CLOSE_VULKAN_LOADER(renderer->loader_handle);
        return false;
    }

    fprintf(stderr, "[fake_unity] %u physical devices:\n", physical_device_count);

    for (uint32_t i = 0; i < physical_device_count; i += 1)
    {
        VkPhysicalDeviceProperties properties;
        renderer->vkGetPhysicalDeviceProperties(physical_devices[i], &properties);

        fprintf(stderr, "[fake_unity] [%u] %s (type = %s) (api version = %u.%u.%u)\n",
                        i, properties.deviceName, __fake_unity_vk_physical_device_type_to_string(properties.deviceType),
                        VK_VERSION_MAJOR(properties.apiVersion), VK_VERSION_MINOR(properties.apiVersion), VK_VERSION_PATCH(properties.apiVersion));
    }

    if ((device_index < 0) || (device_index >= (int32_t) physical_device_count))
    {
        // TODO: find best device
        device_index = 0;
    }

    if ((device_index < 0) || (device_index >= (int32_t) physical_device_count))
    {
        fprintf(stderr, "[fake_unity] error: device_index = %d is out of bounds [0, %u).\n", device_index, physical_device_count);
        free(physical_devices);
        CLOSE_VULKAN_LOADER(renderer->loader_handle);
        return false;
    }

    VkPhysicalDevice physical_device = physical_devices[device_index];

    free(physical_devices);

    fprintf(stderr, "[fake_unity] selected device at index %d\n", device_index);

    renderer->physical_device = physical_device;

    uint32_t graphics_queue_index = 0; // TODO: correct index

    float queue_priority = 1.0f;

    VkDeviceQueueCreateInfo queue_create_info;
    queue_create_info.sType               = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queue_create_info.pNext               = 0;
    queue_create_info.flags               = 0;
    queue_create_info.queueFamilyIndex    = graphics_queue_index;
    queue_create_info.queueCount          = 1;
    queue_create_info.pQueuePriorities    = &queue_priority;

    VkDeviceCreateInfo device_create_info;
    device_create_info.sType                   = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    device_create_info.pNext                   = 0;
    device_create_info.flags                   = 0;
    device_create_info.queueCreateInfoCount    = 1;
    device_create_info.pQueueCreateInfos       = &queue_create_info;
    device_create_info.enabledLayerCount       = 0;
    device_create_info.ppEnabledLayerNames     = 0;
    device_create_info.enabledExtensionCount   = 0;
    device_create_info.ppEnabledExtensionNames = 0;
    device_create_info.pEnabledFeatures        = 0;

    VkDevice device;

    if (renderer->vkCreateDevice(physical_device, &device_create_info, 0, &device) != VK_SUCCESS)
    {
        CLOSE_VULKAN_LOADER(renderer->loader_handle);
        return false;
    }

    renderer->device = device;

#define load_function(name)                                                                       \
    do                                                                                            \
    {                                                                                             \
        renderer->name = (PFN_##name) renderer->vkGetDeviceProcAddr(device, #name);               \
        if (!renderer->name)                                                                      \
        {                                                                                         \
            fprintf(stderr, "[fake_unity] error: could not load vulkan function '" #name "'.\n"); \
            CLOSE_VULKAN_LOADER(renderer->loader_handle);                                         \
            return false;                                                                         \
        }                                                                                         \
    } while (0)

    __FAKE_UNITY_VULKAN_DEVICE_FUNCTIONS(load_function);

#undef load_function

    VkQueue graphics_queue;

    renderer->vkGetDeviceQueue(device, graphics_queue_index, 0, &graphics_queue);

    renderer->graphics_queue_index = graphics_queue_index;
    renderer->graphics_queue = graphics_queue;

#undef CLOSE_VULKAN_LOADER

    __fake_unity_state.renderer_type = kUnityGfxRendererVulkan;

    for (int32_t i = 0; i < __fake_unity_state.graphics_device_event_callbacks.count; i += 1)
    {
        __fake_unity_state.graphics_device_event_callbacks.items[i](kUnityGfxDeviceEventInitialize);
    }

    return true;
}

FAKE_UNITY_DEF void
(*fake_unity_vulkan_get_instance_proc_address(const char *proc_name))(void)
{
    if ((__fake_unity_state.renderer_type == kUnityGfxRendererVulkan) &&
        (__fake_unity_state.renderer.vulkan.vkGetInstanceProcAddr))
    {
        return __fake_unity_state.renderer.vulkan.vkGetInstanceProcAddr(__fake_unity_state.renderer.vulkan.instance, proc_name);
    }

    return NULL;
}

FAKE_UNITY_DEF void
(*fake_unity_vulkan_get_device_proc_address(const char *proc_name))(void)
{
    if ((__fake_unity_state.renderer_type == kUnityGfxRendererVulkan) &&
        (__fake_unity_state.renderer.vulkan.vkGetDeviceProcAddr))
    {
        return __fake_unity_state.renderer.vulkan.vkGetDeviceProcAddr(__fake_unity_state.renderer.vulkan.device, proc_name);
    }

    return NULL;
}

#undef ARRAY_ENSURE_SPACE

#endif // defined(FAKE_UNITY_IMPLEMENTATION)

/*
MIT License

Copyright (c) 2025 Julius Range-Lüdemann

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
