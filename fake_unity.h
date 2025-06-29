// fake_unity.h - MIT License
// See end of file for full license

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
    // TODO:
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
    __name__(vkGetPhysicalDeviceProperties)

#define declare_function(name) PFN_##name name

typedef struct FakeUnityVulkanRenderer
{
#if FAKE_UNITY_PLATFORM_WINDOWS
    // TODO:
#elif FAKE_UNITY_PLATFORM_ANDROID || FAKE_UNITY_PLATFORM_LINUX || FAKE_UNITY_PLATFORM_MACOS
    void *loader_handle;
#endif

    PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr;

    __FAKE_UNITY_VULKAN_GLOBAL_FUNCTIONS(declare_function);

    __FAKE_UNITY_VULKAN_INSTANCE_FUNCTIONS(declare_function);
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
    IUnityGraphics unity_graphics;
    IUnityGraphicsVulkan unity_graphics_vulkan;

    union FakeUnityRenderer
    {
        FakeUnityVulkanRenderer vulkan;
    } renderer;
} FakeUnityState;

FAKE_UNITY_DEF bool fake_unity_initialize(UnityGfxRenderer renderer, int32_t max_plugin_count);
FAKE_UNITY_DEF uint32_t fake_unity_load_native_plugin(const char *filename);
FAKE_UNITY_DEF void *fake_unity_native_plugin_get_proc_address(uint32_t plugin_handle, const char *proc_name);
FAKE_UNITY_DEF bool fake_unity_create_vulkan_renderer(int32_t device_index);

#endif // __FAKE_UNITY_INCLUDE__

#if defined(FAKE_UNITY_IMPLEMENTATION)

static FakeUnityState __fake_unity_state;

#if FAKE_UNITY_PLATFORM_WINDOWS
    // TODO:
#elif FAKE_UNITY_PLATFORM_ANDROID || FAKE_UNITY_PLATFORM_LINUX || FAKE_UNITY_PLATFORM_MACOS
#  include <dlfcn.h>
#endif

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
    fprintf(stderr, "[fake_unity] TODO: Instance\n");
    UnityVulkanInstance instance;
    return instance;
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
fake_unity_initialize(UnityGfxRenderer renderer_type, int32_t max_plugin_count)
{
    __fake_unity_state.renderer_type = kUnityGfxRendererNull;

    // for now we only support vulkan as a renderer
    if (renderer_type != kUnityGfxRendererVulkan)
    {
        return false;
    }

    __fake_unity_state.renderer_type = renderer_type;

    __fake_unity_state.unity_interfaces.GetInterface           = IUnityInterfaces_GetInterface;
    __fake_unity_state.unity_interfaces.RegisterInterface      = IUnityInterfaces_RegisterInterface;
    __fake_unity_state.unity_interfaces.GetInterfaceSplit      = IUnityInterfaces_GetInterfaceSplit;
    __fake_unity_state.unity_interfaces.RegisterInterfaceSplit = IUnityInterfaces_RegisterInterfaceSplit;

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

    IUnityInterfaces_RegisterInterfaceSplit(0x7CBA0A9CA4DDB544ULL, 0x8C5AD4926EB17B11ULL, &__fake_unity_state.unity_graphics);

    switch (__fake_unity_state.renderer_type)
    {
        case kUnityGfxRendererVulkan:
        {
            IUnityInterfaces_RegisterInterfaceSplit(0x95355348d4ef4e11ULL, 0x9789313dfcffcc87ULL, &__fake_unity_state.unity_graphics_vulkan);
        } break;

        default: break;
    }

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
        // TODO:
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
        // TODO:
#elif FAKE_UNITY_PLATFORM_ANDROID || FAKE_UNITY_PLATFORM_LINUX || FAKE_UNITY_PLATFORM_MACOS
        plugin->UnityPluginLoad = (PFN_UnityPluginLoad) dlsym(plugin->handle, "UnityPluginLoad");
        plugin->UnityPluginUnload = (PFN_UnityPluginUnload) dlsym(plugin->handle, "UnityPluginUnload");

        if (plugin->UnityPluginLoad)
        {
            plugin->UnityPluginLoad(&__fake_unity_state.unity_interfaces);
        }
#endif
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
        // TODO:
#elif FAKE_UNITY_PLATFORM_ANDROID || FAKE_UNITY_PLATFORM_LINUX || FAKE_UNITY_PLATFORM_MACOS
        result = dlsym(plugin->handle, proc_name);
#endif
    }

    return result;
}

FAKE_UNITY_DEF bool
fake_unity_create_vulkan_renderer(int32_t device_index)
{
    if (__fake_unity_state.renderer_type != kUnityGfxRendererVulkan)
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
        fprintf(stderr, "[fake_unity] error: could not load vulkan loader -> %s\n", dlerror());
        return false;
    }

    renderer->vkGetInstanceProcAddr = (PFN_vkGetInstanceProcAddr) GetProcAddress(renderer->loader_handle, "vkGetInstanceProcAddr");

#  define CLOSE_VULKAN_LOADER(handle) \
    FreeLibrary(handle); \
    handle = 0
#elif FAKE_UNITY_PLATFORM_ANDROID || FAKE_UNITY_PLATFORM_LINUX || FAKE_UNITY_PALTFORM_MACOS
    renderer->loader_handle = dlopen("libvulkan.so.1", RTLD_NOW);

    if (!renderer->loader_handle)
    {
        fprintf(stderr, "[fake_unity] error: could not load vulkan loader -> %s\n", dlerror());
        return false;
    }

    renderer->vkGetInstanceProcAddr = (PFN_vkGetInstanceProcAddr) dlsym(renderer->loader_handle, "vkGetInstanceProcAddr");

#  define CLOSE_VULKAN_LOADER(handle) \
    dlclose(handle); \
    handle = 0
#endif

    if (!renderer->vkGetInstanceProcAddr)
    {
        fprintf(stderr, "[fake_unity] error: could not load vulkan function 'vkGetInstanceProcAddr'.\n");
        CLOSE_VULKAN_LOADER(renderer->loader_handle);
        return false;
    }

    PFN_vkGetInstanceProcAddr plugin_vkGetInstanceProcAddr = 0;

    if (__fake_unity_state.unity_vulkan_init_callback)
    {
        plugin_vkGetInstanceProcAddr = __fake_unity_state.unity_vulkan_init_callback(renderer->vkGetInstanceProcAddr,
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

    // TODO: what does unity set here?
    VkApplicationInfo application_info;
    application_info.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    application_info.pNext              = 0;
    application_info.pApplicationName   = "unity";
    application_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    application_info.pEngineName        = "unity";
    application_info.engineVersion      = VK_MAKE_VERSION(1, 0, 0);
    application_info.apiVersion         = VK_API_VERSION_1_1;

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

        fprintf(stderr, "[fake_unity]   [%u] %s (type = %d) (api version = %u.%u.%u)\n", i, properties.deviceName, properties.deviceType,
                        VK_VERSION_MAJOR(properties.apiVersion), VK_VERSION_MINOR(properties.apiVersion), VK_VERSION_PATCH(properties.apiVersion));
    }

    if (device_index < 0)
    {
        // TODO: find best device
        device_index = 0;
    }

    if (device_index < 0)
    {
        CLOSE_VULKAN_LOADER(renderer->loader_handle);
        return false;
    }

#undef CLOSE_VULKAN_LOADER

    return true;
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
