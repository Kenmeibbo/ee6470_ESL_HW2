import numpy as np
from PIL import Image

def read_bmp_pixels(file_path, output_file):
    # 使用PIL库打开BMP图像文件
    img = Image.open(file_path)
    # 将图像转换为numpy数组
    img_array = np.array(img)
    # 获取图像的宽度和高度
    width, height = img.size
    # 获取图像的通道数
    channels = img_array.shape[2] if len(img_array.shape) > 2 else 1

    # 打开输出文件以写入模式
    with open(output_file, 'w') as f:
        # 遍历图像的像素值
        for y in range(height):
            for x in range(width):
                # 从数组中获取像素值
                pixel_value = img_array[y, x]
                if channels > 1:
                    # 如果是彩色图像，则分别输出每个通道的值
                    f.write(f"Pixel at ({x}, {y}): {pixel_value}\n")
                    #print("here is color")
                    #print("channel",channels)
                else:
                    # 如果是灰度图像，则直接输出像素值
                    f.write(f"Pixel at ({x}, {y}): {pixel_value}\n")
                    #print("here is grey")
                    #print("channel",channels)

# 调用函数读取BMP图像的像素值
bmp_file_path = "out.bmp"##"lena_std_short.bmp" ##"lena_color_256_noise_gaussian.bmp"
output_file_path = "out.txt"
read_bmp_pixels(bmp_file_path, output_file_path)
