using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Xml;

namespace Update_XML
{
    class Program
    {
        static void Main(string[] args)
        {
            // Get new names.
            var doc = new XmlDocument();
            doc.Load(@"D:\Magic_Items.xml");
            var name_things = doc.GetElementsByTagName("thing");
            var names = new List<string>();
            foreach (XmlNode thing in name_things) {
                names.Add(thing.Attributes["name"].InnerText);
            }

            // Get the dat files.
            var dat_files = Directory.GetFiles(@"C:\ProgramData\Hero Lab\data\pathfndr2\", "thing_treasure_magic_items*");
            
            // Update each file.
            foreach (var dat_file in dat_files) {
                var dat_xml = new XmlDocument();
                dat_xml.Load(dat_file);
                var dat_things = dat_xml.GetElementsByTagName("thing");
                foreach (XmlNode thing in dat_things) {
                    var name = names.FirstOrDefault(n => string.Equals(n, thing.Attributes["name"].InnerText, StringComparison.InvariantCultureIgnoreCase));
                    if (name != null) {
                        thing.Attributes["name"].InnerText = name;
                        names.Remove(name);
                    }
                }

                dat_xml.Save(dat_file);

                var contents = File.ReadAllText(dat_file);
                contents = contents.Replace(@" />", @"/>");
                File.WriteAllText(dat_file, contents);
            }
        }
    }
}
