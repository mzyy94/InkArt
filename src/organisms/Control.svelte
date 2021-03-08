<script lang="ts">
  import { onMount } from "svelte";
  import { Button, Select, Slider, Snackbar, Switch } from "smelte";
  import dark from "smelte/src/dark";
  import api from "../api";
  import type { Orientation } from "../api";

  const darkMode = dark();

  const orientations = [
    { value: "portrait", text: "Portrait" },
    { value: "landscape-right", text: "Landscape Right" },
    { value: "landscape-left", text: "Landscape Left" },
    { value: "upside-down", text: "Upside Down" },
  ];

  let orientation: Orientation;
  let interval = 0;
  let margin = 0;

  function initSettings() {
    api.display().then((display) => {
      darkMode.set(display.inverted);
      orientation = display.orientation;
      interval = display.interval;
      margin = display.margin;
    });
  }

  function applySettings() {
    api
      .display({
        inverted: $darkMode,
        orientation,
        interval,
        margin,
      })
      .then((res) => {
        snackbar.text = `Update settings ${res.ok ? "succeeded" : "failed"}`;
        snackbar.color = res.ok ? "primary" : "error";
        snackbar.show = true;
      });
  }

  onMount(initSettings);

  let snackbar = {
    show: false,
    text: "",
    color: "primary",
  };
</script>

<Switch label="Invert black/white" bind:value={$darkMode} />

<Select
  label="Orientation"
  name="orientation"
  bind:value={orientation}
  items={orientations}
/>

<fieldset class="my-3">
  <p class="text-gray-700">Update Interval: {interval}min</p>
  <Slider min={0} step={5} max={480} bind:value={interval} />
</fieldset>

<fieldset class="my-3">
  <p class="text-gray-700">Margin: {margin}pixel</p>
  <Slider min={-20} max={20} bind:value={margin} />
</fieldset>

<div class="flex space-x-2 justify-end">
  <Button on:click={applySettings}>Apply</Button>
  <Button color="secondary" on:click={initSettings}>Reset</Button>
</div>

<Snackbar color={snackbar.color} bind:value={snackbar.show}>
  <div>{snackbar.text}</div>
</Snackbar>
