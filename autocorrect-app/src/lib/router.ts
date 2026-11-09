import { useCallback, useEffect, useRef, useState } from "react";

export type Route = "/" | "/start" | "/settings";

function parse(): Route {
  const hash = window.location.hash.replace(/^#/, "");
  return hash === "/start" || hash === "/settings" ? hash : "/";
}

/** Minimal hash router: no dependency needed for three screens. */
export function useRoute() {
  const [route, setRoute] = useState<Route>(parse);
  const previous = useRef<Route | null>(null);

  useEffect(() => {
    const onChange = () => {
      setRoute((current) => {
        previous.current = current;
        return parse();
      });
    };
    window.addEventListener("hashchange", onChange);
    return () => window.removeEventListener("hashchange", onChange);
  }, []);

  const navigate = useCallback((to: Route) => {
    window.location.hash = to;
  }, []);

  /** Used by the settings "X": return to whichever screen opened settings. */
  const back = useCallback(() => {
    window.location.hash = previous.current && previous.current !== "/settings" ? previous.current : "/";
  }, []);

  return { route, navigate, back };
}
