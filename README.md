# Autocorrect Suggestions: Setup Guide

This is a program where users type a word and get suggestions. The suggestions are produced by a C++ backend that can use either a **Trie** or a **Red-Black Tree**, and a React frontend shows them. You can switch between the two data structures on the Settings page, and the Start page shows how long the server spent on each lookup.

This is a guide to help you properly set up the files and get everything running on your own computer. It is geared towards using Windows OS and MSYS2 for the C++ compiler.

While the app runs there are two programs:

1. **The C++ server** listens on port **8080** and answers `/api/suggest` and `/api/accept`.
2. **The frontend dev server** (Vite) serves the web page on port **5173** and forwards any `/api` request to port 8080.

You need both running at the same time, each in its own terminal window.
---
## 1. Install the tools
You need two things: a **C++ compiler with `make`** (from MSYS2) and **Node.js** (for the frontend).

### 1a. MSYS2 (C++ compiler and make)
Check whether the folder `C:\msys64` exists. If it does, skip to "Install the compiler packages" below.

**Install it:**
- Download the installer from <https://www.msys2.org> and run it with the default options (it installs to `C:\msys64`)

**Install the compiler packages:**
1. Open **"MSYS2 UCRT64"** from the Start menu. Use this, not the plain "MSYS2 MSYS", because the compiler is installed for the UCRT64 environment.
2. Update the package database (if it asks you to close the window afterwards, do so, reopen "MSYS2 UCRT64" and run the command once more):
   ```bash
   pacman -Syu
   ```
3. Install the compiler, `make`:
   ```bash
   pacman -S --needed mingw-w64-ucrt-x86_64-gcc make curl
   ```
   Press Enter at the prompts to accept the defaults.
4. Check that it worked:
   ```bash
   g++ --version
   make --version
   ```
   Both should print a version number.

### 1b. Node.js (frontend)
 Open PowerShell and run `node --version`. You need **version 20.19 or newer** (the current LTS release is the safest choice). If the command isn't found or the version is older, install the LTS release from <https://nodejs.org> and then open a **new** PowerShell window.
Check it:
```powershell
node --version
npm --version
```
---
## 2. Get the project in place
Put the three folders (`backend`, `server`, `autocorrect-app`) side by side inside one project folder, as shown in the diagram above. The server's Makefile expects the backend at `../backend`, so don't rename or move it relative to `server/`.
---
## 3. Build and start the C++ server
1. Open **"MSYS2 UCRT64"** again.
2. Go to the `server` folder. 
   ```bash
   cd "/c/dsa proj2/server"
   ```
3. Double check if there's a `httplib.h` in this folder. Check by running "ls" and seeing if its in the list that is output. If you're offline, download `httplib.h` yourself from <https://github.com/yhirose/cpp-httplib> and put it in the `server` folder.
4. We run "make" to build the server, and then start it up. You can do this in the same MSYS2 window or from PowerShell, or by running "server.exe" from your file manager:
   ```powershell
   cd "C:\dsa proj2\server"
   make
   .\server.exe words.txt 8080
   ```
   Output:
   ```
   Loaded 100000 words
   Listening on http://localhost:8080
   ```
   **Leave this window open.** The server runs until you close it or press `Ctrl+C`.
---
## 4. Start the frontend
Open a **second** PowerShell window (leave the server window running):
```powershell
cd "C:\dsa proj2\autocorrect-app"
npm install
npm run dev
```
http://localhost:5173
```
Open that in your browser. Click through to the Start page, type a word, and suggestions should appear.
---
## 5. Using the app
- **Start page:** just type a word, and look at the suggestions that appear. Under the editor, it reports how long the server took to get the suggestions. Ex. `Suggestions took 19 ms (Trie)`.
  - The timer only measures the server side, starting from the request being recieved to the lookup finishing and the JSON being built. It doesn't include the network or the browser.
  - The first lookup after starting is often a bit slower than later ones.
- **Settings page:**
  - *How many suggestions?* sets how many words are suggested (1 to 9).
  - *Data structure?* switches between using the **Trie** and **Red-black tree**. Both have the same words and give the same suggestions, so the main difference is the lookup time.
  - Your choices are remembered in the browser.
- Accepting a suggestion increases that word's frequency in both structures, so it ranks higher next time. The frequencies are stored in the server's memory and do reset when the server restarts.
