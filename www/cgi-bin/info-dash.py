#!/usr/bin/env python3

import time
import os

print("Content-Type: text/html\r\n\r\n")

print("""<!DOCTYPE html>
<html lang="en">
  <head>
    <meta charset="UTF-8" />
    <meta name="viewport" content="width=device-width, initial-scale=1.0" />
    <title>SpySocket - System Info Dashboard</title>
    <link rel="stylesheet" href="../style.css" />
    <link rel="preconnect" href="https://fonts.googleapis.com" />
    <link rel="preconnect" href="https://fonts.gstatic.com" crossorigin />
    <link href="https://fonts.googleapis.com/css2?family=Roboto+Mono:ital,wght@0,100..700;1,100..700&display=swap"
        rel="stylesheet" />
    <link rel="preconnect" href="https://fonts.googleapis.com" />
    <link rel="preconnect" href="https://fonts.gstatic.com" crossorigin />
    <link
        href="https://fonts.googleapis.com/css2?family=Comfortaa:wght@300..700&family=M+PLUS+Rounded+1c:wght@400;900&family=Roboto+Mono:ital,wght@0,100..700;1,100..700&display=swap"
        rel="stylesheet" />
    <meta http-equiv="refresh" content="10">
    <style>
        .info-section {
            position: relative;
            font-size: 18px;
            # margin-bottom: 25px; 
            color: hsl(154 84% 70%);
            font-family: 'Courier New', monospace;
            text-align: left;
            text-shadow: 0 0 8px hsl(154 84% 70%), 0 0 12px hsl(154 84% 70%);
            font-weight: bold;
            padding: 10px;
            top : 170px;
            margin-left: 100px;
        }
        .info-title {
            font-size: 18px;
            margin-bottom: 10px;
            color: hsl(154 84% 70%);
        }
        .refresh-indicator {
            position: absolute;
            top: 20px;
            right: 20px;
            color: hsl(154 84% 70%);
            font-size: 12px;
        }
        .fade-in {
            animation: fadeIn 0.5s ease-in;
        }
        @keyframes fadeIn {
            from { opacity: 0; }
            to { opacity: 1; }
        }
        .Back-text-crt {
        font-size: 25px;
        font-weight: 500;
        color: hsl(154 84% 70%);
        font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, Oxygen,
            Ubuntu, Cantarell, sans-serif;
        }

        .back-button-crt {
        display: flex;
        align-items: center;
        gap: 8px;
        cursor: pointer;
        position: absolute;
        top: 47px;
        left: 51px;
        z-index: 100;
        }
        .crt {
        background: hsl(154 50% 5%);
        color: hsl(154 84% 70%);
        text-shadow: 0 0 4px hsl(154 84% 70%);
        font-family: monospace;
        font-size: 16px;
        }
        @keyframes blink {
        0% {
            opacity: 0;
        }
        30% {
            opacity: 1;
        }
        70% {
            opacity: 1;
        }
        100% {
            opacity: 0;
        }
        }

        .blink {
        animation: blink 0.4s linear infinite;
        position: relative;
        top:200px;
        left:400px;
        }
        /* low-resolution screen overlay interlacing */
        @keyframes lines {
        0% {
            background-position: 0px 0px;
        }
        50% {
            background-position: 0px 0px;
        }
        51% {
            background-position: 0px 2px;
        }
        100% {
            background-position: 0px 2px;
        }
        }

        #interlaced {
        position: fixed;
        background: repeating-linear-gradient(
            transparent 0px 1px,
            hsl(154 0% 0%/0.3) 3px 4px
        );
        top: 0;
        left: 0;
        right: 0;
        bottom: 0;
        z-index: 1;
        pointer-events: none;
        animation: lines 0.066666666s linear infinite;
        }

        /* The curvy screen glare effect */
        #glare {
        position: fixed;
        top: 0;
        left: 0;
        right: 0;
        bottom: 0;
        z-index: -1; /* ensure the effect doesn't cover the text */
        background: radial-gradient(hsl(154 5% 15%) 0%, hsl(154 50% 5%) 70%);
        }

        .crt-text {
        /* retro stuff */
        color: hsl(154 84% 70%);
        font-family: "Courier New", monospace;
        font-size: 40px;
        text-align: center;
        text-shadow: 0 0 10px hsl(154 84% 70%);

        /* flex stuff */
        display: flex;
        flex-direction: column;
        align-items: center;
        position: relative;
        top: 120px;
        }
    </style>
    <script>
        let countdown = 10;
        function updateCountdown() {
            const indicator = document.querySelector('.refresh-indicator');
            if (indicator) {
                indicator.textContent = `Auto-refresh: ${countdown}s`;
                countdown--;
                if (countdown < 0) countdown = 10;
            }
        }
        window.onload = function() {
            document.body.classList.add('fade-in');
            setInterval(updateCountdown, 1000);
        }
    </script>
</head>
  <body class="crt">
    <header>
        <div class="back-button-crt" onclick="window.location.href='../main-page.html'">
            <span class="Back-text-crt">&lt;Back</span>
        </div>  
    </header>
    <h1 class="crt-text" style="color: hsl(154 84% 70%);">System Information</h1>
    <div class="info-container">
""")

