<script lang="ts">
  import { onMount } from "svelte";
  import api from "../../api";
  import type { Orientation } from "../../api";
  import Container from "../templates/Container.svelte";
  import Snackbar from "../atoms/Snackbar.svelte";

  const orientations: Array<{ value: Orientation; text: string }> = [
    { value: "landscape", text: "Landscape" },
    { value: "portrait-right", text: "Portrait Right" },
    { value: "portrait-left", text: "Portrait Left" },
    { value: "upside-down", text: "Upside Down" },
  ];

  let canvas: HTMLCanvasElement;
  let ctx: CanvasRenderingContext2D;

  let width = 800;
  let height = 600;
  let model = "Inkplate";

  let orientation: Orientation;
  let paddingTop = 0;
  let paddingLeft = 0;
  let paddingRight = 0;
  let paddingBottom = 0;
  let invert = false;
  let dithering = false;

  async function initSettings() {
    const display = await api.display();
    invert = display.invert;
    dithering = display.dithering;
    orientation = display.orientation;
    paddingTop = display.padding.top;
    paddingLeft = display.padding.left;
    paddingRight = display.padding.right;
    paddingBottom = display.padding.bottom;
  }

  function applySettings() {
    api
      .display({
        invert,
        dithering,
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
        snackbar.error = !res.ok;
        snackbar.show = true;
      });
  }

  function previewSettings() {
    api
      .preview({
        invert,
        orientation,
        padding: {
          top: paddingTop,
          left: paddingLeft,
          right: paddingRight,
          bottom: paddingBottom,
        },
      })
      .then((res) => {
        if (!res.ok) {
          snackbar.text = "Preview settings failed";
          snackbar.error = true;
          snackbar.show = true;
        }
      });
  }

  onMount(async () => {
    ctx = canvas.getContext("2d")!;
    let { display, system } = await api.info();
    width = display.width;
    height = display.height;
    model = system.model;

    await initSettings();
  });

  $: if (ctx) {
    ctx.clearRect(0, 0, canvas.width, canvas.height);
    const portrait = orientation.startsWith("portrait");
    const scale = width < 960 ? 0.5 : 0.3;

    const rotate = () => {
      ctx.save();
      ctx.translate(center.x, center.y);
      if (orientation == "portrait-right") {
        ctx.rotate((-Math.PI / 2) * 1);
      } else if (orientation == "upside-down") {
        ctx.rotate((-Math.PI / 2) * 2);
      } else if (orientation == "portrait-left") {
        ctx.rotate((-Math.PI / 2) * 3);
      }
      ctx.scale(scale, scale);
    };

    const center = {
      x: canvas.width / 2,
      y: canvas.height / 2,
    };

    rotate();
    // PCB
    ctx.fillStyle = "#009";
    ctx.fillRect(
      -(width + 20) / 2,
      -(height + 20) / 2,
      width + 20,
      height + 60
    );

    // Touch Pads
    ctx.strokeStyle = "#fff";
    ctx.lineWidth = 4;
    for (let i = 0; i < 3; i++) {
      ctx.strokeRect(-200 + 160 * i, height / 2 + 8, 60, 30);
    }

    // Panel
    ctx.fillStyle = "#888";
    ctx.fillRect(-width / 2, -height / 2, width, height);
    ctx.restore();

    // Active area
    ctx.save();
    ctx.translate(center.x, center.y);
    ctx.scale(scale, scale);
    ctx.fillStyle = invert ? "#444" : "#aaa";
    ctx.fillRect(
      -((portrait ? height : width) / 2 - paddingLeft),
      -((portrait ? width : height) / 2 - paddingTop),
      (portrait ? height : width) - paddingLeft - paddingRight,
      (portrait ? width : height) - paddingTop - paddingBottom
    );
    ctx.restore();

    rotate();
    ctx.textAlign = "center";
    ctx.fillStyle = invert ? "#ccc" : "#000";
    ctx.font = "60px sans-serif";
    ctx.fillText(model, 0, 0);
    ctx.restore();
  }

  let snackbar = {
    show: false,
    text: "",
    error: false,
  };
</script>

<main>
  <Container>
    <span slot="title">Display Control</span>
    <fieldset>
      <label>
        <input type="checkbox" role="switch" bind:checked={invert} />
        Invert black/white
      </label>
    </fieldset>

    <fieldset>
      <label>
        <input type="checkbox" role="switch" bind:checked={dithering} />
        Dithering
      </label>
    </fieldset>

    <select bind:value={orientation}>
      {#each orientations as { value, text }}
        <option {value}>{text}</option>
      {/each}
    </select>

    <fieldset>
      <p>Top: {paddingTop}pixel</p>
      <input type="range" min={0} max={200} bind:value={paddingTop} />
    </fieldset>

    <fieldset>
      <p>Left: {paddingLeft}pixel</p>
      <input type="range" min={0} max={200} bind:value={paddingLeft} />
    </fieldset>

    <fieldset>
      <p>Right: {paddingRight}pixel</p>
      <input type="range" min={0} max={200} bind:value={paddingRight} />
    </fieldset>

    <fieldset>
      <p>Bottom: {paddingBottom}pixel</p>
      <input type="range" min={0} max={200} bind:value={paddingBottom} />
    </fieldset>

    <div>
      <button class="contrast" on:click={previewSettings}>Preview</button>
      <button on:click={applySettings}>Apply</button>
      <button class="secondary" on:click={initSettings}>Reset</button>
    </div>

    <canvas bind:this={canvas} width={500} height={500} />

    <Snackbar error={snackbar.error} bind:open={snackbar.show}>
      <div>{snackbar.text}</div>
    </Snackbar>
  </Container>
</main>

<style>
  canvas {
    max-width: 100%;
    margin: 1rem auto;
    border: 2px solid gray;
  }
</style>
