{
    files = {
        [[build\.objs\rt\windows\x64\debug\rtBVH.cu.obj]],
        [[build\.objs\rt\windows\x64\debug\rtBuffer.cu.obj]]
    },
    values = {
        [[C:\Program Files\NVIDIA GPU Computing Toolkit\CUDA\v11.7\bin\nvcc]],
        {
            [[-LC:\Program Files\NVIDIA GPU Computing Toolkit\CUDA\v11.7\lib\x64]],
            "-lcudadevrt",
            "-lcudart_static",
            "-m64",
            "-dlink"
        }
    }
}