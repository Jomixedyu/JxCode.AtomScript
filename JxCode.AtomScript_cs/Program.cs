using System;
using System.IO;
using JxCode.AtomLang;

public class role
{
    public string name;

    public void print(string text)
    {
        Console.WriteLine(this.name + ":  " + text);
    }
    public void Invoke(string text)
    {
        print(text);
    }
    public static role create(string str)
    {
        return new role() { name = str };
    }
}

namespace JxCode.AtomScript_cs
{
    class Program
    {
        public static bool Print(string str)
        {
            Console.WriteLine(str);
            return true;
        }
        public static string load(string path)
        {
            return System.IO.File.ReadAllText(@"C:\Users\Jayshonyves\Desktop\" + path);
        }
        static void Main(string[] args)
        {
            Console.ReadKey();
            Interpreter inter = new Interpreter(load);

            inter.ExecuteCode("jumpfile \"def.txt\"");
            inter.Next();
            //MemoryStream ms = new MemoryStream();


            //Console.WriteLine(inter.GetVariableCount());

            //inter.Serialize(ms);

            //Interpreter inter2 = new Interpreter(load);

            //ms.Position = 0;

            //inter2.Deserialize(ms);
            //Console.WriteLine(inter2.GetVariableCount());

            Console.WriteLine("end");
            Console.ReadKey();
        }
    }
}
