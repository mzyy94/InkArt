import "smelte/src/tailwind.css";
import "material-icons/iconfont/material-icons.css";

import Router from "svelte-spa-router";

import App from "./App.svelte";
import Welcome from "./pages/Welcome.svelte";

const routes = {
  "/": Welcome,
  "/:name": App,
};

const app = new Router({
  target: document.body,
  props: {
    routes,
  },
});

export default app;
