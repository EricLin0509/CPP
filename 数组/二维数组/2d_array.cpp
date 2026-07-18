#include <iostream>

int main()
{
    std::string arr[3][3] = {
        {"E46 M3 CSL", "E60 M5", "E63 M6"}, // BMW
        {"W204 C63 AMG", "W212 E63 AMG", "C218 CLS63 AMG"}, // Mercedes-Benz
        {"B7 RS4", "B7 RS5", "C6 RS6"} // Audi
    };

    int rows = sizeof(arr) / sizeof(arr[0]); // 行数
    int cols = sizeof(arr[0]) / sizeof(arr[0][0]); // 列数

    for (int i = 0; i < rows; i++) // 遍历行
    {
        for (int j = 0; j < cols; j++) // 遍历列
        {
            std::cout << arr[i][j] << "    ";
        }
        std::cout << "\n";
    }

    return 0;
}