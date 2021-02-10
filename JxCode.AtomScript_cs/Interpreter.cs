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
    public class NickAttribute : Attribute
    {
        public string NickName { get; set; }
        public NickAttribute(string name)
        {
            this.NickName = name;
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
        private delegate string LoadfileCallBack(int id, [MarshalAs(UnmanagedType.LPWStr)] string path);
        private delegate int FunctionCallBack(int id, int var_id, TokenGroup doman, TokenGroup path, TokenGroup param);

        const string DLL_NAME = @"JxCode.AtomScript.dll";

        [DllImport(DLL_NAME, CharSet = CharSet.Unicode)]
        private extern static void GetErrorMessage(int id, StringBuilder sb);
        [DllImport(DLL_NAME, CharSet = CharSet.Unicode)]
        private extern static int Initialize(
            LoadfileCallBack loadfile,
            FunctionCallBack funcall);

        [DllImport(DLL_NAME)]
        private extern static int NewInterpreter(ref int id);
        [DllImport(DLL_NAME)]
        private extern static void Terminate(int id);
        [DllImport(DLL_NAME, CharSet = CharSet.Unicode)]
        private extern static int ResetState(int id);
        [DllImport(DLL_NAME, CharSet = CharSet.Unicode)]
        private extern static int ExecuteCode(int id, string code);
        [DllImport(DLL_NAME, CharSet = CharSet.Unicode)]
        private extern static int Next(int id);

        [DllImport(DLL_NAME, CharSet = CharSet.Unicode)]
        private extern static int GetVarType(int id, string varname, ref int out_type);
        [DllImport(DLL_NAME, CharSet = CharSet.Unicode)]
        private extern static int GetVarString(int id, string varname, StringBuilder out_str);
        [DllImport(DLL_NAME, CharSet = CharSet.Unicode)]
        private extern static int GetVarNumber(int id, string varname, ref float out_num);
        [DllImport(DLL_NAME, CharSet = CharSet.Unicode)]
        private extern static int GetVarUser(int id, string varname, ref int out_userid);

        [DllImport(DLL_NAME, CharSet = CharSet.Unicode)]
        private extern static int SetVarString(int id, string varname, string value);
        [DllImport(DLL_NAME, CharSet = CharSet.Unicode)]
        private extern static int SetVarNum(int id, string varname, float num);
        [DllImport(DLL_NAME, CharSet = CharSet.Unicode)]
        private extern static int SetVarUser(int id, string varname, int userid);

        [DllImport(DLL_NAME, CharSet = CharSet.Unicode)]
        private extern static int DelVar(int id, string varname);

        [DllImport(DLL_NAME, CharSet = CharSet.Unicode)]
        private extern static int SerializeState(int id, StringBuilder ser_str);
        [DllImport(DLL_NAME, CharSet = CharSet.Unicode)]
        private extern static int DeserializeState(int id, string deser_str);
        [DllImport(DLL_NAME, CharSet = CharSet.Unicode)]
        private extern static int GetStateStatus(int id, ref int exeptr, ref int var_counts);
        [DllImport(DLL_NAME, CharSet = CharSet.Unicode)]
        private extern static int ReleaseSerializeStr(StringBuilder str);
        [DllImport(DLL_NAME, CharSet = CharSet.Unicode)]
        private extern static void GetLibVersion(StringBuilder str);

        private const int kSuccess = 0;
        private const int kNullResult = 1;
        private const int kErrorMsg = 2;

        public const string __return = "__return";

        public const int kRunBreak = 0;
        public const int kRunNext = 1;

        public enum VariableType : int
        {
            Null,
            Numeric,
            String,
            UserVarId
        }
        public struct Variable
        {
            public VariableType type;
            public float num;
            public string str;
            public int user_type_ptr;
        }



        private static Dictionary<int, Interpreter> interstates = new Dictionary<int, Interpreter>();

        private static string OnLoadFile(int id, string path)
        {
            return interstates[id].OnLoadFile(path);
        }
        private static int OnFuncall(int id, int userid, TokenGroup domain, TokenGroup path, TokenGroup param)
        {
            return interstates[id].OnFuncall(userid, domain, path, param);
        }



        static Interpreter()
        {
            Initialize(OnLoadFile, OnFuncall);
        }


        private int id;
        public int Id { get => this.id; }
        private Func<string, string> loadfile;

        //private Dictionary<Type, Dictionary<string, MethodInfo>> typeTable = new Dictionary<Type, Dictionary<string, MethodInfo>>();
        //public Interpreter AddType(Type type)
        //{
        //    if (this.typeTable.ContainsKey(type))
        //    {
        //        return this;
        //    }
        //    var methodTable = new Dictionary<string, MethodInfo>();
        //    this.typeTable.Add(type, methodTable);
        //    var methods = type.GetMethods();
        //    foreach (var item in methods)
        //    {
        //        methodTable.Add(item.Name, item);
        //        //方法别名
        //        if (Attribute.IsDefined(item, typeof(NickAttribute)))
        //        {
        //            var nick = (NickAttribute)Attribute.GetCustomAttribute(item, typeof(NickAttribute));
        //            methodTable.Add(nick.NickName, item);
        //        }
        //    }
        //    return this;
        //}

        private Dictionary<int, object> insts = new Dictionary<int, object>();
        private int instcount = 0;
        private int GetNewLocalVarId()
        {
            return ++this.instcount;
        }
        private void SetLocalVar(int userid, object o)
        {
            if (this.insts.ContainsKey(userid))
            {
                this.insts[userid] = o;
            }
            else
            {
                this.insts.Add(userid, o);
            }
        }
        private int AddLocalVar(object o)
        {
            ++this.instcount;
            this.insts.Add(this.instcount, o);
            return this.instcount;
        }
        private void DelLocalVar(int id)
        {
            if (this.insts.ContainsKey(id))
            {
                this.insts.Remove(id);
            }
        }

        private object GetLocalVar(int id)
        {
            object o = null;
            this.insts.TryGetValue(id, out o);
            return o;
        }
        public Variable GetVar(string name)
        {
            int type = 0;
            int r = GetVarType(this.id, name, ref type);
            if (r != kSuccess)
            {
                throw new InterpreterException(GetErrorMessage());
            }
            Variable v = new Variable();
            v.type = (VariableType)type;
            switch (v.type)
            {
                case VariableType.Null:
                    break;
                case VariableType.Numeric:
                    GetVarNumber(this.id, name, ref v.num);
                    break;
                case VariableType.String:
                    StringBuilder sb = new StringBuilder(1024);
                    GetVarString(this.id, name, sb);
                    v.str = sb.ToString();
                    break;
                case VariableType.UserVarId:
                    GetVarUser(this.id, name, ref v.user_type_ptr);
                    break;
                default:
                    break;
            }
            return v;
        }
        private void DelUserVar(string name)
        {
            int type = 0;
            int r = GetVarType(this.id, name, ref type);
            if (r == kSuccess)
            {
                if (type == (int)VariableType.UserVarId)
                {
                    int cuser_id = 0;
                    GetVarUser(this.id, name, ref cuser_id);
                    this.DelLocalVar(cuser_id);
                }
            }
        }
        public void SetVar(string name, float num)
        {
            DelUserVar(name);
            SetVarNum(this.id, name, num);
        }
        public void SetVar(string name, string str)
        {
            DelUserVar(name);
            SetVarString(this.id, name, str);
        }
        public void SetVar(string name, object obj)
        {
            int type = 0;
            int r = GetVarType(this.id, name, ref type);
            if (r == kNullResult)
            {
                //为空则新建
                int userid = this.AddLocalVar(obj);
                SetVarUser(this.id, name, userid);
            }
            else
            {
                //有该名字的变量，进行类型判断
                if (type == (int)VariableType.UserVarId)
                {
                    //如果是userid获取
                    int cuser_id = 0;
                    GetVarUser(this.id, name, ref cuser_id);
                    this.SetLocalVar(cuser_id, obj);
                    SetVarUser(this.id, name, cuser_id);
                }
                else
                {
                    //其他类型同名变量，获取一个变量id
                    int cu_id = this.GetNewLocalVarId();
                    this.SetLocalVar(cu_id, obj);
                    SetVarUser(this.id, name, cu_id);
                }
            }
        }

        public Interpreter(Func<string, string> loadfile)
        {
            int _id = 0;
            this.loadfile = loadfile;

            NewInterpreter(ref _id);
            this.id = _id;

            interstates.Add(_id, this);
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

        public void Serialize(Stream stream)
        {
            StringBuilder sb = new StringBuilder(1024);
            SerializeState(this.id, sb);
            string c_ser_str = sb.ToString();

            BinaryWriter bw = new BinaryWriter(stream);

            bw.Write(c_ser_str);
            bw.Write(this.insts.Count);
            foreach (var item in this.insts)
            {
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
                    MemoryStream ms = new MemoryStream();
                    mi.Invoke(item.Value, new object[] { ms });
                    var buf = ms.ToArray();
                    bw.Write(buf.Length);
                    bw.Write(buf);
                    ms.Close();
                }
            }
            bw.Flush();
        }
        public void Deserialize(Stream stream)
        {
            BinaryReader br = new BinaryReader(stream);
            DeserializeState(this.id, br.ReadString());

            int varCount = br.ReadInt32();
            for (int i = 0; i < varCount; i++)
            {
                //name, Type, value
                int user_id = br.ReadInt32();
                int buf_length = br.ReadInt32();

                MemoryStream ms = new MemoryStream();
                if (buf_length != 0)
                {
                    ms.Write(br.ReadBytes(buf_length), 0, buf_length);
                    ms.Position = 0;
                }

                MethodInfo mi = GetDeserializeMethodInfo(br.ReadString());
                object v = mi.Invoke(null, new object[] { ms });
                ms.Close();
                this.insts.Add(user_id, v);
            }
        }
        private string GetExceptionInfo(TokenInfo info, string content)
        {
            return string.Format("{0}, value: {1}, line: {2}, pos: {3}",
                content, info.value, info.line, info.position);
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
            TokenInfo methodNameToken = path.tokens[path.size - 1];
            string method = (path.tokens[path.size - 1].value);

            string[] paramstrs = new string[param.size];
            for (int i = 0; i < paramstrs.Length; i++)
            {
                paramstrs[i] = (param.tokens[i].value);
            }

            Type type = null;
            MethodInfo methodInfo = null;
            object inst = this.GetLocalVar(userid);


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
            //如果没有按方法名找到那就用别名找
            methodInfo = type.GetMethod(method);

            if (methodInfo == null)
            {
                //找别名
                var methodinfos = type.GetMethods();
                foreach (var item in methodinfos)
                {
                    if (Attribute.IsDefined(item, typeof(NickAttribute)))
                    {
                        NickAttribute nick = (NickAttribute)Attribute.GetCustomAttribute(item, typeof(NickAttribute));
                        if (nick.NickName == method)
                        {
                            //别名和方法名匹配
                            methodInfo = item;
                            break;
                        }
                    }
                }
                if (methodInfo == null)
                {
                    //别名查找还是为空则抛出异常
                    throw new InterpreterException(GetExceptionInfo(methodNameToken, "未找到方法"));
                }
            }

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
                Type retType = rst.GetType();
                if (retType == typeof(string))
                {
                    this.SetVar(__return, (string)rst);
                }
                else if (retType.IsPrimitive)
                {
                    this.SetVar(__return, Convert.ToSingle(rst));
                }
                else
                {
                    this.SetVar(__return, rst);
                }
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
        public int GetVariableCount()
        {
            int exeptr = 0;
            int var_count = 0;
            GetStateStatus(this.id, ref exeptr, ref var_count);
            return var_count;
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
            interstates.Remove(this.id);
            GC.SuppressFinalize(this);
        }
    }
}
