# Autocorrect HTTP server (cpp-httplib)

Wraps the **unmodified** `Trie` from `DSA-Project-2`. `main.cpp` there (the test driver) is not compiled.

Layout expected:
```
DSA-Project-2/      <- your backend, untouched
server/             <- this folder
autocorrect-app/    <- frontend
```

## Build & run
```bash
cd server
make                      # downloads httplib.h if missing (or copy it in yourself), then builds ./server
./server words.txt 8080   # words.txt = starter dictionary, earlier lines = more common
```
Windows/MSVC: compile `server.cpp` + `../DSA-Project-2/Trie.cpp` with C++17 and put `httplib.h` next to `server.cpp`.

## API
| | |
|---|---|
| `GET /api/suggest?prefix=ap&limit=5&engine=trie` (or `rbt`) | `{"engine":"trie","suggestions":["apple","app"]}` (`complete` on the chosen structure) |
| `POST /api/accept?word=apple` | `{"ok":true}` (`incrementFrequency` on both structures) |
| `GET /api/health` | `{"ok":true}` |

## One-origin production run
```bash
(cd ../autocorrect-app && npm run build)
./server words.txt 8080 ../autocorrect-app/dist   # now http://localhost:8080 serves the UI and the API
```
