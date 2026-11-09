# Autocorrect suggestions

React + TypeScript + Vite implementation of the **final designs** page in the `dsa-proj2` Figma file
(home, settings, start).

```bash
npm install
npm run dev      # http://localhost:5173
npm run build
```

## Running with the C++ backend
1. Build and start the server (see `../server/README.md`) on port 8080.
2. `npm install && npm run dev` - vite proxies `/api` to `localhost:8080`.

## Structure
- `src/pages/` - `Home`, `Start`, `Settings` (one per Figma frame)
- `src/components/IconButton.tsx` - gear + close icons (exact vector paths from Figma)
- `src/lib/suggest.ts` - `suggest(word, count)` -> `GET /api/suggest`; `recordAccepted(word)` -> `POST /api/accept` (bumps frequency via `Trie::incrementFrequency`)
- `src/lib/settings.tsx` - "How many suggestions?" (1-9, default 5, saved in localStorage)
- `src/lib/router.ts` - tiny hash router (`#/`, `#/start`, `#/settings`)
- `src/styles.css` - design tokens + layout. Sizes use `--u` (1 design px, scaled from the 1440x1024 frames).
