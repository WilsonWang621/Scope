# Scope

## 作业目标

* 学会使用 STL 库
* 加深对于字符串处理的掌握
* 提高模拟水平,掌握基本的拆分功能、规划项目的能力
* 学会处理边界情况
* 规范代码风格
* 学会自己设计测试数据

## 背景

在 C++ 程序中,我们有时会见到这样的代码:

```cpp
int test = 100;
for(int i = 0; i < 100; i++) {
    std::string test = "New String";
    ...
}

bool fun() {
    int test = 0;
    {
        bool test = true;
    }
    { 
        std::string test = "test"; 
    }
}
```

我们可以发现,在不同的代码段中,同一个变量名可以代表不同的数据类型,当然也可以拥有不同的值。
那么,这是怎么做到的呢?这就要说到我们本次大作业的主角: **作用域(scope)** 了。

具体而言,上述代码之所以能够正常运行,是因为在C++中,通常我们的 **大括号** 会引入一个新的作用域
(这么说是为了便于大家理解。大家可以自行尝试,在 `for` 循环中,在函数定义时,到底哪些部分是一个作用域)。

如果一个变量名 `a` 在本作用域中没有被定义过,那么我们就可以在该作用域中定义一个名为 `a` 的新变量。这也是上述代码可以实现的一个原因。

但是,在作用域 A 中,我们能否使用定义在作用域 B 中的变量呢? 我们得分情况讨论。

请看下面的代码:

```cpp
bool test = true;
if(test) {
    int test1 = 100;
} else {
    test1 = 1000; // Invalid!
}
```

这样的代码是不合法的。这很符合人类的直觉:这两个作用域没有交集,那么它们自然不能互相访问对应作用域中的变量。

```cpp
bool test = true;
bool test1 = true;
if(test) {
    int value = 3;
    if(test1) {
        value += 5;// Valid!
    }
}
```

这样的代码是合法的。这也很符合直观: `value` 定义在一个大作用域中,我们在大作用域中定义了一个小作用域,那么小作用域自然可以访问大作用域中的变量。

```cpp
bool test = true;
bool test1 = true;
if(test) {
    if(test1) {
        int value = 3;
    }
    value += 5;// Invalid!
}
```

这样的代码是非法的。我们在小作用域里定义了一个变量,但是当我们输入那个 `}` 符号时,我们就退出了这个小作用域,并且在大作用域中无法再访问到小作用域中的变量了。

让我们再考虑一些特殊情况:

```cpp
int a = 3;
{
    int a = 4;
    {
        std::cout << a;// result is 4!
    }
}
```

如果在嵌套的作用域中,一个变量名被使用了多次,那么在使用该变量名时,程序会使用离自己最“**近**”的那个变量。也就是相对内层的变量。

## 作业说明

### 分数组成

|                          得分项                           | 分数占比 |
| :-------------------------------------------------------: | :------: |
|       通过STL练习&前置作业 **1346. Michelle 的学生会工作**        |   20%    |
|      通过 **2902. Scope-2025**       |   60%    |
|     Code Review    |   20%    |

