\# 🚀 DarkSophia — Real-Time WebSocket Command \& Control Framework



KEMET-OMNIS 2025 is a modern, fully real-time Command-and-Control (C2) framework designed for red-team operations, adversary simulation, and controlled research environments.  

It delivers instant, bidirectional communication between Windows implants and an operator dashboard using \*\*native WebSockets\*\*, eliminating traditional HTTP polling and significantly reducing detection footprint.



---



\## 🔥 Features



\### 🛰 Real-Time WebSocket C2

\- Full-duplex communication  

\- Instant command execution  

\- Immediate output streaming  

\- No HTTP polling — stealthier, faster, cleaner  



\### 🎛 Cyber-Ops Dashboard

\- Real-time implant listing  

\- Online/offline indicators  

\- Per-implant terminal  

\- Command history  

\- Modern neon cyber UI  

\- Entire UI uses WebSockets  



\### 👾 Native Windows Implant

\- Written in C++ using \*\*WinHTTP WebSocket API\*\*  

\- No external libraries or dependencies  

\- Hidden PowerShell execution  

\- Lightweight \& stealthy  

\- Works on Windows 8 / 10 / 11 / Server  



---



\## ⚙️ Building the Windows Implant



The implant uses the native \*\*WinHTTP WebSocket API\*\*, so you only need MSVC.



\### ✔ Compile using the exact command below:



cl /O2 /EHsc ghost\_implant.cpp winhttp.lib user32.lib advapi32.lib /link /OUT:ghost.exe



\## 🛠 Running the C2 Server



You need \*\*Python 3.10+\*\*.



\### Install dependencies:



pip install fastapi uvicorn websockets



\### Run C2



python c2\_server.py



\### 🕹 Operator Dashboard



After starting the server, open:



http://YOUR-SERVER:8001/





\### Dashboard features:



Live list of implants



Host/user identity



Full command terminal



Live output stream



Command history per implant



Everything updates in real time via WebSockets.



\### 🧩 Implant Behavior (ghost\_implant.cpp)



The implant:



Generates a unique implant ID



Opens a WebSocket connection:



ws://SERVER:8001/ws/<implant\_id>



Sends system metadata:



INFO::hostname|username





Waits for operator commands



Executes them via hidden PowerShell



Sends the command output back instantly



This design provides high responsiveness and low network signature.





\### ⚠️ Legal \& Ethical Notice



This project is for authorized security testing, red-team operations, and educational research only.

Do NOT use this tool for unauthorized access, real-world exploitation, or illegal activity of any kind.



You are solely responsible for any use or misuse.

