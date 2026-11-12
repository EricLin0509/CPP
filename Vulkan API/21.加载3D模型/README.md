# 加载 3D 模型

目前，我们是在 C++ 代码中直接硬编码模型的顶点数据

```cpp
MyModel::Builder modelBuilder{};
modelBuilder.vertices = {
    // left face (white)
    {{-.5f, -.5f, -.5f}, {.9f, .9f, .9f}},
    {{-.5f, .5f, .5f}, {.9f, .9f, .9f}},
    {{-.5f, -.5f, .5f}, {.9f, .9f, .9f}},
    {{-.5f, .5f, -.5f}, {.9f, .9f, .9f}},
    // ... 其余五个面各 4 个顶点 ...
};
for (auto& v : modelBuilder.vertices) {
    v.position += offset;
}

modelBuilder.indices = {0, 1, 2, 0, 3, 1, 4, 5, 6, 4, 7, 5, 8, 9, 10, 8, 11, 9,
                        12, 13, 14, 12, 15, 13, 16, 17, 18, 16, 19, 17, 20, 21, 22, 20, 23, 21};
```

但这种方式并不优雅，可维护性差，也不便于扩展

## Wavefront OBJ 格式模型

解决方案是：把模型的顶点数据和索引数据存储在一个单独的文件中，再由代码读取该文件中的数据

Wavefront OBJ 是一种工业标准的模型格式，用于存储 3D 模型的顶点数据和索引数据

其格式大致如下：

```
o model_name
v <x> <y> <z>
vt <u> <v>
vn <nx> <ny> <nz>
f <v/vt/vn> <v/vt/vn> <v/vt/vn> <v/vt/vn> <v/vt/vn>
```

- `o`: 模型名称
- `v`: 顶点位置
- `vt`: 顶点纹理坐标
- `vn`: 顶点法线
- `f`: 三角形面
    - 每个顶点都需要指定索引，格式为 `v/vt/vn`，分别对应顶点索引、纹理坐标索引和法线索引

