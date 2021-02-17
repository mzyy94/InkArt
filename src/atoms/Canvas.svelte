<script context="module" lang="ts">
  export class DrawImageEvent extends CustomEvent<{
    x: number;
    y: number;
  }> {}

  export class ResizeImageEvent extends CustomEvent<{
    mode: string;
    image: HTMLImageElement;
  }> {}
</script>

<script lang="ts">
  import { grayscale } from "./Grayscale.svelte";

  export let img: HTMLImageElement;
  export let mode = "fill";

  export let offsetX = 0;
  export let offsetY = 0;
  export let active = false;

  let canvas: HTMLCanvasElement;

  function resizeImage(mode: string) {
    canvas?.dispatchEvent(
      new ResizeImageEvent("resizeimage", {
        detail: {
          mode,
          image: img,
        },
      })
    );
  }

  function drawImage(x: number, y: number) {
    canvas?.dispatchEvent(
      new DrawImageEvent("drawimage", {
        detail: {
          x,
          y,
        },
      })
    );
  }

  img.onload = () => {
    resizeImage(mode);
    drawImage(offsetX, offsetY);
    active = true;
  };

  $: resizeImage(mode);
  $: drawImage(offsetX, offsetY);
</script>

<canvas bind:this={canvas} width={800} height={600} use:grayscale />

<style>
  canvas {
    background: gray;
    max-width: 100%;
  }
</style>
