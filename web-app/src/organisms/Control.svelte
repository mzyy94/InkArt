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
  let paddingTop = 0;
  let paddingLeft = 0;
  let paddingRight = 0;
  let paddingBottom = 0;

  async function initSettings() {
    const display = await api.display();
    $darkMode = display.inverted;
    orientation = display.orientation;
    paddingTop = display.padding.top;
    paddingLeft = display.padding.left;
    paddingRight = display.padding.right;
    paddingBottom = display.padding.bottom;
  }

  function applySettings() {
    api
      .display({
        inverted: $darkMode,
        orientation,
        padding: {
          top: paddingTop,
          left: paddingLeft,
          right: paddingRight,
          bottom: paddingBottom,
        },
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
  <p class="text-gray-700">Top: {paddingTop}pixel</p>
  <Slider min={0} max={200} bind:value={paddingTop} />
</fieldset>

<fieldset class="my-3">
  <p class="text-gray-700">Left: {paddingLeft}pixel</p>
  <Slider min={0} max={200} bind:value={paddingLeft} />
</fieldset>

<fieldset class="my-3">
  <p class="text-gray-700">Right: {paddingRight}pixel</p>
  <Slider min={0} max={200} bind:value={paddingRight} />
</fieldset>

<fieldset class="my-3">
  <p class="text-gray-700">Bottom: {paddingBottom}pixel</p>
  <Slider min={0} max={200} bind:value={paddingBottom} />
</fieldset>

<div class="flex space-x-2 justify-end">
  <Button on:click={applySettings}>Apply</Button>
  <Button color="secondary" on:click={initSettings}>Reset</Button>
</div>

<Snackbar color={snackbar.color} bind:value={snackbar.show}>
  <div>{snackbar.text}</div>
</Snackbar>
