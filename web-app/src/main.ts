import "@picocss/pico";
import "material-icons/iconfont/material-icons.css";

import Router from "svelte-spa-router";

import Menu from "./components/pages/Menu.svelte";
import Upload from "./components/pages/Upload.svelte";
import Files from "./components/pages/Files.svelte";
import Display from "./components/pages/Display.svelte";
import Settings from "./components/pages/Settings.svelte";
import Info from "./components/pages/Info.svelte";

const routes = {
  "/": Menu,
  "/upload": Upload,
  "/files": Files,
  "/control": Display,
  "/settings": Settings,
  "/info": Info,
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
  document.body.style.backgroundRepeat = "repeat";
}

const app = new Router({
  target: document.body,
  props: {
    routes,
  },
});

export default app;
