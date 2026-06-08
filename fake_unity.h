// SPDX-License-Identifier: MIT
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
//   #include <stddef.h>
//
//   #include "IUnityLog.h" // includes IUnityInterface.h
//   #include "IUnityProfiler.h"
//   #include "IUnityGraphics.h"
//   #define VK_NO_PROTOTYPES
//   #include "IUnityGraphicsVulkan.h" // includes vulkan/vulkan.h
//
//   #define FAKE_UNITY_IMPLEMENTATION
//   #define FAKE_UNITY_GRAPHICS_VULKAN
//   #include "fake_unity.h"
//
//   typedef int32_t (*PFN_MyNativeFunction)(int32_t a, int32_t b);
//
//   PFN_MyNativeFunction MyNativeFunction;
//
//   int main()
//   {
//       fake_unity_initialize(8, 4);
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

#if defined(FAKE_UNITY_GRAPHICS_OPENGLES) || defined(FAKE_UNITY_GRAPHICS_OPENGL_CORE)

#  define FAKE_UNITY_MAKE_OPENGL_VERSION(major, minor) (((uint32_t) (major) << 16) | (uint32_t) (minor))

#  include <EGL/egl.h>
#  include <GLES/gl.h>

#ifndef EGL_EXT_device_base
#define EGL_EXT_device_base 1
typedef void *EGLDeviceEXT;
#define EGL_NO_DEVICE_EXT                 EGL_CAST(EGLDeviceEXT,0)
#define EGL_BAD_DEVICE_EXT                0x322B
#define EGL_DEVICE_EXT                    0x322C
typedef EGLBoolean (EGLAPIENTRYP PFNEGLQUERYDEVICEATTRIBEXTPROC) (EGLDeviceEXT device, EGLint attribute, EGLAttrib *value);
typedef const char *(EGLAPIENTRYP PFNEGLQUERYDEVICESTRINGEXTPROC) (EGLDeviceEXT device, EGLint name);
typedef EGLBoolean (EGLAPIENTRYP PFNEGLQUERYDEVICESEXTPROC) (EGLint max_devices, EGLDeviceEXT *devices, EGLint *num_devices);
typedef EGLBoolean (EGLAPIENTRYP PFNEGLQUERYDISPLAYATTRIBEXTPROC) (EGLDisplay dpy, EGLint attribute, EGLAttrib *value);
#ifdef EGL_EGLEXT_PROTOTYPES
EGLAPI EGLBoolean EGLAPIENTRY eglQueryDeviceAttribEXT (EGLDeviceEXT device, EGLint attribute, EGLAttrib *value);
EGLAPI const char *EGLAPIENTRY eglQueryDeviceStringEXT (EGLDeviceEXT device, EGLint name);
EGLAPI EGLBoolean EGLAPIENTRY eglQueryDevicesEXT (EGLint max_devices, EGLDeviceEXT *devices, EGLint *num_devices);
EGLAPI EGLBoolean EGLAPIENTRY eglQueryDisplayAttribEXT (EGLDisplay dpy, EGLint attribute, EGLAttrib *value);
#endif
#endif /* EGL_EXT_device_base */

#ifndef EGL_EXT_device_query_name
#define EGL_EXT_device_query_name 1
#define EGL_RENDERER_EXT                  0x335F
#endif /* EGL_EXT_device_query_name */

#ifndef EGL_EXT_device_type
#define EGL_EXT_device_type 1
#define EGL_DEVICE_TYPE_EXT               0x3590
#define EGL_DEVICE_TYPE_OTHER_EXT         0x3591
#define EGL_DEVICE_TYPE_INTEGRATED_GPU_EXT 0x3592
#define EGL_DEVICE_TYPE_DISCRETE_GPU_EXT  0x3593
#define EGL_DEVICE_TYPE_CPU_EXT           0x3594
#endif /* EGL_EXT_device_type */

#ifndef EGL_EXT_platform_device
#define EGL_EXT_platform_device 1
#define EGL_PLATFORM_DEVICE_EXT           0x313F
#endif /* EGL_EXT_platform_device */

#ifndef EGL_KHR_create_context
#define EGL_KHR_create_context 1
#define EGL_CONTEXT_MAJOR_VERSION_KHR     0x3098
#define EGL_CONTEXT_MINOR_VERSION_KHR     0x30FB
#define EGL_CONTEXT_FLAGS_KHR             0x30FC
#define EGL_CONTEXT_OPENGL_PROFILE_MASK_KHR 0x30FD
#define EGL_CONTEXT_OPENGL_RESET_NOTIFICATION_STRATEGY_KHR 0x31BD
#define EGL_NO_RESET_NOTIFICATION_KHR     0x31BE
#define EGL_LOSE_CONTEXT_ON_RESET_KHR     0x31BF
#define EGL_CONTEXT_OPENGL_DEBUG_BIT_KHR  0x00000001
#define EGL_CONTEXT_OPENGL_FORWARD_COMPATIBLE_BIT_KHR 0x00000002
#define EGL_CONTEXT_OPENGL_ROBUST_ACCESS_BIT_KHR 0x00000004
#define EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT_KHR 0x00000001
#define EGL_CONTEXT_OPENGL_COMPATIBILITY_PROFILE_BIT_KHR 0x00000002
#define EGL_OPENGL_ES3_BIT_KHR            0x00000040
#endif /* EGL_KHR_create_context */

#ifndef EGL_KHR_no_config_context
#define EGL_KHR_no_config_context 1
#define EGL_NO_CONFIG_KHR                 EGL_CAST(EGLConfig,0)
#endif /* EGL_KHR_no_config_context */

typedef struct FakeUnityOpenGlRenderer
{
    PFNEGLQUERYDEVICESEXTPROC eglQueryDevicesEXT;
    PFNEGLQUERYDEVICEATTRIBEXTPROC eglQueryDeviceAttribEXT;
    PFNEGLQUERYDEVICESTRINGEXTPROC eglQueryDeviceStringEXT;
} FakeUnityOpenGlRenderer;

#endif

#if defined(FAKE_UNITY_GRAPHICS_VULKAN)

#  define __FAKE_UNITY_VULKAN_GLOBAL_FUNCTIONS(__name__) \
    __name__(vkCreateInstance)

#  define __FAKE_UNITY_VULKAN_INSTANCE_FUNCTIONS(__name__) \
    __name__(vkGetDeviceProcAddr); \
    __name__(vkEnumeratePhysicalDevices); \
    __name__(vkGetPhysicalDeviceProperties); \
    __name__(vkGetPhysicalDeviceQueueFamilyProperties); \
    __name__(vkCreateDevice)

#  define __FAKE_UNITY_VULKAN_DEVICE_FUNCTIONS(__name__) \
    __name__(vkGetDeviceQueue); \
    __name__(vkCreateImageView); \
    __name__(vkDestroyImageView)

#  define declare_function(name) PFN_##name name

typedef struct FakeUnityVulkanRenderer
{
#  if FAKE_UNITY_PLATFORM_WINDOWS
    HMODULE loader_handle;
#  elif FAKE_UNITY_PLATFORM_ANDROID || FAKE_UNITY_PLATFORM_LINUX || FAKE_UNITY_PLATFORM_MACOS
    void *loader_handle;
#  endif

    VkInstance instance;
    VkPhysicalDevice physical_device;
    VkDevice device;
    VkQueue graphics_queue;
    uint32_t graphics_queue_index;

    PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr;
    PFN_vkGetInstanceProcAddr loader_vkGetInstanceProcAddr;

    PFN_vkEnumerateInstanceVersion vkEnumerateInstanceVersion;
    __FAKE_UNITY_VULKAN_GLOBAL_FUNCTIONS(declare_function);

    __FAKE_UNITY_VULKAN_INSTANCE_FUNCTIONS(declare_function);

    __FAKE_UNITY_VULKAN_DEVICE_FUNCTIONS(declare_function);
} FakeUnityVulkanRenderer;

#  undef declare_function

#endif

