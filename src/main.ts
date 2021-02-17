import "smelte/src/tailwind.css";
import "material-icons/iconfont/material-icons.css";
import { worker } from "./mocks/browser";

import Router from "svelte-spa-router";

import App from "./App.svelte";
import Welcome from "./pages/Welcome.svelte";

const routes = {
  "/": Welcome,
  "/:name": App,
};

// @ts-ignore
if (process.env.NODE_ENV === "development") {
  worker.start();
}

const app = new Router({
  target: document.body,
  props: {
    routes,
  },
});

export default app;
