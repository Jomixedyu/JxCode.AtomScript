# JxCode.AtomScript
 简易的非栈脚本语言，使用一行一个上下文无关的命令编写脚本，使用反射调用函数与对象，可以方便的序列化与反序列化 。


## 基础内容
* 变量的设置与删除
* 标签声明与跳转
* 逻辑判断与跳转
* 远程函数的调用
* 基础数字库

每个操作符都有一个关键字与符号相对应，函数的执行取决于使用该核心的解释器如何执行，默认提供了C#侧的解释器，通过反射执行函数，支持静态与实例对象的调用。  

## 数据类型
* Number （使用C++中的float储存）
* StrPtr （字符串整数Id，在使用C++中字符串池使用std::wstring储存）
* UserPtr（一个整数Id，由外部解释器来绑定对象）

关于变量：所有使用 set / $ 来声明的变量都是全局变量。  
关于字符串池GC：每隔128行执行一次GC，序列化时都会执行一次GC  

## 在项目中使用解释器
### 在C#项目中添加：
在项目中添加引用JxCode.AtomScript_cs.dll或者添加Interpreter.cs到项目当中。  
使用解释器需要引用命名空间```using JxCode.AtomScript```
```C#
//构造函数传入读取脚本的回调
Interpreter inter = new Interpreter(f => File.ReadAllText(f + ".txt"));
//执行程序，执行后构造传入的回调会被执行并传入该名字，读取代码后进行词法分析处理
inter.ExecuteProgram("eazytest");
//执行程序，直到遇到一个打断的函数或程序执行到底为止
inter.Next();
//对解释器进行序列化并储存
var fs = File.OpenWrite("ser.dat");
inter.Serialize(fs);
fs.Flush();
fs.Close();
//反序列化
var deser_fs = File.OpenRead("ser.dat");
inter.Deserialize(deser_fs);
deser_fs.Close();
//从打断序列化挂起的位置继续执行
inter.Next();
```
atomscript脚本代码：
```
$a = 3
$b = "hello"
//命名空间需要使用::来分割，类成员由.运算符来访问
@JxCode::AtomScript_cs::Program.print: b
@role.create: "jay"
//所有函数调用的返回值都在__return中
$jay = __return
//调用实例对象的函数，在C#中该函数为一个打断函数，所以程序会执行完这行代码后挂起
@jay.print: "print a string"
//当程序再次Next时继续执行
@jay.print: "happy"
```
在C#中函数是被反射调用的。  
```C#
public class role
{
    public string name;

    public static void show(string text)
    {
        Console.WriteLine("show text: " + text);
    }
    //该函数为一个特殊函数
    //第一个参数类型为Interpreter，第二个参数为ref bool类型的，就是特殊函数
    //在脚本侧调用此函数时，前两个参数会被忽略
    //当isNext设置为false时，解释器会被打断
    public void print(Interpreter inter, ref bool isNext, string text)
    {
        Console.WriteLine(this.name + ":  " + text);
        isNext = false;
    }
    //返回至脚本侧的__return中
    public static role create(string str)
    {
        return new role() { name = str };
    }

    public void Serialize(Stream stream)
    {
        BinaryWriter br = new BinaryWriter(stream);
        br.Write(this.name);
    }
    public void Deserialize(Stream stream)
    {
        Console.WriteLine("Deserialize");
        BinaryReader br = new BinaryReader(stream);
    }
}
```
如果需要解释器持有C#侧实例对象（UserPtr），为了保证序列化的可行性，需要使C#类必须拥有以下条件：  
* 类型必须拥有0个参数的构造函数
* 需要实现public void Serialize(Stream stream);
* 需要实现public void Deserialize(Stream stream);

## 支持更多的语言
查看JxCode.AtomScript\JxCode.AtomScript目录中的DLL.h查看导出的函数



## AtomScript基础语法
语法演示：  
使用基础关键字
```
goto IsZero__end
::IsZero
    if IsZero__parm1 == 0 else goto IsZero__logic1
        set IsZero__return = true
        goto var IsZero__back
    ::IsZero__logic1
        set IsZero__return = false
        goto var IsZero__back
::IsZero__end

set IsZero__back = "IsZeroCall1"
set IsZero__param1 = 3
goto IsZero
::IsZeroCall1
set ret = IsZero__return
clear IsZero

//static
call AtomLang::Sys.Print: "helloworld"
//inst
call AtomLang::Sys.obj.Print: "inst helloworld"
```
也可以使用符号
```
->IsZero__end
::IsZero
    ? IsZero__parm1 == 0 else->IsZero__logic1
        $IsZero__return = true
        ->var IsZero__back
    ::IsZero__logic1
        $IsZero__return = false
        ->var IsZero__back
::IsZero__end

$IsZero__back = "IsZeroCall1"
$IsZero__param1 = 3
->IsZero
::IsZeroCall1
$ret = IsZero__return
~IsZero

//static
@AtomLang::Sys.Print: "helloworld"
//inst
@AtomLang::Sys.obj.Print: "inst helloworld"
```

### 执行一个文件
```jumpfile "atomscript"```   
符号  
```=>"atomscript"```

### 设置一个变量
```set a = "helloworld"```  
符号  
```$a = "helloworld"```

### 删除一个变量
```del a```  
符号  
```-a```

### 声明一个标签
```label start```  
符号  
```::start```  

### 跳到一个标签  
```goto start```  
符号  
```->start```  

### 跳转至变量中的标签
```
set target = "start"
goto var target
```
符号  
```
$target = "start"
->var target
```

### 清除所有子变量
对start执行clear后，删除变量池中名字以 start__ 开头的所有变量  
```clear start```  
符号  
```~start```

### 执行函数
::在函数调用中为域运算符，一般指定为类型名称空间的路径，.是子对象运算符  
```call Atom::Sys.Print: "hello world"```  
符号  
```@Atom::Sys.Print: "hello world"```

### 逻辑
如果表达式运算成立，则向下继续执行，否则执行goto  
```if a == 0 else goto start```  
符号  
```? a == 0 else->start```
