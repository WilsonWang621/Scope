//
// Created by lenovo on 2025/10/22.
//
#include <iostream>
#include <unordered_map>
#include <vector>
#include <utility>
#include <optional>
#include <tuple>
#include <variant>

struct Variable {
  std::string type;
  std::variant<int, std::string> value;   //存储 int 或 std::string 两种类型中的一种
};

int n;
std::vector<std::unordered_map<std::string, Variable>> scope;  //scope 是一个向量，储存map，每个元素代表一个作用域
const int INT_MAX = 2147483647;
const int INT_MIN = -2147483648;

std::pair<Variable*, int> findVariable(const std::string& name) {   //查找函数  返回对应指针和scope层数
  for (int i = scope.size() - 1; i >= 0; i--) {  //从最内层作用域开始向外查找
    auto it = scope[i].find(name);
    if (it != scope[i].end()) {    //如果没找到：返回 end() 迭代器
      return {&(it->second), i};
    }
  }
  return {nullptr, -1};
}

std::vector<std::string> Split(const std::string& str) {      //切分命令成不同单词
  std::vector<std::string> v;
  std::string word = "";       // 当前正在构建的单词
  bool in_quotes = false;    // 是否在引号内

  for (char c : str) {
    if (c == '"') {
      in_quotes = !in_quotes;   // 遇到引号要切换状态
    }

    if (in_quotes || c != ' ') {   //在引号内或非空格字符，添加到当前单词
      word += c;
    }

    if (c == ' ' && !in_quotes) {  //引号外，并且是空格
      if (!word.empty()) {   //非空，有词
        v.push_back(word);
        word = "";           //重置
      }
      continue;
    }
  }

  if (!word.empty()) {        //特殊处理一下最后一个次，保证切割储存完整
    v.push_back(word);
  }
  return v;
}

bool Check(const std::string& name) {
  return !scope.empty() && scope.back().find(name) != scope.back().end();  //当前状态下scope非空，且最内层没有（避免重复命名）
}

void Indent() {   //加一个map
  scope.push_back(std::unordered_map<std::string, Variable>());
}

bool CheckVariableName(const std::string& name) {  //检查命名是否合理
  if (name.empty()) {
    return false;
  }
  if (!isalpha(name[0]) && name[0] != '_') {
    return false;
  }
  for (size_t i = 1; i < name.length(); i++) {
    if (!isalnum(name[i]) && name[i] != '_') {
      return false;
    }
  }
  return true;
}

void Dedent() {
  if (scope.size() <= 1) {  //保持至少有一个作用域存在（全局）
    std::cout << "Invalid operation\n";
    return;
  }
  scope.pop_back();
}

void Declare(const std::vector<std::string>& v) {
  //检查是否重复定义
  if (Check(v[2])) {
    std::cout <<  "Invalid operation\n" ;
    return;
  }
  //不在两种要求里
  if (v[1] != "int" && v[1] != "string") {
    std::cout << "Invalid operation\n";
    return;
  }
  else {
    if (CheckVariableName(v[2])) {  //没重复命名
      if (v[1] == "int") {
          long long transfer = 0;   //string -> longlong
          bool negative = false;
          for (char c : v[3]) {
              if (c == '-') {     //处理负数
                  negative = true;
                  continue;
              }
              if (!isdigit(c)) {
                  std::cout <<  "Invalid operation\n";
                  return;
              }
              transfer = transfer * 10 + (c - '0');
          }
          if (negative) {     //补上-
              transfer *= -1;
          }
        if (transfer < INT_MAX && transfer > INT_MIN) {  //先检查，在转化，否则一直 RE
          scope.back()[v[2]] = Variable{"int", std::stoi(v[3])};
        }
        else{
          std::cout << "Invalid operation\n";
          return;
        }
      }
      else {   //检查输入合法性
        if (v[3].length() < 2 || v[3][0] != '"' || v[3].back() != '"') {
          std::cout << "Invalid operation\n";
          return;
        }
        scope.back()[v[2]] = Variable{"string", v[3]};  //插入map
      }
    }
    else {
      std::cout << "Invalid operation\n";
      return;
    }
  }
  return;
}

