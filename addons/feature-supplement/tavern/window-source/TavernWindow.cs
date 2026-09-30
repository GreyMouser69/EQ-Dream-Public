using System;
using System.Diagnostics;
using System.Drawing;
using System.IO;
using System.Runtime.InteropServices;
using System.Threading.Tasks;
using System.Windows.Forms;
using Microsoft.Web.WebView2.Core;
using Microsoft.Web.WebView2.WinForms;

// A separate WebView2 process, parented to the TEST client. No EQ DLL patch or injection.
public sealed class TavernWindow : Form {
    [DllImport("user32.dll")] static extern IntPtr SetParent(IntPtr child, IntPtr parent);
    [DllImport("user32.dll")] static extern IntPtr GetParent(IntPtr child);
    [DllImport("user32.dll")] static extern bool IsWindow(IntPtr window);
    [DllImport("user32.dll")] static extern bool GetClientRect(IntPtr window, out Rect rect);
    [DllImport("user32.dll")] static extern IntPtr GetWindowDpiAwarenessContext(IntPtr window);
    [DllImport("user32.dll")] static extern IntPtr SetThreadDpiAwarenessContext(IntPtr context);
    [StructLayout(LayoutKind.Sequential)] struct Rect { public int Left, Top, Right, Bottom; }
    readonly IntPtr eq;
    readonly string url, dataDir;
    readonly WebView2 web = new WebView2();
    readonly Timer watch = new Timer();
    readonly Label status = new Label();
    internal TavernWindow(IntPtr parent, string address, string directory, bool live = false) {
        eq = parent; url = address; dataDir = directory;
        Text = live ? "EQ Dream | Tavern Duels" : "EQ Dream | Tavern Duels (test)";
        TopLevel = false; FormBorderStyle = FormBorderStyle.Sizable;
        MinimizeBox = false; MaximizeBox = false; ShowInTaskbar = false;
        BackColor = Color.FromArgb(16,23,22); MinimumSize = new Size(640,420);
        web.Dock = DockStyle.Fill; Controls.Add(web);
        status.Text = "Opening Tavern Duels..."; status.ForeColor = Color.White;
        status.BackColor = BackColor; status.Dock = DockStyle.Top; status.Height = 25;
        Controls.Add(status);
        Rect area; GetClientRect(eq, out area);
        Size = new Size(Math.Max(640,Math.Min(1400,area.Right-40)),Math.Max(420,area.Bottom-60));
        Location = new Point(20,20);
        SetParent(Handle,eq);
        if(GetParent(Handle)!=eq)throw new InvalidOperationException("Could not attach the card window to the test client.");
        watch.Interval = 500;
        watch.Tick += delegate { if(!IsWindow(eq))Close(); };
        watch.Start();
        Shown += async delegate { await OpenGame(); };
        FormClosed += delegate { watch.Stop(); web.Dispose(); Application.ExitThread(); };
    }
    async Task OpenGame() {
        try {
            var environment = await CoreWebView2Environment.CreateAsync(null,dataDir);
            await web.EnsureCoreWebView2Async(environment);
            web.CoreWebView2.Settings.AreDevToolsEnabled = false;
            web.CoreWebView2.Settings.AreDefaultContextMenusEnabled = false;
            web.CoreWebView2.Settings.IsStatusBarEnabled = false;
            var allowed = new Uri(url).GetLeftPart(UriPartial.Authority);
            web.CoreWebView2.NavigationStarting += delegate(object sender, CoreWebView2NavigationStartingEventArgs e) {
                Uri next;
                if(!Uri.TryCreate(e.Uri,UriKind.Absolute,out next)||next.GetLeftPart(UriPartial.Authority)!=allowed)e.Cancel = true;
            };
            web.CoreWebView2.NewWindowRequested += delegate(object sender, CoreWebView2NewWindowRequestedEventArgs e) { e.Handled = true; };
            web.CoreWebView2.DownloadStarting += delegate(object sender, CoreWebView2DownloadStartingEventArgs e) { e.Cancel = true; };
            web.CoreWebView2.PermissionRequested += delegate(object sender, CoreWebView2PermissionRequestedEventArgs e) { e.State = CoreWebView2PermissionState.Deny; };
            web.CoreWebView2.NavigationCompleted += delegate(object sender, CoreWebView2NavigationCompletedEventArgs e) {
                status.Visible = !e.IsSuccess;
                if(!e.IsSuccess)status.Text = "Unable to connect. Close this window and use /tavern again.";
            };
            web.CoreWebView2.Navigate(url);
        } catch(Exception e) { status.Text = "Tavern Duels could not open: "+e.Message; Console.Error.WriteLine(e.Message); }
    }
    [STAThread] public static int Main(string[] args) {
        try {
            if(args.Length!=4)throw new ArgumentException("Expected test EQ PID, client directory, loopback URL and browser data directory.");
            int id; if(!int.TryParse(args[0],out id))throw new ArgumentException("Invalid EQ process.");
            var process = Process.GetProcessById(id);
            string expected = Path.GetFullPath(Path.Combine(args[1],"eqgame.exe"));
            if(!String.Equals(new DirectoryInfo(args[1]).Name,"NMS-Local",StringComparison.OrdinalIgnoreCase)||
               !String.Equals(process.MainModule.FileName,expected,StringComparison.OrdinalIgnoreCase))
                throw new InvalidOperationException("Tavern Duels test mode only supports the NMS-Local test client.");
            var parent = process.MainWindowHandle;
            if(parent==IntPtr.Zero)throw new InvalidOperationException("The test EQ window is not ready.");
            var address = new Uri(args[2]);
            if(address.Scheme!="http"||address.Host!="127.0.0.1"||address.AbsolutePath!="/bootstrap")throw new ArgumentException("Only the local test service is allowed.");
            SetThreadDpiAwarenessContext(GetWindowDpiAwarenessContext(parent));
            Application.EnableVisualStyles();
            var window = new TavernWindow(parent,args[2],args[3]);
            Task.Run(delegate {
                try { while(Console.ReadLine()!=null) { if(!window.IsDisposed)window.BeginInvoke(new Action(window.Close)); return; } }
                catch { }
                if(!window.IsDisposed)try { window.BeginInvoke(new Action(window.Close)); } catch { }
            });
            window.Show();
            Application.Run();
            return 0;
        } catch(Exception e) { Console.Error.WriteLine(e.Message); return 1; }
    }
}
