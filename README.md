# JxCode.AtomScript
 简易的非栈脚本语言，使用一行一个上下文无关的命令编写脚本，使用反射调用函数与对象，可以方便的序列化与反序列化 。


## 基础内容
* 变量的设置与删除
* 标签声明与跳转
* 逻辑判断与跳转
* 远程函数的调用

每个操作符都有一个关键字与符号相对应，函数的执行取决于使用该核心的解释器如何执行，默认提供了C#侧的解释器，通过反射执行函数，支持静态与实例对象的调用。  

## 数据类型
* Number （使用C++中的float储存）
* String （使用C++中的std::wstring储存）
* UserPtr（一个整数Id，由外部解释器来绑定对象）

## 基础语法
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
    ? IsZero__parm1 != 0 else ->IsZero__logic1
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
