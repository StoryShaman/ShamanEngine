workspace "ShamanEngine"
	architecture "x64"
	
	configurations
	{
        "Debug",
        "Release"
    }

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

project "ShamanEngine"
kind "ConsoleApp"
language "C++"

targetdir ("build/bin/" .. outputdir .. "/%{prj.name}")
objdir ("build/bin-obj/" .. outputdir .. "/%{prj.name}")

files
{
    "shaders/*",
    "src/**.h",   
    "src/**.cpp",
    "config/**.ini"
}


includedirs
{
    "include/",
    "vendor/",
    "vendor/vulkan/"
}

libdirs
{
    "vendor/GLFW/lib-vc2022",
    "vendor/vulkan"
}
links { "glfw3_mt", "vulkan-1" }


filter "system:windows"
cppdialect "C++17"
staticruntime "On"
systemversion "latest"

filter { "configurations:Debug" }
    buildoptions "/MTd"
    defines { "DEBUG" }
    runtime "Debug"
    symbols "On"
    

filter { "configurations:Release" }
    buildoptions "/MT"
    defines { "NDEBUG" }
    runtime "Release"
    optimize "On"
    