void Add(const std::vector<std::string>& v) {
  auto [resVar, rescope] = findVariable(v[1]);  //查找结果
  auto [var1, scope1] = findVariable(v[2]);    //操作数1
  auto [var2,scope2] = findVariable(v[3]);  //操作数2

  if (!var1 || !var2 || !resVar) {     //任何一个没找到
    std::cout <<  "Invalid operation\n";
    return;
  }

  if (var1->type != var2->type || var1->type != resVar->type) {  //类型不匹配
    std::cout << "Invalid operation\n";
    return;
  }

  if (var1->type == "int") {
    int val1 = std::get<int>(var1->value);
    int val2 = std::get<int>(var2->value);
    resVar->value = val1 + val2;
  }
  else {
    std::string str1 = std::get<std::string>(var1->value);
    std::string str2 = std::get<std::string>(var2->value);
    std::string s1 = str1.substr(1, str1.length() - 2);   //去除“ ” 下同
    std::string s2 = str2.substr(1, str2.length() - 2);
    resVar->value = "\"" + s1 + s2 + "\"";    //补上“ ”
  }
}

void SelfAdd(const std::vector<std::string>& v) {
  auto [var1, scope1] = findVariable(v[1]);
  if (!var1) {
    std::cout << "Invalid operation\n";
    return;
  }

  if (var1->type == "int") {
      long long transfer = 0;
      bool negative = false;
      for (char c : v[2]) {
          if (c == '-') {
              negative = true;
              continue;
          }
          if (!isdigit(c)) {
              std::cout <<  "Invalid operation\n";
              return;
          }
          transfer = transfer * 10 + (c - '0');
      }
      if (negative) {
          transfer *= -1;
      }
      if (transfer < INT_MAX && transfer > INT_MIN) {
          int current = std::get<int>(var1->value);
          int to_add = std::stoi(v[2]);
          var1->value = current + to_add;
      }
      else{
          std::cout << "Invalid operation\n";
          return;
      }
    }
  else if (var1->type == "string") {
    if (v[2].length() < 2 || v[2][0] != '"' || v[2].back() != '"') {
      std::cout << "Invalid operation\n";
      return;
    }

    std::string cur = std::get<std::string>(var1->value);
    std::string add = v[2];
    std::string s1 = cur.substr(1, cur.length() - 2);
    std::string s2 = add.substr(1, add.length() - 2);
    var1->value = "\"" + s1 + s2 + "\"";
  }
}

void Print(const std::vector<std::string>& v) {
  auto result = findVariable(v[1]);   //查找到结果
  Variable* var = result.first;
  int scope = result.second;

  if (var == nullptr) {    //没有
    std::cout << "Invalid operation\n";
    return;
  }

  if (var->type == "int") {
    std::cout << v[1] << ":" << std::get<int>(var->value) << "\n";
  }
  else {
    std::string str_value = std::get<std::string>(var->value);
    if (str_value.length() >= 2 && str_value[0] == '"' && str_value.back() == '"') {   //检查答案的存储方式 是否有”“
      std::cout << v[1] << ":" << str_value.substr(1, str_value.length() - 2) << "\n";
    }
    else {
      std::cout << v[1] << ":" << str_value << "\n";
    }
  }
}
int main() {
  std::ios::sync_with_stdio(false);
  std::cin.tie(nullptr);

  std::cin >> n;
  std::cin.ignore();    // 忽略换行符 清除输入缓冲区
  //初始化
  scope.clear();
  scope.push_back(std::unordered_map<std::string, Variable>());  //先放一个必须存在的全局

  while (n--) {
    std::string in;
    getline(std::cin,in);
    std::vector<std::string> order = Split(in);

    // std::cout << order[1] ;
    if (order[0] == "Dedent") {
      Dedent();
    }
    else if (order[0] == "Indent") {
      Indent();
    }
    else if (order[0] == "Declare") {
      Declare(order);
    }
    else if (order[0] == "Add") {
      Add(order);
    }
    else if (order[0] == "SelfAdd") {
      SelfAdd(order);
    }
    else if(order[0] == "Print") {
      Print(order);
    }
  }

  return 0;
}