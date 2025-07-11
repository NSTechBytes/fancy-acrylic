#include <napi.h>

#ifdef _WIN32
#include <windows.h>
#include <dwmapi.h>
#include <versionhelpers.h>
#include <winreg.h>
#include <string>
#include <sstream>
#include <iomanip>

#pragma comment(lib, "dwmapi.lib")

enum ACCENT_STATE {
    ACCENT_DISABLED = 0,
    ACCENT_ENABLE_GRADIENT = 1,
    ACCENT_ENABLE_TRANSPARENTGRADIENT = 2,
    ACCENT_ENABLE_BLURBEHIND = 3,
    ACCENT_ENABLE_ACRYLICBLURBEHIND = 4,
};

struct ACCENT_POLICY {
    ACCENT_STATE AccentState;
    DWORD        AccentFlags;
    DWORD        GradientColor; 
    DWORD        AnimationId;
};

struct WINDOWCOMPOSITIONATTRIBDATA {
    DWORD Attrib;
    PVOID pvData;
    SIZE_T cbData;
};

typedef BOOL(WINAPI* pSetWindowCompositionAttribute)(HWND, WINDOWCOMPOSITIONATTRIBDATA*);

#ifndef DWMWA_WINDOW_CORNER_PREFERENCE
#define DWMWA_WINDOW_CORNER_PREFERENCE 33
#endif

#ifndef DWMWA_BORDER_COLOR       
#define DWMWA_BORDER_COLOR 34
#endif

DWORD GetWindowsBuild()
{
    wchar_t buf[64] = {};
    DWORD bufSize = sizeof(buf);
    if (RegGetValueW(
        HKEY_LOCAL_MACHINE,
        L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion",
        L"CurrentBuildNumber",
        RRF_RT_REG_SZ,
        nullptr,
        buf,
        &bufSize) == ERROR_SUCCESS)
    {
        return std::stoi(buf);
    }
    return 0;
}

