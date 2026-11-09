import type { Route } from "../lib/router";

export default function Home({ navigate }: { navigate: (to: Route) => void }) {
  return (
    <>
      <h1 className="title title--two-line">
        Autocorrect
        <br />
        suggestions
      </h1>
      <button type="button" className="btn home-btn home-btn--start" onClick={() => navigate("/start")}>
        Start
      </button>
      <button type="button" className="btn home-btn home-btn--settings" onClick={() => navigate("/settings")}>
        Settings
      </button>
    </>
  );
}
