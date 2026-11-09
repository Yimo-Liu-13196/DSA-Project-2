import { useEffect, useRef, useState } from "react";
import { GearGlyph, IconButton } from "../components/IconButton";
import { recordAccepted, suggest } from "../lib/suggest";
import { ENGINES, useSettings } from "../lib/settings";
import type { Route } from "../lib/router";

interface Props {
  text: string;
  setText: (value: string) => void;
  navigate: (to: Route) => void;
}

/** The word currently being typed = everything after the last whitespace. */
function currentWord(text: string): string {
  const match = text.match(/(\S+)$/);
  return match ? match[1] : "";
}

function formatMs(ms: number): string {
  if (ms < 1) return `${Math.round(ms * 1000)} \u00b5s`;
  return ms < 10 ? `${ms.toFixed(2)} ms` : `${ms.toFixed(1)} ms`;
}

export default function Start({ text, setText, navigate }: Props) {
  const { suggestionCount, engine } = useSettings();
  const inputRef = useRef<HTMLTextAreaElement>(null);

  const word = currentWord(text);
  const [suggestions, setSuggestions] = useState<string[]>([]);
  // Server-side time of the last lookup and which structure answered it (null = nothing to show yet).
  const [timing, setTiming] = useState<{ ms: number; engine: string } | null>(null);

  // Ask the backend whenever the current word, suggestion count or data structure changes.
  // Cleanup aborts the previous request so a slow, stale reply can't overwrite a newer one.
  useEffect(() => {
    if (!word) {
      setSuggestions([]);
      setTiming(null);
      return;
    }
    const controller = new AbortController();
    suggest(word, suggestionCount, engine, controller.signal)
      .then(({ suggestions, ms }) => {
        setSuggestions(suggestions);
        setTiming(ms === null ? null : { ms, engine: ENGINES.find((e) => e.value === engine)?.label ?? engine });
      })
      .catch((error) => {
        if (error.name !== "AbortError") {
          setSuggestions([]);
          setTiming(null);
        }
      });
    return () => controller.abort();
  }, [word, suggestionCount, engine]);

  // One slot per requested suggestion; empty slots show "..." like the design.
  const slots = Array.from({ length: suggestionCount }, (_, i) => suggestions[i] ?? null);

  const accept = (replacement: string) => {
    recordAccepted(replacement);
    setText(text.slice(0, text.length - word.length) + replacement + " ");
    inputRef.current?.focus();
  };

  return (
    <>
      <h1 className="title title--two-line">
        Autocorrect
        <br />
        suggestions
      </h1>

      <IconButton label="Open settings" className="corner-btn corner-btn--start" onClick={() => navigate("/settings")}>
        <GearGlyph />
      </IconButton>

      <textarea
        ref={inputRef}
        className="editor"
        placeholder="Start typing here"
        value={text}
        onChange={(e) => setText(e.target.value)}
        spellCheck={false}
        autoComplete="off"
        autoCorrect="off"
        autoCapitalize="off"
        autoFocus
      />

      <p className="timer" aria-live="polite">
        {timing ? `Suggestions took ${formatMs(timing.ms)} (${timing.engine})` : "Suggestions took --"}
      </p>

      <ul className="suggestions" aria-label="Suggestions">
        {slots.map((s, i) => (
          <li key={i} className="suggestions__slot">
            <button
              type="button"
              className="btn suggestions__btn"
              disabled={!s}
              onClick={() => s && accept(s)}
            >
              {s ?? "..."}
            </button>
          </li>
        ))}
      </ul>
    </>
  );
}