typedef struct FakeUnityTexture
{
    int32_t width;
    int32_t height;

    union
    {
#if defined(FAKE_UNITY_GRAPHICS_OPENGLES) || defined(FAKE_UNITY_GRAPHICS_OPENGL_CORE)
        GLuint gl_texture;
#endif
#if defined(FAKE_UNITY_GRAPHICS_VULKAN)
        VkImageView vk_image_view;
#endif
    } api;
} FakeUnityTexture;

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

    FakeUnityTexture *textures;
    uint16_t *free_texture_indices;
    uint16_t *texture_generations;
    int32_t free_texture_count;
    int32_t max_texture_count;

#if defined(FAKE_UNITY_GRAPHICS_VULKAN)
    UnityVulkanInitCallback unity_vulkan_init_callback;
    void *unity_vulkan_init_userdata;
#endif

    IUnityInterfaces unity_interfaces;
#if !defined(FAKE_UNITY_NO_LOG)
    IUnityLog unity_log;
#endif
#if !defined(FAKE_UNITY_NO_PROFILER)
    IUnityProfiler unity_profiler;
#endif
    IUnityGraphics unity_graphics;
#if defined(FAKE_UNITY_GRAPHICS_VULKAN)
    IUnityGraphicsVulkan unity_graphics_vulkan;
#endif

    union FakeUnityRenderer
    {
#if defined(FAKE_UNITY_GRAPHICS_OPENGLES) || defined(FAKE_UNITY_GRAPHICS_OPENGL_CORE)
        FakeUnityOpenGlRenderer opengl;
#endif
#if defined(FAKE_UNITY_GRAPHICS_VULKAN)
        FakeUnityVulkanRenderer vulkan;
#endif
    } renderer;
} FakeUnityState;

// IMPORTANT: These format values are NOT the same as
// the TextureFormat values defined in the C# scripting api.
// So FakeUnity_TextureFormat_BGRA32 != TextureFormat.BGRA32
typedef enum FakeUnity_TextureFormat
{
    FakeUnity_TextureFormat_Alpha8   = 0,
    FakeUnity_TextureFormat_ARGB4444 = 1,
    FakeUnity_TextureFormat_RGB24    = 2,
    FakeUnity_TextureFormat_RGBA32   = 3,
    FakeUnity_TextureFormat_ARGB32   = 4,
    FakeUnity_TextureFormat_RGB565   = 5,

    // TODO: all the rest
} FakeUnity_TextureFormat;

typedef uint32_t FakeUnity_Texture2D;

// This function initializes the fake_unity library and preallocates space
// for the native plugins. max_plugin_count determines how many plugins can
// be loaded at the same time, so this is best set to the upper bound of the
// expected number of plugins. Same for the max_texture_count which determines
// how many external textures can be created. Returns true on success.
FAKE_UNITY_DEF bool fake_unity_initialize(int32_t max_plugin_count, int32_t max_texture_count);

// Loads a native plugin from a given filename and calls UnityPluginLoad if
// available. Returns a non zero plugin handle on success and zero on error.
FAKE_UNITY_DEF uint32_t fake_unity_load_native_plugin(const char *filename);

// Retrieves the pointer to a function from the native plugin.
FAKE_UNITY_DEF void *fake_unity_native_plugin_get_proc_address(uint32_t plugin_handle, const char *proc_name);

#if defined(FAKE_UNITY_GRAPHICS_OPENGLES)

// Initializes the rendering subsystem with opengl es. gl_version is the opengl es
// version to use. If gl_version is zero a default version is used. device_index
// selects the egl device to use. If device_index is negative a default device is
// used. Returns true on success.
FAKE_UNITY_DEF bool fake_unity_create_opengles_renderer(uint32_t gl_version, int32_t device_index);

#endif

#if defined(FAKE_UNITY_GRAPHICS_OPENGL_CORE)

// Initializes the rendering subsystem with opengl core. gl_version is the opengl
// version to use. If gl_version is zero a default version is used. device_index
// selects the egl device to use. If device_index is negative a default device is
// used. Returns true on success.
FAKE_UNITY_DEF bool fake_unity_create_opengl_core_renderer(uint32_t gl_version, int32_t device_index);

#endif

#if defined(FAKE_UNITY_GRAPHICS_VULKAN)

// Initializes the rendering subsystem with vulkan. device_index selects the
// physical vulkan device to use. If device_index is negative a default
// device is used. Returns true on success.
// It should be called after fake_unity_load_native_plugin so the native
// plugin can hook into the vulkan instance and device creation.
FAKE_UNITY_DEF bool fake_unity_create_vulkan_renderer(int32_t device_index);

// Returns the address of a vulkan instance procedure. Is only expected to be
// called after a successful call to fake_unit_create_vulkan_renderer.
// Returns non-NULL on success.
FAKE_UNITY_DEF PFN_vkVoidFunction fake_unity_vulkan_get_instance_proc_address(const char *proc_name);

// Returns the address of a vulkan device procedure. Is only expected to be
// called after a successful call to fake_unit_create_vulkan_renderer.
// Returns non-NULL on success.
FAKE_UNITY_DEF PFN_vkVoidFunction fake_unity_vulkan_get_device_proc_address(const char *proc_name);

#endif

// This implements the C# scripting api function Texture2D.CreateExternalTexture.
// See https://docs.unity3d.com/6000.0/Documentation/ScriptReference/Texture2D.CreateExternalTexture.html.
FAKE_UNITY_DEF FakeUnity_Texture2D fake_unity_Texture2D_CreateExternalTexture(int32_t width, int32_t height, FakeUnity_TextureFormat format, bool mip_chain, bool linear, void *native_texture);

FAKE_UNITY_DEF void fake_unity_Texture2D_Destroy(FakeUnity_Texture2D texture_handle);

#endif // __FAKE_UNITY_INCLUDE__

#if defined(FAKE_UNITY_IMPLEMENTATION)

#define PREFIX "[fake_unity] "

static FakeUnityState __fake_unity_state;

#include <stdio.h>
#include <stdlib.h>

#if FAKE_UNITY_PLATFORM_ANDROID || FAKE_UNITY_PLATFORM_LINUX || FAKE_UNITY_PLATFORM_MACOS
#  include <dlfcn.h>
#endif

static inline bool
__fake_unity_sized_string_equal(size_t a_count, const char *a, const char *b)
{
    for (size_t i = 0; i < a_count; i += 1)
    {
        if (a[i] != b[i])
        {
            return false;
        }
    }

    return (b[a_count] == 0);
}

#if defined(FAKE_UNITY_GRAPHICS_OPENGLES) || defined(FAKE_UNITY_GRAPHICS_OPENGL_CORE)

static inline const char *
__fake_unity_egl_error_to_string(EGLint error)
{
    const char *str = "<unknown-egl-error>";

#define NAME(name) case name: str = #name; break

    switch (error)
    {
        NAME(EGL_SUCCESS);
        NAME(EGL_NOT_INITIALIZED);
        NAME(EGL_BAD_ACCESS);
        NAME(EGL_BAD_ALLOC);
        NAME(EGL_BAD_ATTRIBUTE);
        NAME(EGL_BAD_CONTEXT);
        NAME(EGL_BAD_CONFIG);
        NAME(EGL_BAD_CURRENT_SURFACE);
        NAME(EGL_BAD_DISPLAY);
        NAME(EGL_BAD_SURFACE);
        NAME(EGL_BAD_MATCH);
        NAME(EGL_BAD_PARAMETER);
        NAME(EGL_BAD_NATIVE_PIXMAP);
        NAME(EGL_BAD_NATIVE_WINDOW);
        NAME(EGL_CONTEXT_LOST);
        NAME(EGL_BAD_DEVICE_EXT);
        default: break;
    }

#undef NAME

    return str;
}

static inline const char *
__fake_unity_egl_device_type_to_string(EGLAttrib type)
{
    const char *str = "<unknown-egl-device-type>";

#  define NAME(name) case name: str = #name; break

    switch (type)
    {
        NAME(EGL_DEVICE_TYPE_OTHER_EXT);
        NAME(EGL_DEVICE_TYPE_INTEGRATED_GPU_EXT);
        NAME(EGL_DEVICE_TYPE_DISCRETE_GPU_EXT);
        NAME(EGL_DEVICE_TYPE_CPU_EXT);
        default: break;
    }

#  undef NAME

    return str;
}

