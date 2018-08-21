using System;
using System.Collections.Generic;
using System.Drawing;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Xfinium.Pdf;
using Xfinium.Pdf.Content;
//using Xfinium.Pdf.Rendering;

namespace Splicer
{
    class Program
    {
        static void Main(string[] args)
        {
            PdfFixedDocument doc = new PdfFixedDocument(File.OpenRead(@"D:\LWD\PDFCarver\pathfinder2e\rules\Core_01_Overview.pdf"));
            PdfFixedDocument new_doc = new PdfFixedDocument();
            foreach (var page in doc.Pages) {
                new_doc.Pages.Add(page);
            }
            new_doc.Save(File.OpenWrite(@"D:\LWD\PDFCarver\pathfinder2e\rules\Core_01_Overview_b.pdf"));

            //PdfPageRenderer renderer = new PdfPageRenderer(doc.Pages[16]);
            //renderer.ConvertPageToImage(192, File.OpenWrite(@"D:\LWD\PDFCarver\pathfinder2e\rules\page.png"), PdfPageImageFormat.Png);
            Console.WriteLine("DONE");
        }
    }
}
