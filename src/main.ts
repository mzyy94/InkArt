import "smelte/src/tailwind.css";
import "material-icons/iconfont/material-icons.css";

import Router from "svelte-spa-router";

import App from "./App.svelte";
import Welcome from "./pages/Welcome.svelte";
import WiFiSettings from "./pages/WiFiSettings.svelte";
import PhotoUpload from "./pages/PhotoUpload.svelte";
import FileManagement from "./pages/FileManagement.svelte";
import DisplayControl from "./pages/DisplayControl.svelte";

const routes = {
  "/": Welcome,
  "/wifi": WiFiSettings,
  "/upload": PhotoUpload,
  "/files": FileManagement,
  "/control": DisplayControl,
  "/:name": App,
};

// @ts-ignore
if (process.env.NODE_ENV === "development") {
  // @ts-ignore
  import("./mocks/browser").then(({worker}) => {
    worker.start();
  });
}

const app = new Router({
  target: document.body,
  props: {
    routes,
  },
});

export default app;
