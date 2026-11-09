import { defineConfig } from "vite";
import react from "@vitejs/plugin-react";

export default defineConfig({
  plugins: [react()],
  // Forward /api calls to the C++ server so the browser sees one origin (no CORS setup needed).
  server: { proxy: { "/api": "http://localhost:8080" } },
});
