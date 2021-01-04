using System;
using System.Collections.Generic;
using System.Reflection;
using System.Runtime.InteropServices;
using System.Text;

namespace AtomLang
{
    public class InterpreterException : Exception
    {
        public InterpreterException(string msg) : base(msg)
        {

        }
    }

    public static class Sys
    {
        public static void Print(string str)
        {
            System.Console.WriteLine(str);
        }
        //如果第一参数为Interpreter，第二参数为ref bool则这个函数为功能函数，可以获取解释器状态和打断执行状态
        public static string ExampleMethod(Interpreter inter, ref bool isNext)
        {
            //不继续执行
            isNext = false;
            //返回一个字符串
            return "getmethod";
        }
    }

    public unsafe class Interpreter : IDisposable
    {
        [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Unicode)]
        public struct TokenInfo
        {
            //[MarshalAs(UnmanagedType.LPWStr)]
            public string value;
            public int line;
            public int position;
        }
        [StructLayout(LayoutKind.Sequential)]
        public struct TokenGroup
        {
            [MarshalAs(UnmanagedType.ByValArray, SizeConst = 8)]
            public TokenInfo[] tokens;
            public int size;
        }

        [return: MarshalAs(UnmanagedType.LPWStr)]
        private delegate string LoadfileCallBack([MarshalAs(UnmanagedType.LPWStr)] string path);
        private delegate int FunctionCallBack(int id, TokenGroup doman, TokenGroup path, TokenGroup param);
        private delegate void ErrorInfoCallBack([MarshalAs(UnmanagedType.LPWStr)] string errorInfo);

        const string DLL_NAME = @"E:\JxCCode.Lang\x64\DLLDebug\JxCCode.Lang.dll";

        [DllImport(DLL_NAME, CharSet = CharSet.Unicode)]
        private extern static void GetErrorMessage(int id, StringBuilder sb);
        [DllImport(DLL_NAME, CharSet = CharSet.Unicode)]
        private extern static int Initialize(
            LoadfileCallBack loadfile,
            FunctionCallBack funcall,
            ErrorInfoCallBack errorinfo,
            ref int id);
        [DllImport(DLL_NAME)]
        private extern static void Terminate(int id);
        [DllImport(DLL_NAME, CharSet = CharSet.Unicode)]
        private extern static int ExecuteCode(int id, string code);
        [DllImport(DLL_NAME, CharSet = CharSet.Unicode)]
        private extern static int Next(int id);
        [DllImport(DLL_NAME, CharSet = CharSet.Unicode)]
        private extern static int SerializeState(int id, StringBuilder ser_str);
        [DllImport(DLL_NAME, CharSet = CharSet.Unicode)]
        private extern static int DeserializeState(int id, string deser_str);
        [DllImport(DLL_NAME, CharSet = CharSet.Unicode)]
        private extern static void GetLibVersion(StringBuilder str);

        private int id;
        private Func<string, string> loadfile;

        private Dictionary<int, object> insts = new Dictionary<int, object>();


        private object GetVar(int id)
        {
            object o = null;
            this.insts.TryGetValue(id, out o);
            return o;
        }

        public Interpreter(Func<string, string> loadfile)
        {
            int _id = 0;
            this.loadfile = loadfile;
            Initialize(OnLoadFile, OnFuncall, OnError, ref _id);
            this.id = _id;
        }

        private string OnLoadFile(string path)
        {
            return this.loadfile(path);
        }

        private int OnFuncall(int userid, TokenGroup domain, TokenGroup path, TokenGroup param)
        {
            string[] domains = new string[domain.size];
            for (int i = 0; i < domains.Length; i++)
            {
                domains[i] = (domain.tokens[i].value);
            }

            string[] paths = new string[path.size - 1];
            for (int i = 0; i < paths.Length; i++)
            {
                paths[i] = (path.tokens[i].value);
            }

            //获取最后一个方法名
            string method = (path.tokens[path.size - 1].value);

            string[] @params = new string[param.size];
            for (int i = 0; i < @params.Length; i++)
            {
                @params[i] = (param.tokens[i].value);
            }

            Type type = null;
            MethodInfo methodInfo = null;
            object inst = this.GetVar(userid);


            if (userid != 0)
            {
                //实例对象，用路径找最后对象
                for (int i = 0; i < paths.Length; i++)
                {
                    inst = GetSubObject(inst, inst.GetType(), paths[i]);
                }

            }
            else
            {
                //静态
                string domainPath = string.Join(".", domains);
                type = Type.GetType(domainPath);
                if (type == null)
                {
                    throw new InterpreterException(
                        string.Format("未找到域: {0},  line: {1}, position: {2}",
                        domainPath, domain.tokens[0].line, domain.tokens[0].position));
                }
                for (int i = 0; i < paths.Length; i++)
                {
                    inst = GetSubObject(inst, inst == null ? type : inst.GetType(), paths[i]);
                    if (inst == null)
                    {
                        throw new InterpreterException(
                            string.Format("未找到对象: {0},  line: {1}, position: {2}",
                            path.tokens[i].value, path.tokens[i].line, path.tokens[i].position));
                    }
                    type = inst.GetType();
                }
            }
            methodInfo = type.GetMethod(method);

            ParameterInfo[] paramTypes = methodInfo.GetParameters();
            object[] _params = new object[paramTypes.Length];

            for (int i = 0; i < _params.Length; i++)
            {
                if (@params.Length > i)
                    _params[i] = @params[i];
            }

            object rst = methodInfo.Invoke(inst, _params);

            return 1;
        }
        private object GetSubObject(object obj, Type t, string name)
        {
            Type type = t;
            FieldInfo fi = null;
            PropertyInfo pi = null;
            if ((fi = type.GetField(name)) != null)
            {
                if (fi == null)
                {
                    return null;
                }
                return fi.GetValue(obj);
            }
            else if ((pi = type.GetProperty(name)) != null)
            {
                if (pi == null)
                {
                    return null;
                }
                return pi.GetValue(obj, null);
            }
            return null;
        }

        private void OnError(string info)
        {
            throw new InterpreterException(info);
        }

        private string GetErrorMessage()
        {
            StringBuilder sb = new StringBuilder(1024);
            GetErrorMessage(this.id, sb);
            return sb.ToString();
        }
        public Interpreter ExecuteCode(string code)
        {
            if (ExecuteCode(this.id, code) != 0)
            {
                throw new InterpreterException(GetErrorMessage());
            }
            return this;
        }
        public Interpreter Next()
        {
            if (Next(this.id) != 0)
            {
                throw new InterpreterException(GetErrorMessage());
            }

            return this;
        }

        private bool disaposed = false;
        public void Dispose()
        {
            if (disaposed)
            {
                return;
            }
            disaposed = true;
            Terminate(this.id);
            GC.SuppressFinalize(this);
        }
    }
}