Napi::Value applyEffect(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    
    if (info.Length() < 1) {
        Napi::TypeError::New(env, "Expected at least 1 argument").ThrowAsJavaScriptException();
        return env.Null();
    }

    if (!info[0].IsObject()) {
        Napi::TypeError::New(env, "Expected an object as first argument").ThrowAsJavaScriptException();
        return env.Null();
    }

    Napi::Object options = info[0].As<Napi::Object>();
    
    // Get window handle
    if (!options.Has("hwnd") || !options.Get("hwnd").IsNumber()) {
        Napi::TypeError::New(env, "hwnd property is required and must be a number").ThrowAsJavaScriptException();
        return env.Null();
    }
    
    HWND hwnd = reinterpret_cast<HWND>(options.Get("hwnd").As<Napi::Number>().Int64Value());
    
    // Get type (blur or acrylic)
    std::string type = "acrylic";
    if (options.Has("type") && options.Get("type").IsString()) {
        type = options.Get("type").As<Napi::String>().Utf8Value();
    }
    
    if (type != "blur" && type != "acrylic") {
        Napi::TypeError::New(env, "type must be 'blur' or 'acrylic'").ThrowAsJavaScriptException();
        return env.Null();
    }
    
    // Get corner style
    std::string corner = "none";
    if (options.Has("corner") && options.Get("corner").IsString()) {
        corner = options.Get("corner").As<Napi::String>().Utf8Value();
    }
    
    if (corner != "none" && corner != "round" && corner != "roundsmall") {
        Napi::TypeError::New(env, "corner must be 'none', 'round', or 'roundsmall'").ThrowAsJavaScriptException();
        return env.Null();
    }
    
    // Get opacity
    int opacity = 204;
    if (options.Has("opacity") && options.Get("opacity").IsNumber()) {
        opacity = options.Get("opacity").As<Napi::Number>().Int32Value();
        if (opacity < 0 || opacity > 255) {
            Napi::TypeError::New(env, "opacity must be between 0 and 255").ThrowAsJavaScriptException();
            return env.Null();
        }
    }
    
    // Get tint color (optional)
    DWORD tintRGB = 0xFFFFFF;
    bool hasTintColor = false;
    if (options.Has("tintColor") && options.Get("tintColor").IsString()) {
        std::string tintStr = options.Get("tintColor").As<Napi::String>().Utf8Value();
        std::istringstream iss(tintStr);
        iss >> std::hex >> tintRGB;
        if (iss.fail() || tintRGB > 0xFFFFFF) {
            Napi::TypeError::New(env, "tintColor must be a valid hex color (RRGGBB)").ThrowAsJavaScriptException();
            return env.Null();
        }
        hasTintColor = true;
    }
    
    // Get border visibility (default true)
    bool borderVis = true;
    if (options.Has("borderVisible") && options.Get("borderVisible").IsBoolean()) {
        borderVis = options.Get("borderVisible").As<Napi::Boolean>().Value();
    }
    
    // Get border color (default white)
    DWORD borderRGB = 0xFFFFFF;
    if (options.Has("borderColor") && options.Get("borderColor").IsString()) {
        std::string borderStr = options.Get("borderColor").As<Napi::String>().Utf8Value();
        std::istringstream iss(borderStr);
        iss >> std::hex >> borderRGB;
        if (iss.fail() || borderRGB > 0xFFFFFF) {
            Napi::TypeError::New(env, "borderColor must be a valid hex color (RRGGBB)").ThrowAsJavaScriptException();
            return env.Null();
        }
    }
    
    // Validate window handle
    if (!IsWindow(hwnd)) {
        Napi::Error::New(env, "Invalid window handle").ThrowAsJavaScriptException();
        return env.Null();
    }
    
    // Get SetWindowCompositionAttribute function
    HMODULE hUser = GetModuleHandleW(L"user32.dll");
    if (!hUser) {
        Napi::Error::New(env, "Failed to get user32.dll module handle").ThrowAsJavaScriptException();
        return env.Null();
    }
    
    auto SetWCA = reinterpret_cast<pSetWindowCompositionAttribute>(
        GetProcAddress(hUser, "SetWindowCompositionAttribute"));
    if (!SetWCA) {
        Napi::Error::New(env, "SetWindowCompositionAttribute is not available").ThrowAsJavaScriptException();
        return env.Null();
    }
    
    // Apply acrylic/blur effect
    ACCENT_POLICY policy = {};
    policy.AccentState = (type == "blur")
        ? ACCENT_ENABLE_BLURBEHIND
        : ACCENT_ENABLE_ACRYLICBLURBEHIND;
    policy.AccentFlags = 0;
    policy.AnimationId = 0;
    
    if (hasTintColor) {
        policy.GradientColor = (static_cast<DWORD>(opacity) << 24) | tintRGB;
    } else {
        // Use default tint color with specified opacity
        policy.GradientColor = (static_cast<DWORD>(opacity) << 24) | tintRGB;
    }
    
    WINDOWCOMPOSITIONATTRIBDATA data = {};
    data.Attrib = 19; // WCA_ACCENT_POLICY
    data.pvData = &policy;
    data.cbData = sizeof(policy);
    
    if (!SetWCA(hwnd, &data)) {
        Napi::Error::New(env, "Failed to apply accent policy").ThrowAsJavaScriptException();
        return env.Null();
    }
    
    // Apply corner style
    int pref = (corner == "none") ? 1  
        : (corner == "round") ? 2 
        : 3; // roundsmall
    
    HRESULT hr = DwmSetWindowAttribute(
        hwnd,
        DWMWA_WINDOW_CORNER_PREFERENCE,
        &pref,
        sizeof(pref)
    );
    
    // Apply border color - this should always be applied on Windows 11+
    DWORD build = GetWindowsBuild();
    if (IsWindowsVersionOrGreater(10, 0, 22000) || build >= 22000) {
        // Convert RGB to the format expected by Windows API
        // FIXED LOGIC: Use 0x00 for visible, 0xFF for hidden
        DWORD cr = ((borderVis ? 0x00 : 0xFF) << 24)  // Fixed: 0x00 for visible, 0xFF for hidden
            | ((borderRGB & 0x0000FF) << 16)
            | (borderRGB & 0x00FF00)
            | ((borderRGB & 0xFF0000) >> 16);
        
        hr = DwmSetWindowAttribute(
            hwnd,
            DWMWA_BORDER_COLOR,
            &cr,
            sizeof(cr)
        );
    }
    
    return Napi::Boolean::New(env, true);
}

Napi::Value IsSupported(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    
    HMODULE hUser = GetModuleHandleW(L"user32.dll");
    if (!hUser) {
        return Napi::Boolean::New(env, false);
    }
    
    auto SetWCA = reinterpret_cast<pSetWindowCompositionAttribute>(
        GetProcAddress(hUser, "SetWindowCompositionAttribute"));
    
    return Napi::Boolean::New(env, SetWCA != nullptr);
}

Napi::Value GetWindowsBuildNumber(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    return Napi::Number::New(env, GetWindowsBuild());
}

#else
// Non-Windows stub implementations
Napi::Value applyEffect(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    Napi::Error::New(env, "Acrylic effects are only supported on Windows").ThrowAsJavaScriptException();
    return env.Null();
}

Napi::Value IsSupported(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    return Napi::Boolean::New(env, false);
}

Napi::Value GetWindowsBuildNumber(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    return Napi::Number::New(env, 0);
}
#endif

Napi::Object Init(Napi::Env env, Napi::Object exports) {
    exports.Set("applyEffect", Napi::Function::New(env, applyEffect));
    exports.Set("isSupported", Napi::Function::New(env, IsSupported));
    exports.Set("getWindowsBuildNumber", Napi::Function::New(env, GetWindowsBuildNumber));
    return exports;
}

NODE_API_MODULE(fancy_acrylic, Init)