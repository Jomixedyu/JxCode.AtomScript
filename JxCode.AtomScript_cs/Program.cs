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

    public void Serialize(Stream stream)
    {
        Console.WriteLine("Serialize");
    }
    public void Deserialize(Stream stream)
    {
        Console.WriteLine("Deserialize");
    }
}

namespace JxCode.AtomScript_cs
{
    class Program
    {
        public static void print(string name)
        {
            Console.WriteLine("print: " + name);
        }
        static void Main(string[] args)
        {
            Console.ReadKey();
            Interpreter inter = new Interpreter(f => File.ReadAllText(@"C:\Users\Jayshonyves\Desktop\" + f + ".txt"));
            inter.ExecuteProgram("eazytest");
            inter.Next();
            Console.ReadKey();
        }
    }
}
