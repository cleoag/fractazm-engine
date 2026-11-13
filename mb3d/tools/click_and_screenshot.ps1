param(
    [string]$WindowTitle,
    [int]$ClickX = -1,
    [int]$ClickY = -1,
    [string]$OutputPath = "",
    [int]$WaitMs = 1000,
    [string]$ScreenshotTitle = ""
)

Add-Type -AssemblyName System.Windows.Forms
Add-Type -AssemblyName System.Drawing
Add-Type -ReferencedAssemblies @("System.Windows.Forms", "System.Drawing") -TypeDefinition @"
using System;
using System.Drawing;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Windows.Forms;

public class Win32Click {
    [DllImport("user32.dll")] public static extern bool SetForegroundWindow(IntPtr hWnd);
    [DllImport("user32.dll")] public static extern bool GetWindowRect(IntPtr hWnd, out RECT lpRect);
    [DllImport("user32.dll")] public static extern bool IsWindowVisible(IntPtr hWnd);
    [DllImport("user32.dll")] public static extern int GetWindowText(IntPtr hWnd, System.Text.StringBuilder s, int n);
    [DllImport("user32.dll")] public static extern bool EnumWindows(EnumWindowsProc cb, IntPtr lp);
    [DllImport("user32.dll")] public static extern bool SetCursorPos(int X, int Y);
    [DllImport("user32.dll")] public static extern void mouse_event(uint f, int dx, int dy, uint d, IntPtr e);
    public delegate bool EnumWindowsProc(IntPtr hWnd, IntPtr lParam);
    [StructLayout(LayoutKind.Sequential)]
    public struct RECT { public int Left, Top, Right, Bottom; }

    public static List<KeyValuePair<IntPtr,string>> AllWindows = new List<KeyValuePair<IntPtr,string>>();

    public static void CollectWindows() {
        AllWindows.Clear();
        EnumWindows(delegate(IntPtr hWnd, IntPtr lParam) {
            if (IsWindowVisible(hWnd)) {
                var sb = new System.Text.StringBuilder(256);
                GetWindowText(hWnd, sb, 256);
                string t = sb.ToString();
                if (t.Length > 0) AllWindows.Add(new KeyValuePair<IntPtr,string>(hWnd, t));
            }
            return true;
        }, IntPtr.Zero);
    }

    public static void ClickAt(int x, int y) {
        SetCursorPos(x, y);
        System.Threading.Thread.Sleep(50);
        mouse_event(0x0002, 0, 0, 0, IntPtr.Zero);
        System.Threading.Thread.Sleep(50);
        mouse_event(0x0004, 0, 0, 0, IntPtr.Zero);
    }

    public static void Screenshot(IntPtr hwnd, string path) {
        RECT rect; GetWindowRect(hwnd, out rect);
        int w = rect.Right - rect.Left, h = rect.Bottom - rect.Top;
        if (w <= 0 || h <= 0) return;
        var bmp = new Bitmap(w, h);
        var g = Graphics.FromImage(bmp);
        g.CopyFromScreen(rect.Left, rect.Top, 0, 0, new Size(w, h));
        g.Dispose();
        bmp.Save(path, System.Drawing.Imaging.ImageFormat.Png);
        bmp.Dispose();
    }
}
"@

function Find-Window($pattern) {
    [Win32Click]::CollectWindows()
    foreach ($kv in [Win32Click]::AllWindows) {
        if ($kv.Value -like $pattern) {
            return @{ Handle = $kv.Key; Title = $kv.Value }
        }
    }
    return $null
}

# Find main window
$win = Find-Window $WindowTitle
if (-not $win) {
    Write-Host "Window not found: $WindowTitle"
    Write-Host "Available windows:"
    [Win32Click]::CollectWindows()
    foreach ($kv in [Win32Click]::AllWindows) { Write-Host "  - $($kv.Value)" }
    exit 1
}
Write-Host "Found: $($win.Title)"
[Win32Click]::SetForegroundWindow($win.Handle) | Out-Null
Start-Sleep -Milliseconds 200

$rect = New-Object Win32Click+RECT
[Win32Click]::GetWindowRect($win.Handle, [ref]$rect) | Out-Null
Write-Host "Window at: $($rect.Left),$($rect.Top) to $($rect.Right),$($rect.Bottom)"

# Click if coordinates given
if ($ClickX -ge 0 -and $ClickY -ge 0) {
    $absX = $rect.Left + $ClickX
    $absY = $rect.Top + $ClickY
    Write-Host "Clicking at ($ClickX,$ClickY) -> screen ($absX,$absY)"
    [Win32Click]::ClickAt($absX, $absY)
    Start-Sleep -Milliseconds $WaitMs
}

# Screenshot
if ($OutputPath -ne "") {
    $shotTitle = if ($ScreenshotTitle -ne "") { $ScreenshotTitle } else { $WindowTitle }
    $shotWin = Find-Window $shotTitle
    if (-not $shotWin) {
        Write-Host "Screenshot window not found: $shotTitle"
        Write-Host "Available windows:"
        foreach ($kv in [Win32Click]::AllWindows) { Write-Host "  - $($kv.Value)" }
        exit 1
    }
    Write-Host "Screenshotting: $($shotWin.Title)"
    [Win32Click]::SetForegroundWindow($shotWin.Handle) | Out-Null
    Start-Sleep -Milliseconds 300
    [Win32Click]::Screenshot($shotWin.Handle, $OutputPath)
    Write-Host "Screenshot saved to $OutputPath"
}
