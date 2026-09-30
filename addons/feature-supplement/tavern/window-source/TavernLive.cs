using System;
using System.Diagnostics;
using System.IO;
using System.Runtime.InteropServices;
using System.Text.RegularExpressions;
using System.Web.Script.Serialization;
using System.Windows.Forms;
public sealed class TavernLive {
    public sealed class Request { public int pid; public string character; public string client; public string token; public string action; }
    [DllImport("user32.dll")] static extern IntPtr GetWindowDpiAwarenessContext(IntPtr window);
    [DllImport("user32.dll")] static extern IntPtr SetThreadDpiAwarenessContext(IntPtr context);
    [STAThread] public static int Main(string[] args) {
        string marker = null;
        try {
            if(args.Length!=1)throw new ArgumentException("Use /tavern from MacroQuest.");
            int requestedPid;
            if(!int.TryParse(args[0],out requestedPid)||requestedPid<=0)throw new ArgumentException("Invalid EQ process.");
            string config=Path.GetFullPath(Path.Combine(AppDomain.CurrentDomain.BaseDirectory,"..","config","tavern-live"));
            string file=Path.Combine(config,"request-"+requestedPid+".json");
            if(!file.StartsWith(config+Path.DirectorySeparatorChar,StringComparison.OrdinalIgnoreCase))throw new ArgumentException("Invalid request location.");
            var json=new JavaScriptSerializer();
            var request=json.Deserialize<Request>(File.ReadAllText(file));
            if(request.pid<=0||Path.GetFileName(file)!="request-"+request.pid+".json"||request.action!="open"||request.token==null||!Regex.IsMatch(request.token,"^[a-f0-9]{64}$"))throw new ArgumentException("Invalid launch request.");
            var process=Process.GetProcessById(request.pid);
            if(!String.Equals(process.MainModule.FileName,Path.GetFullPath(Path.Combine(request.client,"eqgame.exe")),StringComparison.OrdinalIgnoreCase))throw new ArgumentException("EQ process changed.");
            if(request.character==null||!Regex.IsMatch(request.character,"^[A-Za-z][A-Za-z0-9_]{2,31}$"))throw new ArgumentException("Invalid character.");
            IntPtr parent=process.MainWindowHandle;
            if(parent==IntPtr.Zero)throw new InvalidOperationException("EQ window is unavailable.");
            SetThreadDpiAwarenessContext(GetWindowDpiAwarenessContext(parent));
            Application.EnableVisualStyles();
            string data=Path.Combine(config,"browser",request.character.ToLowerInvariant());
            string origin=File.ReadAllText(Path.Combine(AppDomain.CurrentDomain.BaseDirectory,"server-url.txt")).Trim().TrimEnd('/');
            Uri endpoint;
            if(!Uri.TryCreate(origin,UriKind.Absolute,out endpoint)||endpoint.Scheme!="https"||!String.IsNullOrEmpty(endpoint.UserInfo)||endpoint.AbsolutePath!="/"||!String.IsNullOrEmpty(endpoint.Query)||!String.IsNullOrEmpty(endpoint.Fragment))throw new ArgumentException("Set server-url.txt to your server HTTPS origin, e.g. https://eq.example.org");
            var window=new TavernWindow(parent,origin+"/tavern/bootstrap?ticket="+request.token,data,true);
            marker=Path.Combine(config,"open-"+request.pid);
            File.WriteAllText(marker,request.token);
            var watch=new Timer();watch.Interval=250;
            watch.Tick+=delegate {
                try {var next=json.Deserialize<Request>(File.ReadAllText(file));if(next.action=="close"||next.token!=request.token)window.Close();}
                catch { }
            };
            window.FormClosed+=delegate {watch.Stop();try{if(File.Exists(marker)&&File.ReadAllText(marker)==request.token)File.Delete(marker);}catch{}};
            watch.Start();window.Show();Application.Run();return 0;
        } catch(Exception e) {
            MessageBox.Show(e.Message,"Tavern Duels could not open");return 1;
        }
    }
}