// The order in this table matters for preference.
// They are ordered from most to least preferable.
static const EGLAttrib __fake_unity_preferred_egl_device_types[] = {
    EGL_DEVICE_TYPE_DISCRETE_GPU_EXT,
    EGL_DEVICE_TYPE_INTEGRATED_GPU_EXT,
    EGL_DEVICE_TYPE_CPU_EXT,
};

static inline uint32_t
__fake_unity_get_prefer_index_from_egl_device_type(EGLAttrib device_type)
{
    uint32_t result = UINT32_MAX;

    for (size_t i = 0; i < (sizeof(__fake_unity_preferred_egl_device_types) / sizeof(__fake_unity_preferred_egl_device_types[0])); i += 1)
    {
        if (__fake_unity_preferred_egl_device_types[i] == device_type)
        {
            result = (uint32_t) i;
            break;
        }
    }

    return result;
}

static inline bool
__fake_unity_egl_device_type_is_better(EGLAttrib a, EGLAttrib b)
{
    uint32_t a_prefer_index = __fake_unity_get_prefer_index_from_egl_device_type(a);
    uint32_t b_prefer_index = __fake_unity_get_prefer_index_from_egl_device_type(b);

    return (a_prefer_index < b_prefer_index);
}

static bool
__fake_unity_create_opengl_renderer(EGLenum api, uint32_t gl_version, int32_t device_index)
{
    if (__fake_unity_state.renderer_type != kUnityGfxRendererNull)
    {
        return false;
    }

    if (gl_version == 0)
    {
        if (api == EGL_OPENGL_ES_API)
        {
            gl_version = FAKE_UNITY_MAKE_OPENGL_VERSION(2, 0);
        }
        else if (api == EGL_OPENGL_API)
        {
            gl_version = FAKE_UNITY_MAKE_OPENGL_VERSION(3, 2);
        }
    }

    FakeUnityOpenGlRenderer *renderer = &__fake_unity_state.renderer.opengl;

    const char *client_extensions = eglQueryString(EGL_NO_DISPLAY, EGL_EXTENSIONS);

    if (!client_extensions)
    {
        fprintf(stderr, PREFIX "error: egl does not have client extensions\n");
        return false;
    }

    bool has_EGL_EXT_device_query = false;
    bool has_EGL_EXT_platform_base = false;
    bool has_EGL_EXT_platform_device = false;
    bool has_EGL_EXT_device_enumeration = false;

    const char *at = client_extensions;

    for (;;)
    {
        while (*at == ' ')  at += 1;
        if (!*at)  break;
        const char *start = at;
        while (*at && (*at != ' '))  at += 1;

        if (__fake_unity_sized_string_equal(at - start, start, "EGL_EXT_device_query"))
        {
            has_EGL_EXT_device_query = true;
        }
        else if (__fake_unity_sized_string_equal(at - start, start, "EGL_EXT_platform_base"))
        {
            has_EGL_EXT_platform_base = true;
        }
        else if (__fake_unity_sized_string_equal(at - start, start, "EGL_EXT_platform_device"))
        {
            has_EGL_EXT_platform_device = true;
        }
        else if (__fake_unity_sized_string_equal(at - start, start, "EGL_EXT_device_enumeration"))
        {
            has_EGL_EXT_device_enumeration = true;
        }
    }

    if (!has_EGL_EXT_device_enumeration)
    {
        fprintf(stderr, PREFIX "error: egl does not have client extension 'EGL_EXT_device_enumeration'\n");
        return false;
    }

    if (!has_EGL_EXT_platform_base)
    {
        fprintf(stderr, PREFIX "error: egl does not have client extension 'EGL_EXT_platform_base'\n");
        return false;
    }

    if (!has_EGL_EXT_platform_device)
    {
        fprintf(stderr, PREFIX "error: egl does not have client extension 'EGL_EXT_platform_device'\n");
        return false;
    }

    renderer->eglQueryDevicesEXT = (PFNEGLQUERYDEVICESEXTPROC) eglGetProcAddress("eglQueryDevicesEXT");

    if (has_EGL_EXT_device_query)
    {
        renderer->eglQueryDeviceAttribEXT = (PFNEGLQUERYDEVICEATTRIBEXTPROC) eglGetProcAddress("eglQueryDeviceAttribEXT");

        if (!renderer->eglQueryDeviceAttribEXT)
        {
            fprintf(stderr, PREFIX "error: could not load egl client function 'eglQueryDeviceAttribEXT'.\n");
            return false;
        }

        renderer->eglQueryDeviceStringEXT = (PFNEGLQUERYDEVICESTRINGEXTPROC) eglGetProcAddress("eglQueryDeviceStringEXT");

        if (!renderer->eglQueryDeviceStringEXT)
        {
            fprintf(stderr, PREFIX "error: could not load egl client function 'eglQueryDeviceStringEXT'.\n");
            return false;
        }
    }

    if (!renderer->eglQueryDevicesEXT)
    {
        fprintf(stderr, PREFIX "error: could not load egl client function 'eglQueryDevicesEXT'.\n");
        return false;
    }

    EGLint device_count = 0;

    if (!renderer->eglQueryDevicesEXT(0, NULL, &device_count))
    {
        fprintf(stderr, PREFIX "error: eglQueryDevicesEXT.0 failed -> %s\n",
                        __fake_unity_egl_error_to_string(eglGetError()));
        return false;
    }

    EGLDeviceEXT *devices = (EGLDeviceEXT *) malloc(sizeof(*devices) * device_count);

    if (!renderer->eglQueryDevicesEXT(device_count, devices, &device_count))
    {
        fprintf(stderr, PREFIX "error: eglQueryDevicesEXT.1 failed -> %s\n",
                        __fake_unity_egl_error_to_string(eglGetError()));
        free(devices);
        return false;
    }

    int32_t best_device_index = -1;
    EGLAttrib best_device_type = EGL_DEVICE_TYPE_OTHER_EXT;

    fprintf(stderr, PREFIX "%d egl devices:\n", device_count);

    for (EGLint i = 0; i < device_count; i += 1)
    {
        const char *vendor_name = "<unknown-vendor>";
        const char *renderer_name = "<unknown-renderer>";
        EGLAttrib device_type = EGL_DEVICE_TYPE_INTEGRATED_GPU_EXT;

        if (has_EGL_EXT_device_query)
        {
            const char *device_extensions = renderer->eglQueryDeviceStringEXT(devices[i], EGL_EXTENSIONS);

            if (device_extensions)
            {
                bool has_EGL_EXT_device_type = false;
                bool has_EGL_MESA_device_software = false;
                bool has_EGL_EXT_device_query_name = false;

                const char *at = device_extensions;

                for (;;)
                {
                    while (*at == ' ')  at += 1;
                    if (!*at)  break;
                    const char *start = at;
                    while (*at && (*at != ' '))  at += 1;

                    if (__fake_unity_sized_string_equal(at - start, start, "EGL_EXT_device_type"))
                    {
                        has_EGL_EXT_device_type = true;
                    }
                    else if (__fake_unity_sized_string_equal(at - start, start, "EGL_MESA_device_software"))
                    {
                        has_EGL_MESA_device_software = true;
                    }
                    else if (__fake_unity_sized_string_equal(at - start, start, "EGL_EXT_device_query_name"))
                    {
                        has_EGL_EXT_device_query_name = true;
                    }
                }

                if (has_EGL_EXT_device_type)
                {
                    EGLAttrib type;

                    if (renderer->eglQueryDeviceAttribEXT(devices[i], EGL_DEVICE_TYPE_EXT, &type))
                    {
                        device_type = type;
                    }
                }
                else if (has_EGL_MESA_device_software)
                {
                    device_type = EGL_DEVICE_TYPE_CPU_EXT;
                }

                if (has_EGL_EXT_device_query_name)
                {
                    const char *vendor = renderer->eglQueryDeviceStringEXT(devices[i], EGL_VENDOR);
                    const char *renderer_ext = renderer->eglQueryDeviceStringEXT(devices[i], EGL_RENDERER_EXT);

                    if (vendor)  vendor_name = vendor;
                    if (renderer_ext)  renderer_name = renderer_ext;
                }
            }
        }

        fprintf(stderr, PREFIX "[%d] %s - %s (type = %s)\n", i, vendor_name, renderer_name,
                        __fake_unity_egl_device_type_to_string(device_type));

        if (device_type == EGL_DEVICE_TYPE_OTHER_EXT)
        {
            continue;
        }

        if (__fake_unity_egl_device_type_is_better(device_type, best_device_type))
        {
            best_device_type = device_type;
            best_device_index = i;
        }
    }

    if ((device_index < 0) || (device_index >= device_count))
    {
        device_index = best_device_index;
    }

    if ((device_index < 0) || (device_index >= device_count))
    {
        fprintf(stderr, PREFIX "error: device_index = %d is out of bounds [0, %u).\n", device_index, device_count);
        free(devices);
        return false;
    }

    EGLDeviceEXT device = devices[device_index];

    free(devices);

    fprintf(stderr, PREFIX "selected device at index %d\n", device_index);

    EGLDisplay display = eglGetPlatformDisplay(EGL_PLATFORM_DEVICE_EXT, device, NULL);

    if (display == EGL_NO_DISPLAY)
    {
        fprintf(stderr, PREFIX "error: could not get a display from device -> %s\n",
                        __fake_unity_egl_error_to_string(eglGetError()));
        return false;
    }

    EGLint version_major, version_minor;

    if (!eglInitialize(display, &version_major, &version_minor))
    {
        fprintf(stderr, PREFIX "error: could not initialize the egl display -> %s\n",
                        __fake_unity_egl_error_to_string(eglGetError()));
        return false;
    }

    fprintf(stderr, PREFIX "egl version %d.%d\n", version_major, version_minor);

    if (!eglBindAPI(api))
    {
        fprintf(stderr, PREFIX "error: could not bind opengl api -> %s\n",
                        __fake_unity_egl_error_to_string(eglGetError()));
        eglTerminate(display);
        return false;
    }

    const char *display_extensions = eglQueryString(display, EGL_EXTENSIONS);

    if (!display_extensions)
    {
        fprintf(stderr, PREFIX "error: egl does not have display extensions\n");
        eglTerminate(display);
        return false;
    }

    bool has_EGL_KHR_create_context = false;
    bool has_EGL_KHR_no_config_context = false;
    bool has_EGL_KHR_surfaceless_context = false;

    at = display_extensions;

    for (;;)
    {
        while (*at == ' ')  at += 1;
        if (!*at)  break;
        const char *start = at;
        while (*at && (*at != ' '))  at += 1;

        if (__fake_unity_sized_string_equal(at - start, start, "EGL_KHR_create_context"))
        {
            has_EGL_KHR_create_context = true;
        }
        else if (__fake_unity_sized_string_equal(at - start, start, "EGL_KHR_no_config_context"))
        {
            has_EGL_KHR_no_config_context = true;
        }
        else if (__fake_unity_sized_string_equal(at - start, start, "EGL_KHR_surfaceless_context"))
        {
            has_EGL_KHR_surfaceless_context = true;
        }
    }

    if (!has_EGL_KHR_create_context)
    {
        fprintf(stderr, PREFIX "error: egl does not have display extension 'EGL_KHR_create_context'\n");
        eglTerminate(display);
        return false;
    }

    if (!has_EGL_KHR_no_config_context)
    {
        fprintf(stderr, PREFIX "error: egl does not have display extension 'EGL_KHR_no_config_context'\n");
        eglTerminate(display);
        return false;
    }

    if (!has_EGL_KHR_surfaceless_context)
    {
        fprintf(stderr, PREFIX "error: egl does not have display extension 'EGL_KHR_surfaceless_context'\n");
        eglTerminate(display);
        return false;
    }

    int index = 0;
    EGLint context_attribs[8];

    context_attribs[index++] = EGL_CONTEXT_MAJOR_VERSION_KHR;
    context_attribs[index++] = (EGLint) (gl_version >> 16);

    context_attribs[index++] = EGL_CONTEXT_MINOR_VERSION_KHR;
    context_attribs[index++] = (EGLint) (gl_version & 0xFFFF);

    if (api == EGL_OPENGL_API)
    {
        context_attribs[index++] = EGL_CONTEXT_OPENGL_PROFILE_MASK_KHR;
        context_attribs[index++] = EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT_KHR;
    }

    context_attribs[index++] = EGL_NONE;

    EGLContext context = eglCreateContext(display, EGL_NO_CONFIG_KHR, EGL_NO_CONTEXT, context_attribs);

    if (context == EGL_NO_CONTEXT)
    {
        fprintf(stderr, PREFIX "error: could not create an opengl context -> %s\n",
                        __fake_unity_egl_error_to_string(eglGetError()));
        eglTerminate(display);
        return false;
    }

    if (!eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, context))
    {
        fprintf(stderr, PREFIX "error: could not make openg context current -> %s\n",
                        __fake_unity_egl_error_to_string(eglGetError()));
        eglDestroyContext(display, context);
        eglTerminate(display);
        return false;
    }

    fprintf(stderr, PREFIX "%s\n", glGetString(GL_VERSION));

    if (api == EGL_OPENGL_ES_API)
    {
        if (gl_version >= FAKE_UNITY_MAKE_OPENGL_VERSION(3, 0))
        {
            __fake_unity_state.renderer_type = kUnityGfxRendererOpenGLES30;
        }
        else
        {
            __fake_unity_state.renderer_type = kUnityGfxRendererOpenGLES20;
        }
    }
    else if (api == EGL_OPENGL_API)
    {
        __fake_unity_state.renderer_type = kUnityGfxRendererOpenGLCore;
    }

    for (int32_t i = 0; i < __fake_unity_state.graphics_device_event_callbacks.count; i += 1)
    {
        __fake_unity_state.graphics_device_event_callbacks.items[i](kUnityGfxDeviceEventInitialize);
    }

    return true;
}