* 在 Code Review 中我们会 **严格审查你的代码风格**，请遵循[代码风格要求](https://acm.sjtu.edu.cn/wiki/C%2B%2B代码风格)。

* 本次作业提供部分样例数据供同学们测试使用，数据存放在本仓库 `data` 目录下。
* 注意，这些数据并非 Online Judge 上的全部测试数据点，本地通过所有数据点并不能保证你能够通过 OJ 上的测试。

* 除本地提供的样例数据外，我们也鼓励同学们根据自己的程序逻辑，尝试自己设计测试数据辅助调试。
* 在开发和调试的过程中，你可以 **保存下自己设计过的测试数据** ，并记录下自己遇到过哪些困难的问题，
* 以及最终是如何解决的，我们将会**在 Code Review 时检查**这些内容。

### ⏱截止时间

- STL 练习：10 月 20 日 18:30
- 前置作业：10 月 27 日 18:30
- Scope-2025：11 月 3 日 18:30

## 作业要求

在本次作业中,你需要模拟作用域的基本行为,并且完成对应的指令操作。

具体而言,我们有如下六种指令:

* `Indent`
* `Dedent`
* `Declare [type] [variable_name] [value]`
* `Add [result] [value1] [value2]`
* `SelfAdd [variable_name] [value]`
* `Print [variable_name]`

(`[]` 内表示参数)

请你运用毕生所学,完成这一任务。

## 指令介绍

### Indent

相当于 C++ 程序中的 `{}`.它表示引入了一层嵌套的作用域。比如说,

```cpp
int a = 3;
{
    int a = 4;
}
```

用我们模拟器的话来说,可以是:

```
int a = 3;
Indent
int a = 4;
```

### Dedent

代表退出当前的最后一层作用域。

### Declare [type] [variable_name] [value]

表示于当前作用域中声明一个变量。比如说,

```cpp
int a = 3
string b = "acm"
```

在模拟器中写作:

```
Declare int a 3
Declare string b "acm"
```

在我们的模拟器中,取值要求为:

* `[type]`: `int` 或 `string`.
* `[variable_name]`: 同 c++ 要求。
* `value`: 若 `type` 为 `int`,则合法输入为整型数(保证输入若为整型数,一定在 `int` 表示范围内);
* 若 `type` 为 `string`,则合法输入为由 `"` 开头和结尾的任意字符串常量。

### Add [result] [value1] [value2]

`Add` 操作代表 c++ 中对应的加法操作。`[result]` 中的值会变为 `[value1]` 和 `[value2]` 中的值
在 c++ 中执行 `[value1] + [value2]` 的结果。三个变量必须已经被声明,并且其对应类型相同。

```cpp
std::string a, b, c;
int d, e, f;
...
a = b + c;
d = e + f;
```

在我们的模拟器中写作:

```
Add a b c
Add d e f
```

合法输入要求为: `[result]`, `[value1]`, `[value2]` 都为已声明的在作用域中可访问的变量名。

### SelfAdd [variable_name] [value]

相当于 c++ 中的 `+=` 操作。`[variable_name]` 中的值会变成 `[variable_name] + [value]`.即:

```cpp
int a = 3;
a += 3;
std::string b = "simple";
b += "naive";
```

在模拟器中为:

```
Declare int a 3
SelfAdd a 3
Declare string b "simple"
SelfAdd string b "naive"
```

合法输入要求为: `[variable_name]` 为已声明的在作用域中可访问的变量名。
若其类型为 `int`,则合法的 `[value]` 为整型数(如果为整型数,保证在 `int` 范围内); 若其类型为 `string`,
则合法的 `[value]` 为用 `"` 包括起来的字符串常量。

### Print [variable_name]

打印一个变量,和其对应的值。比如说:

```cpp
int a = 2;
std::string b = "SJTU";
std::cout << "a" << ":" << a << "\n";
std::cout << "b" << ":" << b << "\n";
```

在我们的模拟器中,等价于:

```
Declare int a 2
Declare string b "SJTU"
Print a
Print b
```

合法输入要求为: `[variable_name]` 为已声明的在作用域中可访问的变量名。

## 输入格式

第一行一个整数 n,表示有多少条指令。

之后共有 n 行,每行一条指令。

关于输入数据,保证如下:

* 指令名必然合法。也就是说,每一行的一个输入单词,必然是上述六种指令的开头单词中的一个。
* 每条指令后的参数数量合法。比如说,在 `Add` 指令中,那一行中必然有且仅有三个参数,不会多,也不会少。
* 保证在任何时刻,变量值不会发生溢出。
* 不保证 输入的参数值合法。

数据范围为 n < 1e6, `Indent` 次数少于 100 次。

## 输出格式

当执行到 `Print` 语句时,按上述要求输出。

如果当前语句不合法,那么认为当前语句无效,并且输出 `Invalid operation\n` .

## Tips
温馨提示：由于数据量达到了1e6，建议在main函数开头加上:
```c++
std::ios::sync_with_stdio(false);
std::cin.tie(nullptr);
```
加快读入速度，否则可能导致正解超时

除了你常用的输入输出头文件,字符串头文件外,你可能用到这些头文件的一部分:

* `<map>`
* `<unordered_map>`
* `<vector>`
* `<utility>`
* `<optional>`
* `<tuple>`
* `<variant>`


## Q&A:
### 1. 字符串中有转义字符吗
没有