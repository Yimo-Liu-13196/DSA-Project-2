import { useState } from "react";
import { SettingsProvider } from "./lib/settings";
import { useRoute } from "./lib/router";
import Home from "./pages/Home";
import Settings from "./pages/Settings";
import Start from "./pages/Start";

export default function App() {
  const { route, navigate, back } = useRoute();
  // Lifted here so typed text survives a trip to the settings screen.
  const [text, setText] = useState("");

  return (
    <SettingsProvider>
      <main className="viewport">
        <div className="stage">
          {route === "/" && <Home navigate={navigate} />}
          {route === "/settings" && <Settings onClose={back} />}
          {route === "/start" && <Start text={text} setText={setText} navigate={navigate} />}
        </div>
      </main>
    </SettingsProvider>
  );
}