#endif

#if defined(FAKE_UNITY_GRAPHICS_VULKAN)

static inline const char *
__fake_unity_vk_result_to_string(VkResult result)
{
    const char *str = "<unknown-vk-result>";

#define NAME(name) case name: str = #name; break

    switch (result)
    {
        NAME(VK_SUCCESS);
        NAME(VK_NOT_READY);
        NAME(VK_TIMEOUT);
        NAME(VK_EVENT_SET);
        NAME(VK_EVENT_RESET);
        NAME(VK_INCOMPLETE);
        NAME(VK_ERROR_OUT_OF_HOST_MEMORY);
        NAME(VK_ERROR_OUT_OF_DEVICE_MEMORY);
        NAME(VK_ERROR_INITIALIZATION_FAILED);
        NAME(VK_ERROR_DEVICE_LOST);
        NAME(VK_ERROR_MEMORY_MAP_FAILED);
        NAME(VK_ERROR_LAYER_NOT_PRESENT);
        NAME(VK_ERROR_EXTENSION_NOT_PRESENT);
        NAME(VK_ERROR_FEATURE_NOT_PRESENT);
        NAME(VK_ERROR_INCOMPATIBLE_DRIVER);
        NAME(VK_ERROR_TOO_MANY_OBJECTS);
        NAME(VK_ERROR_FORMAT_NOT_SUPPORTED);
        NAME(VK_ERROR_FRAGMENTED_POOL);
        // VK_ERROR_UNKNOWN was only defined in the headers starting with version 1.2.13
        case ((VkResult) -13): str = "VK_ERROR_UNKNOWN"; break;
        default: break;
    }

#undef NAME

    return str;
}

