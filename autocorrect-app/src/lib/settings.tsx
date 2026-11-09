import { createContext, useContext, useEffect, useMemo, useState, type ReactNode } from "react";

export const MIN_SUGGESTIONS = 1;
export const MAX_SUGGESTIONS = 9;
const DEFAULT_SUGGESTIONS = 5; // the design's default selection
const STORAGE_KEY = "autocorrect.suggestionCount";
const ENGINE_KEY = "autocorrect.engine";

/** Which backend data structure answers suggestion requests. */
export type Engine = "trie" | "rbt";
export const ENGINES: { value: Engine; label: string }[] = [
  { value: "trie", label: "Trie" },
  { value: "rbt", label: "Red-black tree" },
];
const DEFAULT_ENGINE: Engine = "trie";

interface SettingsValue {
  suggestionCount: number;
  setSuggestionCount: (n: number) => void;
  engine: Engine;
  setEngine: (engine: Engine) => void;
}

const SettingsContext = createContext<SettingsValue | null>(null);

function load(): number {
  try {
    const n = Number(window.localStorage.getItem(STORAGE_KEY));
    if (Number.isInteger(n) && n >= MIN_SUGGESTIONS && n <= MAX_SUGGESTIONS) return n;
  } catch {
    /* storage unavailable */
  }
  return DEFAULT_SUGGESTIONS;
}

function loadEngine(): Engine {
  try {
    const value = window.localStorage.getItem(ENGINE_KEY);
    if (ENGINES.some((e) => e.value === value)) return value as Engine;
  } catch {
    /* storage unavailable */
  }
  return DEFAULT_ENGINE;
}

export function SettingsProvider({ children }: { children: ReactNode }) {
  const [suggestionCount, setSuggestionCount] = useState(load);
  const [engine, setEngine] = useState<Engine>(loadEngine);

  useEffect(() => {
    try {
      window.localStorage.setItem(STORAGE_KEY, String(suggestionCount));
    } catch {
      /* ignore */
    }
  }, [suggestionCount]);

  useEffect(() => {
    try {
      window.localStorage.setItem(ENGINE_KEY, engine);
    } catch {
      /* ignore */
    }
  }, [engine]);

  const value = useMemo(
    () => ({ suggestionCount, setSuggestionCount, engine, setEngine }),
    [suggestionCount, engine],
  );
  return <SettingsContext.Provider value={value}>{children}</SettingsContext.Provider>;
}

export function useSettings() {
  const ctx = useContext(SettingsContext);
  if (!ctx) throw new Error("useSettings must be used inside <SettingsProvider>");
  return ctx;
}
