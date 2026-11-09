import { CloseGlyph, IconButton } from "../components/IconButton";
import { ENGINES, MAX_SUGGESTIONS, MIN_SUGGESTIONS, useSettings } from "../lib/settings";

const OPTIONS = Array.from({ length: MAX_SUGGESTIONS - MIN_SUGGESTIONS + 1 }, (_, i) => i + MIN_SUGGESTIONS);

export default function Settings({ onClose }: { onClose: () => void }) {
  const { suggestionCount, setSuggestionCount, engine, setEngine } = useSettings();

  return (
    <>
      <h1 className="title title--settings">Settings</h1>

      <IconButton label="Close settings" className="corner-btn corner-btn--settings" onClick={onClose}>
        <CloseGlyph />
      </IconButton>

      <div className="setting-row">
        <span className="setting-row__label" id="count-label">
          How many suggestions?
        </span>
        <div className="segmented" role="radiogroup" aria-labelledby="count-label">
          {OPTIONS.map((n) => (
            <button
              key={n}
              type="button"
              role="radio"
              aria-checked={n === suggestionCount}
              className={`btn segmented__item${n === suggestionCount ? " is-selected" : ""}`}
              onClick={() => setSuggestionCount(n)}
            >
              {n}
            </button>
          ))}
        </div>
      </div>

      <div className="setting-row setting-row--engine">
        <span className="setting-row__label" id="engine-label">
          Data structure?
        </span>
        <div className="segmented" role="radiogroup" aria-labelledby="engine-label">
          {ENGINES.map(({ value, label }) => (
            <button
              key={value}
              type="button"
              role="radio"
              aria-checked={value === engine}
              className={`btn segmented__item segmented__item--wide${value === engine ? " is-selected" : ""}`}
              onClick={() => setEngine(value)}
            >
              {label}
            </button>
          ))}
        </div>
      </div>
    </>
  );
}
