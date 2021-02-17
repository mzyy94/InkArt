<script context="module" lang="ts">
  export class DrawImageEvent extends CustomEvent<{
    offsetX: number;
    offsetY: number;
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

  function drawImage(mode: string, offsetX, offsetY) {
    if (!img.src) {
      return;
    }
    const ctx = canvas.getContext("2d");
    let x = 0;
    let y = 0;
    let { width, height } = canvas;

    ctx.fillStyle = "white";
    ctx.fillRect(x, y, width, height);

    switch (mode) {
      case "cover":
      case "fit": {
        const imageAspect = img.width / img.height;
        const canvasAspect = canvas.width / canvas.height;
        const scaleFactor = imageAspect / canvasAspect;
        const threshold = mode == "cover" ? scaleFactor : 1 / scaleFactor;
        if (threshold > 1) {
          width = width * scaleFactor;
          x = (canvas.width - width) / 2;
        } else {
          height = height / scaleFactor;
          y = (canvas.height - height) / 2;
        }
        break;
      }
      case "none": {
        x = (width - img.width) / 2;
        y = (height - img.height) / 2;
        width = img.width;
        height = img.height;
        break;
      }
      case "fill":
      default: {
        break;
      }
    }

    ctx.drawImage(img, x, y, width, height);
    canvas.dispatchEvent(
      new DrawImageEvent("drawimage", {
        detail: {
          offsetX,
          offsetY,
        },
      })
    );
  }

  img.onload = () => {
    drawImage(mode, offsetX, offsetY);
    active = true;
  };

  $: drawImage(mode, offsetX, offsetY);
</script>

<canvas bind:this={canvas} width={800} height={600} use:grayscale />

<style>
  canvas {
    background: gray;
    max-width: 100%;
  }
</style>
