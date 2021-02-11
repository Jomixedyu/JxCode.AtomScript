using System;
using System.Collections.Generic;
using System.IO;
using System.Reflection;
using System.Runtime.InteropServices;
using System.Text;

namespace JxCode.AtomLang
{
    public class InterpreterException : Exception
    {
        public InterpreterException(string msg) : base(msg)
        {

        }
    }

    public enum VariableType : int
    {
        Undefined,
        Null,
        Number,
        Strptr,
        Userptr,
    }
    [StructLayout(LayoutKind.Explicit)]
    public unsafe struct Variable
    {
        [FieldOffset(0)]
        public VariableType type;
        [FieldOffset(4)]
        public float num;
        [FieldOffset(4)]
        public int ptr;
    }
    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Unicode)]
    public unsafe struct TokenInfo
    {
        public char* value;
        public int line;
        public int position;
    }

    [StructLayout(LayoutKind.Sequential)]
    public unsafe struct TokenGroup
    {
        public TokenInfo* tokens;
        public int size;
    }

    [StructLayout(LayoutKind.Sequential)]
    public unsafe struct VariableGroup
    {
        public Variable* vars;
        public int size;
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
        const string DLL_NAME = @"JxCode.AtomScript.dll";

        [return: MarshalAs(UnmanagedType.LPWStr)]
        private delegate string LoadfileCallBack(int id, [MarshalAs(UnmanagedType.LPWStr)] string path);
        private delegate int FunctionCallBack(int id, int userptr, TokenGroup doman, TokenGroup path, VariableGroup param);


        [DllImport(DLL_NAME, CharSet = CharSet.Unicode)]
        private extern static void GetErrorMessage(int id, StringBuilder sb);
        [DllImport(DLL_NAME)]
        private extern static int NewInterpreter(ref int id);

        [DllImport(DLL_NAME, CharSet = CharSet.Unicode)]
        private extern static int Initialize(int id, LoadfileCallBack loadfile, FunctionCallBack funcall);

        [DllImport(DLL_NAME)]
        private extern static void Terminate(int id);
        [DllImport(DLL_NAME)]
        private extern static int ResetState(int id);
        [DllImport(DLL_NAME)]
        private extern static int ResetMemory(int id);

        [DllImport(DLL_NAME, CharSet = CharSet.Unicode)]
        private extern static int ExecuteProgram(int id, string file);
        [DllImport(DLL_NAME)]
        private extern static int Next(int id);

        [DllImport(DLL_NAME, CharSet = CharSet.Unicode)]
        private extern static int GetVariable(int id, string varname, ref Variable out_var);
        [DllImport(DLL_NAME, CharSet = CharSet.Unicode)]
        private extern static int SetVariable(int id, string varname, Variable var);
        [DllImport(DLL_NAME, CharSet = CharSet.Unicode)]
        private extern static int DelVariable(int id, string varname);
        [DllImport(DLL_NAME, CharSet = CharSet.Unicode)]
        private extern static int SetStringVariable(int id, string varname, string str);
        [DllImport(DLL_NAME, CharSet = CharSet.Unicode)]
        private extern static int NewString(int id, string str, ref int out_ptr);
        [DllImport(DLL_NAME, CharSet = CharSet.Unicode)]
        private extern static int GetString(int id, int str_ptr, StringBuilder out_str);
        [DllImport(DLL_NAME)]
        private extern static int GetStringLength(int id, int str_ptr, ref int out_length);

        [DllImport(DLL_NAME, CharSet = CharSet.Unicode)]
        private extern static int GetProgramName(int id, StringBuilder out_name);

        [DllImport(DLL_NAME)]
        private extern static int SerializeState(int id, ref int out_length);
        [DllImport(DLL_NAME, CharSet = CharSet.Unicode)]
        private extern static int TakeSerializationData(int id, byte[] buf);
        [DllImport(DLL_NAME, CharSet = CharSet.Unicode)]
        private extern static int DeserializeState(int id, byte[] deser_buf, int buf_size);

        [DllImport(DLL_NAME, CharSet = CharSet.Unicode)]
        private extern static void GetLibVersion(StringBuilder str);

        private const int kSuccess = 0;
        private const int kNullResult = 1;
        private const int kErrorMsg = 2;

        public const string __return = "__return";

        public const int kRunBreak = 0;
        public const int kRunNext = 1;

        private static Dictionary<int, Interpreter> interstates = new Dictionary<int, Interpreter>();

        private static string _OnLoadFile(int id, string path)
        {
            return interstates[id].OnLoadFile(path);
        }
        private static int _OnFuncall(int id, int userptr, TokenGroup domain, TokenGroup path, VariableGroup param)
        {
            return interstates[id].OnFuncall(userptr, domain, path, param);
        }

        private int id;
        public int Id { get => this.id; }
        private Func<string, string> loadfile;

        private Dictionary<int, object> userInstance = new Dictionary<int, object>();
        private int instanceAllocPtr = 0;

        private int AllocNewUserPtr(object obj)
        {
            ++this.instanceAllocPtr;
            this.userInstance.Add(this.instanceAllocPtr, obj);
            return this.instanceAllocPtr;
        }

        private void ThrowLastError()
        {
            throw new InterpreterException(this.GetErrorMessage());
        }

        public Interpreter(Func<string, string> loadfile)
        {
            int _id = 0;
            this.loadfile = loadfile;

            NewInterpreter(ref _id);
            this.id = _id;

            interstates.Add(_id, this);
            Initialize(_id, _OnLoadFile, _OnFuncall);
        }
        public Interpreter ExecuteProgram(string file)
        {
            if (ExecuteProgram(this.id, file) != kSuccess)
            {
                throw new InterpreterException(this.GetErrorMessage());
            }
            return this;
        }
        public Interpreter Next()
        {
            if (Next(this.id) != kSuccess)
            {
                throw new InterpreterException(GetErrorMessage());
            }
            return this;
        }

        private MethodInfo GetSerializeMethodInfo(object obj)
        {
            if (obj == null)
            {
                return null;
            }
            Type type = obj.GetType();
            try
            {
                MethodInfo methodInfo = type.GetMethod("Serialize");
                var parms = methodInfo.GetParameters();
                //有且只能有一个Stream类型形参
                if (parms.Length != 1 || parms[0].ParameterType != typeof(Stream))
                {
                    return null;
                }
                return methodInfo;
            }
            catch
            {
                return null;
            }
        }
        private MethodInfo GetDeserializeMethodInfo(string typePath)
        {
            Type type = Type.GetType(typePath);
            if (type == null)
            {
                return null;
            }
            try
            {
                MethodInfo mi = type.GetMethod("Deserialize");
                if (mi == null)
                {
                    return null;
                }
                var parms = mi.GetParameters();
                if (parms.Length != 1 || parms[0].ParameterType != typeof(Stream))
                {
                    return null;
                }
                return mi;
            }
            catch
            {
                return null;
            }
        }

        public Variable GetVariable(string name)
        {
            Variable variable = new Variable();
            if (GetVariable(this.id, name, ref variable) != kSuccess)
            {
                throw new InterpreterException(this.GetErrorMessage());
            }
            return variable;
        }
        public void SetVariable(string name, Variable variable)
        {
            if (SetVariable(this.id, name, variable) != kSuccess)
            {
                throw new InterpreterException(this.GetErrorMessage());
            }
        }
        public void DelVariable(string name)
        {
            if (DelVariable(this.id, name) != kSuccess)
            {
                throw new InterpreterException(this.GetErrorMessage());
            }
        }
        public void SetStringVariable(string name, string str)
        {
            if (SetStringVariable(this.id, name, str) != kSuccess)
            {
                throw new InterpreterException(this.GetErrorMessage());
            }
        }

        private int NewString(string name)
        {
            int strptr = 0;
            if (NewString(this.id, name, ref strptr) != kSuccess)
            {
                ThrowLastError();
            }
            return strptr;
        }
        private string GetString(int strptr)
        {
            int length = 0;
            if (GetStringLength(this.id, strptr, ref length) != kSuccess)
                this.ThrowLastError();
            StringBuilder buf = new StringBuilder(length);
            if (GetString(this.id, strptr, buf) != kSuccess)
                this.ThrowLastError();
            return buf.ToString();
        }

        public string GetProgramName()
        {
            StringBuilder sb = new StringBuilder(256);
            if (GetProgramName(this.id, sb) != kSuccess)
            {
                this.ThrowLastError();
            }
            return sb.ToString();
        }

        public void Serialize(Stream stream)
        {
            int native_length = 0;
            if (SerializeState(this.id, ref native_length) != kSuccess)
            {
                this.ThrowLastError();
            }
            byte[] native_buf = new byte[native_length];
            if (TakeSerializationData(this.id, native_buf) != kSuccess)
            {
                this.ThrowLastError();
            }

            BinaryWriter bw = new BinaryWriter(stream);

            bw.Write(this.userInstance.Count);
            foreach (var item in this.userInstance)
            {
                //id 类型全名 序列化数据
                bw.Write(item.Key);
                bw.Write(item.Value.GetType().FullName);
                MethodInfo mi = GetSerializeMethodInfo(item.Value);
                if (mi == null)
                {
                    //WARNING: Unable to complete serialization
                    //continue;
                    bw.Write(0);
                }
                else
                {
                    //流对象隔离
                    MemoryStream ms = new MemoryStream();
                    mi.Invoke(item.Value, new object[] { ms });
                    var buf = ms.ToArray();
                    bw.Write(buf.Length);
                    bw.Write(buf);
                    ms.Close();
                }
            }

            //写解释器数据
            bw.Write(native_buf.Length);
            bw.Write(native_buf);

            bw.Flush();
        }
        public void Deserialize(Stream stream)
        {
            BinaryReader br = new BinaryReader(stream);

            int varCount = br.ReadInt32();
            for (int i = 0; i < varCount; i++)
            {
                //name, Type, value
                int user_id = br.ReadInt32();
                string typeFullName = br.ReadString();

                int bufLength = br.ReadInt32();

                //流数据隔离
                MemoryStream ms = new MemoryStream();
                if (bufLength != 0)
                {
                    ms.Write(br.ReadBytes(bufLength), 0, bufLength);
                    ms.Position = 0;
                }

                Type t = Type.GetType(typeFullName);
                object instance = Activator.CreateInstance(t);

                MethodInfo mi = GetDeserializeMethodInfo(typeFullName);
                object v = mi.Invoke(instance, new object[] { ms });
                ms.Close();
                this.userInstance.Add(user_id, v);
            }

            //native data
            int nativeLength = br.ReadInt32();
            byte[] nativeBuf = br.ReadBytes(nativeLength);
            if(DeserializeState(this.id, nativeBuf, nativeLength) != kSuccess)
            {
                this.ThrowLastError();
            }
        }

        private object GetLocalUserInstance(int userptr)
        {
            object value = null;
            this.userInstance.TryGetValue(userptr, out value);
            return value;
        }
        public void SetNumberVariable(string name, float num)
        {
            Variable v = new Variable();
            v.type = VariableType.Number;
            v.num = num;
            this.SetVariable(name, v);
        }
        public void SetUserVariable(string name, object obj)
        {
            int ptr = this.AllocNewUserPtr(obj);
            Variable v = new Variable();
            v.type = VariableType.Userptr;
            v.ptr = ptr;
            this.SetVariable(name, v);
        }

        public object VariableToAny(Variable variable)
        {
            switch (variable.type)
            {
                case VariableType.Undefined:
                    return null;
                case VariableType.Null:
                    return null;
                case VariableType.Number:
                    return variable.num;
                case VariableType.Strptr:
                    return this.GetString(variable.ptr);
                case VariableType.Userptr:
                    return this.GetLocalUserInstance(variable.ptr);
                default:
                    return null;
            }
        }
        public object GetAnyVariable(string name)
        {
            var _var = this.GetVariable(name);
            return this.VariableToAny(_var);
        }

        public void SetAnyVariable(string name, object obj)
        {
            Type retType = obj.GetType();
            if(retType == typeof(string))
            {
                this.SetStringVariable(name, (string)obj);
            }
            else if (retType.IsPrimitive)
            {
                this.SetNumberVariable(name, Convert.ToSingle(obj));
            }
            else
            {
                this.SetUserVariable(name, obj);
            }
        }

        private string GetExceptionInfo(TokenInfo info, string content)
        {
            string value = new string(info.value);
            return string.Format("{0}, value: {1}, line: {2}, pos: {3}",
                content, value, info.line, info.position);
        }

        private string OnLoadFile(string path)
        {
            return this.loadfile(path);
        }

        private int OnFuncall(int userid, TokenGroup domain, TokenGroup path, VariableGroup param)
        {
            string[] domains = new string[domain.size];
            for (int i = 0; i < domains.Length; i++)
            {
                domains[i] = new string(domain.tokens[i].value);
            }

            string[] paths = new string[path.size - 1];
            for (int i = 0; i < paths.Length; i++)
            {
                paths[i] = new string(path.tokens[i].value);
            }

            //获取最后一个方法名
            TokenInfo methodNameToken = path.tokens[path.size - 1];
            string method = new string(path.tokens[path.size - 1].value);

            object[] paramstrs = new object[param.size];
            for (int i = 0; i < paramstrs.Length; i++)
            {
                paramstrs[i] = this.VariableToAny(param.vars[i]);
            }

            Type type = null;
            MethodInfo methodInfo = null;
            object inst = this.GetLocalUserInstance(userid);


            if (userid != 0)
            {
                //实例对象，用路径找最后对象
                for (int i = 0; i < paths.Length; i++)
                {
                    inst = GetSubObject(inst, inst.GetType(), paths[i]);
                }
                type = inst.GetType();
            }
            else
            {
                //静态
                string domainPath = string.Join(".", domains);
                type = Type.GetType(domainPath);
                if (type == null)
                {
                    throw new InterpreterException(GetExceptionInfo(domain.tokens[0], "未找到域"));
                }
                for (int i = 0; i < paths.Length; i++)
                {
                    inst = GetSubObject(inst, inst == null ? type : inst.GetType(), paths[i]);
                    if (inst == null)
                    {
                        throw new InterpreterException(GetExceptionInfo(path.tokens[i], "未找到对象"));
                    }
                    type = inst.GetType();
                }
            }
            methodInfo = type.GetMethod(method);

            ParameterInfo[] paramTypes = methodInfo.GetParameters();
            object[] _params = new object[paramTypes.Length];

            int realParamPos = 0;
            int inToRealParamOffset = 0;

            bool isSpecialMethod = false;
            if (paramTypes.Length >= 2)
            {
                if (paramTypes[0].ParameterType == typeof(Interpreter)
                    && paramTypes[1].ParameterType == typeof(bool).MakeByRefType())
                {
                    isSpecialMethod = true;
                    realParamPos = 2;
                    inToRealParamOffset = -2;
                    _params[0] = this;
                    _params[1] = true;
                }
            }
            for (; realParamPos < _params.Length; realParamPos++)
            {
                //没有超过传入的参数数量
                if (paramstrs.Length > realParamPos + inToRealParamOffset)
                {
                    int inParamsPos = realParamPos + inToRealParamOffset;
                    var _p = Convert.ChangeType(paramstrs[inParamsPos], paramTypes[realParamPos].ParameterType);
                    _params[realParamPos] = _p;
                }
            }

            object rst = methodInfo.Invoke(inst, _params);
            if (rst != null)
            {
                this.SetAnyVariable(__return, rst);
            }
            if (isSpecialMethod && (bool)_params[1] == false)
            {
                return kRunBreak;
            }
            else
            {
                return kRunNext;
            }
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

        private string GetErrorMessage()
        {
            StringBuilder sb = new StringBuilder(1024);
            GetErrorMessage(this.id, sb);
            return sb.ToString();
        }


        private bool disposed = false;
        public void Dispose()
        {
            if (disposed)
            {
                return;
            }
            disposed = true;
            Terminate(this.id);
            interstates.Remove(this.id);
            GC.SuppressFinalize(this);
        }
        ~Interpreter()
        {
            this.Dispose();
        }
    }
}
