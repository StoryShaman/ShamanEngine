P:\Development\Tools\VulkanSDK\1_4\Bin\glslc.exe %~dp0simple_shader.vert -o %~dp0simple_shader_vert.spv
P:\Development\Tools\VulkanSDK\1_4\Bin\glslc.exe %~dp0simple_shader.frag -o %~dp0simple_shader_frag.spv
echo "simple_shader.vert -o simple_shader_vert.spv"
echo "simple_shader.frag -o simple_shader_frag.spv"
P:\Development\Tools\VulkanSDK\1_4\Bin\glslc.exe %~dp0raytrace.comp -o %~dp0raytrace.spv
echo "raytrace.comp -o raytrace.spv"

exit
