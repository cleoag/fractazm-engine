param(
    [string]$WindowTitle = "*Mandelbulb*",
    [string]$OutputPath = "C:\work\mb3d\screenshots\fpc_screenshot.png"
)

Add-Type -AssemblyName System.Windows.Forms
Add-Type -AssemblyName System.Drawing
Add-Type @"
using System;
using System.Runtime.InteropServices;
public class Win32 {
    [DllImport("user32.dll")]
    public static extern IntPtr FindWindow(string className, string windowName);
    [DllImport("user32.dll")]
    public static extern bool SetForegroundWindow(IntPtr hWnd);
    [DllImport("user32.dll")]
    [return: MarshalAs(UnmanagedType.Bool)]
    public static extern bool GetWindowRect(IntPtr hWnd, out RECT lpRect);
    [DllImport("user32.dll", SetLastError = true)]
    public static extern IntPtr FindWindowEx(IntPtr hwndParent, IntPtr hwndChildAfter, string lpszClass, string lpszWindow);
    [DllImport("user32.dll")]
    public static extern bool EnumWindows(EnumWindowsProc lpEnumFunc, IntPtr lParam);
    [DllImport("user32.dll")]
    public static extern int GetWindowText(IntPtr hWnd, System.Text.StringBuilder lpString, int nMaxCount);
    [DllImport("user32.dll")]
    public static extern bool IsWindowVisible(IntPtr hWnd);
    public delegate bool EnumWindowsProc(IntPtr hWnd, IntPtr lParam);
    [StructLayout(LayoutKind.Sequential)]
    public struct RECT {
        public int Left, Top, Right, Bottom;
    }
}
"@

# Find all visible windows with matching title
$windows = @()
$callback = [Win32+EnumWindowsProc]{
    param($hWnd, $lParam)
    if ([Win32]::IsWindowVisible($hWnd)) {
        $sb = New-Object System.Text.StringBuilder 256
        [Win32]::GetWindowText($hWnd, $sb, 256) | Out-Null
        $title = $sb.ToString()
        if ($title -like $WindowTitle) {
            $script:windows += @{ Handle = $hWnd; Title = $title }
        }
    }
    return $true
}
[Win32]::EnumWindows($callback, [IntPtr]::Zero) | Out-Null

if ($windows.Count -eq 0) {
    Write-Host "No windows found matching '$WindowTitle'"
    # List all visible windows for debugging
    $allWindows = @()
    $cb2 = [Win32+EnumWindowsProc]{
        param($hWnd, $lParam)
        if ([Win32]::IsWindowVisible($hWnd)) {
            $sb = New-Object System.Text.StringBuilder 256
            [Win32]::GetWindowText($hWnd, $sb, 256) | Out-Null
            $title = $sb.ToString()
            if ($title.Length -gt 0) {
                $script:allWindows += $title
            }
        }
        return $true
    }
    [Win32]::EnumWindows($cb2, [IntPtr]::Zero) | Out-Null
    Write-Host "Visible windows:"
    $allWindows | ForEach-Object { Write-Host "  - $_" }
    exit 1
}

foreach ($w in $windows) {
    Write-Host "Found: $($w.Title)"
}

$hwnd = $windows[0].Handle
[Win32]::SetForegroundWindow($hwnd) | Out-Null
Start-Sleep -Milliseconds 200

$rect = New-Object Win32+RECT
[Win32]::GetWindowRect($hwnd, [ref]$rect) | Out-Null

$width = $rect.Right - $rect.Left
$height = $rect.Bottom - $rect.Top

Write-Host "Window rect: $($rect.Left),$($rect.Top) ${width}x${height}"

$bmp = New-Object System.Drawing.Bitmap($width, $height)
$g = [System.Drawing.Graphics]::FromImage($bmp)
$g.CopyFromScreen($rect.Left, $rect.Top, 0, 0, (New-Object System.Drawing.Size($width, $height)))
$g.Dispose()
$bmp.Save($OutputPath, [System.Drawing.Imaging.ImageFormat]::Png)
$bmp.Dispose()
Write-Host "Screenshot saved to $OutputPath"
