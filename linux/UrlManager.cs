using System;
using System.Text;
using System.Text.RegularExpressions;

class UrlManager
{
    public static string Beautifier(string url)
    {
        if (Regex.IsMatch(url, @"^[a-zA-Z][a-zA-Z0-9+.-]*://"))
            return url;

        if (url.StartsWith("/"))
            return $"file://{url}";

        return $"http://{url}";
    }

    public static async Task Parse(Communication communication,string url)
    {
        if (url.StartsWith("http://") || url.StartsWith("https://"))
        {

            byte[] byteArray;

            unsafe
            {
                Console.WriteLine("Befor Call");
                byte *ptr = Kurl.Katzi_Get(url, out int length);
                Console.WriteLine("After Call");

                if (ptr == null)
                {
                    throw new Exception("NULL Pointer");
                }
                else
                {
                    byteArray = new Span<byte>(ptr,length).ToArray();
                }
            }
            
            await communication.BroadcastAsync(1, byteArray);
        }

        if (url.StartsWith("file://"))
        {
            
        }
    }
}