static inline const char *
__fake_unity_vk_format_to_string(VkFormat format)
{
    const char *str = "<unknown-vk-format>";

#  define NAME(name) case name: str = #name; break

    switch (format)
    {
        NAME(VK_FORMAT_R8G8B8A8_UNORM);
        NAME(VK_FORMAT_R8G8B8A8_SRGB);
        default: break;
    }

#  undef NAME

    return str;
}

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

static inline VkFormat
__fake_unity_get_vk_format(FakeUnity_TextureFormat format, bool linear)
{
    switch (format)
    {
        // case FakeUnity_TextureFormat_Alpha8:
        // case FakeUnity_TextureFormat_ARGB4444:
        // case FakeUnity_TextureFormat_RGB24:
        case FakeUnity_TextureFormat_RGBA32:   return linear ? VK_FORMAT_R8G8B8A8_UNORM : VK_FORMAT_R8G8B8A8_SRGB;
        // case FakeUnity_TextureFormat_ARGB32:
        // case FakeUnity_TextureFormat_RGB565:
    }

    return VK_FORMAT_UNDEFINED;
}

// The order in this table matters for preference.
// They are ordered from most to least preferable.
static const VkPhysicalDeviceType __fake_unity_preferred_vk_device_types[] = {
    VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU,
    VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU,
    VK_PHYSICAL_DEVICE_TYPE_CPU,
};

static inline uint32_t
__fake_unity_get_prefer_index_from_vk_device_type(VkPhysicalDeviceType device_type)
{
    uint32_t result = UINT32_MAX;

    for (size_t i = 0; i < (sizeof(__fake_unity_preferred_vk_device_types) / sizeof(__fake_unity_preferred_vk_device_types[0])); i += 1)
    {
        if (__fake_unity_preferred_vk_device_types[i] == device_type)
        {
            result = (uint32_t) i;
            break;
        }
    }

    return result;
}

static inline bool
__fake_unity_vk_device_type_is_better(VkPhysicalDeviceType a, VkPhysicalDeviceType b)
{
    uint32_t a_prefer_index = __fake_unity_get_prefer_index_from_vk_device_type(a);
    uint32_t b_prefer_index = __fake_unity_get_prefer_index_from_vk_device_type(b);

    return (a_prefer_index < b_prefer_index);
}

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
    fprintf(stderr, PREFIX "warning: your program is not compiled as a c++ program,\n"
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
    fprintf(stderr, PREFIX "warning: your program is not compiled as a c++ program,\n"
                    "             but it looks like the native unity plugin you are loading is.\n"
                    "             Due to calling conventions being incompatible, calling RegisterInterface\n"
                    "             may not work. Please compile your program as a c++ program or\n"
                    "             make the native unity plugin call RegisterInterfaceSplit.\n");
#endif

    IUnityInterfaces_RegisterInterfaceSplit(guid.m_GUIDHigh, guid.m_GUIDLow, ptr);
}

#if !defined(FAKE_UNITY_NO_LOG)

static void
IUnityLog_Log(UnityLogType type, const char *message, const char *file_name, const int file_line)
{
    switch (type)
    {
        case kUnityLogTypeError:     fprintf(stderr, PREFIX "%s:%d {error} %s\n"    , file_name, file_line, message); break;
        case kUnityLogTypeWarning:   fprintf(stderr, PREFIX "%s:%d {warning} %s\n"  , file_name, file_line, message); break;
        case kUnityLogTypeLog:       fprintf(stderr, PREFIX "%s:%d {log} %s\n"      , file_name, file_line, message); break;
        case kUnityLogTypeException: fprintf(stderr, PREFIX "%s:%d {expection} %s\n", file_name, file_line, message); break;
    }
}

#endif

#if !defined(FAKE_UNITY_NO_PROFILER)

static void
IUnityProfiler_EmitEvent(const UnityProfilerMarkerDesc* markerDesc, UnityProfilerMarkerEventType eventType, uint16_t eventDataCount, const UnityProfilerMarkerData* eventData)
{
    fprintf(stderr, PREFIX "TODO: EmitEvent\n");
}

static int
IUnityProfiler_IsEnabled()
{
    fprintf(stderr, PREFIX "TODO: IsEnabled\n");
    return 0;
}

static int
IUnityProfiler_IsAvailable()
{
    fprintf(stderr, PREFIX "TODO: IsAvailable\n");
    return 0;
}

static int
IUnityProfiler_CreateMarker(const UnityProfilerMarkerDesc** desc, const char* name, UnityProfilerCategoryId category, UnityProfilerMarkerFlags flags, int eventDataCount)
{
    fprintf(stderr, PREFIX "TODO: CreateMarker\n");
    return 0;
}

static int
IUnityProfiler_SetMarkerMetadataName(const UnityProfilerMarkerDesc* desc, int index, const char* metadataName, UnityProfilerMarkerDataType metadataType, UnityProfilerMarkerDataUnit metadataUnit)
{
    fprintf(stderr, PREFIX "TODO: SetMarkerMetadataName\n");
    return 0;
}

static int
IUnityProfiler_RegisterThread(UnityProfilerThreadId* threadId, const char* groupName, const char* name)
{
    fprintf(stderr, PREFIX "TODO: RegisterThread\n");
    return 0;
}

static int
IUnityProfiler_UnregisterThread(UnityProfilerThreadId threadId)
{
    fprintf(stderr, PREFIX "TODO: UnregisterThread\n");
    return 0;
}

#endif

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
    fprintf(stderr, PREFIX "TODO: ReserveEventIDRange\n");
    // TODO:
    return 0;
}

#if defined(FAKE_UNITY_GRAPHICS_VULKAN)

static bool
UnityGraphicsVulkan_InterceptInitialization(UnityVulkanInitCallback func, void *userdata)
{
    if (__fake_unity_state.renderer_type != kUnityGfxRendererNull)
    {
        return false;
    }

    __fake_unity_state.unity_vulkan_init_callback = func;
    __fake_unity_state.unity_vulkan_init_userdata = userdata;
    return true;
}

static PFN_vkVoidFunction
UnityGraphicsVulkan_InterceptVulkanAPI(const char *name, PFN_vkVoidFunction func)
{
    fprintf(stderr, PREFIX "TODO: InterceptVulkanAPI\n");
    return 0;
}

static void
UnityGraphicsVulkan_ConfigureEvent(int event_id, const UnityVulkanPluginEventConfig *plugin_event_config)
{
    fprintf(stderr, PREFIX "TODO: ConfigureEvent\n");
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
    fprintf(stderr, PREFIX "TODO: CommandRecordingState\n");
    return false;
}

static bool
UnityGraphicsVulkan_AccessTexture(void* native_texture, const VkImageSubresource *sub_resource, VkImageLayout layout,
                                  VkPipelineStageFlags pipeline_stage_flags, VkAccessFlags access_flags,
                                  UnityVulkanResourceAccessMode access_mode, UnityVulkanImage *image)
{
    fprintf(stderr, PREFIX "TODO: AccessTexture\n");
    return false;
}

static bool
UnityGraphicsVulkan_AccessRenderBufferTexture(UnityRenderBuffer native_render_buffer, const VkImageSubresource *sub_resource, VkImageLayout layout,
                                              VkPipelineStageFlags pipeline_stage_flags, VkAccessFlags access_flags,
                                              UnityVulkanResourceAccessMode access_mode, UnityVulkanImage *image)
{
    fprintf(stderr, PREFIX "TODO: AccessRenderBufferTexture\n");
    return false;
}

static bool
UnityGraphicsVulkan_AccessRenderBufferResolveTexture(UnityRenderBuffer native_render_buffer, const VkImageSubresource *sub_resource, VkImageLayout layout,
                                                     VkPipelineStageFlags pipeline_stage_flags, VkAccessFlags access_flags,
                                                     UnityVulkanResourceAccessMode access_mode, UnityVulkanImage *image)
{
    fprintf(stderr, PREFIX "TODO: AccessRenderBufferResolveTexture\n");
    return false;
}

static bool
UnityGraphicsVulkan_AccessBuffer(void* native_buffer, VkPipelineStageFlags pipeline_stage_flags, VkAccessFlags access_flags,
                                 UnityVulkanResourceAccessMode access_mode, UnityVulkanBuffer *buffer)
{
    fprintf(stderr, PREFIX "TODO: AccessBuffer\n");
    return false;
}

