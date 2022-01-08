import "smelte/src/tailwind.css";
import "material-icons/iconfont/material-icons.css";

import Router from "svelte-spa-router";

import App from "./App.svelte";
import Welcome from "./pages/Welcome.svelte";
import PhotoUpload from "./pages/PhotoUpload.svelte";
import FileManagement from "./pages/FileManagement.svelte";
import DisplayControl from "./pages/DisplayControl.svelte";
import Settings from "./pages/Settings.svelte";
import Information from "./pages/Information.svelte";

const routes = {
  "/": Welcome,
  "/upload": PhotoUpload,
  "/files": FileManagement,
  "/control": DisplayControl,
  "/settings": Settings,
  "/info": Information,
  "/:name": App,
};

// @ts-ignore
if (process.env.NODE_ENV === "development") {
  // @ts-ignore
  import("./mocks/browser").then(({ worker }) => {
    worker.start();
  });

  const canvas = document.createElement("canvas");
  canvas.width = 200;
  canvas.height = 200;
  const ctx = canvas.getContext("2d")!;
  ctx.fillStyle = "rgba(128,128,128,0.1)";
  ctx.font = "64px Helvetica";
  ctx.rotate(Math.PI / 4);
  ctx.fillText("Demo", 30, 30);
  const dataUrl = canvas.toDataURL();
  document.body.style.backgroundImage = `url('${dataUrl}')`;
}

const app = new Router({
  target: document.body,
  props: {
    routes,
  },
});

export default app;