这里我们使用 [tinyobjloader](https://github.com/tinyobjloader/tinyobjloader) 库来加载 OBJ 格式的模型文件

## 顶点结构

顶点结构里包含位置、颜色、法线与纹理坐标四组数据：

```cpp
struct Vertex {
    glm::vec3 position;
    glm::vec3 color;
    glm::vec3 normal;
    glm::vec2 uv;

    static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
    static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();

    bool operator==(const Vertex& other) const
    {
        return position == other.position && color == other.color && normal == other.normal && uv == other.uv;
    }
};
```

各字段在内存中的位置：

| 字段 | 类型 | 偏移 | 大小 |
| :-- | :-- | :--: | :--: |
| `position` | `glm::vec3` | 0 | 12 |
| `color` | `glm::vec3` | 12 | 12 |
| `normal` | `glm::vec3` | 24 | 12 |
| `uv` | `glm::vec2` | 36 | 8 |

- **共 44 字节**：四个字段都是 4 字节浮点，没有额外填充，`sizeof(Vertex)` 就是 44
- **`stride` 由结构自身决定**：绑定描述里写的是 `sizeof(Vertex)`，步长始终等于顶点结构的大小
- **属性描述只声明两条**：`getAttributeDescriptions()` 返回的向量长度是 2，`location = 0` 取 `position`、`location = 1` 取 `color`；`normal` 与 `uv` 只是存在顶点缓冲里，管线没有声明对应 location
- **`operator==` 逐字段比较**：四个字段全部相等才算同一个顶点，用于后面判定"这个顶点是否已经存过"
- **`glm::vec3` 的 `==` 是分量比较**：不是按位比较，`-0.0f == 0.0f` 成立，恰好符合几何上"同一个点"的语义

## 解析 OBJ 文件

tinyobjloader 是单头文件库，实现部分由宏打开：

```cpp
#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>
```

- **`TINYOBJLOADER_IMPLEMENTATION` 展开出函数定义**：不定义这个宏时头文件里只有声明，链接会找不到符号
- **只能在一个翻译单元里定义**：多个 `.cpp` 都定义会产生重复符号，所以宏放在 `my_model.cpp` 而不是头文件里
- **宏与链接库的配合**：本翻译单元已经编译出全部实现，`-ltinyobjloader` 的成员不会被链接器拉入；库的作用是让其他翻译单元也能按同一套声明引用这些函数

解析入口拿到四样东西：

```cpp
tinyobj::attrib_t attrib;
std::vector<tinyobj::shape_t> shapes;
std::vector<tinyobj::material_t> materials;
std::string warn, err;

if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filepath.c_str()))
{
    throw std::runtime_error(err + warn);
}

vertices.clear();
indices.clear();

std::unordered_map<Vertex, uint32_t> uniqueVertices;
```

| 参数 | 内容 |
| :-- | :-- |
| `attrib` | 文件里所有 `v`、`vt`、`vn` 的扁平浮点数组，以及可选的顶点颜色 |
| `shapes` | 每个 `o` 或 `g` 对应一个 shape，`shape.mesh.indices` 是按面平铺的索引三元组 |
| `materials` | `mtllib` 与 `usemtl` 收集到的材质，代码后续没有读取 |
| `warn` / `err` | 分别收集警告与错误文本 |

- **返回值是 `bool`**：失败时把 `err` 与 `warn` 拼起来抛 `runtime_error`，两者都为空时异常消息也是空串
- **`attrib` 是"按类型分组"的**：位置、纹理坐标、法线各占一个数组，靠 `v/vt/vn` 里的三个数字分别寻址
- **`vertices.clear()` 与 `indices.clear()` 先执行**：同一个 `Builder` 可以反复 `loadModel`，不会在旧数据上叠加
- **`uniqueVertices` 是局部变量**：去重表只在加载期间存在，加载完随函数一起销毁
- **没有 `reserve`**：表与两个向量都从 0 容量开始，靠多次扩容长到最终大小
- **`LoadObj` 没有开启三角化**：`shape.mesh.indices` 按文件里每个面的顶点数原样平铺，因此文件中的面需要已经是三角形，索引数量才与 `vkCmdDrawIndexed` 的三角形假设一致

## 读取单个顶点

每个面角依次填充四类数据，三类索引各自独立判断：

```cpp
if (index.vertex_index >= 0)
{
    vertex.position = {
        attrib.vertices[index.vertex_index * 3 + 0],
        attrib.vertices[index.vertex_index * 3 + 1],
        attrib.vertices[index.vertex_index * 3 + 2],
    };

    vertex.color = {
        attrib.colors[index.vertex_index * 3 + 0],
        attrib.colors[index.vertex_index * 3 + 1],
        attrib.colors[index.vertex_index * 3 + 2],
    };
}

if (index.normal_index >= 0)
{
    vertex.normal = {
        attrib.normals[index.normal_index * 3 + 0],
        attrib.normals[index.normal_index * 3 + 1],
        attrib.normals[index.normal_index * 3 + 2],
    };
}

if (index.texcoord_index >= 0)
{
    vertex.uv = {
        attrib.texcoords[index.texcoord_index * 2 + 0],
        attrib.texcoords[index.texcoord_index * 2 + 1],
    };
}
```

- **位置用 `3*i + 0/1/2` 寻址**：`attrib.vertices` 是 `x y z` 平铺的，第 `i` 个顶点占三个连续位置
- **法线同样是 `3*i` 步长**：`attrib.normals` 的结构与 `attrib.vertices` 一样
- **纹理坐标用 `2*i` 步长**：`attrib.texcoords` 每项只有 `u v` 两个分量
- **`>= 0` 判断的是"文件里有没有这类索引"**：OBJ 允许 `f 1//3` 这种省略 `vt` 的写法，此时 tinyobjloader 把对应索引置为 `-1`
- **缺失的字段保持 `Vertex{}` 的零值**：法线缺失时是 `(0, 0, 0)`、UV 缺失时是 `(0, 0)`，代码没有给它们补默认值

## 去重与索引生成

### 为什么要去重

OBJ 的 `f` 行记录的是"面角"——每个三角形的每个角，而不是独立顶点，同一个几何顶点会被相邻的多个面反复引用。如果每读到一个面角就 `vertices.push_back(vertex)` 一次，顶点缓冲里会堆满重复数据：

| 模型 | 面角数 | 去重后顶点数 | 倍数 |
| :-- | :--: | :--: | :--: |
| `cube.obj` | 36 | 24 | 1.50 |
| `flat_vase.obj` | 30888 | 23894 | 1.29 |
| `smooth_vase.obj` | 30888 | 5546 | 5.57 |

- **顶点缓冲体积**：`Vertex` 是 44 字节，`smooth_vase.obj` 直接铺面角要 30888 × 44 ≈ 1.36 MB，去重后 5546 × 44 ≈ 244 KB
- **顶点阶段的工作量**：绘制时按索引取顶点，同一个索引只对应一份顶点数据，复用越多顶点着色器要处理的数据越少
- **存储与绘制顺序解耦**：顶点缓冲只回答"有哪些顶点"，索引缓冲回答"按什么顺序连成三角形"，同一份顶点可以被任意多个三角形引用
- **索引比顶点便宜**：一个下标是 `uint32_t` 的 4 字节，复制一份顶点是 44 字节，用索引换掉重复顶点总是划算的
- **两个花瓶的差别在于法线**：`flat_vase.obj` 与 `smooth_vase.obj` 的 `v` 行都是 5150 条，但 `flat_vase.obj` 有 6799 条 `vn`，大量位置在不同面上带着不同法线，去重后留下 23894 个顶点；`smooth_vase.obj` 的法线按位置共享，只有 5150 条 `vn`，去重后的顶点数由 5546 条 `vt` 的接缝决定
- **只按位置合并会破坏着色**：位置相同但法线不同的面角一旦被合并成一份数据，两边就会被同一条法线决定光照，硬边变圆滑、UV 接缝处也会错位——所以键必须是完整的 `Vertex`

### 顶点哈希

去重表以 `Vertex` 为键，而 `std::unordered_map` 要求键能被哈希，所以 `Vertex` 需要一个哈希函数。`my_utils.hpp` 提供了把多个字段混进同一个种子的工具：

```cpp
template <typename T, typename... Rest>
void hashCombine(std::size_t& seed, const T& v, const Rest&... rest) {
  seed ^= std::hash<T>{}(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
  (hashCombine(seed, rest), ...);
};
```

- **`seed` 按引用传递**：整个递归过程共用一个累加值，返回值不需要层层回传
- **变参 + 折叠表达式**：`(hashCombine(seed, rest), ...)` 把剩下的参数逐个展开成逗号表达式；当 `Rest` 为空时这次展开什么也不做，递归自然终止，所以不需要额外的零参数重载
- **`0x9e3779b9` 是 32 位黄金比例常数**：`2³² / φ`，它的比特分布均匀，用来打散 `std::hash` 输出里的规律性
- **`seed << 6` 与 `seed >> 2`**：把已有状态的两个不同位移方向混回自身，让先前混入的字段影响后续字段的位置

`Vertex` 的哈希特化放在 `namespace std` 里：

```cpp
namespace std {

template <>
struct hash<lve::MyModel::Vertex> {
    size_t operator()(const lve::MyModel::Vertex& vertex) const
    {
        size_t seed = 0;
        lve::hashCombine(seed, vertex.position, vertex.color, vertex.normal, vertex.uv);
        return seed;
    }
};

}
```

- **特化必须写在 `namespace std` 内**：`std::hash` 是标准库模板，用户自定义类型的特化只有放在这个命名空间里才会被 `unordered_map` 找到
- **四个字段按固定顺序混入**：顺序只影响哈希值的分布，不影响正确性
- **种子从 0 开始**：每次调用都是独立的一轮混入
- **哈希与 `operator==` 必须一致**：哈希相同只是候选，真正的判定交给 `operator==`；如果 `operator==` 认为相等的两个顶点算出不同的哈希，`unordered_map` 就会把它们当成两个不同的键，去重直接失效

混入 `glm::vec3`、`glm::vec2` 需要 glm 的实验性扩展：

```cpp
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>
```

- **`gtx/hash.hpp` 提供 `std::hash<glm::vec3>` 与 `std::hash<glm::vec2>`**：`hashCombine` 里的 `std::hash<T>{}` 正是靠它才能实例化
- **`GLM_ENABLE_EXPERIMENTAL` 必须在包含之前定义**：glm 用这个宏来开关 `gtx` 系列扩展，顺序颠倒会编译失败

### 合并重复顶点

四个字段完全相同的顶点只保留一份：

```cpp
if (uniqueVertices.count(vertex) == 0)
{
    uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
    vertices.push_back(vertex);
}
indices.push_back(uniqueVertices[vertex]);
```

- **`count` 为 0 表示首次出现**：此时它在 `vertices` 里的下标就是当前的 `vertices.size()`
- **先记下标再 `push_back`**：`vertices.size()` 取的是插入前的长度，正好是新元素将要占据的位置
- **每个面角都产出一个索引**：无论顶点是否重复，`indices` 都会追加一项，所以 `indices.size()` 等于三角形数 × 3
- **顶点顺序就是首次出现的顺序**：`vertices` 按遍历顺序增长，没有排序或重排
- **多个 shape 合并成一份**：外层 `for (const auto &shape : shapes)` 把每个 shape 的面都追加到同一组 `vertices` 与 `indices`
- **去重的粒度是"整个顶点"**：位置相同但法线或 UV 不同的两个顶点不会被合并——这正是 OBJ 里 `v/vt/vn` 三段索引存在的意义
- **索引与顶点共用同一个下标空间**：`uniqueVertices[vertex]` 存的就是 `vertices` 里的位置，`vkCmdDrawIndexed` 直接按它取数据

## 从文件创建模型

构造函数只接收已经准备好的数据，读文件这一步交给静态工厂：

```cpp
std::unique_ptr<MyModel> MyModel::createModel(VulkanDevice& device, const std::string& filepath)
{
    Builder builder{};
    builder.loadModel(filepath);

    std::cout << "Vertex count: " << builder.vertices.size() << "\n";
    return std::make_unique<MyModel>(device, builder);
}
```

- **`Builder` 是局部对象**：解析出的顶点与索引只活到构造函数结束，之后数据已经在设备内存里
- **打印顶点数**：`loadModel` 完成后把去重后的顶点个数输出到标准输出，方便确认文件是否按预期解析
- **返回 `unique_ptr`**：`MyModel` 的拷贝构造与拷贝赋值都被 `delete`，只能通过指针持有
- **设备引用贯穿始终**：`createModel` 只是把 `device` 原样转交给构造函数

## 场景加载

`loadGameObjects` 只用一个文件路径就完成模型准备：

```cpp
void FirstApp::loadGameObjects()
{
  std::shared_ptr<MyModel> model = MyModel::createModel(device, "models/smooth_vase.obj");
  auto gameObject = GameObject::createGameObject();
  gameObject.model = model;
  gameObject.transform.translation = {.0f, .0f, 2.5f};
  gameObject.transform.scale = {.5f, .5f, .5f};

  gameObjects.push_back(std::move(gameObject));
}
```

- **路径是相对路径**：`"models/smooth_vase.obj"` 相对于进程的工作目录，从别的目录启动会读不到文件
- **`shared_ptr` 持有模型**：多个 `GameObject` 可以指向同一个 `MyModel`，顶点与索引缓冲只建一份
- **`translation` 与 `scale` 决定摆放**：平移把模型推到相机前方，缩放统一压缩三个轴，模型文件里的原始尺寸决定缩放后的实际大小
- **对象名是 `gameObject`**：指向该模型的场景对象用通用名称命名，与具体加载的模型解耦