static void
UnityGraphicsVulkan_EnsureOutsideRenderPass()
{
    fprintf(stderr, PREFIX "TODO: EnsureOutsideRenderPass\n");
}

static void
UnityGraphicsVulkan_EnsureInsideRenderPass()
{
    fprintf(stderr, PREFIX "TODO: EnsureInsideRenderPass\n");
}

static void
UnityGraphicsVulkan_AccessQueue(UnityRenderingEventAndData func, int event_id, void* userdata, bool flush)
{
    fprintf(stderr, PREFIX "TODO: AccessQueue\n");
}

static bool
UnityGraphicsVulkan_ConfigureSwapchain(const UnityVulkanSwapchainConfiguration *swapchain_config)
{
    fprintf(stderr, PREFIX "TODO: ConfigureSwapchain\n");
    return false;
}

static bool
UnityGraphicsVulkan_AccessTextureByID(UnityTextureID texture_id, const VkImageSubresource *sub_resource, VkImageLayout layout,
                                      VkPipelineStageFlags pipeline_stage_flags, VkAccessFlags access_flags,
                                      UnityVulkanResourceAccessMode access_mode, UnityVulkanImage *image)
{
    fprintf(stderr, PREFIX "TODO: AccessTextureByID\n");
    return false;
}

#endif

