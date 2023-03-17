{
    files = {
        [[build\.objs\rt\windows\x64\debug\main.cpp.obj]],
        [[build\.objs\rt\windows\x64\debug\rtBVH.cu.obj]],
        [[build\.objs\rt\windows\x64\debug\rtBuffer.cu.obj]],
        [[build\.objs\rt\windows\x64\debug\rules\cuda\devlink\rt_gpucode.cu.obj]]
    },
    values = {
        [[C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Tools\MSVC\14.33.31629\bin\HostX64\x64\link.exe]],
        {
            "-nologo",
            "-dynamicbase",
            "-nxcompat",
            "-machine:x64",
            [[-libpath:C:\Program Files\NVIDIA GPU Computing Toolkit\CUDA\v11.7\lib\x64]],
            "-debug",
            [[-pdb:build\windows\x64\debug\rt.pdb]],
            "cudadevrt.lib",
            "cudart_static.lib"
        }
    }
}