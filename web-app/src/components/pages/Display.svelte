<script lang="ts">
  import { onMount } from "svelte";
  import { Button, Select, Slider, Snackbar, Switch } from "smelte";
  import api from "../../api";
  import type { Orientation } from "../../api";
  import Container from "../templates/Container.svelte";

  const orientations: Array<{ value: Orientation; text: string }> = [
    { value: "landscape", text: "Landscape" },
    { value: "portrait-right", text: "Portrait Right" },
    { value: "portrait-left", text: "Portrait Left" },
    { value: "upside-down", text: "Upside Down" },
  ];

  let canvas: HTMLCanvasElement;
  let ctx: CanvasRenderingContext2D;

  let orientation: Orientation;
  let paddingTop = 0;
  let paddingLeft = 0;
  let paddingRight = 0;
  let paddingBottom = 0;
  let inverted = false;

  async function initSettings() {
    const display = await api.display();
    inverted = display.inverted;
    orientation = display.orientation;
    paddingTop = display.padding.top;
    paddingLeft = display.padding.left;
    paddingRight = display.padding.right;
    paddingBottom = display.padding.bottom;
  }

  function applySettings() {
    api
      .display({
        inverted,
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

  onMount(async () => {
    ctx = canvas.getContext("2d")!;
    await initSettings();
  });

  $: if (ctx) {
    ctx.clearRect(0, 0, canvas.width, canvas.height);
    const portrait = orientation.startsWith("portrait");
    const width = 800;
    const height = 600;
    const scale = 0.5;

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
    ctx.fillStyle = inverted ? "#444" : "#aaa";
    ctx.fillRect(
      -((portrait ? height : width) / 2 - paddingLeft),
      -((portrait ? width : height) / 2 - paddingTop),
      (portrait ? height : width) - paddingLeft - paddingRight,
      (portrait ? width : height) - paddingTop - paddingBottom
    );
    ctx.restore();

    rotate();
    ctx.textAlign = "center";
    ctx.fillStyle = inverted ? "#ccc" : "#000";
    ctx.font = "60px sans-serif";
    ctx.fillText("Inkplate 6", 0, 0);
    ctx.restore();
  }

  let snackbar = {
    show: false,
    text: "",
    color: "primary",
  };
</script>

<main>
  <Container>
    <span slot="title">Display Control</span>
    <Switch label="Invert black/white" bind:value={inverted} />

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

    <canvas bind:this={canvas} width={500} height={500} />

    <Snackbar color={snackbar.color} bind:value={snackbar.show}>
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
