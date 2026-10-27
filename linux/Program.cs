using System.Diagnostics;
using System.Runtime.CompilerServices;
using Color = System.Drawing.Color;
using SDL3;
using SmashFramework;
using KatziDrip;
using System.Numerics;
using System.Runtime.InteropServices;
using System.Text;

class Program
{
    public static async Task Main(string[] args)
    {

        Drip.Load(Drip.DefaultThemeName);
        Smash.Init();

        Window window = Window.Create("SmashFramework example", 800, 600, SDL.WindowFlags.Resizable);
        Renderer renderer = new(window);

        renderer.SetVSyncEnabled(true);
        window.StartPollingTextInput();

        AssetManager.LoadFont(SystemFontResolver.ResolveByName("Iosevka-Regular") ?? throw new Exception("You anal fucker have not installed Iosevka the best Font on earth!"), renderer);

        await using Communication communication = new();
        var serverTask = communication.InitAsync();

        var process = Process.Start(new ProcessStartInfo
        {
            FileName = "./webview",
            ArgumentList = { "/tmp/webview.sock" },
            UseShellExecute = false
        });

        if (process == null)
            throw new Exception("Failed to start webview");


        int FramesPerSecond = 120;
        double _targetFrameTime = 1000.0 / FramesPerSecond;

        ulong lastTime = SDL.GetPerformanceCounter();

        string urlbar = "http://kur.katzi.org";
        string TextInput;

        Button btn_close = new Button(-35, 0, 35, 35, true);

        bool running = true;
        while (running)
        {
            SDL.GetMouseState(out float mx, out float my);
            btn_close.Update(mx, my, window.Width);

            ulong currentTime = SDL.GetPerformanceCounter();
            double deltaTime = (double)(currentTime - lastTime) / SDL.GetPerformanceFrequency();
            lastTime = currentTime;

            while (communication.TryPollMessage(out var msg))
            {
                string text = Encoding.UTF8.GetString(msg!.Payload);
                Console.WriteLine($"Type: {msg!.Type}; Text: {text}");
            }

            while (SDL.PollEvent(out SDL.Event e))
            {
                if (e.Type == (uint)SDL.EventType.Quit)
                {
                    running = false;
                }

                if (e.Type == (uint)SDL.EventType.MouseButtonDown &&
                    e.Button.Button == SDL.ButtonLeft &&
                    btn_close.Hovered)
                {
                    running = false;
                }

                if (e.Type == (uint)SDL.EventType.TextInput)
                {
                    TextInput = Marshal.PtrToStringUTF8(e.Text.Text) ?? "#";
                    urlbar += TextInput;
                }

                if (e.Type == (uint)SDL.EventType.KeyDown)
                {
                    switch (e.Key.Key)
                    {
                        case SDL.Keycode.Backspace:
                            if (urlbar.Length > 0)
                            {
                                urlbar = urlbar[0..(urlbar.Length - 1)];
                            }
                            break;
                        case SDL.Keycode.Return:
                            urlbar = UrlManager.Beautifier(urlbar);

                            Console.WriteLine($"New Url {urlbar}");

                            await UrlManager.Parse(communication,urlbar);
                            break;
                        default:
                            break;
                    }
                }
            }

            renderer.Clear(Color.Black);

            float menuBGHeight = 36;

            Rectangle menuBG = new(0, 0, window.Width, menuBGHeight);
            renderer.RenderFilledRectangle(menuBG, Drip.BaseColor.ToColor()); //49, 44, 54
            Rectangle menuBGBorderButtom = new(0, menuBGHeight, window.Width, 1);
            renderer.RenderFilledRectangle(menuBGBorderButtom, Drip.VeryLightColor.ToColor()); //77, 70, 84

            if (btn_close.Hovered)
                Draw.Circle(renderer, window.Width - 18, 18, 12, 12, 12, Drip.VeryLightColor.ToColor());

            renderer.RenderString(AssetManager.Get<Font>("Iosevka-Regular"), 22, urlbar, new(5, 4), Color.WhiteSmoke);

            renderer.RenderPresent();

            int sleepTime = (int)(_targetFrameTime - deltaTime);
            if (sleepTime > 0)
            {
                Thread.Sleep(sleepTime);
            }
        }

        Smash.Quit();

    }
}