# Current time
print("<div class='info-section'>Current Time: <span style='color: #40E0D0;'>{}</span></div>".format(time.ctime()))
try:
    hostname = socket.gethostname()
    username = os.getenv("USER") or os.getenv("LOGNAME") or "unknown"
    print("<div class='info-section'>Hostname: <span style='color: #40E0D0;'>{}</span> | User: <span style='color: #40E0D0;'>{}</span></div>".format(hostname, username))
except Exception:
    print("<div class='info-section'>Host/User: <span style='color: #FF6B6B;'>(unavailable)</span></div>")



# Kernel version
try:
    kernel = os.uname().release
    print("<div class='info-section'>Kernel Version: <span style='color: #40E0D0;'>{}</span></div>".format(kernel))
except Exception:
    print("<div class='info-section'>Kernel Version: <span style='color: #FF6B6B;'>(unavailable)</span></div>")

# OS Release (/etc/os-release)
try:
    os_release = {}
    with open("/etc/os-release", "r") as f:
        for line in f:
            if "=" in line:
                key, val = line.strip().split("=", 1)
                os_release[key] = val.strip('"')
    distro = os_release.get("PRETTY_NAME", "unknown")
    print("<div class='info-section'>OS: <span style='color: #40E0D0;'>{}</span></div>".format(distro))
except Exception:
    print("<div class='info-section'>OS: <span style='color: #FF6B6B;'>(unavailable)</span></div>")



# Uptime (/proc/uptime)
try:
    with open("/proc/uptime", "r") as f:
        uptime_seconds = float(f.readline().split()[0])
        days = int(uptime_seconds // 86400)
        hours = int((uptime_seconds // 3600) % 24)
        minutes = int((uptime_seconds // 60) % 60)
        seconds = int(uptime_seconds % 60)
        print("<div class='info-section'>System Uptime: <span style='color: #40E0D0;'>{}d {}h {}m {}s</span></div>".format(
            days, hours, minutes, seconds))
except Exception:
    print("<div class='info-section'>System Uptime: <span style='color: #FF6B6B;'>(unavailable)</span></div>")

# Load average (/proc/loadavg)
try:
    with open("/proc/loadavg", "r") as f:
        l1, l5, l15 = f.readline().split()[:3]
        print("<div class='info-section'>Load Average: <span style='color: #40E0D0;'>{} (1m), {} (5m), {} (15m)</span></div>".format(
            l1, l5, l15))
except Exception:
    print("<div class='info-section'>Load Average: <span style='color: #FF6B6B;'>(unavailable)</span></div>")

# CPU count
cpu_count = os.cpu_count()
print("<div class='info-section'>CPU Cores: <span style='color: #40E0D0;'>{}</span></div>".format(cpu_count if cpu_count else "(unknown)"))

# Memory usage (/proc/meminfo)
try:
    meminfo = {}
    with open("/proc/meminfo", "r") as f:
        for line in f:
            parts = line.split(":")
            if len(parts) == 2:
                key, value = parts
                meminfo[key.strip()] = value.strip()
    mem_total = meminfo.get("MemTotal", "unknown")
    mem_free = meminfo.get("MemFree", "unknown")
    mem_available = meminfo.get("MemAvailable", "unknown")
    print("<div class='info-section'>")
    print("  Total:     {}".format(mem_total))
    print("  Free:      {}".format(mem_free))
    print("  Available: {}".format(mem_available))
    print("</div>")
except Exception:
    print("<div class='info-section'>Memory: <span style='color: #FF6B6B;'>(unavailable)</span></div>")
# Disk usage
try:
    df_output = subprocess.check_output(["df", "-h", "/"]).decode().splitlines()[1]
    size, used, avail, percent = df_output.split()[1:5]
    print("<div class='info-section'>Disk Usage (/): <span style='color: #40E0D0;'>{} used / {} total ({}) free</span></div>".format(used, size, avail))
except Exception:
    print("<div class='info-section'>Disk Usage: <span style='color: #FF6B6B;'>(unavailable)</span></div>")

# Top 5 processes by CPU usage
try:
    ps_output = subprocess.check_output(["ps", "-eo", "pid,comm,%cpu,%mem", "--sort=-%cpu"]).decode().splitlines()[1:6]
    print("<div class='info-section'>Top Processes:</div><pre style='margin-left:120px; color:#40E0D0;'>")
    print(" PID   CMD          CPU%   MEM%")
    for line in ps_output:
        print(" " + line)
    print("</pre>")
except Exception:
    print("<div class='info-section'>Top Processes: <span style='color: #FF6B6B;'>(unavailable)</span></div>")  

print("""
    </div> <!-- Close centered container -->
    <div style='margin-top: 30px; text-align: center;'>
        <span class="blink">_</span>
    </div>
    <div id="interlaced"></div> 
    <div id="glare"></div>
  </body>
</html>
""")
