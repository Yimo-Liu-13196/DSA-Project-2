import type { Engine } from "./settings";

/**
 * Suggestions come from the C++ backend over HTTP. `engine` picks the data structure
 * that answers: "trie" (Trie.cpp) or "rbt" (RedBlackTree.cpp).
 *   GET  /api/suggest?prefix=..&limit=..&engine=trie|rbt   -> { engine, suggestions: string[], micros }
 *   POST /api/accept?word=..               -> { ok: boolean }
 * In dev, vite proxies /api to the server (see vite.config.ts).
 */
const API_BASE = (import.meta.env.VITE_API_BASE as string | undefined) ?? "";

/** The backend returns lowercase; keep the user's capitalization when showing it. */
function matchCase(typed: string, word: string): string {
  if (typed.length > 1 && typed === typed.toUpperCase()) return word.toUpperCase();
  const first = typed[0];
  if (first && first !== first.toLowerCase()) return word[0].toUpperCase() + word.slice(1);
  return word;
}

export interface SuggestResult {
  suggestions: string[];
  /**
   * Server-side time in milliseconds: from the C++ handler receiving the parsed request until the
   * lookup has run and the JSON is built. Excludes the network, the proxy and the browser.
   * null if the server didn't report it (an older server build).
   */
  ms: number | null;
}

/** Suggestions for the word being typed, plus how long the server spent producing them. */
export async function suggest(
  typed: string,
  limit: number,
  engine: Engine,
  signal?: AbortSignal,
): Promise<SuggestResult> {
  if (!typed) return { suggestions: [], ms: null };
  const url = `${API_BASE}/api/suggest?prefix=${encodeURIComponent(typed)}&limit=${limit}&engine=${engine}`;
  const res = await fetch(url, { signal });
  if (!res.ok) throw new Error(`suggest failed: ${res.status}`);
  const data = (await res.json()) as { suggestions: string[]; micros?: number };
  return {
    suggestions: data.suggestions.map((word) => matchCase(typed, word)),
    ms: typeof data.micros === "number" ? data.micros / 1000 : null,
  };
}

/** Called when a suggestion is accepted: bumps its frequency so it ranks higher next time. */
export async function recordAccepted(word: string): Promise<void> {
  try {
    await fetch(`${API_BASE}/api/accept?word=${encodeURIComponent(word)}`, { method: "POST" });
  } catch {
    /* backend down: ranking just stays as is */
  }
}