FAKE_UNITY_DEF bool
fake_unity_initialize(int32_t max_plugin_count, int32_t max_texture_count)
{
    __fake_unity_state.renderer_type = kUnityGfxRendererNull;

    __fake_unity_state.unity_interfaces.GetInterface           = IUnityInterfaces_GetInterface;
    __fake_unity_state.unity_interfaces.RegisterInterface      = IUnityInterfaces_RegisterInterface;
    __fake_unity_state.unity_interfaces.GetInterfaceSplit      = IUnityInterfaces_GetInterfaceSplit;
    __fake_unity_state.unity_interfaces.RegisterInterfaceSplit = IUnityInterfaces_RegisterInterfaceSplit;

#if !defined(FAKE_UNITY_NO_LOG)
    __fake_unity_state.unity_log.Log = IUnityLog_Log;

    IUnityInterfaces_RegisterInterfaceSplit(0x9E7507fA5B444D5DULL, 0x92FB979515EA83FCULL, &__fake_unity_state.unity_log);
#endif

#if !defined(FAKE_UNITY_NO_PROFILER)
    __fake_unity_state.unity_profiler.EmitEvent             = IUnityProfiler_EmitEvent;
    __fake_unity_state.unity_profiler.IsEnabled             = IUnityProfiler_IsEnabled;
    __fake_unity_state.unity_profiler.IsAvailable           = IUnityProfiler_IsAvailable;
    __fake_unity_state.unity_profiler.CreateMarker          = IUnityProfiler_CreateMarker;
    __fake_unity_state.unity_profiler.SetMarkerMetadataName = IUnityProfiler_SetMarkerMetadataName;
    __fake_unity_state.unity_profiler.RegisterThread        = IUnityProfiler_RegisterThread;
    __fake_unity_state.unity_profiler.UnregisterThread      = IUnityProfiler_UnregisterThread;

    IUnityInterfaces_RegisterInterfaceSplit(0x2CE79ED8316A4833ULL, 0x87076B2013E1571FULL, &__fake_unity_state.unity_profiler);
#endif

    __fake_unity_state.unity_graphics.GetRenderer                   = IUnityGraphics_GetRenderer;
    __fake_unity_state.unity_graphics.RegisterDeviceEventCallback   = IUnityGraphics_RegisterDeviceEventCallback;
    __fake_unity_state.unity_graphics.UnregisterDeviceEventCallback = IUnityGraphics_UnregisterDeviceEventCallback;
    __fake_unity_state.unity_graphics.ReserveEventIDRange           = IUnityGraphics_ReserveEventIDRange;

    IUnityInterfaces_RegisterInterfaceSplit(0x7CBA0A9CA4DDB544ULL, 0x8C5AD4926EB17B11ULL, &__fake_unity_state.unity_graphics);

#if defined(FAKE_UNITY_GRAPHICS_VULKAN)
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

    IUnityInterfaces_RegisterInterfaceSplit(0x95355348d4ef4e11ULL, 0x9789313dfcffcc87ULL, &__fake_unity_state.unity_graphics_vulkan);
#endif

    {
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
    }

    {
        if (max_texture_count <= 0)
        {
            max_texture_count = 8;
        }

        __fake_unity_state.textures = (FakeUnityTexture *) malloc(max_texture_count * sizeof(FakeUnityTexture));
        __fake_unity_state.free_texture_indices = (uint16_t *) malloc(max_texture_count * sizeof(uint16_t));
        __fake_unity_state.texture_generations = (uint16_t *) malloc(max_texture_count * sizeof(uint16_t));

        __fake_unity_state.max_texture_count = max_texture_count;
        __fake_unity_state.free_texture_count = max_texture_count;

        int32_t index = max_texture_count;

        for (int32_t i = 0; i < max_texture_count; i += 1)
        {
            __fake_unity_state.texture_generations[i] = 1;
            __fake_unity_state.free_texture_indices[i] = --index;
        }
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
            fprintf(stderr, PREFIX "error: could not load native plugin '%s'\n", filename);
            return 0;
        }
#elif FAKE_UNITY_PLATFORM_ANDROID || FAKE_UNITY_PLATFORM_LINUX || FAKE_UNITY_PLATFORM_MACOS
        void *handle = dlopen(filename, RTLD_NOW);

        if (!handle)
        {
            fprintf(stderr, PREFIX "error: could not load native plugin '%s' -> %s\n", filename, dlerror());
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

#if defined(FAKE_UNITY_GRAPHICS_OPENGLES)

FAKE_UNITY_DEF bool
fake_unity_create_opengles_renderer(uint32_t gl_version, int32_t device_index)
{
    return __fake_unity_create_opengl_renderer(EGL_OPENGL_ES_API, gl_version, device_index);
}

#endif

#if defined(FAKE_UNITY_GRAPHICS_OPENGL_CORE)

FAKE_UNITY_DEF bool
fake_unity_create_opengl_core_renderer(uint32_t gl_version, int32_t device_index)
{
    return __fake_unity_create_opengl_renderer(EGL_OPENGL_API, gl_version, device_index);
}

#endif

#if defined(FAKE_UNITY_GRAPHICS_VULKAN)

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

#  if FAKE_UNITY_PLATFORM_WINDOWS
    renderer->loader_handle = LoadLibraryA("vulkan-1.dll");

    if (!renderer->loader_handle)
    {
        fprintf(stderr, PREFIX "error: could not load vulkan loader\n");
        return false;
    }

    renderer->loader_vkGetInstanceProcAddr = (PFN_vkGetInstanceProcAddr) GetProcAddress(renderer->loader_handle, "vkGetInstanceProcAddr");

#    define CLOSE_VULKAN_LOADER(handle) \
    FreeLibrary(handle); \
    handle = 0
#  elif FAKE_UNITY_PLATFORM_ANDROID || FAKE_UNITY_PLATFORM_LINUX || FAKE_UNITY_PLATFORM_MACOS
#    if FAKE_UNITY_PLATFORM_ANDROID
    renderer->loader_handle = dlopen("libvulkan.so", RTLD_NOW);
#    elif FAKE_UNITY_PLATFORM_LINUX
    renderer->loader_handle = dlopen("libvulkan.so.1", RTLD_NOW);
#    elif FAKE_UNITY_PLATFORM_MACOS
    renderer->loader_handle = dlopen("libvulkan.1.dylib", RTLD_NOW);
#    endif

    if (!renderer->loader_handle)
    {
        fprintf(stderr, PREFIX "error: could not load vulkan loader -> %s\n", dlerror());
        return false;
    }

    renderer->loader_vkGetInstanceProcAddr = (PFN_vkGetInstanceProcAddr) dlsym(renderer->loader_handle, "vkGetInstanceProcAddr");

#    define CLOSE_VULKAN_LOADER(handle) \
    dlclose(handle); \
    handle = 0
#  endif

    if (!renderer->loader_vkGetInstanceProcAddr)
    {
        fprintf(stderr, PREFIX "error: could not load vulkan function 'vkGetInstanceProcAddr'.\n");
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

    renderer->vkEnumerateInstanceVersion = (PFN_vkEnumerateInstanceVersion) renderer->vkGetInstanceProcAddr(VK_NULL_HANDLE, "vkEnumerateInstanceVersion");

#  define load_function(name)                                                                 \
    do                                                                                        \
    {                                                                                         \
        renderer->name = (PFN_##name) renderer->vkGetInstanceProcAddr(VK_NULL_HANDLE, #name); \
        if (!renderer->name)                                                                  \
        {                                                                                     \
            fprintf(stderr, PREFIX "error: could not load vulkan function '" #name "'.\n");   \
            CLOSE_VULKAN_LOADER(renderer->loader_handle);                                     \
            return false;                                                                     \
        }                                                                                     \
    } while (0)

    __FAKE_UNITY_VULKAN_GLOBAL_FUNCTIONS(load_function);

#  undef load_function

    VkResult vk_result;

    uint32_t vulkan_instance_version = VK_API_VERSION_1_0;
    uint32_t vulkan_preferred_instance_version = VK_API_VERSION_1_0;

    if (renderer->vkEnumerateInstanceVersion)
    {
        vk_result = renderer->vkEnumerateInstanceVersion(&vulkan_instance_version);

        if (vk_result == VK_SUCCESS)
        {
            if (vulkan_instance_version > VK_API_VERSION_1_1)
            {
                vulkan_preferred_instance_version = VK_API_VERSION_1_1;
            }
            else
            {
                vulkan_preferred_instance_version = vulkan_instance_version;
            }
        }
        else
        {
            fprintf(stderr, PREFIX "warning: vkEnumerateInstanceVersion failed -> %s\n",
                            __fake_unity_vk_result_to_string(vk_result));
        }
    }

    fprintf(stderr, PREFIX "loaded vulkan library with instance version %u.%u.%u\n",
                    VK_API_VERSION_MAJOR(vulkan_instance_version),
                    VK_API_VERSION_MINOR(vulkan_instance_version),
                    VK_API_VERSION_PATCH(vulkan_instance_version));

    VkApplicationInfo application_info;
    application_info.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    application_info.pNext              = 0;
    application_info.pApplicationName   = "application";
    application_info.applicationVersion = 1;
    application_info.pEngineName        = "Unity";
    application_info.engineVersion      = 1;
    application_info.apiVersion         = vulkan_preferred_instance_version;

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

    vk_result = renderer->vkCreateInstance(&instance_create_info, 0, &instance);

    if (vk_result != VK_SUCCESS)
    {
        fprintf(stderr, PREFIX "error: vkCreateInstance failed -> %s\n",
                        __fake_unity_vk_result_to_string(vk_result));
        CLOSE_VULKAN_LOADER(renderer->loader_handle);
        return false;
    }

    renderer->instance = instance;

#  define load_function(name)                                                               \
    do                                                                                      \
    {                                                                                       \
        renderer->name = (PFN_##name) renderer->vkGetInstanceProcAddr(instance, #name);     \
        if (!renderer->name)                                                                \
        {                                                                                   \
            fprintf(stderr, PREFIX "error: could not load vulkan function '" #name "'.\n"); \
            CLOSE_VULKAN_LOADER(renderer->loader_handle);                                   \
            return false;                                                                   \
        }                                                                                   \
    } while (0)

    __FAKE_UNITY_VULKAN_INSTANCE_FUNCTIONS(load_function);

#  undef load_function

    uint32_t physical_device_count = 0;

    vk_result = renderer->vkEnumeratePhysicalDevices(instance, &physical_device_count, 0);

    if (vk_result != VK_SUCCESS)
    {
        fprintf(stderr, PREFIX "error: vkEnumeratePhysicalDevices.0 failed -> %s\n",
                        __fake_unity_vk_result_to_string(vk_result));
        CLOSE_VULKAN_LOADER(renderer->loader_handle);
        return false;
    }

    VkPhysicalDevice *physical_devices = (VkPhysicalDevice *) malloc(sizeof(*physical_devices) * physical_device_count);

    vk_result = renderer->vkEnumeratePhysicalDevices(instance, &physical_device_count, physical_devices);

    if (vk_result != VK_SUCCESS)
    {
        fprintf(stderr, PREFIX "error: vkEnumeratePhysicalDevices.1 failed -> %s\n",
                        __fake_unity_vk_result_to_string(vk_result));
        free(physical_devices);
        CLOSE_VULKAN_LOADER(renderer->loader_handle);
        return false;
    }

    int32_t best_device_index = -1;
    VkPhysicalDeviceType best_device_type = VK_PHYSICAL_DEVICE_TYPE_OTHER;

    fprintf(stderr, PREFIX "%u physical devices:\n", physical_device_count);

    for (uint32_t i = 0; i < physical_device_count; i += 1)
    {
        VkPhysicalDeviceProperties properties;
        renderer->vkGetPhysicalDeviceProperties(physical_devices[i], &properties);

        fprintf(stderr, PREFIX "[%u] %s (type = %s) (api version = %u.%u.%u)\n",
                        i, properties.deviceName, __fake_unity_vk_physical_device_type_to_string(properties.deviceType),
                        VK_API_VERSION_MAJOR(properties.apiVersion), VK_API_VERSION_MINOR(properties.apiVersion), VK_API_VERSION_PATCH(properties.apiVersion));

        if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_OTHER)
        {
            continue;
        }

        if (VK_API_VERSION_MAJOR(properties.apiVersion) < 1)
        {
            continue;
        }

        if (__fake_unity_vk_device_type_is_better(properties.deviceType, best_device_type))
        {
            best_device_type = properties.deviceType;
            best_device_index = i;
        }
    }

    if ((device_index < 0) || (device_index >= (int32_t) physical_device_count))
    {
        device_index = best_device_index;
    }

    if ((device_index < 0) || (device_index >= (int32_t) physical_device_count))
    {
        fprintf(stderr, PREFIX "error: device_index = %d is out of bounds [0, %u).\n", device_index, physical_device_count);
        free(physical_devices);
        CLOSE_VULKAN_LOADER(renderer->loader_handle);
        return false;
    }

    VkPhysicalDevice physical_device = physical_devices[device_index];

    free(physical_devices);

    fprintf(stderr, PREFIX "selected device at index %d\n", device_index);

    renderer->physical_device = physical_device;

    uint32_t queue_family_count = 0;

    renderer->vkGetPhysicalDeviceQueueFamilyProperties(renderer->physical_device, &queue_family_count, NULL);

    VkQueueFamilyProperties *queue_families = (VkQueueFamilyProperties *) malloc(sizeof(*queue_families) * queue_family_count);

    renderer->vkGetPhysicalDeviceQueueFamilyProperties(renderer->physical_device, &queue_family_count, queue_families);

    uint32_t graphics_queue_index = UINT32_MAX;

    for (uint32_t i = 0; i < queue_family_count; i += 1)
    {
        if ((queue_families[i].queueCount > 0) && (queue_families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT))
        {
            graphics_queue_index = i;
            break;
        }
    }

    free(queue_families);

    if (graphics_queue_index >= queue_family_count)
    {
        fprintf(stderr, PREFIX "error: could not find a vulkan queue with graphics cababilities.\n");
        CLOSE_VULKAN_LOADER(renderer->loader_handle);
        return false;
    }

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

    vk_result = renderer->vkCreateDevice(physical_device, &device_create_info, 0, &device);

    if (vk_result != VK_SUCCESS)
    {
        fprintf(stderr, PREFIX "error: vkCreateDevice failed -> %s\n",
                        __fake_unity_vk_result_to_string(vk_result));
        CLOSE_VULKAN_LOADER(renderer->loader_handle);
        return false;
    }

    renderer->device = device;

#  define load_function(name)                                                               \
    do                                                                                      \
    {                                                                                       \
        if (renderer->vkGetInstanceProcAddr != renderer->loader_vkGetInstanceProcAddr)      \
        {                                                                                   \
            renderer->name = (PFN_##name) renderer->vkGetInstanceProcAddr(instance, #name); \
        }                                                                                   \
        else                                                                                \
        {                                                                                   \
            renderer->name = (PFN_##name) renderer->vkGetDeviceProcAddr(device, #name);     \
        }                                                                                   \
        if (!renderer->name)                                                                \
        {                                                                                   \
            fprintf(stderr, PREFIX "error: could not load vulkan function '" #name "'.\n"); \
            CLOSE_VULKAN_LOADER(renderer->loader_handle);                                   \
            return false;                                                                   \
        }                                                                                   \
    } while (0)

    __FAKE_UNITY_VULKAN_DEVICE_FUNCTIONS(load_function);

#  undef load_function

    VkQueue graphics_queue;

    renderer->vkGetDeviceQueue(device, graphics_queue_index, 0, &graphics_queue);

    renderer->graphics_queue_index = graphics_queue_index;
    renderer->graphics_queue = graphics_queue;

#  undef CLOSE_VULKAN_LOADER

    __fake_unity_state.renderer_type = kUnityGfxRendererVulkan;

    for (int32_t i = 0; i < __fake_unity_state.graphics_device_event_callbacks.count; i += 1)
    {
        __fake_unity_state.graphics_device_event_callbacks.items[i](kUnityGfxDeviceEventInitialize);
    }

    return true;
}

FAKE_UNITY_DEF PFN_vkVoidFunction
fake_unity_vulkan_get_instance_proc_address(const char *proc_name)
{
    if ((__fake_unity_state.renderer_type == kUnityGfxRendererVulkan) &&
        (__fake_unity_state.renderer.vulkan.vkGetInstanceProcAddr))
    {
        return __fake_unity_state.renderer.vulkan.vkGetInstanceProcAddr(__fake_unity_state.renderer.vulkan.instance, proc_name);
    }

    return NULL;
}

FAKE_UNITY_DEF PFN_vkVoidFunction
fake_unity_vulkan_get_device_proc_address(const char *proc_name)
{
    if ((__fake_unity_state.renderer_type == kUnityGfxRendererVulkan) &&
        (__fake_unity_state.renderer.vulkan.vkGetDeviceProcAddr))
    {
        return __fake_unity_state.renderer.vulkan.vkGetDeviceProcAddr(__fake_unity_state.renderer.vulkan.device, proc_name);
    }

    return NULL;
}

#endif

FAKE_UNITY_DEF FakeUnity_Texture2D
fake_unity_Texture2D_CreateExternalTexture(int32_t width, int32_t height, FakeUnity_TextureFormat format,
                                           bool mip_chain, bool linear, void *native_texture)
{
    FakeUnity_Texture2D result = 0;

    if (__fake_unity_state.free_texture_count > 0)
    {
        switch (__fake_unity_state.renderer_type)
        {
#if defined(FAKE_UNITY_GRAPHICS_OPENGLES) || defined(FAKE_UNITY_GRAPHICS_OPENGL_CORE)
            case kUnityGfxRendererOpenGLES20:
            case kUnityGfxRendererOpenGLES30:
            case kUnityGfxRendererOpenGLCore:
            {
                uint16_t index = __fake_unity_state.free_texture_indices[--__fake_unity_state.free_texture_count];
                uint16_t generation = __fake_unity_state.texture_generations[index];

                result = ((uint32_t) generation << 16) | (uint32_t) index;

                FakeUnityTexture *texture = __fake_unity_state.textures + index;

                texture->width = width;
                texture->height = height;
                texture->api.gl_texture = (GLuint) (uintptr_t) native_texture;
            } break;
#endif
#if defined(FAKE_UNITY_GRAPHICS_VULKAN)
            case kUnityGfxRendererVulkan:
            {
                FakeUnityVulkanRenderer *renderer = &__fake_unity_state.renderer.vulkan;

                VkFormat vk_format = __fake_unity_get_vk_format(format, linear);

                if (vk_format == VK_FORMAT_UNDEFINED)
                {
                    return 0;
                }

                VkImageViewCreateInfo image_view_create_info;
                image_view_create_info.sType                           = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
                image_view_create_info.pNext                           = NULL;
                image_view_create_info.flags                           = 0;
                image_view_create_info.image                           = *(VkImage *) native_texture;
                image_view_create_info.viewType                        = VK_IMAGE_VIEW_TYPE_2D;
                image_view_create_info.format                          = vk_format;
                image_view_create_info.components.r                    = VK_COMPONENT_SWIZZLE_IDENTITY;
                image_view_create_info.components.g                    = VK_COMPONENT_SWIZZLE_IDENTITY;
                image_view_create_info.components.b                    = VK_COMPONENT_SWIZZLE_IDENTITY;
                image_view_create_info.components.a                    = VK_COMPONENT_SWIZZLE_IDENTITY;
                image_view_create_info.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
                image_view_create_info.subresourceRange.baseMipLevel   = 0;
                image_view_create_info.subresourceRange.levelCount     = 1;
                image_view_create_info.subresourceRange.baseArrayLayer = 0;
                image_view_create_info.subresourceRange.layerCount     = 1;

                VkResult vk_result;

                VkImageView image_view;

                vk_result = renderer->vkCreateImageView(renderer->device, &image_view_create_info, NULL, &image_view);

                if (vk_result != VK_SUCCESS)
                {
                    fprintf(stderr, PREFIX "error: vkCreateImageView(width = %d, height = %d, format = %s, image = %p) failed -> %s\n",
                                    width, height, __fake_unity_vk_format_to_string(vk_format), (void *) *(VkImage *) native_texture,
                                    __fake_unity_vk_result_to_string(vk_result));
                    return 0;
                }

                uint16_t index = __fake_unity_state.free_texture_indices[--__fake_unity_state.free_texture_count];
                uint16_t generation = __fake_unity_state.texture_generations[index];

                result = ((uint32_t) generation << 16) | (uint32_t) index;

                FakeUnityTexture *texture = __fake_unity_state.textures + index;

                texture->width = width;
                texture->height = height;
                texture->api.vk_image_view = image_view;
            } break;
#endif
            default: break;
        }
    }

    return result;
}

FAKE_UNITY_DEF void
fake_unity_Texture2D_Destroy(FakeUnity_Texture2D texture_handle)
{
    uint16_t index = (uint16_t) (texture_handle & 0xFFFF);
    uint16_t generation = (uint16_t) ((texture_handle >> 16) & 0xFFFF);

    if (__fake_unity_state.texture_generations[index] == generation)
    {
        FakeUnityTexture *texture = __fake_unity_state.textures + index;

        switch (__fake_unity_state.renderer_type)
        {
#if defined(FAKE_UNITY_GRAPHICS_OPENGLES) || defined(FAKE_UNITY_GRAPHICS_OPENGL_CORE)
            case kUnityGfxRendererOpenGLES20:
            case kUnityGfxRendererOpenGLES30:
            case kUnityGfxRendererOpenGLCore:
            {
            } break;
#endif
#if defined(FAKE_UNITY_GRAPHICS_VULKAN)
            case kUnityGfxRendererVulkan:
            {
                FakeUnityVulkanRenderer *renderer = &__fake_unity_state.renderer.vulkan;
                renderer->vkDestroyImageView(renderer->device, texture->api.vk_image_view, NULL);
            } break;
#endif
            default: break;
        }

        if (__fake_unity_state.texture_generations[index] == 0xFFFF)
        {
            __fake_unity_state.texture_generations[index] = 0;
        }

        __fake_unity_state.texture_generations[index] += 1;
        __fake_unity_state.free_texture_indices[__fake_unity_state.free_texture_count++] = index;
    }
}

#undef ARRAY_ENSURE_SPACE
#undef PREFIX

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
