using System;
using System.IO;
using JxCode.AtomScript;

public class role
{
    public string name;

    public static void show(string text)
    {
        Console.WriteLine("show text: " + text);
    }

    public void print(Interpreter inter, ref bool isNext, string text)
    {
        Console.WriteLine(this.name + ":  " + text);
        isNext = false;
    }

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

namespace JxCode.AtomScript_cs
{
    class Program
    {
        public static void print(string name)
        {
            Console.WriteLine("print: " + name);
        }
        static void RunInterpreter()
        {
            Interpreter inter = new Interpreter(f => File.ReadAllText(@"C:\Users\Jayshonyves\Desktop\" + f + ".txt"));
            inter.ExecuteProgram("eazytest");
            inter.Next();
            var fs = File.OpenWrite("ser.dat");
            inter.Serialize(fs);
            fs.Flush();
            fs.Close();

            var deser_fs = File.OpenRead("ser.dat");
            inter.Deserialize(deser_fs);
            deser_fs.Close();
            inter.Next();
        }
        static void Main(string[] args)
        {
            RunInterpreter();

            Console.ReadKey();
        }
    }
}
